/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#ifndef OHOS_BLUETOOTH_CONVERT_UTILS_H
#define OHOS_BLUETOOTH_CONVERT_UTILS_H
 
#include <cstdint>
#include <string>
#include <vector>
 
namespace OHOS {
namespace bluetooth {
 
const unsigned char MASK_80 = 0x80;
const unsigned char MASK_C0 = 0xC0;
const int MIN_BYTES = 2;
const int MAX_BYTES = 6;
 
class BluetoothCodeConvertUtils {
public:
    BluetoothCodeConvertUtils() = default;
    ~BluetoothCodeConvertUtils() = default;
 
    static bool IsUtf8(const std::string &str);
    static std::string AsciiToUtf8(const std::string &strAscii);
    static std::string AsciiAndUtf8MixedToUtf8(const std::string &strMixed);
private:
    static std::string Convert(const std::string &str, const std::string &fromCharset,
        const std::string &toCharset);
    static bool Utf8Check(const char *str, size_t length);
    static bool IsUtf8Char(unsigned char chr, int32_t &nBytes);
};
} // namespace bluetooth
} // namespace OHOS
#endif // OHOS_BLUETOOTH_CONVERT_UTILS_H