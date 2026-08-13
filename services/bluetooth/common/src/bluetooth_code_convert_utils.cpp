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
#include <unicode/ucnv.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <securec.h>
#include "bluetooth_code_convert_utils.h"
 
namespace OHOS {
namespace bluetooth {
bool BluetoothCodeConvertUtils::IsUtf8(const std::string &str)
{
    return Utf8Check(str.c_str(), str.length());
}

__attribute__((no_sanitize("shift-base")))
bool BluetoothCodeConvertUtils::IsUtf8Char(unsigned char chr, int32_t &nBytes)
{
    if (nBytes == 0) {
        if ((chr & MASK_80) == 0) {
            return true;
        }
        while ((chr & MASK_80) == MASK_80) {
            chr <<= 1;
            nBytes++;
        }
 
        if (nBytes < MIN_BYTES || nBytes > MAX_BYTES) {
            return false;
        }
        nBytes--;
    } else {
        if ((chr & MASK_C0) != MASK_80) {
            return false;
        }
        nBytes--;
    }
    return true;
}
 
bool BluetoothCodeConvertUtils::Utf8Check(const char *str, size_t length)
{
    size_t i = 0;
    int32_t nBytes = 0;
    unsigned char chr = 0;
    while (i < length) {
        chr = *(str + i);
        if (!IsUtf8Char(chr, nBytes)) {
            return false;
        }
        i++;
    }
    return true;
}
 
std::string BluetoothCodeConvertUtils::Convert(const std::string &str, const std::string &fromCharset,
    const std::string &toCharset)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t resultlen  = ucnv_convert(toCharset.c_str(), fromCharset.c_str(), nullptr, 0, str.c_str(),
        str.length(), &status);
    std::unique_ptr<char[]> result = std::make_unique<char[]>(resultlen + 1);
    memset_s(result.get(), resultlen + 1, 0, resultlen + 1);
    status = U_ZERO_ERROR;
    ucnv_convert(toCharset.c_str(), fromCharset.c_str(), result.get(), resultlen + 1,
        str.c_str(), str.length(), &status);
    if (U_FAILURE(status)) {
        return str;
    }
    return std::string(result.get());
}
 
std::string BluetoothCodeConvertUtils::AsciiToUtf8(const std::string &strAsc)
{
    if (strAsc.length() == 0 || IsUtf8(strAsc)) {
        return strAsc;
    }
    std::string result = Convert(strAsc, "ASCII", "utf8");
    if (result.length() == 0) {
        return strAsc;
    }
    return result;
}

std::string BluetoothCodeConvertUtils::AsciiAndUtf8MixedToUtf8(const std::string &strMixed)
{
    if (strMixed.length() == 0 || IsUtf8(strMixed)) {
        return strMixed;
    }
    // Ascii字符本身就是UTF8编码的一部分，将不完全满足utf8格式的字串需经转换器改为合法格式
    std::string result = Convert(strMixed, "utf8", "utf8");
    if (result.length() == 0) {
        return strMixed;
    }
    return result;
}
} // namespace bluetooth
} // namespace OHOS