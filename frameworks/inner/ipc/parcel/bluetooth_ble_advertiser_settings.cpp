/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "bluetooth_ble_advertiser_settings.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothBleAdvertiserSettings::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(connectable_)) {
        return false;
    }
    if (!parcel.WriteBool(legacyMode_)) {
        return false;
    }
    if (!parcel.WriteUint16(interval_)) {
        return false;
    }
    if (!parcel.WriteInt8(txPower_)) {
        return false;
    }
    if (!parcel.WriteInt32(primaryPhy_)) {
        return false;
    }
    if (!parcel.WriteInt32(secondaryPhy_)) {
        return false;
    }
    std::string addr(ownAddr_.begin(), ownAddr_.end());
    if (!parcel.WriteString(addr)) {
        return false;
    }
    if (!parcel.WriteInt8(ownAddrType_)) {
        return false;
    }
    return true;
}

BluetoothBleAdvertiserSettings *BluetoothBleAdvertiserSettings::Unmarshalling(Parcel &parcel)
{
    BluetoothBleAdvertiserSettings *settings = new BluetoothBleAdvertiserSettings();
    if (settings != nullptr && !settings->ReadFromParcel(parcel)) {
        delete settings;
        settings = nullptr;
    }
    return settings;
}

bool BluetoothBleAdvertiserSettings::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothBleAdvertiserSettings::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadBool(connectable_)) {
        return false;
    }
    if (!parcel.ReadBool(legacyMode_)) {
        return false;
    }
    if (!parcel.ReadUint16(interval_)) {
        return false;
    }
    if (!parcel.ReadInt8(txPower_)) {
        return false;
    }
    if (!parcel.ReadInt32(primaryPhy_)) {
        return false;
    }
    if (!parcel.ReadInt32(secondaryPhy_)) {
        return false;
    }
    std::string address = "";
    if (parcel.ReadString(address) && address.size() == bluetooth::RawAddress::BT_ADDRESS_BYTE_LEN) {
        std::copy(address.begin(), address.end(), ownAddr_.begin());
    } else {
        return false;
    }
    if (!parcel.ReadInt8(ownAddrType_)) {
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS
