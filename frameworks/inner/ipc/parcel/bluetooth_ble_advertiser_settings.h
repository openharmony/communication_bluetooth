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

#ifndef BLUETOOTH_PARCEL_BLE_ADVERTISER_SETTINGS_H
#define BLUETOOTH_PARCEL_BLE_ADVERTISER_SETTINGS_H

#include "ble_parcel_data.h"
#include "parcel.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleAdvertiserSettings : public Parcelable, public bluetooth::AdvertiserSettings {
public:
    explicit BluetoothBleAdvertiserSettings() = default;
    explicit BluetoothBleAdvertiserSettings(const bluetooth::AdvertiserSettings &other)
        : bluetooth::AdvertiserSettings(other)
    {}
    BluetoothBleAdvertiserSettings(const BluetoothBleAdvertiserSettings &other) : bluetooth::AdvertiserSettings(other)
    {}
    ~BluetoothBleAdvertiserSettings() override = default;

    bool Marshalling(Parcel &parcel) const override;
    static BluetoothBleAdvertiserSettings *Unmarshalling(Parcel &parcel);

    bool WriteToParcel(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_PARCEL_BLE_ADVERTISER_SETTINGS_H
