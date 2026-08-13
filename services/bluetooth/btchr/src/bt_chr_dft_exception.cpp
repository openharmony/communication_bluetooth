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
#define LOG_TAG "bt_chr_dft_exception"
#endif

#include <set>
#include <functional>
#include "bt_chr_dft_exception.h"
#include "bt_chr_base.h"
#include "bt_chr_util.h"
#include "bluetooth_log.h"
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {
struct SocketCallingRecord {
    std::string packageName;
    std::string connectTime;

    SocketCallingRecord(std::string name, std::string time)
    {
        packageName = name;
        connectTime = time;
    }
};

// Write when audio volume is 0
static std::mutex g_audioVolume0SetMutex {};
static std::set<std::string> g_audioVolume0Set;

// key: {uuid:macAddr}, value：SocketCallingRecord
SafeMap<std::string, SocketCallingRecord> g_socketCallingRecordMap;

#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
static void BtChrExcpEventComWrite(const std::string& eventName, const std::string& peerAddr, int32_t errCode,
    int32_t subErrCode)
{
    BtChrEventWriteInt(eventName, peerAddr, CHR_SUB_ERRCODE, subErrCode);
    BtChrEventWriteInt(eventName, peerAddr, CHR_ERRCODE, errCode);
}

void BtChrNotifyA2dpStatusChange(const std::string& peerAddr, int32_t paramValue)
{
    BtChrDftEventWriteInt(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, "A2DPSTATUS", paramValue);
    BtChrUpdateA2dpStatus(peerAddr, paramValue);
}

void BtChrNotifyHfpStatusChange(const std::string& peerAddr, int32_t paramValue)
{
    BtChrDftEventWriteInt(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, "HFPSTATUS", paramValue);
    BtChrUpdateHfpStatus(peerAddr, paramValue);
}

static void BtChrIntDataSpecialHandle(
    const std::string &eventName, const std::string &peerAddr, const std::string &paramName, int32_t paramValue)
{
    if (eventName != CHR_USER_DISCONNECT) {
        return;
    }

    const static std::unordered_map<std::string,
        std::function<void(const std::string& peerAddr, int32_t paramValue)>> paramHandlerMap = {
        {"LINKTYPE", BtChrUpdateLinkType},
        {"A2DPSTATUS", BtChrNotifyA2dpStatusChange},
        {"HFPSTATUS", BtChrNotifyHfpStatusChange},
        {"HIDSTATUS", BtChrUpdateHidStatus},
        {"NEEDA2DP", BtChrUpdateSupportA2dpStatus},
        {"NEEDHFP", BtChrUpdateSupportHfpStatus},
        {"NEEDHID", BtChrUpdateSupportHidStatus},
        {"DISCONNECTREASON", BtChrDelConnectedDevice}
    };

    auto it = paramHandlerMap.find(paramName);
    if (it != paramHandlerMap.end()) {
        it->second(peerAddr, paramValue);
    } else {
        HILOGD("not need handle");
    }
}

static void BtChrTimeDataSpecialHandle(const std::string& eventName, const std::string& peerAddr,
    const std::string& paramName)
{
    if (eventName == CHR_USER_DISCONNECT) {
        if (paramName == "ACLCONNECTEDTIME") {
            BtChrDftEventWriteInt(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, "ACLSTATUS", CHR_ACL_CONNECTED);
            BtChrAddConnectedDevice(peerAddr);
        } else if (paramName == "ACLCONNECTIONTIME") {
            BtChrDftEventWriteInt(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, "ACLSTATUS", CHR_ACL_CONNECTION);
        } else if (paramName == "ACLDISCONNECTTIME") {
            BtChrDftEventWriteInt(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, "ACLSTATUS", CHR_ACL_DISCONNECTED);
        } else {
            HILOGD("not need handle");
        }
    }
}

static void BtChrDftReleaseAll(const std::vector<std::string> &notClearAddrs)
{
    BtChrReleaseMap(g_socketCallingRecordMap, notClearAddrs, BT_CHR_SOCKET_RECORD_MAP);
}
#endif

void BtChrDftEventWriteInt(const std::string& eventName, const std::string& peerAddr, const std::string& paramName,
    int32_t paramValue)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    BtChrIntDataSpecialHandle(eventName, peerAddr, paramName, paramValue);
    BtChrEventWriteInt(eventName, peerAddr, paramName, paramValue);
#endif
}

