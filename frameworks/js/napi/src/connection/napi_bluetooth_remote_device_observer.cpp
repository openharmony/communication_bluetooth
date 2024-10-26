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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_remote_device_observer"
#endif

#include <map>
#include <memory>
#include <string>

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_remote_device_observer.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_connection.h"

namespace OHOS {
namespace Bluetooth {
NapiBluetoothRemoteDeviceObserver::NapiBluetoothRemoteDeviceObserver()
    : eventSubscribe_({REGISTER_BOND_STATE_TYPE,
        REGISTER_BATTERY_CHANGE_TYPE},
        BT_MODULE_NAME)
{}

void NapiBluetoothRemoteDeviceObserver::OnAclStateChanged(
    const BluetoothRemoteDevice &device, int state, unsigned int reason)
{}

void NapiBluetoothRemoteDeviceObserver::OnPairStatusChanged(const BluetoothRemoteDevice &device, int status, int cause)
{
    int bondStatus = 0;
    DealPairStatus(status, bondStatus);
    HILOGI("addr:%{public}s, bondStatus:%{public}d, cause:%{public}d", GET_ENCRYPT_ADDR(device), bondStatus, cause);

    auto nativeObject = std::make_shared<NapiNativeBondStateParam>(device.GetDeviceAddr(), bondStatus, cause);
    eventSubscribe_.PublishEvent(REGISTER_BOND_STATE_TYPE, nativeObject);
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteUuidChanged(
    const BluetoothRemoteDevice &device, const std::vector<ParcelUuid> &uuids)
{
    HILOGD("called");
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteNameChanged(
    const BluetoothRemoteDevice &device, const std::string &deviceName)
{
    HILOGD("addr:%{public}s, deviceName:%{public}s", GET_ENCRYPT_ADDR(device), deviceName.c_str());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteAliasChanged(
    const BluetoothRemoteDevice &device, const std::string &alias)
{
    HILOGD("addr:%{public}s, alias:%{public}s", GET_ENCRYPT_ADDR(device), alias.c_str());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteCodChanged(
    const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod)
{
    HILOGD("addr:%{public}s, cod:%{public}d", GET_ENCRYPT_ADDR(device), cod.GetClassOfDevice());
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteBatteryLevelChanged(
    const BluetoothRemoteDevice &device, int batteryLevel)
{
    HILOGD("addr:%{public}s, batteryLevel:%{public}d", GET_ENCRYPT_ADDR(device), batteryLevel);
}

void NapiBluetoothRemoteDeviceObserver ::OnRemoteBatteryChanged(
    const BluetoothRemoteDevice &device, const DeviceBatteryInfo &batteryInfo)
{
    auto nativeObject = std::make_shared<NapiNativeBatteryInfo>(batteryInfo);
    eventSubscribe_.PublishEvent(REGISTER_BATTERY_CHANGE_TYPE, nativeObject);
}

void NapiBluetoothRemoteDeviceObserver ::OnReadRemoteRssiEvent(
    const BluetoothRemoteDevice &device, int rssi, int status)
{
    HILOGD("addr:%{public}s, rssi:%{public}d, status is %{public}d", GET_ENCRYPT_ADDR(device), rssi, status);
}
}  // namespace Bluetooth
}  // namespace OHOS