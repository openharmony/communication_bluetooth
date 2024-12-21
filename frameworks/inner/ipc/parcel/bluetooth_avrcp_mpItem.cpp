/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "bluetooth_avrcp_mpItem.h"

namespace OHOS {
namespace Bluetooth {
const uint32_t AVRCP_READ_FROM_PARCEL_COUNT_MAX = 0xFFFF;
bool BluetoothAvrcpMpItem::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint8(avrcpMpItem_.itemType_)) {
        return false;
    }
    if (!parcel.WriteUint16(avrcpMpItem_.playerId_)) {
        return false;
    }
    if (!parcel.WriteUint8(avrcpMpItem_.majorType_)) {
        return false;
    }
    if (!parcel.WriteUint32(avrcpMpItem_.subType_)) {
        return false;
    }
    if (!parcel.WriteUint8(avrcpMpItem_.playStatus_)) {
        return false;
    }
    if (!parcel.WriteUint32(avrcpMpItem_.features_.size())) {
        return false;
    }
    for (auto &feature : avrcpMpItem_.features_) {
        if (!parcel.WriteUint8(feature)) {
            return false;
        }
    }
    if (!parcel.WriteString(avrcpMpItem_.name_)) {
        return false;
    }
    return true;
}

bool BluetoothAvrcpMpItem::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothAvrcpMpItem *BluetoothAvrcpMpItem::Unmarshalling(Parcel &parcel)
{
    BluetoothAvrcpMpItem *avrcpData = new BluetoothAvrcpMpItem();
    if (avrcpData != nullptr && !avrcpData->ReadFromParcel(parcel)) {
        delete avrcpData;
        avrcpData = nullptr;
    }
    return avrcpData;
}

bool BluetoothAvrcpMpItem::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUint8(avrcpMpItem_.itemType_)) {
        return false;
    }
    if (!parcel.ReadUint16(avrcpMpItem_.playerId_)) {
        return false;
    }
    if (!parcel.ReadUint8(avrcpMpItem_.majorType_)) {
        return false;
    }
    if (!parcel.ReadUint32(avrcpMpItem_.subType_)) {
        return false;
    }
    if (!parcel.ReadUint8(avrcpMpItem_.playStatus_)) {
        return false;
    }
    uint32_t size = 0;
    if (!parcel.ReadUint32(size) || size > AVRCP_READ_FROM_PARCEL_COUNT_MAX) {
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        uint8_t feature;
        if (!parcel.ReadUint8(feature)) {
            return false;
        }
        avrcpMpItem_.features_.push_back(feature);
    }
    if (!parcel.ReadString(avrcpMpItem_.name_)) {
        return false;
    }
    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS
