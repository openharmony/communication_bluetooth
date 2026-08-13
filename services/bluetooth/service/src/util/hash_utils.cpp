/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_hash_util"
#endif

#include "hash_utils.h"
#include "openssl/evp.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {
std::string HashUtil::EncodeString(int hashAlgorithmType, const std::string& str)
{
    if (hashAlgorithmType == HASH_ALGORITHM_SHA256) {
        return ConvertSha256(str);
    }
    return "";
}

std::string HashUtil::ConvertSha256(const std::string& str)
{
    if (str.empty()) {
        return "";
    }
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen {};
    
    int ret = EVP_Digest(str.data(), str.length(), hash, &hashLen, EVP_sha256(), nullptr);
    if (ret != EVP_SUCCESS || hashLen != SHA256_SIZE) {
        return "";
    }
    // 将二进制哈希值转换为十六进制字符串返回
    std::string result;
    result.reserve(hashLen * HEX_CHARS_PER_BYTE);
    for (size_t i = 0; i < hashLen; ++i) {
        result += HEX_DIGITS[hash[i] >> 4];     // 取高四位
        result += HEX_DIGITS[hash[i] & 0x0F];   // 取低四位
    }
    return result;
}
}
}
