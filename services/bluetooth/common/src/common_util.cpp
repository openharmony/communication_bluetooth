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
#ifndef LOG_TAG
#define LOG_TAG "bt_common_util"
#endif

#include "bt_def.h"
#include "common_util.h"

#include <fcntl.h>
#include <unistd.h>
#include <regex>

using namespace std;
namespace OHOS {
namespace bluetooth {
constexpr int START_POS = 6;
constexpr int END_POS = 13;
constexpr int PINCODE_START_POS = 2;
constexpr int PINCODE_END_POS = 4;
constexpr int PINCODE_SIZE = 6;
constexpr size_t DEVICENAME_START_POS = 2;
const uint32_t UINT8_MAX_VALUE_LENGTH = 3;
const uint32_t INT32_MAX_VALUE_LENGTH = 10;
constexpr const char *NULL_ADDRESS {""};
constexpr const char *EMPTY_ADDRESS {"00:00:00:00:00:00"};
static const int INVALID_FD = -1;
static constexpr int UUID_STRING_LENGTH = 36;
static constexpr bool IS_DASH_POSITION[UUID_STRING_LENGTH] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

std::string GetEncryptAddr(std::string addr)
{
    if (addr.length() != ADDRESS_LENGTH) {
        HILOGE("addr is invalid.");
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    // 00:01:**:**:**:05
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

std::string GetEncryptPinCode(std::string pinCode)
{
    if (pinCode.length() != PINCODE_SIZE) {
        HILOGE("pinCode is invalid.");
        return std::string("");
    }
    std::string out = pinCode;
    for (int i = PINCODE_START_POS; i <= PINCODE_END_POS; i++) {
        out[i] = '*';
    }
    return out;
}

std::string GetEncryptPinCode(int pinCode)
{
    string pinNum = std::to_string(pinCode);
    while (pinNum.size() < PINCODE_SIZE) {
        pinNum = "0" + pinNum;
    }
    for (int i = PINCODE_START_POS; i <= PINCODE_END_POS; i++) {
        pinNum[i] = '*';
    }
    return pinNum;
}

std::string GetEncryptDeviceName(std::string deviceName)
{
    if (deviceName.length() <= 0) {
        HILOGD("name is invalid.");
        return std::string("");
    }
    std::string out = deviceName;
    // * or **
    if (deviceName.length() <= DEVICENAME_START_POS) {
        for (size_t i = 0; i <= deviceName.length() - 1; i++) {
            out[i] = '*';
        }
        return out;
    }
    // A*******T
    for (size_t i = 1; i <= deviceName.length() - DEVICENAME_START_POS; i++) {
        out[i] = '*';
    }
    return out;
}

bool IsValidAddr(const std::string &addr)
{
    if (addr == NULL_ADDRESS || addr == EMPTY_ADDRESS || addr.length() != ADDRESS_LENGTH) {
        HILOGE("empty addr");
        return false;
    }
    for (size_t i = 0; i < ADDRESS_LENGTH; i++) {
        char c = addr[i];
        switch (i % ADDRESS_SEPARATOR_UNIT) {
            case 0:
            case 1:
                if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    break;
                }
                return false;
            case ADDRESS_COLON_INDEX:
            default:
                if (c == ':') {
                    break;
                }
                return false;
        }
    }
    return true;
}

bool IsValidUuid(const std::string& uuid)
{
    if (uuid.empty() || uuid.length() != UUID_STRING_LENGTH) {
        HILOGE("uuid is empty.");
        return false;
    }
    for (int i = 0; i < UUID_STRING_LENGTH; ++i) {
        if (IS_DASH_POSITION[i]) {
            if (uuid[i] != '-') {
                return false;
            }
            continue;
        }
        char c = uuid[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    return true;
}

bool IsUuidSupport(std::vector<Uuid> &uuids, const std::string &uuidStr)
{
    return std::find(uuids.begin(), uuids.end(), Uuid::ConvertFromString(uuidStr)) != uuids.end();
}

bool StartWith(const std::string &str, const std::string &prefix)
{
    if (str.length() < prefix.length()) {
        return false;
    }
    return str.substr(0, prefix.length()).compare(prefix) == 0;
}

bool IsNumber(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    for (char ch : str) {
        if (!std::isdigit(ch)) {
            return false;
        }
    }
    return true;
}

bool IsHexNumber(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    for (char ch : str) {
        if (!std::isxdigit(ch)) {
            return false;
        }
    }
    return true;
}

int32_t GenerateRandomNum(int32_t beginNum, int32_t endNum)
{
    CHECK_AND_RETURN_LOG_RET(endNum >= beginNum, INVALID_RANDOM_VALUE, "endNum should be greater than beginNum");
    const char* RANDOM_PATH = "/dev/random";
    int rand = INVALID_RANDOM_VALUE;
    int randFd = open(RANDOM_PATH, O_RDONLY);
    CHECK_AND_RETURN_LOG_RET(randFd != INVALID_FD, INVALID_RANDOM_VALUE, "can't open rand fd");
    read(randFd, &rand, sizeof(rand));
    close(randFd);

    if (rand < 0) {
        rand = -rand;
    }

    rand = rand % (endNum - beginNum + 1) + beginNum; //1 is used to complete the right boundary of the random number.
    return rand;
}

std::string GetCommandValue(const std::string &atCommand)
{
    std::string result = "";
    size_t pos = atCommand.find("=");
    if (pos == std::string::npos) {
        return result;
    }
    result = atCommand.substr(pos + 1);
    return result;
}

std::vector<std::string> SplitValueByDelim(const std::string &commandValue, const std::string &delim)
{
    std::vector<std::string> result {};
    std::string inputStr = commandValue;

    size_t pos = inputStr.find(delim);
    while (pos != std::string::npos) {
        std::string subStr = inputStr.substr(0, pos);
        result.push_back(subStr);
        inputStr.erase(0, pos + 1);
        pos = inputStr.find(delim);
    }
    result.push_back(inputStr);
    return result;
}

int64_t GetTimeStamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

std::string TruncateString(const std::string &input, size_t maxLength) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(input.c_str());
    size_t len = input.length();
    std::string result;
    if (len <= maxLength || len < 4 || maxLength < 3) { //3、4的时候为长度过小场景，直接截断即可，此处限制长度防止反转
        return input;
    }
    std::string ellipsis = "..."; //省略号
    size_t charLen = 1; // 默认是单字节字符
    for (size_t i = 0; i < len; i += charLen) {
        unsigned char c = bytes[i];
        charLen = 1; // 默认是单字节字符
        // 判断字符长度
        if ((c & 0xE0) == 0xC0 && i < len - 1) { // 从数据头判断是否为2字节字符
            charLen = 2; // 2个字节的字符
        } else if ((c & 0xF0) == 0xE0 && i < len - 2) { // 从数据头判断是否为3字节字符
            charLen = 3; // 3个字节的字符
        } else if ((c & 0xF8) == 0xF0 && i < len - 3) { // 从数据头判断是否为4字节字符
            charLen = 4; // 4个字节的字符
        } else if ((c & 0x80) != 0x00) { // 不完整的 UTF-8 字符，跳过
            break;
        }
        if (result.length() + charLen > maxLength - ellipsis.size()) {
            break;
        }

        // 添加字符
        result += input.substr(i, charLen);
    }
    result += ellipsis;
    return result;
}
}  // namespace bluetooth
}  // namespace OHOS