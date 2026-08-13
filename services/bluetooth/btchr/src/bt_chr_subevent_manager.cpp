/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_chr_subevent_manager"
#endif

#include "bt_chr_subevent_manager.h"
#include "bt_chr_util.h"
#include "bt_chr_base.h"
#include "bluetooth_log.h"
#include "cJSON.h"
#include <set>
#include <queue>
#include <functional>
#include "safe_map.h"
#include "safe_vector.h"
#include "securec.h"

namespace OHOS {
namespace bluetooth {
#define MAX_CONNECT_DEVICE_NUM 10
#define MAX_DEVICE_INFO_NUM 100
#define MAX_AUTH_RESULT_INFO_SIZE 5
#define CHR_DEVICE_RSSI_DEFAULT_VALUE (-127)
#define CHR_DEFAULT_VALUE (-1)
#define POWER_LEVEL_MAX 20
static const uint32_t COD_MASK = 0x07FF;
static const std::string BLUETOOTH_BROKER_CALLING_NAME = "broker";

struct BtChrAuthResultInfo {
    int32_t authStatus;
    std::string authTime;
};

struct BtChrConnSceneInfo {
    int32_t pairType;
    std::string pairApp;
    std::string connTime;
    int32_t secureConnectionFlag;
};

struct BtChrChipInfo {
    std::string antennaStatus;
    int32_t goodChannelsNum;
    std::string powerLevel;
    std::string transPerform;
};

struct BtChrAntSwitchInfo {
    uint16_t antGroupIdx;
    uint16_t configState;
};

struct PowerLevelStatistics {
    PowerLevelStatistics();

    std::chrono::milliseconds duration;
    uint32_t times;
};

// key: macAddr, value：device info
SafeMap<std::string, BtChrDeviceInfo> g_btChrDeviceInfoMap;

// key: macAddr, auth result infos
SafeMap<std::string, std::queue<BtChrAuthResultInfo>> g_authResultInfoMap;

// key: macAddr, value：bt conn scene
SafeMap<std::string, BtChrConnSceneInfo> g_btChrConnSceneInfoMap;

// key: macAddr, value：bt chip info
SafeMap<std::string, BtChrChipInfo> g_btChrChipInfoMap;

//天线switch状态实时上报vector，用于连接等需要即时天线状态事件
utility::SafeVector<BtChrAntSwitchInfo> g_btChrAntSwitchStateVec;

//天线switch状态音频记录vector，用于音频卡顿等需要天线状态快照事件
utility::SafeVector<BtChrAntSwitchInfo> g_btChrAudioAntSwitchStateVec;

// key: powerLevel, value: 统计信息，存储各挡位统计信息
SafeMap<uint8_t, PowerLevelStatistics> g_powerLevelStat;

std::mutex g_connectedDeviceSetMutex {};
std::set<std::string> g_ConnectedDeviceSet;
std::string g_localDeviceName;

PowerLevelStatistics::PowerLevelStatistics()
{
    duration = std::chrono::milliseconds::zero();
    times = 0;
}

void BtChrSetLocalDeviceName(char *deviceName)
{
    g_localDeviceName = deviceName;
}

bool GetDeviceInfoFromMap(const std::string& peerAddr, BtChrDeviceInfo& deviceInfo)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    return g_btChrDeviceInfoMap.Find(addr, deviceInfo);
}

void BtChrUpdateLinkType(const std::string& peerAddr, int32_t linkType)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrDeviceInfo deviceInfo;
    if (!g_btChrDeviceInfoMap.Find(addr, deviceInfo) &&
        (g_btChrDeviceInfoMap.Size() >= MAX_DEVICE_INFO_NUM)) {
        HILOGD("device info reached max");
        return;
    }
    deviceInfo.linkType = linkType;
    g_btChrDeviceInfoMap.EnsureInsert(addr, deviceInfo);
}

