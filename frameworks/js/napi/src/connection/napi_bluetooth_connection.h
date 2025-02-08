/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef NAPI_BLUETOOTH_CONNECTION_H
#define NAPI_BLUETOOTH_CONNECTION_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "bluetooth_host.h"

namespace OHOS {
namespace Bluetooth {
enum class BondState { BOND_STATE_INVALID = 0, BOND_STATE_BONDING = 1, BOND_STATE_BONDED = 2 };

enum class ScanMode {
    SCAN_MODE_NONE = 0,
    SCAN_MODE_CONNECTABLE = 1,
    SCAN_MODE_GENERAL_DISCOVERABLE = 2,
    SCAN_MODE_LIMITED_DISCOVERABLE = 3,
    SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE = 4,
    SCAN_MODE_CONNECTABLE_LIMITED_DISCOVERABLE = 5
};

enum class BluetoothTransport { TRANSPORT_BR_EDR = 0, TRANSPORT_LE = 1 };

const char * const REGISTER_DEVICE_FIND_TYPE = "bluetoothDeviceFind";
const char * const REGISTER_DISCOVERY_RESULT_TYPE = "discoveryResult";
const char * const REGISTER_PIN_REQUEST_TYPE = "pinRequired";
const char * const REGISTER_BOND_STATE_TYPE = "bondStateChange";
const char * const REGISTER_BATTERY_CHANGE_TYPE = "batteryChange";
const char * const INVALID_PIN_CODE = "000000";

napi_value DefineConnectionFunctions(napi_env env, napi_value exports);
napi_value GetBtConnectionState(napi_env env, napi_callback_info info);
#ifdef BLUETOOTH_API_SINCE_10
napi_value PairDeviceAsync(napi_env env, napi_callback_info info);
napi_value CancelPairedDeviceAsync(napi_env env, napi_callback_info info);
#else
napi_value PairDevice(napi_env env, napi_callback_info info);
napi_value CancelPairedDevice(napi_env env, napi_callback_info info);
#endif
napi_value GetRemoteDeviceName(napi_env env, napi_callback_info info);
napi_value GetRemoteDeviceClass(napi_env env, napi_callback_info info);
napi_value GetLocalName(napi_env env, napi_callback_info info);
napi_value GetPairedDevices(napi_env env, napi_callback_info info);
napi_value GetProfileConnectionState(napi_env env, napi_callback_info info);
napi_value SetDevicePairingConfirmation(napi_env env, napi_callback_info info);
napi_value SetLocalName(napi_env env, napi_callback_info info);
napi_value SetBluetoothScanMode(napi_env env, napi_callback_info info);
napi_value GetBluetoothScanMode(napi_env env, napi_callback_info info);
napi_value StartBluetoothDiscovery(napi_env env, napi_callback_info info);
napi_value StopBluetoothDiscovery(napi_env env, napi_callback_info info);
napi_value RegisterConnectionObserver(napi_env env, napi_callback_info info);
napi_value DeRegisterConnectionObserver(napi_env env, napi_callback_info info);
#ifdef BLUETOOTH_API_SINCE_10
napi_value SetDevicePinCode(napi_env env, napi_callback_info info);
napi_value CancelPairingDevice(napi_env env, napi_callback_info info);
napi_value PairCredibleDevice(napi_env env, napi_callback_info info);
napi_value GetLocalProfileUuids(napi_env env, napi_callback_info info);
napi_value GetRemoteProfileUuids(napi_env env, napi_callback_info info);
napi_value GetProfileConnectionStateEx(napi_env env, napi_callback_info info);
napi_value IsBluetoothDiscovering(napi_env env, napi_callback_info info);
napi_value GetPairState(napi_env env, napi_callback_info info);
napi_value ConnectAllowedProfiles(napi_env env, napi_callback_info info);
napi_value DisconnectAllowedProfiles(napi_env env, napi_callback_info info);
napi_value GetRemoteProductId(napi_env env, napi_callback_info info);
#endif
napi_value SetRemoteDeviceName(napi_env env, napi_callback_info info);
napi_value SetRemoteDeviceType(napi_env env, napi_callback_info info);
napi_value GetRemoteDeviceType(napi_env env, napi_callback_info info);
napi_value GetRemoteDeviceBatteryInfo(napi_env env, napi_callback_info info);

napi_value ConnectionPropertyValueInit(napi_env env, napi_value exports);
napi_value ScanModeInit(napi_env env);
napi_value BondStateInit(napi_env env);
napi_value UnbondCauseInit(napi_env env);
#ifdef BLUETOOTH_API_SINCE_10
napi_value BluetoothTransportInit(napi_env env);
napi_value PinTypeInit(napi_env env);
#endif

napi_value DeviceTypeInit(napi_env env);
napi_value DeviceChargeStateInit(napi_env env);

napi_value ControlDeviceAction(napi_env env, napi_callback_info info);
napi_value GetRemoteDeviceConnectionTime(napi_env env, napi_callback_info info);
napi_value UpdateCloudBluetoothDevice(napi_env env, napi_callback_info info);

void RegisterObserverToHost();
void DealPairStatus(const int &status, int &bondStatus);
}  // namespace Bluetooth
}  // namespace OHOS

#endif