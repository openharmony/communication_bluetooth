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

#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {

bool BluetoothUuid::Marshalling(Parcel &parcel) const
{
    uint64_t most_sig_bits =
        ((((uint64_t)uuid_[0]) << 56) | (((uint64_t)uuid_[1]) << 48) | (((uint64_t)uuid_[2]) << 40) |
            (((uint64_t)uuid_[3]) << 32) | (((uint64_t)uuid_[4]) << 24) | (((uint64_t)uuid_[5]) << 16) |
            (((uint64_t)uuid_[6]) << 8) | uuid_[7]);

    uint64_t least_sig_bits =
        ((((uint64_t)uuid_[8]) << 56) | (((uint64_t)uuid_[9]) << 48) | (((uint64_t)uuid_[10]) << 40) |
            (((uint64_t)uuid_[11]) << 32) | (((uint64_t)uuid_[12]) << 24) | (((uint64_t)uuid_[13]) << 16) |
            (((uint64_t)uuid_[14]) << 8) | uuid_[15]);

    bool ret = parcel.WriteUint64(most_sig_bits);
    if (!ret) {
        HILOGE("ParcelBtUuid::WriteToParcel write most_sig_bits error");
        return ret;
    }

    return parcel.WriteUint64(least_sig_bits);
}

BluetoothUuid *BluetoothUuid::Unmarshalling(Parcel &parcel)
{
    uint64_t most_sig_bits = parcel.ReadUint64();
    uint64_t least_sig_bits = parcel.ReadUint64();
    BluetoothUuid *btuuid =
        new BluetoothUuid(bluetooth::Uuid::ConvertFromMostAndLeastBit(most_sig_bits, least_sig_bits));
    return btuuid;
}
}  // namespace Bluetooth
}  // namespace OHOS