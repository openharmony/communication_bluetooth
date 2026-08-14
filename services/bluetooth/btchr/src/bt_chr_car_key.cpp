/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_chr_car_key"
#endif

#include <set>
#include <map>
#include <mutex>
#include <queue>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "cJSON.h"
#include "bt_chr_car_key.h"
#include "preferences_manager.h"
#include "thread_util.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "want.h"
#include "ability_manager_ipc_interface_code.h"

namespace OHOS {
namespace bluetooth {
namespace CarKey {

enum class CarKeyCardAction {
    DELETE = 0,
    ADD
};

const std::set<std::string> g_bleChrReportSet = {CHR_UE_BLE_START_SCAN, CHR_UE_BLE_STOP_SCAN, CHR_UE_START_PAIR,
                                                 CHR_UE_REMOVE_PAIR, CHR_UE_CANCEL_PAIR, CHR_UE_BLE_CLIENT_CONN,
                                                 CHR_UE_BLE_CLIENT_DISCONN, CHR_UE_BLE_CONN_CMP,
                                                 CHR_UE_BLE_DISCONN_CMP, CHR_BLE_DISCONNECT, CHR_UE_BRCAST_ACL_CONN,
                                                 CHR_BT_PAIR_EXCEPTION};
const std::map<std::string, std::set<std::string>> g_specialChrMap = {{CHR_BLE_DISCONNECT, {"ENCRYPTIONSTATUS"}},
                                                                      {CHR_BT_PAIR_EXCEPTION, {"SUBERRCODE"}}};
constexpr int MAX_NUM_CHR_SIZE = 1000;
constexpr int MAX_NUM_CHR_NOTIFY_SIZE = 100;
constexpr double CHR_LOAD_FACTOR = 0.9;
constexpr const char *WALLET_ABILITY_NAME = "ReportDumpInfoSrvExtAbility";
constexpr const char *WALLET_PACKAGE_NAME = "com.huawei.wallet";
constexpr const char *WALLET_SERVICE_PACKAGE_NAME = "com.huawei.wallet";
constexpr const char *WALLET_BLE_DUMP_ACTION = "ohos.bluetooth.action.WALLET_BLE_DUMP";
constexpr const char *CHR_EVENT_NAME = "EVENT_NAME";
constexpr char TIME_FORMAT[] = "%m-%d %H:%M:%S.";
constexpr int MILLIS_PER_SEC = 1000;
constexpr int TIME_MILLIS_WIDTH = 3;
constexpr const char FILL_CHARACTER = '0';
const std::u16string ABILITY_MGR_DESCRIPTOR = u"ohos.aafwk.AbilityManager";
constexpr int BT_ERR_INTERNAL_ERROR = 2900099;
constexpr int BT_NO_ERROR = 0;
constexpr int DEFAULT_INVAL_VALUE = -1;

std::set<std::string> g_cardAddrSet;

std::string convertChrParamsToJsonString(const std::string &eventName, const std::vector<BtChrEventParam> &params)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, CHR_EVENT_NAME, cJSON_CreateString(eventName.c_str()));
    for (const auto &param: params) {
        switch (param.type) {
            case BTCHREVENT_INT32:
                cJSON_AddItemToObject(root, param.name.c_str(), cJSON_CreateNumber(param.i32Value));
                break;
            case BTCHREVENT_UINT32:
                cJSON_AddItemToObject(root, param.name.c_str(), cJSON_CreateNumber(param.ui32Value));
                break;
            case BTCHREVENT_STRING:
                cJSON_AddItemToObject(root, param.name.c_str(), cJSON_CreateString(param.strValue.c_str()));
                break;
            default:
                HILOGW("unknown_type");
                break;
        }
    }
    char* chrData = cJSON_PrintUnformatted(root);
    if (chrData == nullptr) {
        HILOGE("cJSON_Print error.");
        cJSON_Delete(root);
        return "";
    }
    std::string result = std::string(chrData);
    cJSON_free(chrData);
    cJSON_Delete(root);
    return result;
}

void updateCardInfo()
{
    g_cardAddrSet.clear();
    const std::map<std::string, PreferencesValue> &allDevices = PreferencesManager::GetAll(CAR_KEY_CARD);
    for (const auto& pair : allDevices) {
        std::string addr = pair.first;
        if (!IsValidAddr(addr)) {
            continue;
        }
        std::transform(addr.begin(), addr.end(), addr.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        g_cardAddrSet.emplace(GET_ENCRYPT_STR_ADDR(addr));
    }
}

void InitCardInfo()
{
    updateCardInfo();
}

bool isWalletChr(const std::vector<BtChrEventParam> &params)
{
    for (const auto &param: params) {
        if (param.name == "PKG_NAME" && param.strValue == WALLET_SERVICE_PACKAGE_NAME) {
            return true;
        }
        if (param.name == "DEV_ADDRESS") {
            std::string addr = param.strValue;
            std::transform(addr.begin(), addr.end(), addr.begin(), [](unsigned char c) {
                return std::tolower(c);
            });
            if (g_cardAddrSet.count(addr)) {
                return true;
            }
        }
    }
    return false;
}

/**
 * Format time to MM-DD HH:mm:ss.SSS
 *
 * @return time str
 */
std::string formatTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream formatTime;
    formatTime << std::put_time(std::localtime(&in_time_t), TIME_FORMAT)
               << std::setw(TIME_MILLIS_WIDTH) << std::setfill(FILL_CHARACTER)
               << (now.time_since_epoch().count() % MILLIS_PER_SEC);
    return formatTime.str();
}

int32_t startAbility(AAFwk::Want &want)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG_RET(systemAbilityManager != nullptr, BT_ERR_INTERNAL_ERROR, "SystemAbilityManager is nullptr");
    sptr<IRemoteObject> remote = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    CHECK_AND_RETURN_LOG_RET(remote != nullptr, BT_ERR_INTERNAL_ERROR, "Remote is nullptr");
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR), BT_ERR_INTERNAL_ERROR,
                             "Write interface token error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&want), BT_ERR_INTERNAL_ERROR, "Write Want error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(DEFAULT_INVAL_VALUE), BT_ERR_INTERNAL_ERROR, "Write UserId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(DEFAULT_INVAL_VALUE), BT_ERR_INTERNAL_ERROR, "Write RequestCode error");
    MessageParcel reply;
    MessageOption option;
    auto task = static_cast<uint32_t>(AAFwk::AbilityManagerInterfaceCode::START_ABILITY);
    int ret = remote->SendRequest(task, data, reply, option);
    CHECK_AND_RETURN_LOG_RET(ret == BT_NO_ERROR, ret, "Send request error");
    return reply.ReadInt32();
}

