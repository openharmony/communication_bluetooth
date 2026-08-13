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
#define LOG_TAG "bt_chr_base"
#endif

#include <string>
#include <set>
#include "bluetooth_log.h"
#include "bt_chr_base.h"
#include "bt_chr_subevent_manager.h"
#include "bt_chr_business_event.h"
#include "bt_chr_util.h"
#include "bt_chr_hci_process_data.h"
#include "hisysevent_c.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "safe_map.h"
#include "bt_chr_car_key.h"

using namespace std;
namespace OHOS {
namespace bluetooth {
// sub-event define
#define SUBEVENT_BTCOMMON 1
#define SUBEVENT_CONNECTEDDEVICE 2
#define SUBEVENT_AUTHRESULTINFO 4
#define SUBEVENT_BTCONNSCENE 8
#define SUBEVENT_BTCHIPINFO 16

struct BtChrEventComData {
    std::string writeParamName;
    HiSysEventEventType eventType;
    int needSubEvent;
};

// event data map, key：eventName, value：BtChrEventComData
std::map<std::string, BtChrEventComData> g_chrEventComDataMap = {
    { CHR_USER_DISCONNECT, { "DISCONNECTREASON", HISYSEVENT_FAULT,
        SUBEVENT_BTCOMMON | SUBEVENT_CONNECTEDDEVICE | SUBEVENT_AUTHRESULTINFO |
        SUBEVENT_BTCONNSCENE | SUBEVENT_BTCHIPINFO } },
    { CHR_BT_EXCEPTION_NEW, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_BT_PAIR_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_BLE_DISCONNECT, { "DISCONNECTREASON", HISYSEVENT_FAULT,
        SUBEVENT_BTCOMMON | SUBEVENT_AUTHRESULTINFO | SUBEVENT_BTCONNSCENE} },
    { CHR_A2DP_CONNECT_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_A2DP_AUDIO_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON|SUBEVENT_BTCHIPINFO } },
    { CHR_HFP_RFCOMM_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_AVRCP_CONNECT_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_HFP_AUDIO_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_SNIFF_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_HID_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_BT_SOCKET_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_BT_WATCH_CONNECT, { "DISCONNECTREASON", HISYSEVENT_FAULT,
        SUBEVENT_BTCOMMON | SUBEVENT_AUTHRESULTINFO | SUBEVENT_BTCONNSCENE } },
    { CHR_BT_WATCH_REMOVE_PAIR, { "REMOVEPAIRRESULT", HISYSEVENT_FAULT, SUBEVENT_BTCOMMON } },
    { CHR_BT_WATCH_SPP_CONNECT, { "GTSOCKETSTATUS", HISYSEVENT_FAULT, SUBEVENT_BTCOMMON} },
    { CHR_BT_WATCH_SPP_DISCONNECT, { "DISCONNECTREASON", HISYSEVENT_FAULT, SUBEVENT_BTCOMMON} },
    { CHR_HDAP_AUDIO_EXCEPTION, { CHR_ERRCODE, HISYSEVENT_FAULT, SUBEVENT_BTCOMMON} },
    { CHR_UNION_HISI_COMMON_INFO, { "CONNHANDLE", HISYSEVENT_FAULT, SUBEVENT_BTCOMMON} },
    { CHR_BLE_ADV_RESOURCE_REACHING_MAX, { "REACHING_MAX_PACKAGE_ARRAY", HISYSEVENT_STATISTIC, 0 } },
};

// key: {eventName:macAddr}, value：event parameters
SafeMap<std::string, std::vector<BtChrEventParam>> g_btChrEventMap;

// need DeviceType DFT Event^M
static std::set<std::string> g_needDeviceTypeEventSet = {
    CHR_USER_DISCONNECT,
};

bool isNeedRelease(const std::string& key, std::vector<BtChrEventParam> cacheParams)
{
    auto pos = key.find(CHR_BT_PAIR_EXCEPTION);
    if (pos == std::string::npos) {
        return true;
    }
    // find CHR_BT_PAIR_EXCEPTION, and errcode=2, not release
    for (BtChrEventParam param : cacheParams) {
        if (param.name == CHR_ERRCODE && param.i32Value == ERRCODE_PAIR_SUCCESS) {
            return false;
        }
    }
    return true;
}

void BtChrEventRelease(const std::string& key, std::vector<BtChrEventParam> cacheParams)
{
    if (!isNeedRelease(key, cacheParams)) {
        return;
    }
    HILOGD("release key: %{public}s", GET_ENCRYPT_EVENTKEY(key));
    g_btChrEventMap.Erase(key);
}

void BtChrAddEventParam(std::vector<BtChrEventParam>& cacheParams, BtChrEventParam param)
{
    auto it = find(cacheParams.begin(), cacheParams.end(), param);
    if (it != cacheParams.end()) {
        cacheParams.erase(it);
    }
    cacheParams.push_back(param);
}

void BtChrSubEventWrite(const std::string& subEventName, const std::string& subEventJsonStr,
    std::vector<BtChrEventParam>& cacheParams, const std::string& eventKey)
{
    size_t size = subEventJsonStr.size() + 1;
    BtChrAddEventParam(cacheParams, BtChrEventParam(subEventName, subEventJsonStr));
    HILOGD("eventKey: %{public}s write %{public}s, size: %{public}zu, jsonStrSize: %{public}zu, jsonStr: %{public}s",
        GET_ENCRYPT_EVENTKEY(eventKey), subEventName.c_str(), cacheParams.size(), size, subEventJsonStr.c_str());
}

void BtChrFillSubEvent(BtChrEventComData commonData, const std::string& peerAddr,
    std::vector<BtChrEventParam>& cacheParams, const std::string& eventKey)
{
    int needSubEvent = commonData.needSubEvent;

    if (needSubEvent & SUBEVENT_BTCOMMON) {
        std::string btCommonJonStr = BtChrGetBtCommonJsonStr(peerAddr);
        BtChrSubEventWrite("BTCOMMON", btCommonJonStr, cacheParams, eventKey);
    }
    if (needSubEvent & SUBEVENT_CONNECTEDDEVICE) {
        std::string connectedDeviceJonStr = BtChrGetConnectedDeviceJsonStr();
        BtChrSubEventWrite("CONNECTEDDEVICEINFO", connectedDeviceJonStr, cacheParams, eventKey);
    }
    if (needSubEvent & SUBEVENT_AUTHRESULTINFO) {
        std::string authResultJonStr = BtChrGetAuthResultJsonStr(peerAddr);
        BtChrSubEventWrite("AUTHRESULTINFO", authResultJonStr, cacheParams, eventKey);
    }
    if (needSubEvent & SUBEVENT_BTCONNSCENE) {
        std::string btConnSceneJonStr = BtChrGetConnSceneJsonStr(peerAddr);
        BtChrSubEventWrite("BTCONNSCENE", btConnSceneJonStr, cacheParams, eventKey);
    }
    if (needSubEvent & SUBEVENT_BTCHIPINFO) {
        std::string btChipInfoJonStr = BtChrGetChipInfoJsonStr(peerAddr, eventKey);
        BtChrSubEventWrite("BTCHIPINFO", btChipInfoJonStr, cacheParams, eventKey);
    }
}

static void convertToHiSysEventParams(const std::vector<BtChrEventParam>& params,
    std::vector<HiSysEventParam>& hiSysEventParams, std::vector<std::vector<char*>>& strArrayStorage)
{
    for (auto iter = params.begin(); iter != params.end(); iter++) {
        BtChrEventParamType type = iter->type;
        HiSysEventParam param;
        errno_t ret = strncpy_s(param.name, sizeof(param.name), iter->name.c_str(), iter->name.size());
        if (ret != EOK) {
            HILOGE("strncpy_s error");
            continue;
        }
        if (type == BTCHREVENT_INT32) {
            param.t = HISYSEVENT_INT32;
            param.v.i32 = iter->i32Value;
        } else if (type == BTCHREVENT_UINT32) {
            param.t = HISYSEVENT_UINT32;
            param.v.ui32 = iter->ui32Value;
        } else if (type == BTCHREVENT_STRING) {
            param.t = HISYSEVENT_STRING;
            param.v.s = const_cast<char*>(iter->strValue.data());
        } else if (type == BTCHREVENT_INT64_ARRAY) {
            param.t = HISYSEVENT_INT64_ARRAY;
            param.v.array = const_cast<int64_t*>(iter->i64ArrayValue.data());
            param.arraySize = iter->i64ArrayValue.size();
        } else if (type == BTCHREVENT_STRING_ARRAY) {
            param.t = HISYSEVENT_STRING_ARRAY;
            strArrayStorage.emplace_back();
            std::vector<char*>& ptrs = strArrayStorage.back();
            ptrs.reserve(iter->strArrayValue.size());
            for (const auto &str : iter->strArrayValue) {
                ptrs.push_back(const_cast<char*>(str.data())); // 此处取用str.data()为局部变量的指针，不涉及多线程。可安全使用
            }
            param.arraySize = iter->strArrayValue.size();
        } else {
            HILOGE("not support param type, paramName: %{public}s, type: %{public}d", iter->name.c_str(), type);
            continue;
        }
        hiSysEventParams.push_back(param);
    }
}

void BtChrHiSysEventWrite(const std::string& eventName, const std::string& eventKey,
    const std::vector<BtChrEventParam>& cacheParams, HiSysEventEventType eventType)
{
    HILOGD("eventKey: %{public}s, cacheParams size: %{public}zu", GET_ENCRYPT_EVENTKEY(eventKey), cacheParams.size());
    std::vector<HiSysEventParam> hiSysEventParams;
    std::vector<std::vector<char*>> strArrayStorage;
    convertToHiSysEventParams(cacheParams, hiSysEventParams, strArrayStorage);
    size_t strArrayIdx = 0;
    for (auto& hp : hiSysEventParams) {
        if (hp.t == HISYSEVENT_STRING_ARRAY) {
            hp.v.array = strArrayStorage[strArrayIdx].data();
            strArrayIdx++;
        }
    }
    if (hiSysEventParams.size() == 0) {
        return;
    }
    HiSysEventParam eventParamArr[hiSysEventParams.size()];
    std::copy(hiSysEventParams.begin(), hiSysEventParams.end(), eventParamArr);
    OH_HiSysEvent_Write("BT_SERVICE", eventName.c_str(), eventType, eventParamArr, hiSysEventParams.size());
}

void BtChrEventWriteParams(const std::string& eventName, const std::string& peerAddr,
    std::vector<BtChrEventParam> params)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    std::string eventKey(eventName);
    eventKey.append(EVENT_KEY_SPLICE_MARK).append(addr);
    
