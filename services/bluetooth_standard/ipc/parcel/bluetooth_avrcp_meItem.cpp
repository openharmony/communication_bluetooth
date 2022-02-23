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

#include "bluetooth_avrcp_meItem.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothAvrcpMeItem::Marshalling(Parcel &parcel) const
{
    bool status = parcel.WriteUint8(itemType_);
    if (!status) {
        return status;
    }
    status = parcel.WriteUint64(uid_);
    if (!status) {
        return status;
    }
    status = parcel.WriteUint8(type_);
    if (!status) {
        return status;
    }
    status = parcel.WriteUint8(playable_);
    if (!status) {
        return status;
    }
    status = parcel.WriteString(name_);
    if (!status) {
        return status;
    }

    status = parcel.WriteInt32(attributes_.size());
    if (!status) {
        return false;
    }
    for (auto &attribute : attributes_) {
        status = parcel.WriteUint32(attribute);
        if (!status) {
            return false;
        }
    }

    status = parcel.WriteInt32(values_.size());
    if (!status) {
        return false;
    }
    for (auto &value : values_) {
       status = parcel.WriteString(value);
        if (!status) {
            return status;
        }
    }

    return true;
}

bool BluetoothAvrcpMeItem::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

BluetoothAvrcpMeItem *BluetoothAvrcpMeItem::Unmarshalling(Parcel &parcel)
{
    BluetoothAvrcpMeItem *avrcpData = new BluetoothAvrcpMeItem();
    if (avrcpData != nullptr && !avrcpData->ReadFromParcel(parcel)) {
        delete avrcpData;
        avrcpData = nullptr;
    }

    return avrcpData;
}

bool BluetoothAvrcpMeItem::ReadFromParcel(Parcel &parcel)
{
    uint8_t value = 0;
    bool status = parcel.ReadUint8(value);
    if (!status) {
        return status;
    }
    itemType_ = value;

    uint64_t uid = 0;
    status = parcel.ReadUint64(uid);
    if (!status) {
        return status;
    }
    uid_ = uid;

    value = 0;
    status = parcel.ReadUint8(value);
    if (!status) {
        return status;
    }
    type_ = value;

    value = 0;
    status = parcel.ReadUint8(value);
    if (!status) {
        return status;
    }
    playable_ = value;

    status = parcel.ReadString(name_);
    if (!status) {
        return status;
    }

    int featureSize = 0;
    int32_t myFeature;
    status = parcel.ReadInt32(featureSize);
    if (!status) {
        return status;
    }
    for (int i = 0; i < featureSize; ++i) {
        status = parcel.ReadInt32(myFeature);
        if (!status) {
            return status;
        }
        attributes_.push_back(myFeature);
    }

    int valueSize = 0;
    std::string myValue;
    status = parcel.ReadInt32(valueSize);
    if (!status) {
        return status;
    }
    for (int i = 0; i < valueSize; ++i) {
        status = parcel.ReadString(myValue);
        if (!status) {
            return status;
        }
        values_.push_back(myValue);
    }

    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS