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

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include <charconv>
#include <string>
#include "bt_uuid.h"
#include "log.h"
static const int INVALID_RANDOM_VALUE = 0;
static const int DEX_STRING_TO_INT = 10;
static const int HEX_STRING_TO_INT = 16;
namespace OHOS {
namespace bluetooth {
std::string GetEncryptAddr(std::string addr);

std::string GetEncryptPinCode(std::string pinCode);

std::string GetEncryptDeviceName(std::string deviceName);

std::string GetEncryptPinCode(int pinCode);
#define GET_ENCRYPT_ADDR(device) (GetEncryptAddr((device).GetAddress()).c_str())
#define GET_ENCRYPT_STR_ADDR(address) (GetEncryptAddr(address).c_str())
#define GET_ENCRYPT_DEVICE_NAME(deviceName) (GetEncryptDeviceName(deviceName).c_str())

bool IsValidAddr(const std::string &addr);
bool IsValidUuid(const std::string &uuid);
bool IsUuidSupport(std::vector<Uuid> &uuids, const std::string &uuidStr);

bool StartWith(const std::string &str, const std::string &prefix);
bool IsNumber(const std::string& str);
bool IsHexNumber(const std::string& str);
int32_t GenerateRandomNum(int32_t beginNum, int32_t endNum);
std::string GetCommandValue(const std::string &atCommand);
std::vector<std::string> SplitValueByDelim(const std::string &commandValue, const std::string &delim);
template <typename T>
bool ConvertStrToDigit(const std::string& str, T &ret, int base = DEX_STRING_TO_INT)
{
    std::from_chars_result res =
        std::from_chars(str.data(), str.data() + str.size(), ret, base);
    if (res.ec != std::errc{} || res.ptr != str.data() + str.size()) {
        HILOGE("FromString failed, error string is %{public}s", str.c_str());
        return false;
    }
    return true;
}
std::string TruncateString(const std::string &input, size_t maxLength);

int64_t GetTimeStamp(void);
}  // namespace bluetooth
}  // namespace OHOS

#endif // COMMON_UTIL_H