void BtChrUpdateDeviceInfoParam(const std::string& peerAddr, int32_t status,
    std::function<void(BtChrDeviceInfo &deviceInfo, int32_t status)> updateFunc)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrDeviceInfo deviceInfo;
    if (!g_btChrDeviceInfoMap.Find(addr, deviceInfo) && (g_btChrDeviceInfoMap.Size() >= MAX_DEVICE_INFO_NUM)) {
        HILOGE("not found map, peerAddr: %{public}s, status: %{public}d", GET_ENCRYPT_ADDR(peerAddr), status);
        return;
    }
    updateFunc(deviceInfo, status);
    g_btChrDeviceInfoMap.EnsureInsert(addr, deviceInfo);
}

void BtChrUpdateSupportA2dpStatus(const std::string& peerAddr, int32_t status)
{
    BtChrUpdateDeviceInfoParam(
        peerAddr, status, [](BtChrDeviceInfo &deviceInfo, int32_t status) { deviceInfo.needA2dp = status; });
}

void BtChrUpdateSupportHfpStatus(const std::string& peerAddr, int32_t status)
{
    BtChrUpdateDeviceInfoParam(
        peerAddr, status, [](BtChrDeviceInfo &deviceInfo, int32_t status) { deviceInfo.needHfp = status; });
}

void BtChrUpdateSupportHidStatus(const std::string& peerAddr, int32_t status)
{
    BtChrUpdateDeviceInfoParam(
        peerAddr, status, [](BtChrDeviceInfo &deviceInfo, int32_t status) { deviceInfo.needHid = status; });
}

void BtChrUpdateA2dpStatus(const std::string& peerAddr, int32_t status)
{
    BtChrUpdateDeviceInfoParam(
        peerAddr, status, [](BtChrDeviceInfo &deviceInfo, int32_t status) { deviceInfo.a2dpStatus = status; });
}

void BtChrUpdateHfpStatus(const std::string& peerAddr, int32_t status)
{
    BtChrUpdateDeviceInfoParam(
        peerAddr, status, [](BtChrDeviceInfo &deviceInfo, int32_t status) { deviceInfo.hfpStatus = status; });
}

void BtChrUpdateHidStatus(const std::string& peerAddr, int32_t status)
{
    BtChrUpdateDeviceInfoParam(
        peerAddr, status, [](BtChrDeviceInfo &deviceInfo, int32_t status) { deviceInfo.hidStatus = status; });
}

void BtChrUpdateDeviceInfo(const std::string& peerAddr, const std::string& deviceName, int rssi, int deviceClass)
{
    HILOGI("peerAddr: %{public}s, rssi: %{public}d, deviceClass: %{public}d",
        GET_ENCRYPT_ADDR(peerAddr), rssi, deviceClass);
    uint32_t cod = deviceClass & COD_MASK;
    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrDeviceInfo deviceInfo;
    if (!g_btChrDeviceInfoMap.Find(addr, deviceInfo) && (g_btChrDeviceInfoMap.Size() >= MAX_DEVICE_INFO_NUM)) {
        HILOGE("device info reached max");
        return;
    }
    deviceInfo.btDeviceName = deviceName;
    deviceInfo.btDeviceRssi = rssi;
    deviceInfo.btDeviceClass = cod;
    g_btChrDeviceInfoMap.EnsureInsert(addr, deviceInfo);
}

void BtChrUpdateChipInfo(const std::string& peerAddr, int btVersion, int btSubVersion, int chipCompanyId)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrDeviceInfo deviceInfo;
    if (!g_btChrDeviceInfoMap.Find(addr, deviceInfo)  && (g_btChrDeviceInfoMap.Size() >= MAX_DEVICE_INFO_NUM)) {
        HILOGE("device info reached max, peerAddr: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
        return;
    }
    deviceInfo.btVersion = btVersion;
    deviceInfo.btSubVersion = btSubVersion;
    deviceInfo.chipCompanyId = chipCompanyId;
    g_btChrDeviceInfoMap.EnsureInsert(addr, deviceInfo);
    HILOGD("peerAddr: %{public}s version %{public}d sub_ver %{public}d manufacturer %{public}d",
        GET_ENCRYPT_ADDR(peerAddr), btVersion, btSubVersion, chipCompanyId);
}

