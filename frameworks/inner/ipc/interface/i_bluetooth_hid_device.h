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
#ifndef OHOS_BLUETOOTH_IBLUETOOTHHIDDEVICE_H
#define OHOS_BLUETOOTH_IBLUETOOTHHIDDEVICE_H

#include <vector>
#include <string_ex.h>
#include <iremote_broker.h>
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_hid_device_observer.h"
#include "bluetooth_hid_device_info.h"
#include "bluetooth_hid_device_sdp_parcel.h"
#include "bluetooth_hid_device_qos_parcel.h"

using OHOS::Bluetooth::IBluetoothHidDeviceObserver;
namespace OHOS {
namespace Bluetooth {
class IBluetoothHidDevice : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Bluetooth.IBluetoothHidDevice");

    virtual int32_t RegisterHidDevice(const BluetoothHidDeviceSdp &sdpSetting,
        const BluetoothHidDeviceQos &inQos,
        const BluetoothHidDeviceQos &outQos) = 0;
    virtual int32_t UnregisterHidDevice() = 0;
    virtual int32_t Connect(const BluetoothRawAddress &device) = 0;
    virtual int32_t Disconnect() = 0;

    virtual int32_t GetConnectionState(const BluetoothRawAddress &device, int32_t &state) = 0;
    virtual int32_t GetConnectedDevices(std::vector<BluetoothRawAddress> &result) = 0;

    virtual int32_t RegisterObserver(const sptr<IBluetoothHidDeviceObserver> observer) = 0;
    virtual int32_t DeregisterObserver(const sptr<IBluetoothHidDeviceObserver> observer) = 0;
    virtual int32_t SendReport(int id, const std::vector<uint8_t> &data) = 0;
    virtual int32_t ReplyReport(ReportType type, int id, const std::vector<uint8_t> &data) = 0;
    virtual int32_t ReportError(ErrorReason type) = 0;
    virtual int32_t SetConnectStrategy(const BluetoothRawAddress &device, int strategy) = 0;
    virtual int32_t GetConnectStrategy(const BluetoothRawAddress &device, int &strategy) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_IBLUETOOTHHIDDEVICE_H