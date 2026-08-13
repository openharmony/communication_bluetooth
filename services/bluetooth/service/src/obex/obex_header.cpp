/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_obex_header"
#endif

#include "obex_header.h"
#include <string>
#include <locale>
#include <codecvt>
#include <sstream>
#include "log.h"
#include "common_util.h"
#include "string_ex.h"

namespace OHOS {
namespace bluetooth {
ObexHeader::ObexHeader()
{
    HILOGI("ObexHeader Create");
}

ObexHeader::~ObexHeader()
{
    HILOGI("ObexHeader destroy");
}

void ObexHeader::Clear()
{
    std::lock_guard<std::mutex> lock(headerMapMutex_);
    obexHeaderMap_.clear();
}

void ObexHeader::ParseHeaderValue(const std::vector<uint8_t> &headerValue)
{
    uint32_t pos = 0;
    uint32_t size = headerValue.size();
    while (pos < size) {
        uint8_t headerId = headerValue[pos++];
        uint8_t headerDataType = headerId & MASK;
        HILOGI("headerId= %{public}02X, headerDataType= %{public}02X, size = %{public}u, pos = %{public}u",
            headerId, headerDataType, size, pos);
        switch (headerDataType) {
            case BYTE:
                ParseByteArray(headerId, headerValue, pos, UINT8_LENGTH);
                break;
            case UNICODE_TEXT: // Fall through
            case BYTES:
                ParseBytes(headerId, headerValue, pos);
                break;
            case WORD:
                ParseByteArray(headerId, headerValue, pos, UINT32_LENGTH);
                break;
            default:
                HILOGE("parse error");
                break;
        }
    }
}

void ObexHeader::ParseByteArray(uint8_t headerId, const std::vector<uint8_t> &headerValue,
    uint32_t &pos, uint32_t dataLen)
{
    uint32_t size = headerValue.size();
    HILOGD("headerId = %{public}02X, pos = %{public}u, dataLen = %{public}u, size = %{public}u",
        headerId, pos, dataLen, size);
    if (size < pos + dataLen) {
        HILOGE("error, size = %{public}u, pos = %{public}u, dataLen = %{public}u", size, pos, dataLen);
        return;
    }
    std::vector<uint8_t> tmpValue {};
    tmpValue.insert(tmpValue.end(), headerValue.begin() + pos, headerValue.begin() + pos + dataLen);
    SetHeader(headerId, tmpValue);
    pos += dataLen;
}

void ObexHeader::ParseBytes(uint8_t headerId, const std::vector<uint8_t> &headerValue, uint32_t &pos)
{
    uint32_t size = headerValue.size();
    HILOGD("headerId = %{public}02X, pos = %{public}u, size = %{public}u", headerId, pos, size);
    if (size < pos + UINT16_LENGTH) { // code的pos已经增加
        HILOGE("error, size = %{public}u, pos = %{public}u", size, pos);
        return;
    }
    uint32_t dataLen = headerValue[pos++];
    dataLen = (dataLen << BIT_LEN_8) + headerValue[pos++];
    if (dataLen < HEADER_PREFIX_LENGTH) {
        HILOGE("dataLen error, dataLen = %{public}u", dataLen);
        return;
    }
    dataLen = dataLen - HEADER_PREFIX_LENGTH;
    HILOGD("dataLen = %{public}u, pos = %{public}u", dataLen, pos);
    if (dataLen == 0 || size < pos + dataLen) {
        HILOGE("error, size = %{public}u, pos = %{public}u, dataLen = %{public}u", size, pos, dataLen);
        return;
    }

    std::vector<uint8_t> tmpValue {};
    tmpValue.insert(tmpValue.end(), headerValue.begin() + pos, headerValue.begin() + pos + dataLen);
    SetHeader(headerId, tmpValue);
    pos += dataLen;
}

void ObexHeader::SetHeader(uint8_t headerId, const std::vector<uint8_t> &headerValue)
{
    HILOGD("headerId = %{public}02X, size = %{public}u", headerId, headerValue.size());
    std::lock_guard<std::mutex> lock(headerMapMutex_);
    auto iter = obexHeaderMap_.find(headerId);
    if (iter == obexHeaderMap_.end()) {
        obexHeaderMap_.insert(std::make_pair(headerId, headerValue));
        return;
    }
    iter->second = headerValue;
}

void ObexHeader::SetResponseCode(uint8_t responseCode)
{
    responseCode_ = responseCode;
}

uint8_t ObexHeader::GetResponseCode()
{
    return responseCode_;
}

std::vector<uint8_t> ObexHeader::GetHeader(uint8_t headerId)
{
    std::lock_guard<std::mutex> lock(headerMapMutex_);
    std::vector<uint8_t> retValue {};
    auto iter = obexHeaderMap_.find(headerId);
    if (iter == obexHeaderMap_.end()) {
        return retValue;
    }
    retValue.insert(retValue.end(), iter->second.begin(), iter->second.end());
    HILOGD("headerId = %{public}02X, size = %{public}d", headerId, retValue.size());
    return retValue;
}

void ObexHeader::GenerateHeaderValue(std::vector<uint8_t> &retValue)
{
    AppendConnectionIdValue(retValue);
    AppendByteValue(COUNT, retValue);
    AppendByteArrayValue(NAME, false, retValue);
    AppendByteArrayValue(TYPE, true, retValue);
    AppendByteValue(LENGTH, retValue);
    AppendByteArrayValue(TIME_ISO_8601, false, retValue);
    AppendByteValue(TIME_4_BYTE, retValue);
    AppendByteArrayValue(DESCRIPTION, false, retValue);
    AppendByteArrayValue(TARGET, false, retValue);
    AppendByteArrayValue(HTTP, false, retValue);
    AppendByteArrayValue(WHO, false, retValue);
    AppendByteArrayValue(APPLICATION_PARAMETER, false, retValue);
    AppendByteArrayValue(OBJECT_CLASS, false, retValue);
    for (int i = 0; i < RESERVE_NUM; i++) {
        AppendByteArrayValue(i + 0x30, false, retValue);
        AppendByteArrayValue(i + 0x70, false, retValue);
        AppendByteValue(i + 0xB0, retValue);
        AppendByteValue(i + 0xF0, retValue);
    }
    AppendByteArrayValue(AUTH_CHALLENGE, false, retValue);
    AppendByteArrayValue(AUTH_RESPONSE, false, retValue);
    AppendByteValue(SINGLE_RESPONSE_MODE, retValue);
}

void ObexHeader::AppendConnectionIdValue(std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> connectionIdValue = GetHeader(CONNECTION_ID);
    std::vector<uint8_t> targetValue = GetHeader(TARGET);
    if (!connectionIdValue.empty() && targetValue.empty()) {
        retValue.push_back(CONNECTION_ID);
        retValue.insert(retValue.end(), connectionIdValue.begin(), connectionIdValue.end());
    }
}

void ObexHeader::AppendByteValue(uint8_t headerId, std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> value = GetHeader(headerId);
    if (value.empty()) {
        return;
    }
    retValue.push_back(headerId);
    retValue.insert(retValue.end(), value.begin(), value.end());
}

void ObexHeader::AppendByteArrayValue(uint8_t headerId, bool isHaveTail, std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> value = GetHeader(headerId);
    if (value.empty()) {
        return;
    }
    uint32_t length = value.size() + HEADER_PREFIX_LENGTH;
    if (isHaveTail) {
        length = length + 1; // 1 mean tail len
    }
    retValue.push_back(headerId);
    retValue.push_back((length >> BIT_LEN_8) & 0xFF);
    retValue.push_back(length & 0xFF);
    retValue.insert(retValue.end(), value.begin(), value.end());
    if (isHaveTail) {
        retValue.push_back(0);
    }
}

uint32_t ObexHeader::FindHeaderEnd(const std::vector<uint8_t> &headerValue, uint32_t start, uint32_t maxSize)
{
    uint32_t fullLength = 0;
    uint32_t lastLength = 0;
    uint32_t index = start;
    uint32_t length = 0;
    uint32_t valueSize = headerValue.size();
    while ((fullLength < maxSize) && (index < valueSize)) {
        int headerID = headerValue[index];
        lastLength = fullLength;
        switch (headerID & MASK) {
            case UNICODE_TEXT: // UNICODE_TEXT 和 BYTES 处理方式一致
            case BYTES:
                if (valueSize < index + HEADER_PREFIX_LENGTH) {
                    HILOGE("valueSize = %{public}d, index = %{public}d", valueSize, index);
                    return 0;
                }
                index++;
                length = headerValue[index++];
                length = (length << BIT_LEN_8) + headerValue[index++];
                if (length < HEADER_PREFIX_LENGTH) {
                    HILOGE("length = %{public}d", length);
                    return 0;
                }
                index += length - HEADER_PREFIX_LENGTH;
                fullLength += length;
                break;
            case BYTE:
                index += BYTE_HEADER_LEN;
                fullLength += BYTE_HEADER_LEN;
                break;
            case WORD:
                index += WORD_HEADER_LEN;
                fullLength += WORD_HEADER_LEN;
                break;
            default:
                return 0;;
        }
    }
    if (lastLength == 0) {
        // 如果最后一个head的长度小于maxSize，则直接返回valueSize，否则返回0
        if (fullLength < maxSize) {
            return valueSize;
        }
        return 0;
    }
    // 返回能发送的最后一个head的位置
    return lastLength + start;
}

std::vector<uint8_t> ObexHeader::GetBody(uint16_t &bodyLen)
{
    std::vector<uint8_t> value = GetHeader(BODY);
    if (value.empty()) {
        bodyLen = 0;
    }
    bodyLen = value.size();
    return value;
}

std::vector<uint8_t> ObexHeader::GetEndOfBody(uint16_t &bodyLen)
{
    std::vector<uint8_t> value = GetHeader(END_OF_BODY);
    if (value.empty()) {
        bodyLen = 0;
    }
    bodyLen = value.size();
    return value;
}

int32_t ObexHeader::GetConnectionId()
{
    std::vector<uint8_t> value = GetHeader(CONNECTION_ID);
    if (value.empty()) {
        return -1;
    }
    return ConvertToInt32(value);
}

int32_t ObexHeader::GetCount()
{
    std::vector<uint8_t> value = GetHeader(COUNT);
    if (value.empty()) {
        return -1;
    }
    return ConvertToInt32(value);
}

void ObexHeader::SetConnectionId(uint32_t connectionId)
{
    std::vector<uint8_t> value;
    ConvertToVector(connectionId, value);
    SetHeader(CONNECTION_ID, value);
}

void ObexHeader::SetBody(std::vector<uint8_t> bodyData, uint16_t length)
{
    std::vector<uint8_t> body{};
    body.push_back((bodyData.size() + HEADER_PREFIX_LENGTH) >> BIT_LEN_8);
    body.push_back(bodyData.size() + HEADER_PREFIX_LENGTH);
    body.insert(body.end(), bodyData.begin(), bodyData.end());
    SetHeader(BODY, body);
}

void ObexHeader::SetEndOfBody(std::vector<uint8_t> bodyData, uint16_t length)
{
    std::vector<uint8_t> body{};
    body.push_back((bodyData.size() + HEADER_PREFIX_LENGTH) >> BIT_LEN_8);
    body.push_back(bodyData.size() + HEADER_PREFIX_LENGTH);
    body.insert(body.end(), bodyData.begin(), bodyData.end());
    SetHeader(END_OF_BODY, body);
}

void ObexHeader::SetName(std::string name)
{
    std::vector<uint8_t> value;
    ConvertUnicodeToVector(name, value);
    SetHeader(NAME, value);
}

void ObexHeader::SetCount(uint32_t count)
{
    std::vector<uint8_t> value;
    ConvertToVector(count, value);
    SetHeader(COUNT, value);
}

void ObexHeader::SetSrm(bool isSupportSrm)
{
    std::vector<uint8_t> value;
    uint8_t srm = 0x00;
    if (isSupportSrm) {
        srm = 0x01;
    }
    ConvertToVector(srm, value);
    SetHeader(SINGLE_RESPONSE_MODE, value);
}

void ObexHeader::SetLength(uint32_t length)
{
    std::vector<uint8_t> value;
    ConvertToVector(length, value);
    SetHeader(LENGTH, value);
}

void ObexHeader::SetSingleResponseMode(uint8_t singleResponseMode)
{
    std::vector<uint8_t> value;
    ConvertToVector(singleResponseMode, value);
    SetHeader(SINGLE_RESPONSE_MODE, value);
}

bool ObexHeader::GetLength(uint32_t &length)
{
    std::vector<uint8_t> value = GetHeader(LENGTH);
    if (value.empty()) {
        HILOGI("GetLength, value is invalid.");
        length = 0;
        return true;
    }
    uint64_t step = 2;
    if ((value.size() % step) != 0) {
        HILOGI("GetLength, value is invalid.");
        length = 0;
        return true;
    }
    return ConvertToUint32(value, length);
}

std::string ObexHeader::GetName()
{
    std::vector<uint8_t> value = GetHeader(NAME);
    if (value.empty()) {
        HILOGI("GetName, value is null.");
        return "";
    }
    uint32_t step = 2;
    if ((value.size() % step) != 0) {
        HILOGI("GetName, value is invalid.");
        return "";
    }
    std::u16string u16Str;
    uint32_t length = value.size() / step;
    std::vector<uint16_t> ch(length);
    for (uint32_t i = 0; i < length; i++) {
        uint8_t upper = value[step * i];
        uint8_t lower = value[(step * i) + 1];
        if (upper == 0 && lower == 0) {
            u16Str.push_back(static_cast<char16_t>(0));
            break;
        }
        ch[i] = (static_cast<uint16_t>(upper) << BIT_LEN_8) | lower;
        u16Str.push_back(static_cast<char16_t>(ch[i]));
    }
    
    return Str16ToStr8(u16Str);
}

std::string ObexHeader::GetType()
{
    std::vector<uint8_t> value = GetHeader(TYPE);
    if (value.empty()) {
        return "";
    }
    return ConvertToString(value);
}

int32_t ObexHeader::ConvertToInt32(std::vector<uint8_t> &value)
{
    if (value.size() != UINT32_LENGTH) {
        return -1;
    }
    int32_t result = 0;
    int32_t tmp;
    int32_t power = 0;
    for (int i = value.size() - 1; i >= 0; i--) {
        tmp = value[i];
        result = result + (tmp << power);
        power += BIT_LEN_8;
    }
    return result;
}

bool ObexHeader::ConvertToUint32(std::vector<uint8_t> &value, uint32_t &length)
{
    if (value.size() != UINT32_LENGTH) {
        return false;
    }
    uint32_t result = 0;
    uint32_t tmp;
    uint32_t power = 0;
    for (int i = static_cast<int>(UINT32_LENGTH) - 1; i >= 0; i--) {
        tmp = value[i];
        result = result + (tmp << power);
        power += BIT_LEN_8;
    }
    length = result;
    return true;
}

void ObexHeader::ConvertToVector(uint8_t value, std::vector<uint8_t> &retValue)
{
    retValue.push_back(value);
}

void ObexHeader::ConvertToVector(uint32_t value, std::vector<uint8_t> &retValue)
{
    for (int8_t i = UINT32_LENGTH - 1; i >= 0; i--) {
        retValue.push_back((value >> (i * BIT_LEN_8)) & 0xFF);
    }
}

void ObexHeader::ConvertUnicodeToVector(std::string strs, std::vector<uint8_t> &retValue)
{
    // one word show as two bytes
    std::u16string u16string = Str8ToStr16(strs);
    for (char16_t c : u16string) {
        uint16_t value = static_cast<uint16_t>(c);
        for (int8_t i = UINT16_LENGTH - 1; i >= 0; i--) {
            retValue.push_back((value >> (i * BIT_LEN_8)) & 0xFF);
        }
    }
    retValue.push_back(0x00);
    retValue.push_back(0x00);
}

std::string ObexHeader::ConvertToString(std::vector<uint8_t> &value)
{
    std::string strs;
    std::stringstream strStream;
    for (uint32_t i = 0; i < value.size(); i++) {
        if (value[i] == 0) {
            continue;
        }
        strStream << value[i];
    }
    strStream >> strs;
    return strs;
}

int ObexHeader::GetRecvTimeoutMs()
{
    return recvTimeoutMs_;
}

void ObexHeader::SetRecvTimeoutMs(int recvTimeoutMs)
{
    recvTimeoutMs_ = recvTimeoutMs;
}
}  // namespace bluetooth
}  // namespace OHOS