void BtChrAddConnectedDevice(const std::string& peerAddr)
{
    HILOGD("peerAddr: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
    std::lock_guard<std::mutex> lock(g_connectedDeviceSetMutex);
    if (g_ConnectedDeviceSet.size() >= MAX_CONNECT_DEVICE_NUM) {
        HILOGE("connect device reached max");
        return;
    }
    std::string addr = GetLowercaseAddr(peerAddr);
    g_ConnectedDeviceSet.insert(addr);
}

void BtChrDelConnectedDevice(const std::string& peerAddr, int32_t status)
{
    (void)status;
    HILOGD("peerAddr: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
    std::lock_guard<std::mutex> lock(g_connectedDeviceSetMutex);
    std::string addr = GetLowercaseAddr(peerAddr);
    g_ConnectedDeviceSet.erase(addr);
}

static cJSON* GetDefaultDeviceInfoJsonValue(const std::string& peerAddr)
{
    cJSON* deviceObj = cJSON_CreateObject();
    cJSON_AddStringToObject(deviceObj, "DEVICENAME", "");
    cJSON_AddStringToObject(deviceObj, "DEVICEADDR", GetEncryptAddr(peerAddr).c_str());
    cJSON_AddNumberToObject(deviceObj, "BTDEVICERSSI", CHR_DEVICE_RSSI_DEFAULT_VALUE);
    cJSON_AddNumberToObject(deviceObj, "LIKETYPE", CHR_DEFAULT_VALUE);
    cJSON_AddNumberToObject(deviceObj, "NEEDA2DP", 0);
    cJSON_AddNumberToObject(deviceObj, "A2DPSTATUS", CHR_DEFAULT_VALUE);
    cJSON_AddNumberToObject(deviceObj, "NEEDHFP", 0);
    cJSON_AddNumberToObject(deviceObj, "HFPSTATUS", CHR_DEFAULT_VALUE);
    cJSON_AddNumberToObject(deviceObj, "NEEDHID", 0);
    cJSON_AddNumberToObject(deviceObj, "HIDSTATUS", CHR_DEFAULT_VALUE);
    cJSON_AddNumberToObject(deviceObj, "CHIPCOMPANYID", CHR_DEFAULT_VALUE);
    cJSON_AddNumberToObject(deviceObj, "BTSUBVERSION", CHR_DEFAULT_VALUE);
    cJSON_AddNumberToObject(deviceObj, "BTVERSION", CHR_DEFAULT_VALUE);
    cJSON_AddStringToObject(deviceObj, "DEVICECLASS", "");
    return deviceObj;
}

static cJSON* GetDeviceInfoJsonValue(const std::string& peerAddr, BtChrDeviceInfo &deviceInfo)
{
    cJSON* deviceObj = cJSON_CreateObject();
    cJSON_AddStringToObject(deviceObj, "DEVICENAME", deviceInfo.btDeviceName.c_str());
    cJSON_AddStringToObject(deviceObj, "DEVICEADDR", GetEncryptAddr(peerAddr).c_str());
    cJSON_AddNumberToObject(deviceObj, "BTDEVICERSSI", deviceInfo.btDeviceRssi);
    cJSON_AddNumberToObject(deviceObj, "LIKETYPE", deviceInfo.linkType);
    cJSON_AddNumberToObject(deviceObj, "NEEDA2DP", deviceInfo.needA2dp);
    cJSON_AddNumberToObject(deviceObj, "A2DPSTATUS", deviceInfo.a2dpStatus);
    cJSON_AddNumberToObject(deviceObj, "NEEDHFP", deviceInfo.needHfp);
    cJSON_AddNumberToObject(deviceObj, "HFPSTATUS", deviceInfo.hfpStatus);
    cJSON_AddNumberToObject(deviceObj, "NEEDHID", deviceInfo.needHid);
    cJSON_AddNumberToObject(deviceObj, "HIDSTATUS", deviceInfo.hidStatus);
    cJSON_AddNumberToObject(deviceObj, "CHIPCOMPANYID", deviceInfo.chipCompanyId);
    cJSON_AddNumberToObject(deviceObj, "BTVERSION", deviceInfo.btVersion);
    cJSON_AddNumberToObject(deviceObj, "BTSUBVERSION", deviceInfo.btSubVersion);
    cJSON_AddNumberToObject(deviceObj, "DEVICECLASS", deviceInfo.btDeviceClass);

    return deviceObj;
}

std::string BtChrGetBtCommonJsonStr(const std::string& peerAddr)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "LOCALDEVICENAME", g_localDeviceName.c_str());
    if (!peerAddr.empty()) {
        BtChrDeviceInfo deviceInfo;
        if (GetDeviceInfoFromMap(peerAddr, deviceInfo)) {
            cJSON_AddItemToObject(root, "DEVICEINFO", GetDeviceInfoJsonValue(peerAddr, deviceInfo));
        } else {
            cJSON_AddItemToObject(root, "DEVICEINFO", GetDefaultDeviceInfoJsonValue(peerAddr));
        }
    }
    char* chrData = cJSON_PrintUnformatted(root);
    if (chrData == nullptr) {
        HILOGE("cJSON_Print error.");
        cJSON_Delete(root);
        return "";
    }
    std::string result(chrData);
    cJSON_free(chrData);
    cJSON_Delete(root);
    return result;
}

