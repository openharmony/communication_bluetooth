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
#define LOG_TAG "bt_service_util_common_event"
#endif

#include "bluetooth_common_event_helper.h"
#include "common_event.h"
#include "common_event_data.h"
#include "log.h"
#include "hitrace_meter.h"
#include "raw_address.h"
#include "adapter_manager.h"
#include "log_utils.h"

using namespace OHOS::EventFwk;
using namespace OHOS::bluetooth;

namespace OHOS {
namespace BluetoothHelper {
static const std::vector<std::string> ACCESS_PERMISSIONS {"ohos.permission.ACCESS_BLUETOOTH"};
static constexpr int INVALID_EVENT_CODE = -1;
enum class BondState { BOND_STATE_INVALID = 0, BOND_STATE_BONDING = 1, BOND_STATE_BONDED = 2 };

bool BluetoothCommonEventHelper::PublishEvent(const OHOS::AAFwk::Want &want, int eventCode,
    bool isOrdered, const std::vector<std::string> &permissions, bool isSticky)
{
    HITRACE_METER(BT_TRACE_TAG);
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    if (eventCode != INVALID_EVENT_CODE) {
        data.SetCode(eventCode);
    }
    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(isOrdered);
    publishInfo.SetSticky(isSticky);
    if (permissions.size() > 0) {
        publishInfo.SetSubscriberPermissions(permissions);
    }
    bool publishResult = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    std::string logKey = "(PublishEvent)action:" + want.GetAction();
    HILOGI_ACCUMULATE(logKey, "result:%d", publishResult);
    return publishResult;
}

bool BluetoothCommonEventHelper::PublishEventWithAccessPermission(const OHOS::AAFwk::Want &want, bool isSticky)
{
    return PublishEvent(want, INVALID_EVENT_CODE, false, ACCESS_PERMISSIONS, isSticky);
}

bool BluetoothCommonEventHelper::PublishEventWithManagePermission(const OHOS::AAFwk::Want &want, bool isSticky)
{
    const std::vector<std::string> MANAGE_PERMISSIONS {"ohos.permission.MANAGE_BLUETOOTH"};
    return PublishEvent(want, INVALID_EVENT_CODE, false, MANAGE_PERMISSIONS, isSticky);
}

bool BluetoothCommonEventHelper::PublishEventWithPeerRealMac(const OHOS::AAFwk::Want &want, bool isSticky)
{
    std::vector<std::string> permissions = {PERMISSION_MANAGE_BLUETOOTH, PERMISSION_GET_BLUETOOTH_PEERS_MAC};
    return PublishEvent(want, INVALID_EVENT_CODE, false, permissions, isSticky);
}

bool BluetoothCommonEventHelper::PublishEventWithoutPermisssion(const OHOS::AAFwk::Want &want)
{
    std::vector<std::string> permissions;
    return PublishEvent(want, INVALID_EVENT_CODE, false, permissions, false);
}

bool BluetoothCommonEventHelper::PublishEventWithIntParam(const std::string &eventAction, const int value,
    const std::vector<std::string> &permissions, bool isSticky)
{
    OHOS::AAFwk::Want want;
    want.SetAction(eventAction);
    return PublishEvent(want, value, false, permissions, isSticky);
}

bool BluetoothCommonEventHelper::PublishEventWithIntParamAndPeerRealMac(const std::string &eventAction,
    const std::string &device, const std::string &key, const int value, bool isSticky)
{
    OHOS::AAFwk::Want want;
    want.SetAction(eventAction);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, device);
    want.SetParam(key, value);
    return PublishEventWithPeerRealMac(want, isSticky);
}

bool BluetoothCommonEventHelper::PublishEventWithIntParam(const std::string &eventAction, const std::string &device,
    const std::string &key, const int value, bool isSticky)
{
    OHOS::AAFwk::Want want;
    want.SetAction(eventAction);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, device);
    want.SetParam(key, value);
    return PublishEventWithAccessPermission(want, isSticky);
}

bool BluetoothCommonEventHelper::PublishEventWithMultiIntParam(const std::string &eventAction,
    const std::string &device, const std::vector<std::pair<std::string, int>> &intParams, bool isSticky)
{
    OHOS::AAFwk::Want want;
    want.SetAction(eventAction);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, device);
    for (const auto &pairParam : intParams) {
        want.SetParam(pairParam.first, pairParam.second);
    }
    return PublishEventWithAccessPermission(want, isSticky);
}

