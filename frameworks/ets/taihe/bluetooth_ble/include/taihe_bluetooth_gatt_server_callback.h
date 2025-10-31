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

#ifndef TAIHE_BLUETOOTH_GATT_SERVER_CALLBACK_H_
#define TAIHE_BLUETOOTH_GATT_SERVER_CALLBACK_H_

#include "bluetooth_gatt_server.h"
#include "taihe_async_callback.h"

namespace OHOS {
namespace Bluetooth {
class TaiheGattServerCallback : public GattServerCallback {
public:
    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state) override;
    void OnServiceAdded(GattService service, int ret) override {}
    void OnCharacteristicReadRequest(const BluetoothRemoteDevice &device,
        GattCharacteristic &characteristic, int requestId) override;
    void OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
        GattCharacteristic &characteristic, int requestId) override;
    void OnDescriptorReadRequest(const BluetoothRemoteDevice &device,
        GattDescriptor &descriptor, int requestId) override;
    void OnDescriptorWriteRequest(const BluetoothRemoteDevice &device,
        GattDescriptor &descriptor, int requestId) override;
    void OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu) override;
    void OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device,
        int result) override;
    void OnConnectionParameterChanged(const BluetoothRemoteDevice &device,
        int interval, int latency, int timeout, int status) override {}

    TaiheAsyncWorkMap asyncWorkMap_ {};
    TaiheGattServerCallback();
    ~TaiheGattServerCallback() override = default;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* TAIHE_BLUETOOTH_GATT_SERVER_CALLBACK_H_ */