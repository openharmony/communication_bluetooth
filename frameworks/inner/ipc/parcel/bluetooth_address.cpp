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

#include "bluetooth_log.h"
#include "bluetooth_address.h"

namespace OHOS {
namespace Bluetooth {

BluetoothAddress::BluetoothAddress(const std::string &address, const int32_t &addressType)
{
    addr_ = address;
    addrType_ = addressType;
}
    
bool BluetoothAddress::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(addr_)) {
        return false;
    }
    if (!parcel.WriteInt32(addrType_)) {
        return false;
    }
    return true;
}

BluetoothAddress *BluetoothAddress::Unmarshalling(Parcel &parcel)
{
    BluetoothAddress *bluetoothAddress = new BluetoothAddress();
    if (bluetoothAddress != nullptr && !bluetoothAddress->ReadFromParcel(parcel)) {
        delete bluetoothAddress;
        bluetoothAddress = nullptr;
    }
    return bluetoothAddress;
}

bool BluetoothAddress::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothAddress::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(addr_)) {
        return false;
    }
    if (!parcel.ReadInt32(addrType_)) {
        return false;
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS