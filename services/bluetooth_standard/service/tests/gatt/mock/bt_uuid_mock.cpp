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

#include "bt_uuid.h"
#include <limits.h>
#include <algorithm>

namespace bluetooth {

int Uuid::GetUuidType() const
{
    for (int i = UUID32_BYTES_TYPE; i < UUID128_BYTES_TYPE; i++) {
        if (BASE_UUID[i] != uuid_[i]) {
            return UUID128_BYTES_TYPE;
        }
    }

    if (uuid_[0] == 0 && uuid_[1] == 0) {
        return UUID16_BYTES_TYPE;
    }

    return UUID32_BYTES_TYPE;
}

Uuid Uuid::ConvertFrom16Bits(uint16_t uuid)
{
    Uuid tmp;
    tmp.uuid_[2] = (uint8_t)((uuid & 0xFF00) >> 8);
    tmp.uuid_[3] = (uint8_t)(uuid & 0x00FF);
    return tmp;
}

bool Uuid::ConvertToBytesLE(uint8_t *value, const size_t size) const
{
    UUID128Bit le;
    std::reverse_copy(uuid_.data(), uuid_.data() + UUID128_BYTES_TYPE, le.begin());
    memcpy(value, le.data(), UUID128_BYTES_TYPE);
    return true;
}

uint16_t Uuid::ConvertTo16Bits() const
{
    uint16_t ret = uuid_[2] & 0xFF;
    ret = (ret << 8) + uuid_[3];
    return ret;
}

uint32_t Uuid::ConvertTo32Bits() const
{
    uint32_t ret = 0;
    for (int i = 0; i < 4; i++) {
        ret = (ret << 8) + uuid_[i];
    }
    return ret;
}

bool Uuid::operator==(const Uuid &rhs) const
{
    return (uuid_ == rhs.uuid_);
}

Uuid Uuid::ConvertFromBytesLE(const uint8_t *uuid, const size_t size)
{
    Uuid leUuid;
    UUID128Bit le;
    memcpy(le.data(), uuid, UUID128_BYTES_TYPE);
    std::reverse_copy(le.data(), le.data() + UUID128_BYTES_TYPE, leUuid.uuid_.begin());
    return leUuid;
}

Uuid Uuid::ConvertFrom32Bits(uint32_t uuid)
{
    Uuid tmp;
    tmp.uuid_[0] = (uint8_t)((uuid & 0xFF000000) >> 24);
    tmp.uuid_[1] = (uint8_t)((uuid & 0x00FF0000) >> 16);
    tmp.uuid_[2] = (uint8_t)((uuid & 0x0000FF00) >> 8);
    tmp.uuid_[3] = (uint8_t)(uuid & 0x000000FF);
    return tmp;
}

Uuid Uuid::ConvertFrom128Bits(const UUID128Bit &uuid)
{
    Uuid tmp;
    tmp.uuid_ = uuid;
    return tmp;
}

Uuid Uuid::ConvertFromString(const std::string &name)
{
    Uuid ret;
    std::string tmp = name;
    std::size_t pos = tmp.find("-");

    while (pos != std::string::npos) {
        tmp.replace(pos, 1, "");
        pos = tmp.find("-");
    }

    for (std::size_t i = 0; (i + 1) < tmp.length();) {
        ret.uuid_[i / 2] = std::stoi(tmp.substr(i, 2), nullptr, 16);
        i += 2;
    }

    return ret;
}

Uuid::UUID128Bit Uuid::ConvertTo128Bits() const
{
    return uuid_;
}

}  // namespace bluetooth