void startWalletExtensionAbility()
{
    HILOGI("begin");
    AAFwk::Want want;
    want.SetElementName(WALLET_PACKAGE_NAME, WALLET_ABILITY_NAME);
    want.SetAction(WALLET_BLE_DUMP_ACTION);
    int32_t ret = startAbility(want);
    if (ret != BT_NO_ERROR) {
        HILOGE("notify wallet failed, reason:%{public}d", ret);
        return;
    }
    HILOGI("notify wallet success");
}

bool IsWalletDftEvent(const std::string &eventName, BtChrEventParam &param)
{
    auto it = g_specialChrMap.find(eventName);
    if (it != g_specialChrMap.end()) {
        const std::set<std::string> &subEventSet = it->second;
        if (subEventSet.count(param.name)) {
            return true;
        }
    }
    return false;
}

void RecordWalletChr(const std::string &eventName, const std::vector<BtChrEventParam> &params)
{
    if (g_cardAddrSet.empty() || !g_bleChrReportSet.count(eventName) || !isWalletChr(params)) {
        return;
    }
    std::map<std::string, PreferencesValue> allChrData = PreferencesManager::GetAll(CAR_KEY_DFX_DATA);
    auto currentSize = static_cast<uint16_t>(allChrData.size());
    HILOGD("currentSize: %{public}d", currentSize);
    if (currentSize >= MAX_NUM_CHR_SIZE) {
        for (const auto &[key, value]: allChrData) {
            if (currentSize > MAX_NUM_CHR_SIZE * CHR_LOAD_FACTOR) {
                PreferencesManager::Delete(key, CAR_KEY_DFX_DATA);
                currentSize--;
            }
        }
    }
    std::string basicString = convertChrParamsToJsonString(eventName, params);
    PreferencesManager::SaveString(formatTime(), basicString, CAR_KEY_DFX_DATA);
    HILOGI("Record wallet chr event success: %{public}s", eventName.c_str());
    currentSize++;
    if (currentSize >= MAX_NUM_CHR_NOTIFY_SIZE) {
        DoInLowPriorityThread([]() {
            startWalletExtensionAbility();
        });
    }
}

std::string GetChrData()
{
    const std::map<std::string, PreferencesValue> &allEntries = PreferencesManager::GetAll(CAR_KEY_DFX_DATA);
    if (allEntries.empty()) {
        return "";
    }
    int dequeueCount = allEntries.size() >= MAX_NUM_CHR_NOTIFY_SIZE ? MAX_NUM_CHR_NOTIFY_SIZE
                                                                    : static_cast<uint16_t>(allEntries.size());
    cJSON *root = cJSON_CreateObject();
    for (const auto &entry: allEntries) {
        if (dequeueCount == 0) {
            break;
        }
        std::string strValue = entry.second;
        cJSON_AddItemToObject(root, entry.first.c_str(), cJSON_CreateString(strValue.c_str()));
        PreferencesManager::Delete(entry.first, CAR_KEY_DFX_DATA);
        dequeueCount--;
    }
    char* chrData = cJSON_PrintUnformatted(root);
    if (chrData == nullptr) {
        HILOGE("cJSON_Print error.");
        cJSON_Delete(root);
        return "";
    }
    std::string result = std::string(chrData);
    cJSON_free(chrData);
    cJSON_Delete(root);
    return result;
}

void SetCardChrInfo(const std::string &address, int32_t action)
{
    HILOGI("address: %{public}s, action: %{public}d", GET_ENCRYPT_STR_ADDR(address), action);
    switch (static_cast<CarKeyCardAction>(action)) {
        case CarKeyCardAction::ADD:
            PreferencesManager::SaveString(address, "", CAR_KEY_CARD);
            break;
        case CarKeyCardAction::DELETE:
            PreferencesManager::Delete(address, CAR_KEY_CARD);
            break;
        default:
            HILOGW("invalid action");
            break;
    }
    updateCardInfo();
}
}
}
}