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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_parcel_uuid"
#endif

#include "parcel_bt_uuid.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
bool ParcelBtUuid::WriteToParcel(MessageParcel &parcel, bluetooth::Uuid &uuid)
{
    bluetooth::Uuid::UUID128Bit uuid128 = uuid.ConvertTo128Bits();

    uint64_t mostSigBits =
        (static_cast<uint64_t>(uuid128[0]) << 56) | (static_cast<uint64_t>(uuid128[1]) << 48) |
            (static_cast<uint64_t>(uuid128[2]) << 40) | (static_cast<uint64_t>(uuid128[3]) << 32) |
            (static_cast<uint64_t>(uuid128[4]) << 24) | (static_cast<uint64_t>(uuid128[5]) << 16) |
            (static_cast<uint64_t>(uuid128[6]) << 8) | static_cast<uint64_t>(uuid128[7]);

    uint64_t leastSigBits =
        (static_cast<uint64_t>(uuid128[8]) <<56) | (static_cast<uint64_t>(uuid128[9]) << 48) |
            (static_cast<uint64_t>(uuid128[10]) << 40) | (static_cast<uint64_t>(uuid128[11]) << 32) |
            (static_cast<uint64_t>(uuid128[12]) << 24) | (static_cast<uint64_t>(uuid128[13]) << 16) |
            (static_cast<uint64_t>(uuid128[14]) << 8) | static_cast<uint64_t>(uuid128[15]);

    bool ret = parcel.WriteUint64(mostSigBits);
    if (!ret) {
        HILOGE("ParcelBtUuid::WriteToParcel write mostSigBits error");
        return ret;
    }

    return parcel.WriteUint64(leastSigBits);
}

bluetooth::Uuid ParcelBtUuid::ReadFromParcel(MessageParcel &parcel)
{
    uint64_t mostSigBits = parcel.ReadUint64();
    uint64_t leastSigBits = parcel.ReadUint64();

    return bluetooth::Uuid::ConvertFromMostAndLeastBit(mostSigBits, leastSigBits);
}
}  // namespace Bluetooth
}  // namespace OHOS