/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_chr_business_event"
#endif

#include "bt_chr_business_event.h"
#include "bt_chr_util.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace bluetooth {
// key: macAddr, value：business info
SafeMap<std::string, BtChrBusinessEvent> g_btChrBusinessEventMap;
#define IF_EQUAL_ASSIGN_VALUE(paramName, name, ret, value) if ((paramName) == (name)) { \
    ret = value; \
}

BtChrBusinessEvent &BtChrBusinessEvent::GetInstance()
{
    static BtChrBusinessEvent instance_;
    return instance_;
}

BtChrBusinessEvent::BtChrBusinessEvent()
{}

void BtChrBusinessEvent::SetOffloadFlag(int offloadFlag)
{
    offloadFlag_ = offloadFlag;
}

void BtChrBusinessEvent::SetVolumeParams(int volumeType, int volumeValue)
{
    volumeType_ = volumeType;
    volumeValue_ = volumeValue;
}

void BtChrBusinessEvent::SetAbsolutVolumeParams(int isAbsolutVolume, int absolutVolumeStatus)
{
    isAbsolutVolume_ = isAbsolutVolume;
    absolutVolumeStatus_ = absolutVolumeStatus;
}

void BtChrBusinessEvent::SetAudioDeviceParams(std::string audioDevicePkg, int activeDeviceResult)
{
    audioDevicePkg_ = audioDevicePkg;
    activeDeviceResult_ = activeDeviceResult;
}

void BtChrBusinessEvent::SetAudioStartTime(std::string audioStartTime)
{
    audioStartTime_ = audioStartTime;
}

int BtChrBusinessEvent::GetOffloadFlag()
{
    return offloadFlag_;
}

int BtChrBusinessEvent::GetVolumeType()
{
    return volumeType_;
}

int BtChrBusinessEvent::GetVolumeValue()
{
    return volumeValue_;
}

int BtChrBusinessEvent::IsAbsolutVolume()
{
    return isAbsolutVolume_;
}

int BtChrBusinessEvent::GetAbsolutVolumeStatus()
{
    return absolutVolumeStatus_;
}

std::string BtChrBusinessEvent::GetAudioDevicePkg()
{
    return audioDevicePkg_;
}

int BtChrBusinessEvent::GetActiveDeviceResult()
{
    return activeDeviceResult_;
}

std::string BtChrBusinessEvent::GetAudioStartTime()
{
    return audioStartTime_;
}

void BtChrBusinessEvent::SavePkgName(const std::string &pkgNames)
{
    savedPkgNames_ = pkgNames;
}
 
std::string BtChrBusinessEvent::GetSavedPkgName()
{
    return savedPkgNames_;
}

bool BtChrBusinessEventHandler(const std::string& eventName, const std::string& peerAddr,
    const std::string& paramName, const int32_t paramValue)
{
    if (peerAddr.empty()) {
        HILOGE("invalid device, eventName: %{public}s", eventName.c_str());
        return true;
    }

    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrBusinessEvent btChrBusinessEvent;
    if (g_btChrBusinessEventMap.Find(addr, btChrBusinessEvent)) {
        HILOGD("find event, peerAddr: %{public}s", GetEncryptAddr(addr).c_str());
    }

    HILOGD("sava param, paramName: %{public}s, paramValue: %{public}d", paramName.c_str(), paramValue);
    if (eventName == CHR_A2DP_CONNECT_EXCEPTION) {
        IF_EQUAL_ASSIGN_VALUE(paramName, "ERRCODE", btChrBusinessEvent.a2dpErrorReason, paramValue);
        return true;
    } else if (eventName == CHR_HID_EXCEPTION) {
        IF_EQUAL_ASSIGN_VALUE(paramName, "ERRCODE", btChrBusinessEvent.hidErrorReason, paramValue);
        return true;
    } else if (eventName == CHR_HFP_RFCOMM_EXCEPTION) {
        IF_EQUAL_ASSIGN_VALUE(paramName, "ERRCODE", btChrBusinessEvent.hfpErrorReason, paramValue);
        return true;
    }

    BtChrAssignIntParam(paramName, paramValue, &btChrBusinessEvent);
    g_btChrBusinessEventMap.EnsureInsert(addr, btChrBusinessEvent);
    return BtChrBusinessFillAllParam(addr, eventName, paramName);
}

