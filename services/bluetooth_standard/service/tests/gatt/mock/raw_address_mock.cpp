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

#include "raw_address.h"
#include <cstring>
#include <vector>

namespace bluetooth {

RawAddress RawAddress::ConvertToString(const uint8_t *src)
{
    char token[BT_ADDRESS_STR_LEN + 1] = {0};
    sprintf(token, "%02X:%02X:%02X:%02X:%02X:%02X", src[5], src[4], src[3], src[2], src[1], src[0]);
    return RawAddress(token);
}

void RawAddress::ConvertToUint8(uint8_t *dst, const size_t size) const
{}

bool RawAddress::operator<(const RawAddress &rhs) const
{
    return (address_.compare(rhs.address_) < 0);
}

bool RawAddress::operator==(const RawAddress &rhs) const
{
    return (address_.compare(rhs.address_) == 0);
}

}  // namespace bluetooth