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

#include "bluetooth_oob_data_parcel.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothOobData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(hasAddressWithType_)) {
        return false;
    }
    if (!parcel.WriteUInt8Vector(addressWithType_)) {
        return false;
    }
    if (!parcel.WriteBool(hasConfirmHash_)) {
        return false;
    }
    if (!parcel.WriteUInt8Vector(confirmationHash_)) {
        return false;
    }
    if (!parcel.WriteBool(hasRandomHash_)) {
        return false;
    }
    if (!parcel.WriteUInt8Vector(randomizerHash_)) {
        return false;
    }
    if (!parcel.WriteBool(hasDeviceName_)) {
        return false;
    }
    if (!parcel.WriteString(deviceName_)) {
        return false;
    }
    if (!parcel.WriteUint8(oobDataType_)) {
        return false;
    }
    if (!parcel.WriteUint8(deviceRole_)) {
        return false;
    }
    return true;
}

BluetoothOobData *BluetoothOobData::Unmarshalling(Parcel &parcel)
{
    BluetoothOobData *oobData = new BluetoothOobData();
    if (oobData != nullptr && !oobData->ReadFromParcel(parcel)) {
        delete oobData;
        oobData = nullptr;
    }
    return oobData;
}

bool BluetoothOobData::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothOobData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadBool(hasAddressWithType_)) {
        return false;
    }
    if (!parcel.ReadUInt8Vector(&addressWithType_)) {
        return false;
    }
    if (!parcel.ReadBool(hasConfirmHash_)) {
        return false;
    }
    if (!parcel.ReadUInt8Vector(&confirmationHash_)) {
        return false;
    }
    if (!parcel.ReadBool(hasRandomHash_)) {
        return false;
    }
    if (!parcel.ReadUInt8Vector(&randomizerHash_)) {
        return false;
    }
    if (!parcel.ReadBool(hasDeviceName_)) {
        return false;
    }
    if (!parcel.ReadString(deviceName_)) {
        return false;
    }
    if (!parcel.ReadUint8(oobDataType_)) {
        return false;
    }
    if (!parcel.ReadUint8(deviceRole_)) {
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS