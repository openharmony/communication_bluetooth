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

#include "bt_oob_data.h"

namespace OHOS {
namespace bluetooth {

std::vector<uint8_t> BtOobData::GetAddressWithType() const
{
    return addressWithType_;
}

void BtOobData::SetAddressWithType(std::vector<uint8_t> addressWithType)
{
    addressWithType_ = addressWithType;
    hasAddressWithType_ = true;
}

std::vector<uint8_t> BtOobData::GetConfirmationHash() const
{
    return confirmationHash_;
}

void BtOobData::SetConfirmationHash(std::vector<uint8_t> confirmationHash)
{
    confirmationHash_ = confirmationHash;
    hasConfirmHash_ = true;
}

std::vector<uint8_t> BtOobData::GetRandomizerHash() const
{
    return randomizerHash_;
}

void BtOobData::SetRandomizerHash(std::vector<uint8_t> randomizerHash)
{
    randomizerHash_ = randomizerHash;
    hasRandomHash_ = true;
}

std::string BtOobData::GetDeviceName() const
{
    return deviceName_;
}

void BtOobData::SetDeviceName(const std::string &deviceName)
{
    deviceName_ = deviceName;
    hasDeviceName_ = true;
}

uint8_t BtOobData::GetOobDataType() const
{
    return oobDataType_;
}

void BtOobData::SetOobDataType(const uint8_t oobDataType)
{
    oobDataType_ = oobDataType;
}

uint8_t BtOobData::GetDeviceRole() const
{
    return deviceRole_;
}

void BtOobData::SetDeviceRole(const uint8_t deviceRole)
{
    deviceRole_ = deviceRole;
}

bool BtOobData::HasOobData() const
{
    return hasConfirmHash_ && hasAddressWithType_;
}

bool BtOobData::HasConfirmHash() const
{
    return hasConfirmHash_;
}

bool BtOobData::HasRandomHash() const
{
    return hasRandomHash_;
}

bool BtOobData::HasDeviceName() const
{
    return hasDeviceName_;
}

bool BtOobData::HasAddressWithType() const
{
    return hasAddressWithType_;
}

}  // namespace bluetooth
}  // namespace OHOS