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

#ifndef HID_DEVICE_SERVICE_IMPL_INTERFACE_H
#define HID_DEVICE_SERVICE_IMPL_INTERFACE_H

#include "raw_address.h"
#include "interface_profile_hid_device.h"
#include "bluetooth_errorcode.h"


namespace OHOS {
namespace Bluetooth {

class HidDeviceServiceImplInterface {
public:
    HidDeviceServiceImplInterface() = default;
    virtual ~HidDeviceServiceImplInterface() = default;

    virtual void RegisterObserver(std::shared_ptr<IHidDeviceObserver> &hidDeviceObserver) = 0;
    virtual void DeregisterObserver(std::shared_ptr<IHidDeviceObserver> &hidDeviceObserver) = 0;
    virtual int Connect(const bluetooth::RawAddress &device) = 0;
    virtual int Disconnect(const bluetooth::RawAddress &device) = 0;
    virtual int RegisterHidDevice(BluetoothHidDeviceSdp sdp, BluetoothHidDeviceQos inQos,
            BluetoothHidDeviceQos outQos, std::string bundleName) = 0;
    virtual int UnregisterHidDevice() = 0;
    virtual int32_t SendReport(int id, const std::vector<uint8_t> &data) = 0;
    virtual int32_t ReplyReport(uint8_t type, int id, const std::vector<uint8_t> &data) = 0;
    virtual int32_t ReportError(uint8_t type) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // HID_DEVICE_SERVICE_IMPL_INTERFACE_H