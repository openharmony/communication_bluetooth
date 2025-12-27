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
#ifndef LOG_TAG
#define LOG_TAG "bt_fwk_address_info"
#endif

#include "bluetooth_address_info.h"

namespace OHOS {
namespace Bluetooth {

std::string AddressInfo::GetAddress() const
{
    return address_;
}

void AddressInfo::SetAddress(const std::string &address)
{
    address_ = address;
}

bool AddressInfo::HasAddressType() const
{
    return hasAddressType_;
}

uint8_t AddressInfo::GetAddressType() const
{
    return addressType_;
}

void AddressInfo::SetAddressType(const uint8_t addressType)
{
    addressType_ = addressType;
    hasAddressType_ = true;
}

bool AddressInfo::HasRawAddressType() const
{
    return hasRawAddressType_;
}

uint8_t AddressInfo::GetRawAddressType() const
{
    return rawAddressType_;
}

void AddressInfo::SetRawAddressType(const uint8_t rawAddressType)
{
    rawAddressType_ = rawAddressType;
    hasRawAddressType_ = true;
}

}  // namespace Bluetooth
}  // namespace OHOS