bool BluetoothCommonEventHelper::PublishEventWithStringParam(const std::string &eventAction, const std::string &device,
    const std::string &key, const std::string &value)
{
    OHOS::AAFwk::Want want;
    want.SetAction(eventAction);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, device);
    want.SetParam(key, value);
    return PublishEventWithAccessPermission(want, false);
}

bool BluetoothCommonEventHelper::PublishEventWithStringParam(const std::string &eventAction, const std::string &key,
    const std::string &value)
{
    OHOS::AAFwk::Want want;
    want.SetAction(eventAction);
    want.SetParam(key, value);
    return PublishEventWithAccessPermission(want, false);
}

template <class vecType>
bool BluetoothCommonEventHelper::PublishEventWithVecParam(const std::string &eventAction, const std::string &device,
    const std::string &key, const std::vector<vecType> &value)
{
    OHOS::AAFwk::Want want;
    want.SetAction(eventAction);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, device);
    want.SetParam(key, value);
    return PublishEventWithAccessPermission(want, false);
}

void BluetoothCommonEventHelper::PublishBluetoothStateChangeEvent(int code, const bluetooth::BTTransport transport)
{
    if ((transport == bluetooth::BTTransport::ADAPTER_BREDR && code == bluetooth::BTStateID::STATE_TURN_ON) ||
        (transport == bluetooth::BTTransport::ADAPTER_BLE && code == bluetooth::BTStateID::STATE_TURN_OFF)) {
        std::vector<std::string> permissions;
        PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE, code, permissions, false);
    } else {
        HILOGD("Not broadcast state change");
    }
    return;
}

void BluetoothCommonEventHelper::PublishDiscoveryStartedEvent(int code)
{
    if (code == bluetooth::DISCOVERY_STARTED) {
        PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_STARTED, code, ACCESS_PERMISSIONS, false);
    }
}

void BluetoothCommonEventHelper::PublishDiscoveryFinishedEvent(int code)
{
    if (code == bluetooth::DISCOVERY_STOPED) {
        PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_FINISHED, code, ACCESS_PERMISSIONS, false);
    }
}

void BluetoothCommonEventHelper::PublishLocalNameChangedEvent(const std::string &localName)
{
    PublishEventWithStringParam(COMMON_EVENT_BLUETOOTH_HOST_NAME_UPDATE, PARAM_KEY_LOCAL_NAME, localName);
}

void BluetoothCommonEventHelper::PublishRemoteNameChangedEvent(const std::string &device, const std::string &remoteName)
{
    PublishEventWithStringParam(COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE,
        device, PARAM_KEY_REMOTE_NAME, remoteName);
}

void BluetoothCommonEventHelper::PublishRemoteUuidsChangedEvent(const std::string &device,
    const std::vector<std::string> &uuids)
{
    PublishEventWithVecParam(COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_UUID_VALUE,
        device, PARAM_KEY_REMOTE_UUIDS, uuids);
}

// 根据是否还存在acl链路上报连接状态
void BluetoothCommonEventHelper::PublishDeviceConnectionStateEvent(const std::string &address, int32_t state)
{
    HILOGI("DeviceConnectionStateIs is = %{public}d", state);
    PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_STATUS_VALUE, address, "state", state, true);
}

void BluetoothCommonEventHelper::PublishDeviceBatteryLevelEvent(const std::string &device, int32_t batteryLevel)
{
    HILOGI("batteryLevel = %{public}d", batteryLevel);
    PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_BATTERY_VALUE_UPDATE,
        device, "batteryLevel", batteryLevel, true);
}

int32_t BluetoothCommonEventHelper::GetNapiProfileConnectionState(int state)
{
    int32_t profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
    switch (state) {
        case static_cast<int32_t>(BTConnectState::CONNECTING):
            HILOGD("STATE_CONNECTING(1)");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::CONNECTED):
            HILOGD("STATE_CONNECTED(2)");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTED;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTING):
            HILOGD("STATE_DISCONNECTING(3)");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTED):
            HILOGD("STATE_DISCONNECTED(0)");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
            break;
        default:
            break;
    }
    return profileConnectionState;
}