void BtChrDftEventWriteTime(const std::string& eventName, const std::string& peerAddr, const std::string& paramName)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    BtChrTimeDataSpecialHandle(eventName, peerAddr, paramName);
    BtChrEventWriteTime(eventName, peerAddr, paramName);
#endif
}

void BtChrBtExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    BtChrExcpEventComWrite(CHR_BT_EXCEPTION_NEW, peerAddr, errCode, subErrCode);
#endif
}

void BtChrRemovePairHandle(const std::string& peerAddr, int32_t bondedCnt)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    HILOGI("peerAddr: %{public}s, bondedCnt: %{public}d", GET_ENCRYPT_ADDR(peerAddr), bondedCnt);
    BtChrEventWriteInt(CHR_BT_PAIR_EXCEPTION, peerAddr, "BONDEDCNT", bondedCnt);
    BtChrEventWriteInt(CHR_BT_PAIR_EXCEPTION, peerAddr, CHR_ERRCODE, ERRCODE_REMOVE_PAIR);
#endif
}

void BtChrRemoveAllPairHandle(std::vector<std::string> removeAddrs)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    for (std::string addr : removeAddrs) {
        BtChrRemovePairHandle(addr, 0);
    }
#endif
}

void BtChrA2dpAudioExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    BtChrExcpEventComWrite(CHR_A2DP_AUDIO_EXCEPTION, peerAddr, errCode, subErrCode);
#endif
}

void BtChrSetAudioVolume(const std::string& peerAddr, int32_t audioVolume)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    std::lock_guard<std::mutex> lock(g_audioVolume0SetMutex);
    std::string addr = GetLowercaseAddr(peerAddr);
    if (audioVolume == 0) {
        g_audioVolume0Set.insert(addr);
        return;
    }
    if (audioVolume > 0) {
        g_audioVolume0Set.erase(addr);
    }
#endif
}

void BtChrAudioStateStart(const std::string& peerAddr)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    std::string addr = GetLowercaseAddr(peerAddr);
    {
        std::lock_guard<std::mutex> lock(g_audioVolume0SetMutex);
        auto iter = g_audioVolume0Set.find(addr);
        if (iter == g_audioVolume0Set.end()) {
            return;
        }
    }
    BtChrA2dpAudioExcpEvent(addr, CHR_A2DP_ERR_TYPE_VOLUME_0, 0);
#endif
}

void BtChrAvrcpConnectExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    BtChrExcpEventComWrite(CHR_AVRCP_CONNECT_EXCEPTION, peerAddr, errCode, subErrCode);
#endif
}

void BtChrCallExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode, int32_t curCodec)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    BtChrEventWriteInt(CHR_HFP_AUDIO_EXCEPTION, peerAddr, "CURRCODEC", curCodec);
    BtChrExcpEventComWrite(CHR_HFP_AUDIO_EXCEPTION, peerAddr, errCode, subErrCode);
#endif
}

void BtChrHidExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    BtChrExcpEventComWrite(CHR_HID_EXCEPTION, peerAddr, errCode, subErrCode);
#endif
}

void BtChrAddSocketCallingRecord(const std::string& peerAddr, const std::string& uuid, const std::string& packageName)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    std::string addr = GetLowercaseAddr(peerAddr);
    std::string recordKey(uuid);
    recordKey.append(EVENT_KEY_SPLICE_MARK).append(addr);
    std::string name = (packageName.size() > 0) ? packageName : "unknow";
    std::string currentTime = GetCurrentTime();

    g_socketCallingRecordMap.EnsureInsert(recordKey, SocketCallingRecord(name, currentTime));

    // write CHR_BT_SOCKET_EXCEPTION
    BtChrEventWriteStr(CHR_BT_SOCKET_EXCEPTION, peerAddr, "CONAPPUID", uuid);
    BtChrEventWriteStr(CHR_BT_SOCKET_EXCEPTION, peerAddr, "CONAPP", name);
    BtChrEventWriteStr(CHR_BT_SOCKET_EXCEPTION, peerAddr, "STARTCONNECTTIME", currentTime);
    BtChrEventWriteInt(CHR_BT_SOCKET_EXCEPTION, peerAddr, CHR_ERRCODE, CLIENT_CONNECT_START);
#endif
}

