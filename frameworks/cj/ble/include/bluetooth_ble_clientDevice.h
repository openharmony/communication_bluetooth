/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BLUETOOTH_BLE_CLIENTDEVICE_H
#define BLUETOOTH_BLE_CLIENTDEVICE_H

#include "bluetooth_def.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_remote_device.h"
#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BluetoothRemoteDevice;
using Bluetooth::GattCharacteristic;
using Bluetooth::GattClient;
using Bluetooth::GattClientCallback;
using Bluetooth::GattDescriptor;
using Bluetooth::INVALID_MAC_ADDRESS;

class FfiGattClientCallback : public GattClientCallback {
public:
    void OnConnectionStateChanged(int connectionState, int ret) override{};
    void OnCharacteristicChanged(const GattCharacteristic &characteristic) override{};
    void OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret) override{};
    void OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret) override{};
    void OnDescriptorReadResult(const GattDescriptor &descriptor, int ret) override{};
    void OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret) override{};
    void OnMtuUpdate(int mtu, int ret) override{};
    void OnServicesDiscovered(int status) override{};
    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override{};
    void OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status) override{};
    void OnReadRemoteRssiValueResult(int rssi, int status) override{};

    FfiGattClientCallback();
    ~FfiGattClientCallback() override = default;

private:
    friend class FfiClientDevice;
    std::string deviceAddr_ = INVALID_MAC_ADDRESS;
};

class FfiClientDevice : public OHOS::FFI::FFIData {
    DECL_TYPE(FfiClientDevice, OHOS::FFI::FFIData)
public:
    explicit FfiClientDevice(std::string deviceId)
    {
        device_ = std::make_shared<BluetoothRemoteDevice>(deviceId, 1);
        client_ = std::make_shared<GattClient>(*device_);
        client_->Init();
        callback_ = std::make_shared<FfiGattClientCallback>();
        callback_->deviceAddr_ = deviceId;
    };

private:
    std::shared_ptr<GattClient> client_ = nullptr;
    std::shared_ptr<FfiGattClientCallback> callback_;
    std::shared_ptr<BluetoothRemoteDevice> device_ = nullptr;
};
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS

#endif // BLUETOOTH_BLE_CLIENTDEVICE_H