    std::vector<BtChrEventParam> cacheParams;
    if (g_btChrEventMap.Find(eventKey, cacheParams)) {
        for (BtChrEventParam param : params) {
            BtChrAddEventParam(cacheParams, param);
            HILOGD("eventKey: %{public}s, cacheParams size: %{public}zu, eventName: %{public}s",
                GET_ENCRYPT_EVENTKEY(eventKey), cacheParams.size(), param.name.c_str());
        }
        g_btChrEventMap.EnsureInsert(eventKey, cacheParams);
    } else {
        HILOGI("Insert eventKey: %{public}s", GET_ENCRYPT_EVENTKEY(eventKey));
        g_btChrEventMap.EnsureInsert(eventKey, params);
    }
}

void BtChrFillHciCode(const std::string& peerAddr, std::vector<BtChrEventParam>& cacheParams)
{
    const string hciCode = BtChrHciProcessData::GetInstance().GetImportantHciProcessData(peerAddr);
    BtChrAddEventParam(cacheParams, BtChrEventParam("HCICODE", hciCode));
}

void BtChrFillDeviceType(const std::string& eventName, const std::string& peerAddr,
    std::vector<BtChrEventParam>& cacheParams)
{
    if (g_needDeviceTypeEventSet.find(eventName) == g_needDeviceTypeEventSet.end()) {
        return;
    }

    BtChrDeviceInfo deviceInfo;
    if (!GetDeviceInfoFromMap(peerAddr, deviceInfo)) {
        HILOGE("device not exist at chr deviceInfo map");
        return;
    }
    int deviceClass = deviceInfo.btDeviceClass;
    HILOGI("device class is %{public}d", deviceClass);
    BtChrAddEventParam(cacheParams, BtChrEventParam("DEVICETYPE", deviceClass));
}

