/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_GATT_SERVER_CALLBACK_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_GATT_SERVER_CALLBACK_INTERFACE_H

#include "bluetooth_service_ipc_interface_code.h"
#include "iremote_broker.h"
#include "bluetooth_gatt_service_parcel.h"
#include "bluetooth_gatt_descriptor_parcel.h"
#include "bluetooth_gatt_device.h"
#include "bluetooth_gatt_characteristic_parcel.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothGattServerCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothGattServerCallback");

    virtual void OnConnectionStateChanged(const BluetoothGattDevice &device, int32_t ret, int32_t state) = 0;
    virtual void OnAddService(int32_t ret, const BluetoothGattService &service) = 0;
    virtual void OnCharacteristicReadRequest(
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic) = 0;
    virtual void OnCharacteristicWriteRequest(
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic, bool needRespones) = 0;
    virtual void OnDescriptorReadRequest(
        const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor) = 0;
    virtual void OnDescriptorWriteRequest(
        const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor) = 0;
    virtual void OnMtuChanged(const BluetoothGattDevice &device, int32_t mtu) = 0;
    virtual void OnNotifyConfirm(
        const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic, int result) = 0;
    virtual void OnConnectionParameterChanged(
        const BluetoothGattDevice &device, int32_t interval, int32_t latency, int32_t timeout, int32_t status) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_GATT_SERVER_CALLBACK_H