std::string BtChrGetConnectedDeviceJsonStr()
{
    cJSON* arrayObj = cJSON_CreateArray();
    {
        std::lock_guard<std::mutex> lock(g_connectedDeviceSetMutex);
        if (g_ConnectedDeviceSet.empty()) {
            HILOGD("g_ConnectedDeviceSet is empty");
            cJSON_Delete(arrayObj);
            return "";
        }
        for (auto it = g_ConnectedDeviceSet.begin(); it != g_ConnectedDeviceSet.end(); it++) {
            BtChrDeviceInfo deviceInfo;
            if (!GetDeviceInfoFromMap(*it, deviceInfo)) {
                continue;
            }
            cJSON* arrayItem = GetDeviceInfoJsonValue(*it, deviceInfo);
            cJSON_AddItemToArray(arrayObj, arrayItem);
        }
    }
    char* chrData = cJSON_PrintUnformatted(arrayObj);
    if (chrData == nullptr) {
        HILOGE("cJSON_Print error.");
        cJSON_Delete(arrayObj);
        return "";
    }
    std::string result(chrData);
    cJSON_free(chrData);
    cJSON_Delete(arrayObj);
    return result;
}

void BtChrAddAuthResultInfo(const std::string& peerAddr, int32_t authStatus)
{
    BtChrAuthResultInfo authInfo = {
        .authStatus = authStatus,
        .authTime = GetCurrentTime(),
    };
    std::string addr = GetLowercaseAddr(peerAddr);
    std::queue<BtChrAuthResultInfo> authQueue;
    if (g_authResultInfoMap.Find(addr, authQueue) && (authQueue.size() >= MAX_AUTH_RESULT_INFO_SIZE)) {
        authQueue.pop();
    }
    authQueue.push(authInfo);
    g_authResultInfoMap.EnsureInsert(addr, authQueue);
}

std::string BtChrGetAuthResultJsonStr(const std::string& peerAddr)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    std::queue<BtChrAuthResultInfo> authQueue;
    if (!g_authResultInfoMap.Find(addr, authQueue)) {
        HILOGI("g_authResultInfoMap not found: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
        return "";
    }
    cJSON* arrayObj = cJSON_CreateArray();
    while (!authQueue.empty()) {
        BtChrAuthResultInfo authInfo = authQueue.front();
        cJSON *arrayItem = cJSON_CreateObject();
        cJSON_AddNumberToObject(arrayItem, "AUTHRESULT", authInfo.authStatus);
        cJSON_AddStringToObject(arrayItem, "AUTHTIME", authInfo.authTime.c_str());
        cJSON_AddItemToArray(arrayObj, arrayItem);
        authQueue.pop();
    }
    g_authResultInfoMap.Erase(addr);
    char* chrData = cJSON_PrintUnformatted(arrayObj);
    if (chrData == nullptr) {
        cJSON_Delete(arrayObj);
        return "";
    }
    std::string result(chrData);
    cJSON_free(chrData);
    cJSON_Delete(arrayObj);
    return result;
}