bool BtChrBusinessEventHandler(const std::string& eventName, const std::string& peerAddr,
    const std::string& paramName, const std::string& paramValue)
{
    if (peerAddr.empty()) {
        HILOGE("invalid device, eventName: %{public}s", eventName.c_str());
        return true;
    }

    std::string addr = GetLowercaseAddr(peerAddr);
    BtChrBusinessEvent btChrBusinessEvent;
    if (g_btChrBusinessEventMap.Find(addr, btChrBusinessEvent)) {
        HILOGD("find event, peerAddr: %{public}s", GetEncryptAddr(addr).c_str());
    }

    HILOGD("sava param, paramName: %{public}s, paramValue: %{public}s", paramName.c_str(), paramValue.c_str());
    BtChrAssignStringParam(paramName, paramValue, &btChrBusinessEvent);
    g_btChrBusinessEventMap.EnsureInsert(addr, btChrBusinessEvent);
    return BtChrBusinessFillAllParam(addr, eventName, paramName);
}

void BtChrAssignIntParam(const std::string& paramName, const int32_t paramValue,
    BtChrBusinessEvent *btChrBusinessEvent)
{
    IF_EQUAL_ASSIGN_VALUE(paramName, "DISCONNECTREASON", btChrBusinessEvent->disconnectReason, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "DISCONNECTRSSI", btChrBusinessEvent->disconnectRssi, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "BTCONNECTSCENE", btChrBusinessEvent->btConnectScene, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "PAIRSTATUS", btChrBusinessEvent->btPairStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "ISPASSIVE", btChrBusinessEvent->isPassive, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "ENCRYPTIONSTATUS", btChrBusinessEvent->encryptionStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "BTCONMTUINITIATOR", btChrBusinessEvent->btMtuInitiator, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "BTCONMTURESULT", btChrBusinessEvent->btMtuResult, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "NEEDHFP", btChrBusinessEvent->needHfp, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HFPSTATUS", btChrBusinessEvent->hfpStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HFPINITIATOR", btChrBusinessEvent->hfpInitiator, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HFPERRORREASON", btChrBusinessEvent->hfpErrorReason, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "NEEDA2DP", btChrBusinessEvent->needA2dp, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "A2DPINITIATOR", btChrBusinessEvent->a2dpInitiator, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "A2DPSTATUS", btChrBusinessEvent->a2dpStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "A2DPERRORREASON", btChrBusinessEvent->a2dpErrorReason, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HIDSTATUS", btChrBusinessEvent->hidStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HIDERRORREASON", btChrBusinessEvent->hidErrorReason, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "SDPINITIATOR", btChrBusinessEvent->sdpInitiator, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "SDPERRORREASON", btChrBusinessEvent->sdpErrorReason, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "GATTSERVICEINITIATOR", btChrBusinessEvent->gattServiceInitiator, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "SMPTYPE", btChrBusinessEvent->smpType, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "SMPSTATUS", btChrBusinessEvent->smpStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "GATTCSTATUS", btChrBusinessEvent->gattClientStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "GATTSTATUS", btChrBusinessEvent->gattServiceStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "GATTCONNECTERRORREASON", btChrBusinessEvent->gattConnectErrorReason, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "REMOVEPAIRRESULT", btChrBusinessEvent->removePairResult, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "REMOVEPAIRINITIATOR", btChrBusinessEvent->removePairInitiator, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "CONNECTSTATUS", btChrBusinessEvent->connectStatus, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "FAILERRORCODE", btChrBusinessEvent->failErrorCode, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "DELETEKEYRESULT", btChrBusinessEvent->deleteLinkkeyResult, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "DISCONNECTRESULT", btChrBusinessEvent->disconnectResult, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "DISCONNECTERRORCODE", btChrBusinessEvent->disconnectErrorCode, paramValue);
}

void BtChrAssignStringParam(const std::string& paramName, const std::string& paramValue,
    BtChrBusinessEvent *btChrBusinessEvent)
{
    IF_EQUAL_ASSIGN_VALUE(paramName, "PKGNAME", btChrBusinessEvent->pkgName, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "ACLCONNECTIONTIME", btChrBusinessEvent->aclConnectTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "ACLCONNECTEDTIME", btChrBusinessEvent->aclConnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "ACLDISCONNECTTIME", btChrBusinessEvent->aclDisconnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "ENCRYPTIONENDTIME", btChrBusinessEvent->encryptionEndTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HFPCONNECTIONTIME", btChrBusinessEvent->hfpConnectTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HFPCONNECTEDTIME", btChrBusinessEvent->hfpConnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HFPDISCONNECTTIME", btChrBusinessEvent->hfpDisconnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "A2DPCONNECTIONTIME", btChrBusinessEvent->a2dpConnectTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "A2DPCONNECTEDTIME", btChrBusinessEvent->a2dpConnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "A2DPDISCONNECTTIME", btChrBusinessEvent->a2dpDisconnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HIDCONNECTTIME", btChrBusinessEvent->hidConnectTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HIDCONNECTEDTIME", btChrBusinessEvent->hidConnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "HIDDISCONNECTEDTIME", btChrBusinessEvent->hidDisconnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "SMPCONNECTIONTIME", btChrBusinessEvent->smpConnectTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "SMPCONNECTEDTIME", btChrBusinessEvent->smpConnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "SMPDISCONNECTTIME", btChrBusinessEvent->smpDisconnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "GATTCONNECTTIME", btChrBusinessEvent->gattConnectTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "GATTCONNECTEDTIME", btChrBusinessEvent->gattConnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "GATTDISCONNECTEDTIME", btChrBusinessEvent->gattDisconnectedTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "REMOVEPAIRTIME", btChrBusinessEvent->removePairTime, paramValue);
    IF_EQUAL_ASSIGN_VALUE(paramName, "DELETEKEYTIME", btChrBusinessEvent->deleteLinkkeyTime, paramValue);
}

bool BtChrBusinessFillAllParam(const std::string& peerAddr, const std::string& eventName,
    const std::string& paramName)
{
    if ((eventName != CHR_BT_WATCH_CONNECT && eventName != CHR_BT_WATCH_REMOVE_PAIR)) {
        HILOGD("not event, eventName: %{public}s", eventName.c_str());
        return true;
    }

    bool isWatchConnectReport = BtChrFillWatchConnectParam(peerAddr, eventName, paramName);
    bool isWatchRemovePairReport = BtChrFillWatchRemovePairParam(peerAddr, eventName, paramName);

    return isWatchConnectReport || isWatchRemovePairReport;
}

bool BtChrFillWatchConnectParam(const std::string& peerAddr, const std::string& eventName,
    const std::string& paramName)
{
    if (!(eventName == CHR_BT_WATCH_CONNECT && paramName == "DISCONNECTREASON")) { // Not reported field, return false
        HILOGD("not report field, paramName: %{public}s", paramName.c_str());
        return false;
    }

    BtChrBusinessEvent btChrBusinessEvent;
    if (!g_btChrBusinessEventMap.Find(peerAddr, btChrBusinessEvent)) {
        return true;
    }

    std::vector<BtChrEventParam> params;
    if (btChrBusinessEvent.disconnectReason == 0) {
        params.push_back(BtChrEventParam("BTPAIRSTATUS", btChrBusinessEvent.btPairStatus));
        BtChrEventWriteParams(eventName, peerAddr, params);
        g_btChrBusinessEventMap.Erase(peerAddr);
        return true;
    }

    BtChrFillWatchConnectParams(peerAddr, eventName, btChrBusinessEvent);
    g_btChrBusinessEventMap.Erase(peerAddr);
    return true;
}

void BtChrFillWatchConnectParams(const std::string& peerAddr, const std::string& eventName,
    const BtChrBusinessEvent btChrBusinessEvent)
{
    std::vector<BtChrEventParam> params;
    params.push_back(BtChrEventParam("BTCONNECTSCENE", btChrBusinessEvent.btConnectScene));
    params.push_back(BtChrEventParam("ISPASSIVE", btChrBusinessEvent.isPassive));
    params.push_back(BtChrEventParam("PKGNAME", btChrBusinessEvent.pkgName));
    params.push_back(BtChrEventParam("ACLCONNECTTIME", btChrBusinessEvent.aclConnectTime));
    params.push_back(BtChrEventParam("ACLCONNECTEDTIME", btChrBusinessEvent.aclConnectedTime));
    params.push_back(BtChrEventParam("ACLDISCONNECTEDTIME", btChrBusinessEvent.aclDisconnectedTime));
    params.push_back(BtChrEventParam("BTMTUINITIATOR", btChrBusinessEvent.btMtuInitiator));
    params.push_back(BtChrEventParam("BTMTURESULT", btChrBusinessEvent.btMtuResult));
    params.push_back(BtChrEventParam("ENCRYPTIONSTATUS", btChrBusinessEvent.encryptionStatus));
    params.push_back(BtChrEventParam("ENCRYPTIONENDTIME", btChrBusinessEvent.encryptionEndTime));
    params.push_back(BtChrEventParam("SDPINITIATOR", btChrBusinessEvent.sdpInitiator));
    params.push_back(BtChrEventParam("SDPERRORREASON", btChrBusinessEvent.sdpErrorReason));
    params.push_back(BtChrEventParam("GATTSERVICEINITIATOR", btChrBusinessEvent.gattServiceInitiator));
    params.push_back(BtChrEventParam("GATTERRORREASON", btChrBusinessEvent.gattErrorReason));
    params.push_back(BtChrEventParam("A2DPINITIATOR", btChrBusinessEvent.a2dpInitiator));
    params.push_back(BtChrEventParam("A2DPSTATUS", btChrBusinessEvent.a2dpStatus));
    params.push_back(BtChrEventParam("A2DPERRORREASON", btChrBusinessEvent.a2dpErrorReason));
    params.push_back(BtChrEventParam("A2DPCONNECTTIME", btChrBusinessEvent.a2dpConnectTime));
    params.push_back(BtChrEventParam("A2DPCONNECTEDTIME", btChrBusinessEvent.a2dpConnectedTime));
    params.push_back(BtChrEventParam("A2DPDISCONNECTEDTIME", btChrBusinessEvent.a2dpDisconnectedTime));
    params.push_back(BtChrEventParam("NEEDA2DP", btChrBusinessEvent.needA2dp));
    params.push_back(BtChrEventParam("NEEDHFP", btChrBusinessEvent.needHfp));
    params.push_back(BtChrEventParam("HFPINITIATOR", btChrBusinessEvent.hfpInitiator));
    params.push_back(BtChrEventParam("HFPSTATUS", btChrBusinessEvent.hfpStatus));
    params.push_back(BtChrEventParam("HFPERRORREASON", btChrBusinessEvent.hfpErrorReason));
    params.push_back(BtChrEventParam("HFPCONNECTTIME", btChrBusinessEvent.hfpConnectTime));
    params.push_back(BtChrEventParam("HFPCONNECTEDTIME", btChrBusinessEvent.hfpConnectedTime));
    params.push_back(BtChrEventParam("HFPDISCONNECTEDTIME", btChrBusinessEvent.hfpDisconnectedTime));
    params.push_back(BtChrEventParam("HIDSTATUS", btChrBusinessEvent.hidStatus));
    params.push_back(BtChrEventParam("HIDERRORREASON", btChrBusinessEvent.hidErrorReason));
    params.push_back(BtChrEventParam("HIDCONNECTTIME", btChrBusinessEvent.hidConnectTime));
    params.push_back(BtChrEventParam("HIDCONNECTEDTIME", btChrBusinessEvent.hidConnectedTime));
    params.push_back(BtChrEventParam("HIDDISCONNECTEDTIME", btChrBusinessEvent.hidDisconnectedTime));
    params.push_back(BtChrEventParam("GATTCSTATUS", btChrBusinessEvent.gattClientStatus));
    params.push_back(BtChrEventParam("GATTSTATUS", btChrBusinessEvent.gattServiceStatus));
    params.push_back(BtChrEventParam("GATTCONNECTERRORREASON", btChrBusinessEvent.gattConnectErrorReason));
    params.push_back(BtChrEventParam("GATTCONNECTTIME", btChrBusinessEvent.gattConnectTime));
    params.push_back(BtChrEventParam("GATTCONNECTEDTIME", btChrBusinessEvent.gattConnectedTime));
    params.push_back(BtChrEventParam("GATTDISCONNECTEDTIME", btChrBusinessEvent.gattDisconnectedTime));
    params.push_back(BtChrEventParam("SMPTYPE", btChrBusinessEvent.smpType));
    params.push_back(BtChrEventParam("SMPSTATUS", btChrBusinessEvent.smpStatus));
    params.push_back(BtChrEventParam("SMPCONNECTTIME", btChrBusinessEvent.smpConnectTime));
    params.push_back(BtChrEventParam("SMPCONNECTEDTIME", btChrBusinessEvent.smpConnectedTime));
    params.push_back(BtChrEventParam("SMPDISCONNECTEDTIME", btChrBusinessEvent.smpDisconnectedTime));
    params.push_back(BtChrEventParam("BTPAIRSTATUS", btChrBusinessEvent.btPairStatus));
    params.push_back(BtChrEventParam("DISCONNECTRSSI", btChrBusinessEvent.disconnectRssi));
    BtChrEventWriteParams(eventName, peerAddr, params);
}

bool BtChrFillWatchRemovePairParam(const std::string& peerAddr, const std::string& eventName,
    const std::string& paramName)
{
    if (!(eventName == CHR_BT_WATCH_REMOVE_PAIR && paramName == "REMOVEPAIRRESULT")) {
        HILOGD("not report field, paramName: %{public}s", paramName.c_str());
        return false;
    }

    BtChrBusinessEvent btChrBusinessEvent;
    if (!g_btChrBusinessEventMap.Find(peerAddr, btChrBusinessEvent)) {
        return true;
    }

    std::vector<BtChrEventParam> params;
    if (btChrBusinessEvent.removePairResult == CHR_REMOVE_PAIR_SUCCESS) {
        params.push_back(BtChrEventParam("CONNECTSTATUS", btChrBusinessEvent.connectStatus));
        params.push_back(BtChrEventParam("REMOVEPAIRTIME", btChrBusinessEvent.removePairTime));
        BtChrEventWriteParams(eventName, peerAddr, params);
        return true;
    }

    params.push_back(BtChrEventParam("REMOVEPAIRINITIATOR", btChrBusinessEvent.removePairInitiator));
    params.push_back(BtChrEventParam("CONNECTSTATUS", btChrBusinessEvent.connectStatus));
    params.push_back(BtChrEventParam("PKGNAME", btChrBusinessEvent.pkgName));
    params.push_back(BtChrEventParam("REMOVEPAIRTIME", btChrBusinessEvent.removePairTime));
    params.push_back(BtChrEventParam("REMOVEPAIRRSCENE", btChrBusinessEvent.removePairScence));
    params.push_back(BtChrEventParam("FAILERRORCODE", btChrBusinessEvent.failErrorCode));
    params.push_back(BtChrEventParam("DELETEKEYRESULT", btChrBusinessEvent.deleteLinkkeyResult));
    params.push_back(BtChrEventParam("DELETEKEYTIME", btChrBusinessEvent.deleteLinkkeyTime));
    params.push_back(BtChrEventParam("DISCONNECTRESULT", btChrBusinessEvent.disconnectResult));
    params.push_back(BtChrEventParam("DISCONNECTTIME", btChrBusinessEvent.aclDisconnectedTime));
    params.push_back(BtChrEventParam("DISCONNECTERRORCODE", btChrBusinessEvent.disconnectErrorCode));
    BtChrEventWriteParams(eventName, peerAddr, params);
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS