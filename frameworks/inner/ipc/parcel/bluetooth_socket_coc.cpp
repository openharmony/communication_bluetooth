/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "bluetooth_socket_coc.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothSocketCocInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(addr)) {
        return false;
    }
    if (!parcel.WriteInt32(minInterval)) {
        return false;
    }
    if (!parcel.WriteInt32(maxInterval)) {
        return false;
    }
    if (!parcel.WriteInt32(peripheralLatency)) {
        return false;
    }
    if (!parcel.WriteInt32(supervisionTimeout)) {
        return false;
    }
    if (!parcel.WriteInt32(minConnEventLen)) {
        return false;
    }
    if (!parcel.WriteInt32(maxConnEventLen)) {
        return false;
    }
    return true;
}

bool BluetoothSocketCocInfo::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothSocketCocInfo *BluetoothSocketCocInfo::Unmarshalling(Parcel &parcel)
{
    BluetoothSocketCocInfo *codecData = new BluetoothSocketCocInfo();
    if (codecData != nullptr && !codecData->ReadFromParcel(parcel)) {
        delete codecData;
        codecData = nullptr;
    }
    return codecData;
}

bool BluetoothSocketCocInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(addr)) {
        return false;
    }
    if (!parcel.ReadInt32(minInterval)) {
        return false;
    }
    if (!parcel.ReadInt32(maxInterval)) {
        return false;
    }
    if (!parcel.ReadInt32(peripheralLatency)) {
        return false;
    }
    if (!parcel.ReadInt32(supervisionTimeout)) {
        return false;
    }
    if (!parcel.ReadInt32(minConnEventLen)) {
        return false;
    }
    if (!parcel.ReadInt32(maxConnEventLen)) {
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS