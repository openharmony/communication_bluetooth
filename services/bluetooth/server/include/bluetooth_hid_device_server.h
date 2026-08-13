/*
 * Copyright (C) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_HID_DEVICE_SERVER_H
#define OHOS_BLUETOOTH_STANDARD_HID_DEVICE_SERVER_H

#include <map>

#include "bluetooth_hid_device_stub.h"
#include "bluetooth_types.h"
#include "i_bluetooth_hid_device.h"

namespace OHOS {
namespace Bluetooth {

class BluetoothHidDeviceServer : public BluetoothHidDeviceStub {
public:
    explicit BluetoothHidDeviceServer();
    ~BluetoothHidDeviceServer() override;

    int32_t Connect(const BluetoothRawAddress &device) override;

    int32_t Disconnect() override;

    int32_t GetConnectionState(const BluetoothRawAddress &device, int32_t &state) override;

    int32_t GetConnectedDevices(std::vector<BluetoothRawAddress> &result) override;

    int32_t RegisterHidDevice(const BluetoothHidDeviceSdp &sdpSetting, const BluetoothHidDeviceQos &inQos,
        const BluetoothHidDeviceQos &outQos) override;

    int32_t UnregisterHidDevice() override;

    int32_t SendReport(int id, const std::vector<uint8_t> &data) override;
    int32_t ReplyReport(ReportType type, int id, const std::vector<uint8_t> &data) override;
    int32_t ReportError(ErrorReason type) override;

    ErrCode RegisterObserver(const sptr<IBluetoothHidDeviceObserver> observer) override;

    ErrCode DeregisterObserver(const sptr<IBluetoothHidDeviceObserver> observer) override;

    int32_t SetConnectStrategy(const BluetoothRawAddress &device, int strategy) override;
    int32_t GetConnectStrategy(const BluetoothRawAddress &device, int &strategy) override;

private:
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothHidDeviceServer);
    BLUETOOTH_DECLARE_IMPL();
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_HID_DEVICE_SERVER_H