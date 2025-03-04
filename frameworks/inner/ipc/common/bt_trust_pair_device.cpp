/*
 * Copyright (C) Huawei Device Co., Ltd. 2025-2025. All rights reserved.
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

#include "bt_trust_pair_device.h"

namespace OHOS {
namespace bluetooth {
void TrustPairDevice::SetMacAddress(const std::string &mac)
{
    macAddress_ = mac;
}

std::string TrustPairDevice::GetMacAddress() const
{
    return macAddress_;
}

void TrustPairDevice::SetBluetoothClass(int32_t bluetoothClass)
{
    bluetoothClass_ = bluetoothClass;
}

int32_t TrustPairDevice::GetBluetoothClass() const
{
    return bluetoothClass_;
}

void TrustPairDevice::SetDeviceName(const std::string &deviceName)
{
    deviceName_ = deviceName;
}

std::string TrustPairDevice::GetDeviceName() const
{
    return deviceName_;
}

void TrustPairDevice::SetToken(const std::vector<uint8_t> &token)
{
    token_ = token;
}

std::vector<uint8_t> TrustPairDevice::GetToken()
{
    return token_;
}

void TrustPairDevice::SetUuid(const std::vector<std::string> &uuids)
{
    uuids_ = uuids;
}

std::vector<std::string> TrustPairDevice::GetUuid() const
{
    return uuids_;
}

std::vector<bluetooth::Uuid> TrustPairDevice::GetParseUuid()
{
    std::vector<bluetooth::Uuid> uuidsExt;
    uuidsExt.reserve(uuids_.size());
    for (const auto &str : uuids_) {
        uuidsExt.emplace_back(bluetooth::Uuid::ConvertFromString(str));
    }
    return uuidsExt;
}

void TrustPairDevice::SetSecureAdvertisingInfo(const std::vector<uint8_t> &randomDeviceInfo)
{
    secureAdvertisingInfo_ = randomDeviceInfo;
}

std::vector<uint8_t> TrustPairDevice::GetSecureAdvertisingInfo()
{
    return secureAdvertisingInfo_;
}
}  // namespace bluetooth
}  // namespace OHOS
