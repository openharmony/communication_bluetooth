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

#ifndef OHOS_BLUETOOTH_COMMON_EVENT_HELPER_H
#define OHOS_BLUETOOTH_COMMON_EVENT_HELPER_H

#include <string>
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "bt_def.h"
#include "interface_profile_a2dp_src.h"

namespace OHOS {
namespace BluetoothHelper {
/* COMMON EVENTS */
const std::string COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE = "usual.event.bluetooth.host.STATE_UPDATE";
const std::string COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_STARTED = "usual.event.bluetooth.host.DISCOVERY_STARTED";
const std::string COMMON_EVENT_BLUETOOTH_HOST_DISCOVERY_FINISHED = "usual.event.bluetooth.host.DISCOVERY_FINISHED";
const std::string COMMON_EVENT_BLUETOOTH_HOST_NAME_UPDATE = "usual.event.bluetooth.host.NAME_UPDATE";
const std::string COMMON_EVENT_BLUETOOTH_HOST_SCAN_MODE_CHANGE = "usual.event.bluetooth.host.SCAN_MODE_CHANGE";
const std::string COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_NAME_UPDATE = "usual.event.bluetooth.remotedevice.NAME_UPDATE";
const std::string COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_UUID_VALUE = "usual.event.bluetooth.remotedevice.UUID_VALUE";
const std::string COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_CONNECT_STATUS_VALUE =
    "usual.event.bluetooth.remotedevice.CONNECT_STATUS_VALUE";
const std::string COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_BATTERY_VALUE_UPDATE =
    "usual.event.bluetooth.remotedevice.BATTERY_VALUE_UPDATE";
const std::string COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED =
    "usual.event.bluetooth.remotedevice.ACL_CONNECTED";
const std::string COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_STATE_CHANGE =
    "usual.event.bluetooth.remotedevice.ACL_STATE_CHANGE";
const std::string COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED =
    "usual.event.bluetooth.remotedevice.ACL_DISCONNECTED";
constexpr const char* COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_CONNECTED_REALMAC =
    "usual.event.bluetooth.remotedevice.ACL_CONNECTED_REALMAC";
constexpr const char* COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_ACL_DISCONNECTED_REALMAC =
    "usual.event.bluetooth.remotedevice.ACL_DISCONNECTED_REALMAC";
constexpr const char* COMMON_EVENT_BLUETOOTH_A2DPSOURCE_AVRCP_CONNECT_STATE_UPDATE =
    "usual.event.bluetooth.a2dpsource.AVRCP_CONNECT_STATE_UPDATE";
constexpr const char* COMMON_EVENT_BLUETOOTH_A2DPSOURCE_AVRCP_CONNECT_STATE_CHANGE =
    "usual.event.bluetooth.a2dpsource.AVRCP_CONNECT_STATE_CHANGE";
constexpr const char* COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_UPDATE =
    "usual.event.bluetooth.handsfree.ag.CONNECT_STATE_UPDATE";
constexpr const char* COMMON_EVENT_BLUETOOTH_HANDSFREE_AG_CONNECT_STATE_CHANGE =
    "usual.event.bluetooth.handsfree.ag.CONNECT_STATE_CHANGE";
constexpr const char* COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_UPDATE =
    "usual.event.bluetooth.a2dpsource.CONNECT_STATE_UPDATE";
constexpr const char* COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CONNECT_STATE_CHANGE =
    "usual.event.bluetooth.a2dpsource.CONNECT_STATE_CHANGE";
constexpr const char* COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE =
    "usual.event.bluetooth.remotedevice.PAIR_STATE";
constexpr const char* COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE_CHANGE =
    "usual.event.bluetooth.remotedevice.PAIR_STATE_CHANGE";
constexpr const char* COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIR_STATE_CHANGE_REALMAC =
    "usual.event.bluetooth.remotedevice.PAIR_STATE_CHANGE_REALMAC";
const std::string COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_REMOVE_PAIR =
    "usual.event.bluetooth.remotedevice.REMOVE_PAIR";
const std::string COMMON_EVENT_BLUETOOTH_ACTIVEADDR_DEVICE_ID =
    "usual.event.bluetooth.activeaddr.DEVICE_ID";
constexpr const char* COMMON_EVENT_BLUETOOTH_HID_CONNECTION_STATE_CHANGED =
    "usual.event.bluetooth.hid.CONNECTION_STATE_CHANGED";
const std::string COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_UPDATE =
    "usual.event.bluetooth.a2dpsource.CODEC_VALUE_UPDATE";
const std::string COMMON_EVENT_BLUETOOTH_A2DPSOURCE_CODEC_VALUE_CHANGE =
    "usual.event.bluetooth.a2dpsource.CODEC_VALUE_CHANGE";
const std::string COMMON_EVENT_BLUETOOTH_AUTO_CONNECT_DEVICE = "usual.event.bluetooth.AUTO_CONNECT_DEVICE";
const std::string COMMON_EVENT_BLUETOOTH_SCO_CONNECT_STATE_CHANGE =
    "usual.event.bluetooth.SCO_CONNECT_STATE_CHANGE";
const std::string COMMON_EVENT_BLUETOOTH_A2DPSOURCE_PLAY_STATE_CHANGE =
    "usual.event.bluetooth.a2dpsource.PLAY_STATE_CHANGE";
/* COMMON EVENT PERMISSIONS */
const std::string PERMISSION_ACCESS_BLUETOOTH = "ohos.permission.ACCESS_BLUETOOTH";
const std::string PERMISSION_MANAGE_BLUETOOTH = "ohos.permission.MANAGE_BLUETOOTH";
const std::string PERMISSION_GET_BLUETOOTH_PEERS_MAC = "ohos.permission.GET_BLUETOOTH_PEERS_MAC";
/* COMMON EVENT PARAM kEYS */
const std::string PARAM_KEY_DEVICE_ADDR = "deviceAddr";
const std::string PARAM_KEY_LOCAL_NAME = "localName";
const std::string PARAM_KEY_REMOTE_NAME = "remoteName";
const std::string PARAM_KEY_REMOTE_UUIDS = "remoteUuids";
enum class BroadcastScoState {SCO_STATE_DISCONNECTED = 0, SCO_STATE_CONNECTED = 1};
enum class BroadcastA2dpState {A2DP_STATE_PAUSE = 0, A2DP_STATE_PLAY = 1};
class BluetoothCommonEventHelper {
public:
    /**
     * @brief publish common event with permissions
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 6
     */
    static bool PublishEvent(const OHOS::AAFwk::Want &want, int eventCode, bool isOrdered,
        const std::vector<std::string> &permissions, bool isSticky);

