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
#include "taihe_bluetooth_gatt_server_callback.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

TaiheGattServerCallback::TaiheGattServerCallback()
{}

void TaiheGattServerCallback::OnCharacteristicReadRequest(
    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
{
}

void TaiheGattServerCallback::OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId)
{
}

void TaiheGattServerCallback::OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
{
}

void TaiheGattServerCallback::OnDescriptorWriteRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
}

void TaiheGattServerCallback::OnDescriptorReadRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
}

void TaiheGattServerCallback::OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu)
{
}

void TaiheGattServerCallback::OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int ret)
{
}
} // namespace Bluetooth
} // namespace OHOS