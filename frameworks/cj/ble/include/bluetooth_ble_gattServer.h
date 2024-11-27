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

#include "bluetooth_ble_ffi.h"
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
    void OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state) override;
    void OnServiceAdded(GattService service, int ret) override{};
    void OnCharacteristicReadRequest(const BluetoothRemoteDevice &device, GattCharacteristic &characteristic,
                                     int requestId) override;
    void OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device, GattCharacteristic &characteristic,
                                      int requestId) override;
    void OnDescriptorReadRequest(const BluetoothRemoteDevice &device, GattDescriptor &descriptor,
                                 int requestId) override;
    void OnDescriptorWriteRequest(const BluetoothRemoteDevice &device, GattDescriptor &descriptor,
                                  int requestId) override;
    void OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu) override;
    void OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int result) override{};
    void OnConnectionParameterChanged(const BluetoothRemoteDevice &device, int interval, int latency, int timeout,
                                      int status) override{};

    void RegisterCharacteristicReadFunc(std::function<void(NativeCharacteristicReadRequest)> cjCallback);
    void RegisterCharacteristicWriteFunc(std::function<void(NativeCharacteristicWriteRequest)> cjCallback);
    void RegisterDescriptorReadFunc(std::function<void(NativeDescriptorReadRequest)> cjCallback);
    void RegisterDescriptorWriteFunc(std::function<void(NativeDescriptorWriteRequest)> cjCallback);
    void RegisterConnectionStateChangeFunc(std::function<void(NativeBLEConnectionChangeState)> cjCallback);
    void RegisterBLEMtuChangeFunc(std::function<void(int32_t)> cjCallback);

    FfiGattServerCallback();
    ~FfiGattServerCallback() override = default;

private:
    std::function<void(NativeCharacteristicReadRequest)> characteristicReadFunc{nullptr};
    std::function<void(NativeCharacteristicWriteRequest)> characteristicWriteFunc{nullptr};
    std::function<void(NativeDescriptorReadRequest)> descriptorReadFunc{nullptr};
    std::function<void(NativeDescriptorWriteRequest)> descriptorWriteFunc{nullptr};
    std::function<void(NativeBLEConnectionChangeState)> connectionStateChangeFunc{nullptr};
    std::function<void(int32_t)> bleMtuChangeFunc{nullptr};
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
    int32_t AddService(NativeGattService service);
    int32_t RemoveService(std::string serviceUuid);
    int32_t Close();
    int32_t NotifyCharacteristicChanged(std::string deviceId, NativeNotifyCharacteristic characteristic);
    int32_t SendResponse(NativeServerResponse serverResponse);
    int32_t RegisterBleGattServerObserver(int32_t callbackType, void (*callback)());

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

    int32_t CreateCharacteristicReadFunc(void (*callback)());
    int32_t CreateCharacteristicWriteFunc(void (*callback)());
    int32_t CreateDescriptorReadFunc(void (*callback)());
    int32_t CreateRegisterDescriptorWriteFunc(void (*callback)());
    int32_t CreateConnectionStateChangeFunc(void (*callback)());
    int32_t CreateBLEMtuChangeFunc(void (*callback)());
};
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS

#endif // BLUETOOTH_BLE_GATTSERVER_H