void BtChrEventWrite(const std::string& eventName, const std::string& peerAddr, BtChrEventParam param)
{
    std::string addr = GetLowercaseAddr(peerAddr);
    if (CarKey::IsWalletDftEvent(eventName, param)) {
        std::vector<BtChrEventParam> params;
        params.push_back(BtChrEventParam("DEV_ADDRESS", GET_ENCRYPT_ADDR(addr)));
        params.push_back(param);
        CarKey::RecordWalletChr(eventName, params);
    }
    std::string eventKey(eventName);
    eventKey.append(EVENT_KEY_SPLICE_MARK).append(addr);
    
    auto dataIter = g_chrEventComDataMap.find(eventName);
    if (dataIter == g_chrEventComDataMap.end()) {
        HILOGE("commonData is null, eventKey: %{public}s", GET_ENCRYPT_EVENTKEY(eventKey));
        return;
    }
    BtChrEventComData commonData = dataIter->second;
    bool isWriteHiSysEvent = (commonData.writeParamName == param.name);
    std::vector<BtChrEventParam> cacheParams;
    if (g_btChrEventMap.Find(eventKey, cacheParams)) {
        BtChrAddEventParam(cacheParams, param);
        HILOGD("eventKey: %{public}s, cacheParams size: %{public}zu, eventName: %{public}s",
            GET_ENCRYPT_EVENTKEY(eventKey), cacheParams.size(), param.name.c_str());
        g_btChrEventMap.EnsureInsert(eventKey, cacheParams);
        if (!isWriteHiSysEvent) {
            return;
        }
        BtChrFillDeviceType(eventName, addr, cacheParams);
        BtChrFillHciCode(addr, cacheParams);
        BtChrFillSubEvent(commonData, addr, cacheParams, eventKey);
        BtChrHiSysEventWrite(eventName, eventKey, cacheParams, commonData.eventType);
        BtChrEventRelease(eventKey, cacheParams);
    } else {
        cacheParams.push_back(param);
        HILOGD("write first param, eventKey: %{public}s, eventName: %{public}s, isWriteHiSysEvent: %{public}d",
            GET_ENCRYPT_EVENTKEY(eventKey), param.name.c_str(), isWriteHiSysEvent);
        if (isWriteHiSysEvent) {
            BtChrFillDeviceType(eventName, addr, cacheParams);
            BtChrFillHciCode(addr, cacheParams);
            BtChrHiSysEventWrite(eventName, eventKey, cacheParams, commonData.eventType);
        } else {
            g_btChrEventMap.Insert(eventKey, cacheParams);
        }
    }
}

