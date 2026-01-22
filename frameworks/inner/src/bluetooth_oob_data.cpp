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
#define LOG_TAG "bt_fwk_oob_data"
#endif

#include "bluetooth_oob_data.h"

namespace OHOS {
namespace Bluetooth {

std::vector<uint8_t> OobData::GetAddressWithType() const
{
    return addressWithType_;
}

void OobData::SetAddressWithType(std::vector<uint8_t> addressWithType)
{
    addressWithType_ = addressWithType;
    hasAddressWithType_ = true;
}

std::vector<uint8_t> OobData::GetConfirmationHash() const
{
    return confirmationHash_;
}

void OobData::SetConfirmationHash(std::vector<uint8_t> confirmationHash)
{
    confirmationHash_ = confirmationHash;
    hasConfirmHash_ = true;
}

std::vector<uint8_t> OobData::GetRandomizerHash() const
{
    return randomizerHash_;
}

void OobData::SetRandomizerHash(std::vector<uint8_t> randomizerHash)
{
    randomizerHash_ = randomizerHash;
    hasRandomHash_ = true;
}

std::string OobData::GetDeviceName() const
{
    return deviceName_;
}

void OobData::SetDeviceName(const std::string &deviceName)
{
    deviceName_ = deviceName;
    hasDeviceName_ = true;
}

uint8_t OobData::GetOobDataType() const
{
    return oobDataType_;
}

void OobData::SetOobDataType(const uint8_t OobDataType)
{
    oobDataType_ = OobDataType;
}

uint8_t OobData::GetDeviceRole() const
{
    return deviceRole_;
}

void OobData::SetDeviceRole(const uint8_t deviceRole)
{
    deviceRole_ = deviceRole;
}

bool OobData::HasOobData() const
{
    return hasConfirmHash_ && hasAddressWithType_; // 必选参数
}

bool OobData::HasConfirmHash() const
{
    return hasConfirmHash_;
}

bool OobData::HasRandomHash() const
{
    return hasRandomHash_;
}

bool OobData::HasDeviceName() const
{
    return hasDeviceName_;
}

bool OobData::HasAddressWithType() const
{
    return hasAddressWithType_;
}

}  // namespace Bluetooth
}  // namespace OHOS