std::string BtChrSocketExcpEvent(const std::string& peerAddr, const std::string& uuid, int32_t errCode,
    int32_t subErrCode)
{
    std::string packageName = "unknow";
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    std::string addr = GetLowercaseAddr(peerAddr);
    std::string recordKey(uuid);
    recordKey.append(EVENT_KEY_SPLICE_MARK).append(addr);
    
    std::string connectTime = "";
    SocketCallingRecord record("", "");
    if (g_socketCallingRecordMap.Find(recordKey, record)) {
        packageName = record.packageName;
        connectTime = record.connectTime;
    } else {
        HILOGI("not found recordKey: %{public}s, packageName: %{public}s", GET_ENCRYPT_EVENTKEY(recordKey),
            packageName.c_str());
    }

    // write CHR_BT_SOCKET_EXCEPTION
    BtChrEventWriteStr(CHR_BT_SOCKET_EXCEPTION, peerAddr, "CONAPPUID", uuid);
    BtChrEventWriteStr(CHR_BT_SOCKET_EXCEPTION, peerAddr, "CONAPP", packageName);
    BtChrEventWriteStr(CHR_BT_SOCKET_EXCEPTION, peerAddr, "STARTCONNECTTIME", connectTime);
    BtChrEventWriteInt(CHR_BT_SOCKET_EXCEPTION, peerAddr, CHR_SUB_ERRCODE, subErrCode);
    BtChrEventWriteInt(CHR_BT_SOCKET_EXCEPTION, peerAddr, CHR_ERRCODE, errCode);
    if ((errCode == CLIENT_CONNECT_SUCCESS) || errCode == CLIENT_CONNECT_PENDING) {
        return packageName;
    }
    g_socketCallingRecordMap.Erase(recordKey);
#endif
    return packageName;
}

void BtChrSocketExcpEvent(const std::string& peerAddr, const std::string& uuid, int32_t errCode,
    int32_t subErrCode, std::string packageName)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE

    // write CHR_BT_SOCKET_EXCEPTION
    BtChrEventWriteStr(CHR_BT_SOCKET_EXCEPTION, peerAddr, "CONAPPUID", uuid);
    BtChrEventWriteStr(CHR_BT_SOCKET_EXCEPTION, peerAddr, "CONAPP", packageName);
    BtChrEventWriteTime(CHR_BT_SOCKET_EXCEPTION, peerAddr, "STARTCONNECTTIME");
    BtChrEventWriteInt(CHR_BT_SOCKET_EXCEPTION, peerAddr, CHR_SUB_ERRCODE, subErrCode);
    BtChrEventWriteInt(CHR_BT_SOCKET_EXCEPTION, peerAddr, CHR_ERRCODE, errCode);
#endif
}

void BtChrBleConnData(const std::string &peerAddr, int type, const std::string &callingName)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    std::string name = (callingName.size() > 0) ? callingName : "unknow";
    if (type == CHR_BLE_COC) {
        BtChrDftEventWriteInt(CHR_BLE_DISCONNECT, peerAddr, "BLEPROFILE", 1);
    } else if (type == CHR_BLE_GATT) {
        BtChrDftEventWriteInt(CHR_BLE_DISCONNECT, peerAddr, "BLEPROFILE", 0);
    } else {
        LOG_INFO("unknow type, peerAddr: %d", type);
        return;
    }
    BtChrEventWriteStr(CHR_BLE_DISCONNECT, peerAddr, "PKGNAME", name);
    BtChrEventWriteTime(CHR_BLE_DISCONNECT, peerAddr, "BLEPROFILECONNECTIONTIME");
#endif
}

void BtChrReleaseByAddrs(const std::vector<std::string> &notClearAddrs)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    // release
    BtChrBaseReleaseAll(notClearAddrs);
    BtChrSubEventReleaseAll(notClearAddrs);
    BtChrDftReleaseAll(notClearAddrs);
#endif
}

void BtChrResetExcpEvent(const std::string& eventValue)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    // reset
    BtChrResetEventWrite(EVENT_TYPE_BT_RESET, eventValue);
#endif
}

void BtChrHdapAudioExcpEvent(const std::string& peerAddr, int32_t errCode, int32_t subErrCode)
{
#ifdef BLUETOOTH_CHR_FEATURE_ENABLE
    BtChrExcpEventComWrite(CHR_HDAP_AUDIO_EXCEPTION, peerAddr, errCode, subErrCode);
#endif
}
}  // namespace bluetooth
}  // namespace OHOS