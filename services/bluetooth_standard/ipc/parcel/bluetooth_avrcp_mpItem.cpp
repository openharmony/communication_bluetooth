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
bool BluetoothAvrcpMpItem::Marshalling(Parcel &parcel) const
{
    bool status = parcel.WriteUint8(itemType_);
    if (!status) {
        return status;
    }
    status = parcel.WriteInt32(playerId_);
    if (!status) {
        return status;
    }
    status = parcel.WriteUint8(majorType_);
    if (!status) {
        return status;
    }
    status = parcel.WriteInt32(subType_);
    if (!status) {
        return status;
    }
    status = parcel.WriteUint8(playStatus_);
    if (!status) {
        return status;
    }

    status = parcel.WriteInt32(features_.size());
    if (!status) {
        return false;
    }
    for (auto &attribute : features_) {
        status = parcel.WriteUint8(attribute);
        if (!status) {
            return false;
        }
    }

    status = parcel.WriteString(name_);

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
    uint8_t int8value = 0;
    bool status = parcel.ReadUint8(int8value);
    if (!status) {
        return status;
    }
    itemType_ = int8value;

    int32_t value = 0;
    status = parcel.ReadInt32(value);
    if (!status) {
        return status;
    }      
    playerId_ = (uint16_t)value;

    int8value = 0;
    status = parcel.ReadUint8(int8value);
    if (!status) {
        return status;
    }
    majorType_ = int8value;

    value = 0;
    status = parcel.ReadInt32(value);
    if (!status) {
        return status;
    }   
    subType_ = value;

    int8value = 0;
    status = parcel.ReadUint8(int8value);
    if (!status) {
        return status;
    }
    playStatus_ = int8value;

    int featureSize = 0;
    uint8_t myFeature;
    status = parcel.ReadInt32(featureSize);
    if (!status) {
        return status;
    }
    for (int i = 0; i < featureSize; ++i) {
        status = parcel.ReadUint8(myFeature);
        if (!status) {
            return status;
        }
        features_.push_back(myFeature);
    }

    status = parcel.ReadString(name_);
    if (!status) {
        return status;
    }

    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS