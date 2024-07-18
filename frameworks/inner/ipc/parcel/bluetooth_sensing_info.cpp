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

#include "bluetooth_log.h"
#include "bluetooth_sensing_info.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothSensingInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(addr_)) {
        return false;
    }
    if (!parcel.WriteString(uuid_)) {
        return false;
    }
    if (!parcel.WriteUint32(resourceId_)) {
        return false;
    }
    if (!parcel.WriteString(pkgName_)) {
        return false;
    }
    if (!parcel.WriteBool(isServer_)) {
        return false;
    }
    if (!parcel.WriteUint16(interval_)) {
        return false;
    }
    return true;
}

BluetoothSensingInfo *BluetoothSensingInfo::Unmarshalling(Parcel &parcel)
{
    BluetoothSensingInfo *sensingInfo = new BluetoothSensingInfo();
    if (sensingInfo != nullptr && !sensingInfo->ReadFromParcel(parcel)) {
        delete sensingInfo;
        sensingInfo = nullptr;
    }
    return sensingInfo;
}

bool BluetoothSensingInfo::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothSensingInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(addr_)) {
        return false;
    }
    if (!parcel.ReadString(uuid_)) {
        return false;
    }
    if (!parcel.ReadUint32(resourceId_)) {
        return false;
    }
    if (!parcel.ReadString(pkgName_)) {
        return false;
    }
    if (!parcel.ReadBool(isServer_)) {
        return false;
    }
    if (!parcel.ReadUint16(interval_)) {
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS