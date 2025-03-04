/*
 * Copyright (C) Huawei Device Co., Ltd. 2025-2025. All rights reserved.
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

#ifndef OHOS_BLUETOOTH_STANDARD_BLUETOOTH_PARCEL_TRUST_DEVICE_H
#define OHOS_BLUETOOTH_STANDARD_BLUETOOTH_PARCEL_TRUST_DEVICE_H

#include "parcel.h"
#include "bt_trust_pair_device.h"

namespace OHOS {
namespace Bluetooth {
constexpr int32_t TRUST_PAIR_DEVICE_SIZE_MAX = 1000;
class BluetoothTrustPairDevice : public Parcelable, public bluetooth::TrustPairDevice {
public:
    BluetoothTrustPairDevice() = default;
    ~BluetoothTrustPairDevice() = default;
   
    bool Marshalling(Parcel &parcel) const override;
    static BluetoothTrustPairDevice *Unmarshalling(Parcel &parcel);
    bool WriteToParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);
private:
    bool MarshallingVecSafe(Parcel &parcel, const std::vector<uint8_t> &vec) const;
    bool ReadParcelVecSafe(Parcel &parcel, std::vector<uint8_t> &res);
    bool MarshallingVecStrSafe(Parcel &parcel, const std::vector<std::string> &vec) const;
    bool ReadParcelVecStrSafe(Parcel &parcel, std::vector<std::string> &res);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_SENSING_INFO_H