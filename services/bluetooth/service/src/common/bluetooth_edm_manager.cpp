/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_service_edm"
#endif

#include "bluetooth_edm_manager.h"
#include <regex>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "cJSON.h"
#include "bluetooth_manager_proxy.h"
#include "log.h"
#include "bluetooth_errorcode.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_util.h"
#include "bluetooth_os_account.h"
#include "permission_manager.h"
#include "sg_collect_client.h"
#include "ipc_skeleton.h"
#include "json_utils.h"

namespace OHOS {
namespace bluetooth {
static constexpr int MAX_ACCOUNT_SIZE = 1000;
static constexpr int MAX_PROFILE_SIZE = 500;
static const char *const BLUETOOTH_WHITELIST_CHANGED_EVENT = "com.ohos.edm.bluetoothdeviceschanged";
static const char *const BLUETOOTH_DENYLIST_CHANGED_EVENT = "com.ohos.edm.disallowedbluetoothdeviceschanged";
static const char *const BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT = "usual.event.EDM_CONFIG_CHANGED";
static const char *const BLUETOOTH_ACCOUNT_DENYLIST_CONFIG_FILEPATH =
    "/data/service/el1/public/edm/config/system/all/bluetooth/config.json";

std::mutex BluetoothEdmManager::instanceMutexLock_;
BluetoothEdmManager& BluetoothEdmManager::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutexLock_);
    static BluetoothEdmManager ins;
    return ins;
}

void BluetoothEdmManager::Init()
{
    CHECK_AND_RETURN_LOG(!isInit_, "Already Init");
    int ret = GetWhitelist();
    HILOGI("GetWhitelist ret: %{public}d", ret);
    ret = GetBlacklist();
    HILOGI("GetBlacklist ret: %{public}d", ret);
    ret = GetAccountSendBlacklist();
    HILOGI("GetAccountSendBlacklist: %{public}d", ret);
    ret = GetAccountRecvBlacklist();
    HILOGI("GetAccountRecvBlacklist: %{public}d", ret);

    EventFwk::MatchingSkills accountMatchingSkills;
    accountMatchingSkills.AddEvent(BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT);
    EventFwk::CommonEventSubscribeInfo accountSubscriberInfo(accountMatchingSkills);
    accountSubscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    accountSubscriberInfo.SetPermission("ohos.permission.MANAGE_EDM_POLICY");
    HILOGI("SetPermission MANAGE_EDM_POLICY");
    accountDisallowedEventSubscriber_ = std::make_shared<BluetoothCommonEventSubscriber>(accountSubscriberInfo);
    CHECK_AND_RETURN_LOG(EventFwk::CommonEventManager::SubscribeCommonEvent(accountDisallowedEventSubscriber_),
        "SubscribeCommonEvent account denylist fail.");

    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(BLUETOOTH_WHITELIST_CHANGED_EVENT);
    matchingSkills.AddEvent(BLUETOOTH_DENYLIST_CHANGED_EVENT);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriberInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    subscriberInfo.SetPermission("ohos.permission.MANAGE_BLUETOOTH");
    HILOGI("SetPermission MANAGE_BLUETOOTH");
    deviceEventSubscriber_ = std::make_shared<BluetoothCommonEventSubscriber>(subscriberInfo);
    CHECK_AND_RETURN_LOG(EventFwk::CommonEventManager::SubscribeCommonEvent(deviceEventSubscriber_),
        "SubscribeCommonEvent device event fail.");
    isInit_ = true;
}

bool BluetoothEdmManager::IsValidAddress(const std::string& address)
{
    const std::regex deviceIdRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return regex_match(address, deviceIdRegex);
}

bool BluetoothEdmManager::IsAllowedConnect(const std::string &mac)
{
    std::shared_lock<std::shared_mutex> lock(whitelistMutexLock_);
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(mac), false, "Invalid address");
    std::string macTemp = mac;
    std::transform(macTemp.begin(), macTemp.end(), macTemp.begin(), ::tolower);
    if (whitelist_.empty()) {
        std::shared_lock<std::shared_mutex> lock(blacklistMutexLock_);
        CHECK_AND_RETURN_LOG_RET(!denylist_.empty(), true, "IsAllowedConnect denylist is empty");
        auto it = find(denylist_.begin(), denylist_.end(), macTemp);
        if (it != denylist_.end()) {
            HILOGE("[EDM MODE]device %{public}s restricted by edm denylist.", GetEncryptAddr(mac).c_str());
            BtChrDftEventWriteInt(CHR_USER_DISCONNECT, mac, "DISCONNECTREASON", Bluetooth::BT_ERR_INTERNAL_ERROR);
            return false;
        } else {
            HILOGI("IsAllowedConnect mac is not in denylist");
            return true;
        }
    }
    auto it = find(whitelist_.begin(), whitelist_.end(), macTemp);
    if (it != whitelist_.end()) {
        HILOGI("IsAllowedConnect mac is in whitelist");
        return true;
    } else {
        HILOGE("[EDM MODE]device %{public}s restricted by edm whitelist.", GetEncryptAddr(mac).c_str());
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, mac, "DISCONNECTREASON", Bluetooth::BT_ERR_INTERNAL_ERROR);
        return false;
    }
}

