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

#ifndef TAIHE_BLUETOOTH_GATT_CLIENT_H_
#define TAIHE_BLUETOOTH_GATT_CLIENT_H_

#include <vector>

#include "bluetooth_gatt_client.h"
#include "bluetooth_log.h"
#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "taihe_bluetooth_gatt_client_callback.h"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {
class GattClientDeviceImpl {
public:
    void Connect();
    void Disconnect();
    void Close();
    uintptr_t ReadCharacteristicValuePromise(::ohos::bluetooth::ble::BLECharacteristic const &characteristic);
    void ReadCharacteristicValueAsync(::ohos::bluetooth::ble::BLECharacteristic const &characteristic,
        uintptr_t callback);
    uintptr_t ReadDescriptorValuePromise(::ohos::bluetooth::ble::BLEDescriptor const &bleDescriptor);
    void ReadDescriptorValueAsync(::ohos::bluetooth::ble::BLEDescriptor const &bleDescriptor, uintptr_t callback);

    uintptr_t SetCharacteristicChangeIndicationPromise(::ohos::bluetooth::ble::BLECharacteristic const &characteristic,
        bool enable);
    void SetCharacteristicChangeIndicationAsync(::ohos::bluetooth::ble::BLECharacteristic const &characteristic,
        bool enable, uintptr_t callback);

    void SetBLEMtuSize(int mtu);
    taihe::string GetDeviceNameSync();
    static int GattStatusFromService(int status);

    std::shared_ptr<GattClient> &GetClient()
    {
        return client_;
    }

    std::shared_ptr<TaiheGattClientCallback> GetCallback()
    {
        return callback_;
    }

    std::shared_ptr<BluetoothRemoteDevice> GetDevice()
    {
        return device_;
    }

    explicit GattClientDeviceImpl(taihe::string_view deviceId)
    {
        HILOGI("enter");
        std::string remoteAddr = std::string(deviceId);
        device_ = std::make_shared<BluetoothRemoteDevice>(remoteAddr, 1);
        client_ = std::make_shared<GattClient>(*device_);
        client_->Init();
        callback_ = std::make_shared<TaiheGattClientCallback>();
        callback_->SetDeviceAddr(remoteAddr);
    }
    ~GattClientDeviceImpl() = default;

    static const std::vector<std::pair<int, int>> g_gattStatusSrvToNapi;
private:
    std::shared_ptr<GattClient> client_ = nullptr;
    std::shared_ptr<TaiheGattClientCallback> callback_;
    std::shared_ptr<BluetoothRemoteDevice> device_ = nullptr;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* TAIHE_BLUETOOTH_GATT_CLIENT_H_ */