void BluetoothCommonEventHelper::GetNapiPairStatus(const int &status, int &bondStatus)
{
    HILOGD("status is %{public}d", status);
    switch (status) {
        case BT_BOND_STATE_NONE:
            bondStatus = static_cast<int>(BondState::BOND_STATE_INVALID);
            break;
        case BT_BOND_STATE_BONDING:
            bondStatus = static_cast<int>(BondState::BOND_STATE_BONDING);
            break;
        case BT_BOND_STATE_BONDED:
            bondStatus = static_cast<int>(BondState::BOND_STATE_BONDED);
            break;
        default:
            break;
    }
}

void BluetoothCommonEventHelper::PublishAclConnectedEvent(const std::string &address, int linktype)
{
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    PublishEventWithIntParam(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED, randomAddr.GetAddress(), "linktype", linktype, false);
    std::vector<std::pair<std::string, int>> params = {
        {"linktype", linktype},
        {"connectionstate", CONNECTION_STATE_CONNECTED}
    };
    PublishEventWithMultiIntParam(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_STATE_CHANGE, randomAddr.GetAddress(), params, false);
    PublishEventWithIntParamAndPeerRealMac(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED_REALMAC, address, "linktype", linktype, false);
}

void BluetoothCommonEventHelper::PublishAclDisconnectedEvent(const std::string &address, int linktype)
{
    HILOGI("linktype is = %{public}d", linktype);
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    PublishEventWithIntParam(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED, randomAddr.GetAddress(), "linktype", linktype, false);
    std::vector<std::pair<std::string, int>> params = {
        {"linktype", linktype},
        {"connectionstate", CONNECTION_STATE_DISCONNECTED}
    };
    PublishEventWithMultiIntParam(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_STATE_CHANGE, randomAddr.GetAddress(), params, false);
    PublishEventWithIntParamAndPeerRealMac(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED_REALMAC, address, "linktype", linktype, false);
}

void BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(const std::string &address, int32_t state)
{
    int32_t getNapiState = GetNapiProfileConnectionState(static_cast<int>(state));
    HILOGI("AvrcpConnectState is = %{public}d", getNapiState);
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_A2DPSOURCE_AVRCP_CONNECT_STATE_UPDATE,
        randomAddr.GetAddress(), "state", getNapiState, false);
    PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_A2DPSOURCE_AVRCP_CONNECT_STATE_CHANGE,
        randomAddr.GetAddress(), "state", getNapiState, false);
}

void BluetoothCommonEventHelper::PublishHfpConnectStateUpdateEvent(const std::string &address, int32_t state)
{
    int32_t getNapiState = GetNapiProfileConnectionState(static_cast<int>(state));
    HILOGI("HfpConnectState is = %{public}d", getNapiState);
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_UPDATE,
        randomAddr.GetAddress(), "state", getNapiState, false);
    PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_CHANGE,
        randomAddr.GetAddress(), "state", getNapiState, false);
}

void BluetoothCommonEventHelper::PublishA2dpConnectStateUpdateEvent(const std::string &address, int32_t state)
{
    int32_t getNapiState = GetNapiProfileConnectionState(static_cast<int>(state));
    HILOGI("A2dpConnectState is = %{public}d", getNapiState);
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    PublishEventWithIntParam(
        COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_UPDATE, randomAddr.GetAddress(), "state", getNapiState, false);
    PublishEventWithIntParam(
        COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_CHANGE, randomAddr.GetAddress(), "state", getNapiState, false);
}

void BluetoothCommonEventHelper::PublishRemoteDevicePairStateEvent(const std::string &address, int32_t state)
{
    int bondStatus = 0;
    GetNapiPairStatus(static_cast<int>(state), bondStatus);
    HILOGI("RemoteDevicePairState is = %{public}d", bondStatus);
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    PublishEventWithIntParam(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE, randomAddr.GetAddress(), "state", bondStatus, false);
    PublishEventWithIntParam(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE_CHANGE, randomAddr.GetAddress(), "state", bondStatus, false);
    PublishEventWithIntParamAndPeerRealMac(COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE_CHANGE_REALMAC,
        addr.GetAddress(), "state", bondStatus, false);
}

void BluetoothCommonEventHelper::PublishRemovePairEvent(const std::string &address)
{
    PublishEventWithStringParam(
        COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_REMOVE_PAIR, address, "bondState", "BOND_NONE");
}