bool BluetoothEdmManager::IsAccountConfigAllowedInternal(const ControlInterceptMessage &msg,
    const std::string &profile, std::shared_mutex &mutexLock,
    std::map<std::string, std::vector<std::string>> &denylist)
{
    std::shared_lock<std::shared_mutex> lock(mutexLock);
    int osAccountId = BluetoothOsAccount::GetActiveOsAccountId();
    HILOGI("GetActiveOsAccountId %{public}d", osAccountId);
    auto it = denylist.find(std::to_string(osAccountId));
    if (it == denylist.end()) {
        HILOGI("osAccountId not found");
        return true;
    } else {
        if (find(it->second.begin(), it->second.end(), profile) == it->second.end()) {
            HILOGI("profile not in denylist");
            return true;
        } else if (profile == "OPP") {
            HILOGE("profile %{public}s restricted by edm account denylist.", profile.c_str());
            BtChrDftEventWriteInt(CHR_USER_DISCONNECT, msg.addr, "DISCONNECTREASON", Bluetooth::BT_ERR_INTERNAL_ERROR);
            return false;
        } else if (((profile == "GATT") || (profile == "SPP")) && !msg.isSystemHap && !msg.isNativeCaller) {
            HILOGI("profile %{public}s restricted by edm account denylist.", profile.c_str());
            BtChrDftEventWriteInt(CHR_USER_DISCONNECT, msg.addr, "DISCONNECTREASON", Bluetooth::BT_ERR_INTERNAL_ERROR);
            return false;
        } else {
            HILOGI("profile %{public}s allow connect.", profile.c_str());
            return true;
        }
    }
}

bool BluetoothEdmManager::IsAccountConfigAllowedConnect(const ControlInterceptMessage &msg, const std::string &profile)
{
    return IsAccountConfigAllowedInternal(msg, profile, accountBlacklistMutexLock_, accountBlacklist_);
}

bool BluetoothEdmManager::IsAccountConfigAllowedReceive(const ControlInterceptMessage &msg, const std::string &profile)
{
    return IsAccountConfigAllowedInternal(msg, profile, accountRecvBlacklistMutexLock_, accountRecvBlacklist_);
}

int BluetoothEdmManager::GetWhitelist()
{
    std::unique_lock<std::shared_mutex> lock(whitelistMutexLock_);
    whitelist_.clear();
    auto proxy = EDM::BluetoothManagerProxy::GetBluetoothManagerProxy();
    CHECK_AND_RETURN_LOG_RET(proxy, -1, "GetBluetoothManagerProxy failed");
    std::vector<std::string> whitelistTmp;
    int result = proxy->GetAllowedBluetoothDevices(whitelistTmp);
    if (result != 0) {
        HILOGE("GetAllowedBluetoothDevices failed, ret: %{public}d", result);
    } else {
        for (std::vector<std::string>::iterator it = whitelistTmp.begin(); it != whitelistTmp.end(); ++it) {
            std::string macIt = *it;
            if (!IsValidAddress(macIt)) {
                continue;
            }
            std::transform(macIt.begin(), macIt.end(), macIt.begin(), ::tolower);
            whitelist_.push_back(macIt);
        }
    }
    return result;
}

int BluetoothEdmManager::GetBlacklist()
{
    std::unique_lock<std::shared_mutex> lock(blacklistMutexLock_);
    denylist_.clear();
    auto proxy = EDM::BluetoothManagerProxy::GetBluetoothManagerProxy();
    CHECK_AND_RETURN_LOG_RET(proxy, -1, "GetBluetoothManagerProxy failed");
    std::vector<std::string> blacklistTmp;
    int result = proxy->GetDisallowedBluetoothDevices(blacklistTmp);
    if (result != 0) {
        HILOGE("GetDisallowedBluetoothDevices failed, ret: %{public}d", result);
    } else {
        for (std::vector<std::string>::iterator it = blacklistTmp.begin(); it != blacklistTmp.end(); ++it) {
            std::string macIt = *it;
            if (!IsValidAddress(macIt)) {
                continue;
            }
            std::transform(macIt.begin(), macIt.end(), macIt.begin(), ::tolower);
            denylist_.push_back(macIt);
        }
    }
    return result;
}

