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

#ifndef BLUETOOTH_PARCEL_HID_DEVICE_QOS_H
#define BLUETOOTH_PARCEL_HID_DEVICE_QOS_H

#include "hid_device_data.h"
#include "bluetooth_hid_device_info.h"
#include "hid_device_utils.h"
#include "parcel.h"
namespace OHOS {
namespace Bluetooth {
class BluetoothHidDeviceQos : public Parcelable, public bluetooth::QosSetting {
public:
    BluetoothHidDeviceQos() = default;
    explicit BluetoothHidDeviceQos(const bluetooth::QosSetting &other) : bluetooth::QosSetting(other)
    {}
    BluetoothHidDeviceQos& operator=(const bluetooth::QosSetting &other);
    ~BluetoothHidDeviceQos() override = default;

    bool Marshalling(Parcel &parcel) const override;
    static BluetoothHidDeviceQos *Unmarshalling(Parcel &parcel);
    bool WriteToParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_PARCEL_HID_DEVICE_QOS_H