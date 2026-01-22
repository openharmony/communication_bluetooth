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

#ifndef BLUETOOTH_ADDRESS_INFO_H
#define BLUETOOTH_ADDRESS_INFO_H

#include "bluetooth_def.h"

namespace OHOS {
namespace Bluetooth {
class BLUETOOTH_API AddressInfo {
public:
    AddressInfo() = default;

    ~AddressInfo() = default;

    std::string GetAddress() const;

    void SetAddress(const std::string &address);

    bool HasAddressType() const;

    uint8_t GetAddressType() const;

    void SetAddressType(const uint8_t addressType);

    bool HasRawAddressType() const;

    uint8_t GetRawAddressType() const;

    void SetRawAddressType(const uint8_t rawAddressType);

private:
    std::string address_ = {};
    bool hasAddressType_ = false;
    uint8_t addressType_ = AddressType::UNSET_ADDRESS; // real or virtual
    bool hasRawAddressType_ = false;
    uint8_t rawAddressType_ = RawAddressType::UNSET_RAW_ADDRESS; // public or random
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_ADDRESS_INFO_H