static void ReadStringFromFile(const std::string &path, std::string &fileStr)
{
    if (path.length() >= PATH_MAX) {
        HILOGE("length invalid");
        return;
    }
    char resolvedPath[PATH_MAX] = {0};
    char *result = realpath(path.c_str(), resolvedPath);
    if (!result) {
        HILOGE("realpath failed");
        return;
    }
    std::ifstream ifs(result);
    if (!ifs.is_open()) {
        HILOGE("ifstream open failed");
        return;
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    fileStr = std::string(buffer.str());
    ifs.close();
}

int BluetoothEdmManager::ParseAccountBlacklist(cJSON *obj, std::map<std::string, std::vector<std::string>> &denylist)
{
    int accountSize = cJSON_GetArraySize(obj);
    if (accountSize > MAX_ACCOUNT_SIZE) {
        HILOGE("accountSize %{public}d invalid", accountSize);
        return -1;
    }
    std::vector<std::string> profileBlacklist;
    for (int i = 0; i < accountSize; ++i) {
        cJSON *childNode = cJSON_GetArrayItem(obj, i);
        if ((childNode == nullptr) || (childNode->string == nullptr)) {
            continue;
        }
        HILOGI("childNode string is %{public}s", childNode->string);
        int profileSize = cJSON_GetArraySize(childNode);
        if (profileSize > MAX_PROFILE_SIZE) {
            HILOGE("profileSize invalid");
            continue;
        }
        HILOGI("childNode profileSize is %{public}d", profileSize);
        profileBlacklist.clear();
        for (int j = 0; j < profileSize; ++j) {
            cJSON *profileNode = cJSON_GetArrayItem(childNode, j);
            if ((profileNode == nullptr) || (profileNode->valuestring == nullptr)) {
                continue;
            }
            std::string profile = profileNode->valuestring;
            HILOGI("profile:%{public}s", profile.c_str());
            std::transform(profile.begin(), profile.end(), profile.begin(), ::toupper);
            profileBlacklist.push_back(profile);
        }
        denylist[childNode->string] = profileBlacklist;
    }
    return 0;
}

int BluetoothEdmManager::GetAccountBlacklistInternal(std::shared_mutex& mutexLock,
    std::map<std::string, std::vector<std::string>>& denylist, const char* jsonKey)
{
    std::unique_lock<std::shared_mutex> lock(mutexLock);
    denylist.clear();
    std::string protocolDenyList;
    ReadStringFromFile(std::string(BLUETOOTH_ACCOUNT_DENYLIST_CONFIG_FILEPATH), protocolDenyList);
    CHECK_AND_RETURN_LOG_RET(!protocolDenyList.empty(), -1, "get protocolDenyList failed");
    cJSON *root = JsonUtil::ReadJsonContents(protocolDenyList);
    CHECK_AND_RETURN_LOG_RET(root, -1, "json parse failed");
    cJSON *obj = cJSON_GetObjectItem(root, jsonKey);
    if (obj == nullptr) {
        HILOGE("get %{public}s failed", jsonKey);
        cJSON_Delete(root);
        return -1;
    }

    if (ParseAccountBlacklist(obj, denylist) != 0) {
        cJSON_Delete(root);
        return -1;
    }
    cJSON_Delete(root);
    return 0;
}

int BluetoothEdmManager::GetAccountSendBlacklist()
{
    return GetAccountBlacklistInternal(accountBlacklistMutexLock_, accountBlacklist_, "ProtocolDenyList");
}

int BluetoothEdmManager::GetAccountRecvBlacklist()
{
    return GetAccountBlacklistInternal(accountRecvBlacklistMutexLock_, accountRecvBlacklist_, "ProtocolRecDenyList");
}

void BluetoothEdmManager::OnEdmListChanged(const std::string &action)
{
    int ret = -1;
    if (action == BLUETOOTH_WHITELIST_CHANGED_EVENT) {
        HILOGI("OnReceiveEvent whitelist changed");
        ret = GetWhitelist();
        if (ret != 0) {
            HILOGE("GetWhitelist failed, ret: %{public}d", ret);
        }
    } else if (action == BLUETOOTH_DENYLIST_CHANGED_EVENT) {
        HILOGI("OnReceiveEvent denylist changed");
        ret = GetBlacklist();
        if (ret != 0) {
            HILOGE("GetBlacklist failed, ret: %{public}d", ret);
        }
    } else if (action == BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT) {
        HILOGI("OnReceiveEvent account denylist changed");
        ret = GetAccountSendBlacklist();
        if (ret != 0) {
            HILOGE("GetAccountSendBlacklist failed, ret: %{public}d", ret);
        }
        ret = GetAccountRecvBlacklist();
        if (ret != 0) {
            HILOGE("GetAccountRecvBlacklist failed, ret: %{public}d", ret);
        }
    }
}

BluetoothEdmManager::BluetoothEdmManager()
{
}

BluetoothEdmManager::~BluetoothEdmManager()
{
}

/*************************      CONTROL_INTERCEPT_PLUGIN       *******************************/
static bool EdmIsAllowedConn(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    return true;
}

static bool EdmIsAllowedGattConn(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedConnect(msg, "GATT");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedGattReceive(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedReceive(msg, "GATT");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedSppConn(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedConnect(msg, "SPP");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedOppWrite(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        if (msg.filePaths.empty()) {
            BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        }
        for (const auto& path : msg.filePaths) {
            BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid, path);
        }
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedConnect(msg, "OPP");
    if (!accountAllowed) {
        if (msg.filePaths.empty()) {
            BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        }
        for (const auto& path : msg.filePaths) {
            BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid, path);
        }
    }
    return accountAllowed;
}

static bool EdmIsAllowedOppRead(const ControlInterceptMessage &msg)
{
    bool ret = BluetoothEdmManager::GetInstance().IsAllowedConnect(msg.addr);
    if (!ret) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
        return false;
    }
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedReceive(msg, "OPP");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedGattAddService(const ControlInterceptMessage &msg)
{
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedConnect(msg, "GATT");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static bool EdmIsAllowedSocketListen(const ControlInterceptMessage &msg)
{
    bool accountAllowed = BluetoothEdmManager::GetInstance().IsAccountConfigAllowedReceive(msg, "SPP");
    if (!accountAllowed) {
        BluetoothEdmManager::GetInstance().ReportEdmAuditInfo(msg.addr, msg.pid, msg.uid);
    }
    return accountAllowed;
}

static ControlInterceptPlugin g_edmManagerPlugin = {
    .fastestGattConn = EdmIsAllowedGattConn,
    .gattConn = EdmIsAllowedGattConn,
    .avrcpConn = EdmIsAllowedConn,
    .pairConn = EdmIsAllowedConn,
    .aclConn = EdmIsAllowedConn,
    .a2dpConn = EdmIsAllowedConn,
    .hfpConn = EdmIsAllowedConn,
    .hidConn = EdmIsAllowedConn,
    .socketConn = EdmIsAllowedSppConn,
    .oppWrite = EdmIsAllowedOppWrite,
    .gattAddService = EdmIsAllowedGattAddService,
    .socketListen = EdmIsAllowedSocketListen,
    .oppRead = EdmIsAllowedOppRead,
    .gattReceive = EdmIsAllowedGattReceive,
};

REGISTER_CONTROL_INTERCEPT_PLUGIN(g_edmManagerPlugin);
/*************************      CONTROL_INTERCEPT_PLUGIN       *******************************/

void BluetoothEdmManager::ReportEdmAuditInfo(const std::string &connectedDeviceId, int appId, int userId,
    const std::string &filePath)
{
    std::string appIdStr = std::to_string(appId);
    int64_t happenTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    cJSON *outJson = cJSON_CreateObject();
    if (outJson == nullptr) {
        HILOGE("ReportEdmAuditInfo json object created error");
        return;
    }
    cJSON_AddStringToObject(outJson, "connectedDeviceId", GetEncryptAddr(connectedDeviceId).c_str());
    cJSON_AddStringToObject(outJson, "appId", appIdStr.c_str());
    cJSON_AddNumberToObject(outJson, "userId", userId);
    cJSON_AddNumberToObject(outJson, "happTime", happenTime);
    cJSON_AddStringToObject(outJson, "file_path", filePath.c_str());
    char *jsonContent = cJSON_PrintUnformatted(outJson);
    if (jsonContent == nullptr) {
        HILOGE("ReportEdmAuditInfo print json unformatted error");
        cJSON_Delete(outJson);
        outJson = nullptr;
        return;
    }
    std::string content = std::string(jsonContent);
    cJSON_free(jsonContent);
    cJSON_Delete(outJson);
    outJson = nullptr;
    std::shared_ptr<Security::SecurityGuard::EventInfo> eventInfo =
        std::make_shared<Security::SecurityGuard::EventInfo>(0x03000200, "1.0", content);
    int ret = Security::SecurityGuard::NativeDataCollectKit::ReportSecurityInfo(eventInfo);
    HILOGI("report appIdStr:%{public}s,userId:%{public}d, ret:%{public}d", appIdStr.c_str(), userId, ret);
}
} // namespace bluetooth
} // namespace OHOS