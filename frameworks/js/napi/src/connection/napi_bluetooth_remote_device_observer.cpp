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
void NapiBluetoothRemoteDeviceObserver::OnAclStateChanged(
    const BluetoothRemoteDevice &device, int state, unsigned int reason)
{}

void NapiBluetoothRemoteDeviceObserver::OnPairStatusChanged(const BluetoothRemoteDevice &device, int status)
{
    std::shared_ptr<NapiCallback> napiPairStatusChangeCallback = GetCallback(REGISTER_BOND_STATE_TYPE);
    if (!napiPairStatusChangeCallback) {
        HILOGD("PairStatusChangeCallback is not registered");
        return;
    }

    int bondStatus = 0;
    DealPairStatus(status, bondStatus);
    HILOGI("addr:%{public}s, bondStatus:%{public}d", GET_ENCRYPT_ADDR(device), bondStatus);

    auto func = [bondStatus, addr = device.GetDeviceAddr(), callback = napiPairStatusChangeCallback]() {
        CHECK_AND_RETURN_LOG(callback, "PairStatusChangeCallback is not registered");
        auto napiNative = std::make_shared<NapiNativeBondStateParam>(addr, bondStatus);
        callback->CallFunction(napiNative);
    };
    DoInJsMainThread(napiPairStatusChangeCallback->GetNapiEnv(), func);
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
    std::shared_ptr<NapiCallback> napiBatteryChangeCallback = GetCallback(REGISTER_BATTERY_CHANGE_TYPE);
    CHECK_AND_RETURN_LOG(napiBatteryChangeCallback != nullptr, "BatteryChangeCallback is not registered");

    auto func = [batteryInfo, callback = napiBatteryChangeCallback]() {
        CHECK_AND_RETURN_LOG(callback, "BatteryChangeCallback is not registered");
        auto napiNative = std::make_shared<NapiNativeBatteryInfo>(batteryInfo);
        callback->CallFunction(napiNative);
    };
    DoInJsMainThread(napiBatteryChangeCallback->GetNapiEnv(), func);
}

void NapiBluetoothRemoteDeviceObserver ::OnReadRemoteRssiEvent(
    const BluetoothRemoteDevice &device, int rssi, int status)
{
    HILOGD("addr:%{public}s, rssi:%{public}d, status is %{public}d", GET_ENCRYPT_ADDR(device), rssi, status);
}

void NapiBluetoothRemoteDeviceObserver::RegisterCallback(
    const std::string &callbackName, const std::shared_ptr<NapiCallback> &callback)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    callbacks_[callbackName] = callback;
}

void NapiBluetoothRemoteDeviceObserver::DeRegisterCallback(const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    callbacks_.erase(callbackName);
}

std::shared_ptr<NapiCallback> NapiBluetoothRemoteDeviceObserver::GetCallback(const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(callbacksMapLock_);
    if (callbacks_.find(callbackName) != callbacks_.end()) {
        return callbacks_[callbackName];
    }
    return nullptr;
}
}  // namespace Bluetooth
}  // namespace OHOS