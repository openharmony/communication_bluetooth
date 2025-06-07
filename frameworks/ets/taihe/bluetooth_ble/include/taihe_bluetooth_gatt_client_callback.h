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

#ifndef TAIHE_BLUETOOTH_GATT_CLIENT_CALLBACK_H_
#define TAIHE_BLUETOOTH_GATT_CLIENT_CALLBACK_H_

#include <shared_mutex>
#include "bluetooth_gatt_client.h"
#include "ohos.bluetooth.ble.proj.hpp"
#include "ohos.bluetooth.ble.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Bluetooth {
class TaiheGattClientCallback : public GattClientCallback {
public:
    void OnConnectionStateChanged(int connectionState, int ret) override;
    void OnCharacteristicChanged(const GattCharacteristic &characteristic) override;
    void OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)  override;
    void OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret) override;
    void OnDescriptorReadResult(const GattDescriptor &descriptor, int ret) override;
    void OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret) override;
    void OnMtuUpdate(int mtu, int ret) override;
    void OnServicesDiscovered(int status) override;
    void OnConnectionParameterChanged(int interval, int latency, int timeout, int status) override {}
    void OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status) override;
    void OnReadRemoteRssiValueResult(int rssi, int status)  override;

    std::string GetDeviceAddr(void)
    {
        return deviceAddr_;
    }

    void SetDeviceAddr(std::string deviceAddr)
    {
        deviceAddr_ = deviceAddr;
    }

    TaiheGattClientCallback();
    ~TaiheGattClientCallback() override = default;

private:
    std::string deviceAddr_ = INVALID_MAC_ADDRESS;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif /* TAIHE_BLUETOOTH_GATT_CLIENT_CALLBACK_H_ */