bool BluetoothCommonEventHelper::PublishActiveDeviceChangeEvent(const std::string &address,
    const std::string profiletype, bool isSticky)
{
    HILOGI("profiletype is = %{public}s", profiletype.c_str());
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_ACTIVEADDR_DEVICE_ID);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, address);
    want.SetParam("profiletype", profiletype);
    return PublishEventWithAccessPermission(want, true);
}

void BluetoothCommonEventHelper::PublishHidConnectionStateEvent(const std::string &device,
    int32_t state)
{
    PublishEventWithIntParam(COMMON_EVENT_BLUETOOTH_HID_CONNECTION_STATE_CHANGED, device, "state", state, true);
}

bool BluetoothCommonEventHelper::PublishA2dpCodecUpdateEvent(const std::string &address,
    const bluetooth::A2dpSrcCodecInfo &codecInfo)
{
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_UPDATE);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, randomAddr.GetAddress());
    want.SetParam("codecPriority", static_cast<int>(codecInfo.codecPriority));
    want.SetParam("codecType", static_cast<int>(codecInfo.codecType));
    want.SetParam("sampleRate", static_cast<int>(codecInfo.sampleRate));
    want.SetParam("bitsPerSample", static_cast<int>(codecInfo.bitsPerSample));
    want.SetParam("channelMode", static_cast<int>(codecInfo.channelMode));
    want.SetParam("codecSpecific1", static_cast<int>(codecInfo.codecSpecific1));
    want.SetParam("codecSpecific2", static_cast<int>(codecInfo.codecSpecific2));
    want.SetParam("codecSpecific3", static_cast<int>(codecInfo.codecSpecific3));
    want.SetParam("codecSpecific4", static_cast<int>(codecInfo.codecSpecific4));
    return PublishEventWithAccessPermission(want, true);
}

bool BluetoothCommonEventHelper::PublishA2dpCodecChangeEvent(const std::string &address,
    const bluetooth::A2dpSrcCodecInfo &codecInfo)
{
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_CHANGE);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, randomAddr.GetAddress());
    want.SetParam("codecPriority", static_cast<int>(codecInfo.codecPriority));
    want.SetParam("codecType", static_cast<int>(codecInfo.codecType));
    want.SetParam("sampleRate", static_cast<int>(codecInfo.sampleRate));
    want.SetParam("bitsPerSample", static_cast<int>(codecInfo.bitsPerSample));
    want.SetParam("channelMode", static_cast<int>(codecInfo.channelMode));
    want.SetParam("codecSpecific1", static_cast<int>(codecInfo.codecSpecific1));
    want.SetParam("codecSpecific2", static_cast<int>(codecInfo.codecSpecific2));
    want.SetParam("codecSpecific3", static_cast<int>(codecInfo.codecSpecific3));
    want.SetParam("codecSpecific4", static_cast<int>(codecInfo.codecSpecific4));
    return PublishEventWithAccessPermission(want, true);
}

bool BluetoothCommonEventHelper::PublishScanModeChangeEvent(const int scanMode)
{
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_HOST_SCAN_MODE_CHANGE);
    want.SetParam("scanMode", scanMode);
    return PublishEventWithAccessPermission(want, true);
}

bool BluetoothCommonEventHelper::PublishScoConnectStateChangeEvent(const std::string &address,
    const int scoConnectState)
{
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_SCO_CONNECT_STATE_CHANGE);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, randomAddr.GetAddress());
    want.SetParam("scoState", scoConnectState);
    return PublishEventWithAccessPermission(want, true);
}

bool BluetoothCommonEventHelper::PublishA2dpPlayStateChangeEvent(const std::string &address,
    const int a2dpPlayState)
{
    bluetooth::RawAddress randomAddr;
    bluetooth::RawAddress addr(address);
    bluetooth::AdapterManager::GetInstance()->GetDeviceRandomAddr(addr, randomAddr);
    OHOS::AAFwk::Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_A2DPSOURCE_PLAY_STATE_CHANGE);
    want.SetParam(PARAM_KEY_DEVICE_ADDR, randomAddr.GetAddress());
    want.SetParam("a2dpState", a2dpPlayState);
    return PublishEventWithAccessPermission(want, true);
}
}
}