std::string BtChrGetConnSceneJsonStr(const std::string& peerAddr)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    cJSON* root = cJSON_CreateObject();
    if (!peerAddr.empty()) {
        BtChrConnSceneInfo connSceneInfo;
        if (g_btChrConnSceneInfoMap.Find(peerAddr, connSceneInfo)) {
            cJSON_AddNumberToObject(root, "PAIRTYPE", connSceneInfo.pairType);
            cJSON_AddStringToObject(root, "PAIRAPP", connSceneInfo.pairApp.c_str());
            cJSON_AddStringToObject(root, "CONNTIME", connSceneInfo.connTime.c_str());
            cJSON_AddNumberToObject(root, "SECURECONNECTIONFLAG", connSceneInfo.secureConnectionFlag);
        } else {
            cJSON_Delete(root);
            return "";
        }
    }
    g_btChrConnSceneInfoMap.Erase(addr);
    char* chrData = cJSON_PrintUnformatted(root);
    if (chrData == nullptr) {
        cJSON_Delete(root);
        return "";
    }
    std::string result(chrData);
    cJSON_free(chrData);
    cJSON_Delete(root);
    return result;
}

void BtChrAddConnSceneInfo(const std::string& peerAddr, BtChrPairType pairType, const std::string& pairApp,
    int32_t secureConnectionFlag)
{
    HILOGI("add ConnSceneInfo: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrConnSceneInfo chrConnSceneInfo {};
    if ((g_btChrConnSceneInfoMap.Find(addr, chrConnSceneInfo)) && (!chrConnSceneInfo.connTime.empty())) {
        HILOGI("device connect scene info is aleady exist , peerAddr: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
        return;
    }

    if (chrConnSceneInfo.pairApp.empty()) {
        chrConnSceneInfo.pairApp = pairApp;
    } else {
        chrConnSceneInfo.pairApp = pairApp + ':' + chrConnSceneInfo.pairApp;
    }
    chrConnSceneInfo.pairType = static_cast<int32_t>(pairType);
    chrConnSceneInfo.connTime = GetCurrentTime();
    chrConnSceneInfo.secureConnectionFlag = secureConnectionFlag;
    g_btChrConnSceneInfoMap.EnsureInsert(addr, chrConnSceneInfo);
    BtChrEventWriteStr("BT_WATCH_CONNECT", peerAddr, "PKGNAME", pairApp);
}

void BtChrAddCallingPackageName(const std::string& peerAddr, const std::string& packageName)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    HILOGI("add CallingPackageName, peerAddr: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
    BtChrConnSceneInfo chrConnSceneInfo {};
    if (!g_btChrConnSceneInfoMap.Find(addr, chrConnSceneInfo)) {
        chrConnSceneInfo.pairApp = packageName;
        g_btChrConnSceneInfoMap.EnsureInsert(addr, chrConnSceneInfo);
        return;
    }

    if (chrConnSceneInfo.pairApp.find(packageName) != std::string::npos) {
        HILOGI("packageName is aleady exist, peerAddr: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
        return;
    }
    // 当前仅东湖场景
    if (chrConnSceneInfo.pairApp != BLUETOOTH_BROKER_CALLING_NAME) {
        HILOGI("this peerAddr:%{public}s is Non-broker initiated ACL connection", GET_ENCRYPT_ADDR(peerAddr));
        return;
    }
    chrConnSceneInfo.pairApp += (":" + packageName);
    g_btChrConnSceneInfoMap.EnsureInsert(addr, chrConnSceneInfo);
}

std::string BtChrGetAntSwitchStatusJsonStr(bool isNeedRealTimeAntState)
{
    cJSON* arrayObj = cJSON_CreateArray();
    if (arrayObj == nullptr) {
        HILOGE("cJSON_Create error");
        return "";
    }
    auto func = [&arrayObj] (const BtChrAntSwitchInfo &info) {
        cJSON* item = cJSON_CreateObject();
        if (item != nullptr) {
            cJSON_AddNumberToObject(item, "antGroupIdx", info.antGroupIdx);
            cJSON_AddNumberToObject(item, "configState", info.configState);
            cJSON_AddItemToArray(arrayObj, item);
        }
    };
    if (isNeedRealTimeAntState) {
        g_btChrAntSwitchStateVec.ForEach(func);
    } else {
        g_btChrAudioAntSwitchStateVec.ForEach(func);
    }
    
    char* chrData = cJSON_PrintUnformatted(arrayObj);
    if (chrData == nullptr) {
        cJSON_Delete(arrayObj);
        return "";
    }
    std::string result(chrData);
    cJSON_free(chrData);
    cJSON_Delete(arrayObj);
    return result;
}

bool IsNeedRealTimeAntState(const std::string& eventKey)
{
    return eventKey.find(CHR_USER_DISCONNECT) != std::string::npos ||
        eventKey.find(CHR_BLE_DISCONNECT) != std::string::npos;
}

std::string BtChrGetChipInfoJsonStr(const std::string& peerAddr, const std::string& eventKey)
{
    if (peerAddr.empty()) {
        return "";
    }

    BtChrChipInfo btChipInfo {};
    std::string addr = GetLowercaseAddr(peerAddr);
    bool isNeedRealTimeAntState = IsNeedRealTimeAntState(eventKey);
    if ((!isNeedRealTimeAntState) && (!g_btChrChipInfoMap.Find(addr, btChipInfo))) {
        return "";
    }

    cJSON* root = cJSON_CreateObject();
    if (root == nullptr) {
        HILOGE("cJSON_Create error");
        return "";
    }
    // SIGHUB实时上报天线交叉状态不携带前4个参数，相应值置空
    cJSON_AddStringToObject(root, "ANTENNASTATUS", btChipInfo.antennaStatus.c_str());
    cJSON_AddNumberToObject(root, "GOODCHANNELSNUM", btChipInfo.goodChannelsNum);
    cJSON_AddStringToObject(root, "POWERLEVEL", btChipInfo.powerLevel.c_str());
    cJSON_AddStringToObject(root, "TRANSPERFORM", btChipInfo.transPerform.c_str());
    cJSON_AddStringToObject(root, "ANTENNASWITCHSTATE",
        BtChrGetAntSwitchStatusJsonStr(isNeedRealTimeAntState).c_str());

    char* chrData = cJSON_PrintUnformatted(root);
    if (chrData == nullptr) {
        cJSON_Delete(root);
        return "";
    }
    std::string result(chrData);
    cJSON_free(chrData);
    cJSON_Delete(root);
    return result;
}

void BtChrClearAudioAntSwitchState()
{
    g_btChrAudioAntSwitchStateVec.Clear();
}

void BtChrDelAudioChipInfo(const std::string& peerAddr)
{
    if (peerAddr.empty()) {
        return;
    }
    g_btChrChipInfoMap.Erase(peerAddr);
}

void BtChrUpdateGoodChannelNum(const std::string& peerAddr, int goodChannelsNum)
{
    HILOGI("add ChipInfo: %{public}s", GET_ENCRYPT_ADDR(peerAddr));
    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrChipInfo btChipInfo = {"", 0, "", ""};
    g_btChrChipInfoMap.Find(addr, btChipInfo);
    btChipInfo.goodChannelsNum = goodChannelsNum;
    g_btChrChipInfoMap.EnsureInsert(addr, btChipInfo);
}

void BtChrUpdateAntennaStatus(const std::string& peerAddr, const std::string &antennaStatus)
{
    HILOGI("add ChipInfo antennaStatus : %{public}s", GET_ENCRYPT_ADDR(peerAddr));
    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrChipInfo btChipInfo = {"", 0, "", ""};
    g_btChrChipInfoMap.Find(addr, btChipInfo);
    btChipInfo.antennaStatus = antennaStatus;
    g_btChrChipInfoMap.EnsureInsert(addr, btChipInfo);
}

void BtChrUpdatePowerLevel(const std::string &peerAddr, std::string powerLevel)
{
    HILOGI("add ChipInfo powerLevel : %{public}s", GET_ENCRYPT_ADDR(peerAddr));
    std::string addr = GetLowercaseAddr(peerAddr);
    g_btChrChipInfoMap.ChangeValueByLambda<>(addr,
        [&powerLevel](BtChrChipInfo &value) {
            value.powerLevel = std::move(powerLevel);
        });
}

void BtChrUpdateTransPerform(const std::string &peerAddr, std::string transPerform)
{
    HILOGI("add ChipInfo transPerform : %{public}s", GET_ENCRYPT_ADDR(peerAddr));
    std::string addr = GetLowercaseAddr(peerAddr);
    g_btChrChipInfoMap.ChangeValueByLambda<>(addr,
        [&transPerform](BtChrChipInfo &value) {
            value.transPerform = std::move(transPerform);
        });
}

void BtChrSubEventReleaseAll(const std::vector<std::string> &notClearAddrs)
{
    BtChrReleaseMap(g_btChrDeviceInfoMap, notClearAddrs, BT_CHR_DEVICE_INFO_MAP);
}

void AddPowerLevelDuration(uint8_t powerLevel, std::chrono::milliseconds duration)
{
    if (powerLevel >= POWER_LEVEL_MAX || duration.count() <= 0) {
        return;
    }
    g_powerLevelStat.ChangeValueByLambda<>(powerLevel, [duration](PowerLevelStatistics &stat) {
        stat.duration += duration;
    });
}

void AddPowerLevelTimes(uint8_t powerLevel)
{
    if (powerLevel >= POWER_LEVEL_MAX) {
        return;
    }
    g_powerLevelStat.ChangeValueByLambda<>(powerLevel, [](PowerLevelStatistics &stat) {
        stat.times++;
    });
}

std::string GetPowerLevelStatistics()
{
    // timeId:指示该统计数据的批次，可根据同批次中，最新的数据作为最终的统计数据
    static std::chrono::milliseconds timeId = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "TIMEID", timeId.count());

    cJSON *array = cJSON_CreateArray();
    g_powerLevelStat.Iterate([&array](const uint8_t powerLevel, PowerLevelStatistics &stat) {
        cJSON *object = cJSON_CreateObject();
        cJSON_AddNumberToObject(object, "POWERLEVEL", powerLevel);
        cJSON_AddNumberToObject(object, "DURATION", stat.duration.count());
        cJSON_AddNumberToObject(object, "TIMES", stat.times);
        cJSON_AddItemToArray(array, object);
    });
    cJSON_AddItemToObject(root, "STATISTICS", array);

    char *str = cJSON_PrintUnformatted(root);
    if (str == nullptr) {
        cJSON_Delete(root);
        return "";
    }

    std::string ret(str);
    cJSON_free(str);
    cJSON_Delete(root);
    return ret;
}

// 定义一个模板函数
template <typename T>
void UpdateAntennaSwitchStatus(T *antGroupIdx, T *configStatus, uint32_t length,
    utility::SafeVector<BtChrAntSwitchInfo> &vec) {
    if (length > CHR_FUSION_SIGHUB_ANT_MAX_NUM) {
        HILOGE("antSwicthState length err : %{public}d, more than 3", length);
        return;
    }

    vec.Clear();
    for (uint32_t i = 0; i < length; i++) {
        BtChrAntSwitchInfo antInfo = {static_cast<uint16_t>(antGroupIdx[i]), static_cast<uint16_t>(configStatus[i])};
        vec.Push(antInfo);
    }
}

void BtChrUpdateAntennaSwitchStatus(uint8_t *antGroupIdx, uint8_t *configStatus, uint32_t length) {
    UpdateAntennaSwitchStatus(antGroupIdx, configStatus, length, g_btChrAntSwitchStateVec);
}

void BtChrUpdateAudioAntennaSwitchStatus(uint16_t *antGroupIdx, uint16_t *configStatus, uint32_t length) {
    UpdateAntennaSwitchStatus(antGroupIdx, configStatus, length, g_btChrAudioAntSwitchStateVec);
}
}  // namespace bluetooth
}  // namespace OHOS
