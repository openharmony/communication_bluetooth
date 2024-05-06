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

#include "bluetooth_remote_device_info.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothRemoteDeviceInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint16(vendorId_)) {
        return false;
    }
    if (!parcel.WriteUint16(productId_)) {
        return false;
    }
    if (!parcel.WriteString(modelId_)) {
        return false;
    }
    if (!parcel.WriteInt32(customType_)) {
        return false;
    }
    return true;
}

BluetoothRemoteDeviceInfo *BluetoothRemoteDeviceInfo::Unmarshalling(Parcel &parcel)
{
    BluetoothRemoteDeviceInfo *info = new BluetoothRemoteDeviceInfo();
    if (info != nullptr && !info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool BluetoothRemoteDeviceInfo::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothRemoteDeviceInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUint16(vendorId_)) {
        return false;
    }
    if (!parcel.ReadUint16(productId_)) {
        return false;
    }
    if (!parcel.ReadString(modelId_)) {
        return false;
    }
    if (!parcel.ReadInt32(customType_)) {
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS