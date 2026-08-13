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

#ifndef BLUETOOTH_UTILS_SERVER_H
#define BLUETOOTH_UTILS_SERVER_H

#include <algorithm>
#include <set>
#include <sstream>
#include <string>

namespace OHOS {
namespace Bluetooth {
std::string GetEncryptAddr(std::string addr);
std::string GetScanModeName(int mode);
bool IsWalletService(const std::string& callingName, const uint32_t& tokenId);

#define GET_ENCRYPT_ADDR(device) (GetEncryptAddr((device).GetAddress()).c_str())
#define GET_ENCRYPT_GATT_ADDR(device) (GetEncryptAddr((device).addr_.GetAddress()).c_str())
#define GET_ENCRYPT_AVRCP_ADDR(rawAddr) (GetEncryptAddr((rawAddr).GetAddress()).c_str())

// HW Device type
#define HW_CAR_DEVICE_TYPE 105

template <typename T>
std::string ToLogString(std::set<T> vec)
{
    std::stringstream result;
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(result, " "));
    return result.str();
}
}  // namespace Bluetooth
}  // namespace OHOS

#endif // BLUETOOTH_UTILS_SERVER_H