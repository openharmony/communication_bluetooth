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

#ifndef BLUETOOTH_BLE_GATTSERVER_H
#define BLUETOOTH_BLE_GATTSERVER_H

#include "bluetooth_gatt_server.h"
#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BluetoothRemoteDevice;
using Bluetooth::GattCharacteristic;
using Bluetooth::GattDescriptor;
using Bluetooth::GattServer;
using Bluetooth::GattServerCallback;
using Bluetooth::GattService;

class FfiGattServerCallback : public GattServerCallback {
public:
    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state) override{};
    void OnServiceAdded(GattService *Service, int ret) override{};
    void OnCharacteristicReadRequest(const BluetoothRemoteDevice &device, GattCharacteristic &characteristic,
                                     int requestId) override{};
    void OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device, GattCharacteristic &characteristic,
                                      int requestId) override{};
    void OnDescriptorReadRequest(const BluetoothRemoteDevice &device, GattDescriptor &descriptor,
                                 int requestId) override{};
    void OnDescriptorWriteRequest(const BluetoothRemoteDevice &device, GattDescriptor &descriptor,
                                  int requestId) override{};
    void OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu) override{};
    void OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int result) override{};
    void OnConnectionParameterChanged(const BluetoothRemoteDevice &device, int interval, int latency, int timeout,
                                      int status) override{};

    FfiGattServerCallback();
    ~FfiGattServerCallback() override = default;

private:
};

class FfiGattServer : public OHOS::FFI::FFIData {
    DECL_TYPE(FfiGattServer, OHOS::FFI::FFIData)
public:
    explicit FfiGattServer()
    {
        callback_ = std::make_shared<FfiGattServerCallback>();
        std::shared_ptr<GattServerCallback> tmp = std::static_pointer_cast<GattServerCallback>(callback_);
        server_ = GattServer::CreateInstance(tmp);
    };

    std::shared_ptr<GattServer> &GetServer()
    {
        return server_;
    }
    std::shared_ptr<FfiGattServerCallback> GetCallback()
    {
        return callback_;
    }
    static std::vector<std::string> deviceList_;
    static std::mutex deviceListMutex_;

private:
    std::shared_ptr<GattServer> server_ = nullptr;
    std::shared_ptr<FfiGattServerCallback> callback_;
};
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS

#endif // BLUETOOTH_BLE_GATTSERVER_H