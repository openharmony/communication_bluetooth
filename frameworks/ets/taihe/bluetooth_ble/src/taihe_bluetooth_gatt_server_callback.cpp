/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_gatt_server.h"
#include "taihe_bluetooth_gatt_server_callback.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

TaiheGattServerCallback::TaiheGattServerCallback()
{}

void TaiheGattServerCallback::OnCharacteristicReadRequest(
    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
}

void TaiheGattServerCallback::OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
}

void TaiheGattServerCallback::OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
{
    HILOGI("enter, state: %{public}d, remote device address: %{public}s", state, GET_ENCRYPT_ADDR(device));
    std::lock_guard<std::mutex> lock(GattServerImpl::deviceListMutex_);
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("connected");
        bool hasAddr = false;
        for (auto it = GattServerImpl::deviceList_.begin();
                it != GattServerImpl::deviceList_.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                hasAddr = true;
                break;
            }
        }
        if (!hasAddr) {
            HILOGI("add devices");
            GattServerImpl::deviceList_.push_back(device.GetDeviceAddr());
        }
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("disconnected");
        for (auto it = GattServerImpl::deviceList_.begin();
                it != GattServerImpl::deviceList_.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                HILOGI("romove device");
                GattServerImpl::deviceList_.erase(it);
                break;
            }
        }
    }
}

void TaiheGattServerCallback::OnDescriptorWriteRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
}

void TaiheGattServerCallback::OnDescriptorReadRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
}

void TaiheGattServerCallback::OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu)
{
    HILOGI("enter, remote device address: %{public}s, mtu: %{public}d",
        GET_ENCRYPT_ADDR(device), mtu);
}

void TaiheGattServerCallback::OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int ret)
{
    HILOGI("ret: %{public}d", ret);
}
} // namespace Bluetooth
} // namespace OHOS