void BtChrEventWriteInt(const std::string& eventName, const std::string& peerAddr, const std::string& paramName,
    int32_t paramValue)
{
    if (!BtChrBusinessEventHandler(eventName, peerAddr, paramName, paramValue)) {
        return;
    }
    BtChrEventWrite(eventName, peerAddr, BtChrEventParam(paramName, paramValue));
}

void BtChrEventWriteStr(const std::string& eventName, const std::string& peerAddr, const std::string& paramName,
    const std::string& paramValue)
{
    if (!BtChrBusinessEventHandler(eventName, peerAddr, paramName, paramValue)) {
        return;
    }
    BtChrEventWrite(eventName, peerAddr, BtChrEventParam(paramName, paramValue));
}

void BtChrEventWriteTime(const std::string& eventName, const std::string& peerAddr, const std::string& paramName)
{
    std::string currentTime = (eventName == CHR_USER_DISCONNECT) ? GetCurrentTimeMs() : GetCurrentTime();
    BtChrEventWriteStr(eventName, peerAddr, paramName, currentTime);
}

void BtChrBaseReleaseAll(const std::vector<std::string> &notClearAddrs)
{
    BtChrReleaseMap(g_btChrEventMap, notClearAddrs, BT_CHR_EVENT_MAP);
}

void BtChrUeEventWrite(const std::string& eventName, std::vector<BtChrEventParam>& params)
{
    HITRACE_METER(BT_TRACE_TAG);
    HILOGD("eventName: %{public}s, params size: %{public}zu", eventName.c_str(), params.size());
    CarKey::RecordWalletChr(eventName, params);
    // Ue event common Parameters
    params.push_back(BtChrEventParam("PNAMEID", "Bluetooth"));
    params.push_back(BtChrEventParam("PVERSIONID", "1.0"));
    std::vector<HiSysEventParam> hiSysEventParams;
    std::vector<std::vector<char*>> strArrayStorage;
    convertToHiSysEventParams(params, hiSysEventParams, strArrayStorage);
    size_t strArrayIdx = 0;
    for (auto& hp : hiSysEventParams) {
        if (hp.t == HISYSEVENT_STRING_ARRAY) {
            hp.v.array = strArrayStorage[strArrayIdx].data();
            strArrayIdx++;
        }
    }
    if (hiSysEventParams.size() == 0) {
        return;
    }
    HiSysEventParam eventParamArr[hiSysEventParams.size()];
    std::copy(hiSysEventParams.begin(), hiSysEventParams.end(), eventParamArr);
    OH_HiSysEvent_Write(
        "BLUETOOTH_UE", eventName.c_str(), HISYSEVENT_BEHAVIOR, eventParamArr, hiSysEventParams.size());
}

void BtChrStaticticsEventWrite(uint8_t eventType, const std::string& eventValue)
{
    HITRACE_METER(BT_TRACE_TAG);
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, CHR_BT_STATISTICS_ATOMIC_EVENT,
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "EVENT_TYPE", eventType, "EVENT_VALUE", eventValue);
}
 
void BtChrResetEventWrite(uint8_t eventType, const std::string& eventValue)
{
    HITRACE_METER(BT_TRACE_TAG);
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, CHR_BT_SWITCH_ATOMIC_EVENT,
        HiviewDFX::HiSysEvent::EventType::FAULT, "EVENT_TYPE", eventType, "EVENT_VALUE", eventValue);
}

}  // namespace bluetooth
}  // namespace OHOS