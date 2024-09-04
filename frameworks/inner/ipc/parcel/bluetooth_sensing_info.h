/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_BLUETOOTH_PARCEL_SENSING_INFO_H
#define OHOS_BLUETOOTH_STANDARD_BLUETOOTH_PARCEL_SENSING_INFO_H

#include "parcel.h"
#include "../common/bt_sensing_info.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothSensingInfo : public Parcelable, public bluetooth::SensingInfo {
public:
    BluetoothSensingInfo() = default;
    explicit BluetoothSensingInfo(const bluetooth::SensingInfo &other) : bluetooth::SensingInfo(other)
    {}
    explicit BluetoothSensingInfo(const BluetoothSensingInfo &other) : bluetooth::SensingInfo(other)
    {}
    BluetoothSensingInfo& operator=(const bluetooth::SensingInfo &other) = delete;
    BluetoothSensingInfo& operator=(const BluetoothSensingInfo &other) = delete;
    ~BluetoothSensingInfo() override = default;

    bool Marshalling(Parcel &parcel) const override;

    static BluetoothSensingInfo *Unmarshalling(Parcel &parcel);

    bool WriteToParcel(Parcel &parcel);

    bool ReadFromParcel(Parcel &parcel);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_SENSING_INFO_H