    /**
     * @brief publish common event with access permission
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 10
     */
    static bool PublishEventWithAccessPermission(const OHOS::AAFwk::Want &want, bool isSticky);

    /**
     * @brief publish common event with ohos.bluetooth.MANAGE_BLUETOOTH permission
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 22
     */
    static bool PublishEventWithManagePermission(const OHOS::AAFwk::Want &want, bool isSticky);

    /**
     * @brief publish common event with peer real mac
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    static bool PublishEventWithPeerRealMac(const OHOS::AAFwk::Want &want, bool isSticky);

    /**
     * @brief publish common event without permission
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 10
     */
    static bool PublishEventWithoutPermisssion(const OHOS::AAFwk::Want &want);

    /**
     * @brief publish common event with int param and peer real mac, correlation with device
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 12
     */
    static bool PublishEventWithIntParamAndPeerRealMac(const std::string &eventAction,
        const std::string &device, const std::string &key, const int value, bool isSticky);

    /**
     * @brief publish common event with only int param, not correlation with device
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 10
     */
    static bool PublishEventWithIntParam(const std::string &eventAction, const int value,
        const std::vector<std::string> &permissions, bool isSticky);

    /**
     * @brief publish common event with int param, correlation with device
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 10
     */
    static bool PublishEventWithIntParam(const std::string &eventAction, const std::string &device,
        const std::string &key, const int value, bool isSticky);

    /**
     * @brief publish common event with multiple int params, correlation with device
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 20
     */
    static bool PublishEventWithMultiIntParam(const std::string &eventAction, const std::string &device,
        const std::vector<std::pair<std::string, int>> &intParams, bool isSticky);

    /**
     * @brief publish common event with string param, correlation with device
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 10
     */
    static bool PublishEventWithStringParam(const std::string &eventAction, const std::string &device,
        const std::string &key, const std::string &value);

    /**
     * @brief publish common event with string param, correlation with device
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 10
     */
    static bool PublishEventWithStringParam(const std::string &eventAction, const std::string &key,
        const std::string &value);

    /**
     * @brief publish common event with vector param, correlation with device
     *
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     * @since 10
     */
    template <class vecType>
    static bool PublishEventWithVecParam(const std::string &eventAction, const std::string &device,
        const std::string &key, const std::vector<vecType> &value);

    static void PublishBluetoothStateChangeEvent(int code, const bluetooth::BTTransport transport);
    static void PublishDiscoveryStartedEvent(int code);
    static void PublishDiscoveryFinishedEvent(int code);
    static void PublishLocalNameChangedEvent(const std::string &localName);
    static void PublishRemoteNameChangedEvent(const std::string &device, const std::string &remoteName);
    static void PublishRemoteUuidsChangedEvent(const std::string &device, const std::vector<std::string> &uuids);
    static void PublishDeviceConnectionStateEvent(const std::string &device, int32_t state);
    static void PublishDeviceBatteryLevelEvent(const std::string &device, int32_t batteryLevel);
    static void PublishAclConnectedEvent(const std::string &address, int linktype);
    static void PublishAclDisconnectedEvent(const std::string &address, int linktype);
    static void PublishAvrcpConnectStateUpdateEvent(const std::string &address, int32_t state);
    static void PublishHfpConnectStateUpdateEvent(const std::string &address, int32_t state);
    static void PublishA2dpConnectStateUpdateEvent(const std::string &address, int32_t state);
    static void PublishRemoteDevicePairStateEvent(const std::string &address, int32_t state);
    static void PublishRemovePairEvent(const std::string &address);
    static bool PublishActiveDeviceChangeEvent(const std::string &address,
        const std::string profiletype, bool isSticky);
    static void PublishHidConnectionStateEvent(const std::string &device, int32_t state);
    static int32_t GetNapiProfileConnectionState(int state);
    static void GetNapiPairStatus(const int &status, int &bondStatus);
    static bool PublishA2dpCodecChangeEvent(const std::string &address, const bluetooth::A2dpSrcCodecInfo &codecInfo);
    static bool PublishA2dpCodecUpdateEvent(const std::string &address, const bluetooth::A2dpSrcCodecInfo &codecInfo);
    static bool PublishScanModeChangeEvent(const int scanMode);
    static bool PublishScoConnectStateChangeEvent(const std::string &address, const int scoConnectState);
    static bool PublishA2dpPlayStateChangeEvent(const std::string &address, const int a2dpPlayState);
};
}  // namespace BluetoothHelper
}  // namespace OHOS
#endif