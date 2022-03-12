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

#include "bluetooth_bt_uuid.h"
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothGattCharacteristic::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint16(handle_)) {
        return false;
    }
    if (!parcel.WriteUint16(endHandle_)) {
        return false;
    }
    if (!parcel.WriteUint16(valueHandle_)) {
        return false;
    }
    if (!parcel.WriteInt32(properties_)) {
        return false;
    }
    if (!parcel.WriteInt32(permissions_)) {
        return false;
    }
    if (!parcel.WriteUint32(length_)) {
        return false;
    }
    for (size_t i = 0; i < length_; i++) {
        if (!parcel.WriteUint8(value_[i])) {
            return false;
        }
    }
    BluetoothUuid uuid = BluetoothUuid(uuid_);
    if (!parcel.WriteParcelable(&uuid)) {
        return false;
    }
    uint32_t size = descriptors_.size();
    if (!parcel.WriteUint32(size)) {
        return false;
    }
    for (auto &des : descriptors_) {
        BluetoothGattDescriptor descriptor = BluetoothGattDescriptor(des);
        if (!parcel.WriteParcelable(&descriptor)) {
            return false;
        }
    }
    return true;
}

BluetoothGattCharacteristic *BluetoothGattCharacteristic::Unmarshalling(Parcel &parcel)
{
    BluetoothGattCharacteristic *characteristic = new BluetoothGattCharacteristic();
    if (characteristic != nullptr && !characteristic->ReadFromParcel(parcel)) {
        delete characteristic;
        characteristic = nullptr;
    }
    return characteristic;
}

bool BluetoothGattCharacteristic::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothGattCharacteristic::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUint16(handle_)) {
        return false;
    }
    if (!parcel.ReadUint16(endHandle_)) {
        return false;
    }
    if (!parcel.ReadUint16(valueHandle_)) {
        return false;
    }
    if (!parcel.ReadInt32(properties_)) {
        return false;
    }
    if (!parcel.ReadInt32(permissions_)) {
        return false;
    }
    uint32_t length;
    if (!parcel.ReadUint32(length)) {
        return false;
    }
    length_ = length;
    for (size_t i = 0; i < length_; i++) {
        if (!parcel.ReadUint8(value_[i])) {
            return false;
        }
    }
    std::shared_ptr<BluetoothUuid> uuid(parcel.ReadParcelable<BluetoothUuid>());
    if (!uuid) {
        return false;
    }
    uuid_ = bluetooth::Uuid(*uuid);
    uint32_t size;
    if (!parcel.ReadUint32(size)) {
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        std::shared_ptr<BluetoothGattDescriptor> descriptor(parcel.ReadParcelable<BluetoothGattDescriptor>());
        if (!descriptor) {
            return false;
        }
        descriptors_.push_back(*descriptor);
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS
