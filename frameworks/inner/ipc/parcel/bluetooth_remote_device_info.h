/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_PARCEL_REMOTE_DEVICE_INFO_H
#define BLUETOOTH_PARCEL_REMOTE_DEVICE_INFO_H

#include "parcel.h"
#include <string>
#include <vector>

namespace OHOS {
namespace Bluetooth {
class BluetoothRemoteDeviceInfo : public Parcelable {
public:
    explicit BluetoothRemoteDeviceInfo() = default;
    ~BluetoothRemoteDeviceInfo() = default;
    bool Marshalling(Parcel &parcel) const override;
    static BluetoothRemoteDeviceInfo *Unmarshalling(Parcel &parcel);
    bool WriteToParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);

    uint16_t vendorId_;
    uint16_t productId_;
    std::string modelId_;
    int32_t customType_;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_REMOTE_DEVICE_INFO_H