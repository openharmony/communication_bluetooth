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

#ifndef OBEX_HEADER_H
#define OBEX_HEADER_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include "obex_def.h"

namespace OHOS {
namespace bluetooth {

constexpr uint8_t COUNT = 0xC0;
constexpr uint8_t NAME = 0x01;
constexpr uint8_t TYPE = 0x42;
constexpr uint8_t LENGTH = 0xC3;
constexpr uint8_t TIME_ISO_8601 = 0x44;
constexpr uint8_t TIME_4_BYTE = 0xC4;
constexpr uint8_t DESCRIPTION = 0x05;
constexpr uint8_t TARGET = 0x46;
constexpr uint8_t HTTP = 0x47;
constexpr uint8_t BODY = 0x48;
constexpr uint8_t END_OF_BODY = 0x49;
constexpr uint8_t WHO = 0x4A;
constexpr uint8_t CONNECTION_ID = 0xCB;
constexpr uint8_t APPLICATION_PARAMETER = 0x4C;
constexpr uint8_t AUTH_CHALLENGE = 0x4D;
constexpr uint8_t AUTH_RESPONSE = 0x4E;
constexpr uint8_t OBJECT_CLASS = 0x4F;
constexpr uint8_t SINGLE_RESPONSE_MODE = 0x97;
constexpr uint8_t SINGLE_RESPONSE_MODE_PARAMETER = 0x98;

constexpr uint8_t MASK = 0xC0;

constexpr uint8_t UNICODE_TEXT = 0x00;
constexpr uint8_t BYTES = 0x40;
constexpr uint8_t BYTE = 0x80;
constexpr uint8_t WORD = 0xC0;


constexpr uint8_t UINT8_LENGTH = 1;
constexpr uint8_t UINT16_LENGTH = 2;
constexpr uint8_t UINT32_LENGTH = 4;
constexpr uint16_t HEADER_PREFIX_LENGTH = 3;
constexpr uint8_t BYTE_HEADER_LEN = 2;
constexpr uint8_t WORD_HEADER_LEN = 5;
constexpr uint8_t PER_WORD_LEN = 2;

constexpr uint8_t BIT_LEN_8 = 8;
constexpr uint8_t RESERVE_NUM = 16;

class ObexHeader {
public:
    explicit ObexHeader();
    ~ObexHeader();

    void SetHeader(uint8_t headerId, const std::vector<uint8_t> &headerValue);
    void SetResponseCode(uint8_t responseCode);
    uint8_t GetResponseCode();

    void ParseHeaderValue(const std::vector<uint8_t> &headerValue);
    void GenerateHeaderValue(std::vector<uint8_t> &retValue);
    std::vector<uint8_t> GetHeader(uint8_t headerId);
    static uint32_t FindHeaderEnd(const std::vector<uint8_t> &headerValue, uint32_t start, uint32_t maxSize);

    int32_t GetConnectionId();
    int32_t GetCount();
    void SetConnectionId(uint32_t connectionId);
    void SetBody(std::vector<uint8_t> bodyData, uint16_t length);
    void SetEndOfBody(std::vector<uint8_t> bodyData, uint16_t length);
    void SetName(std::string name);
    void SetLength(uint32_t length);
    void SetCount(uint32_t count);
    void SetSrm(bool isSupportSrm);
    void SetSingleResponseMode(uint8_t singleResponseMode);
    bool GetLength(uint32_t &length);

    std::string GetName();
    std::string GetType();
    std::vector<uint8_t> GetBody(uint16_t &bodyLen);
    std::vector<uint8_t> GetEndOfBody(uint16_t &bodyLen);

    int GetRecvTimeoutMs();
    void SetRecvTimeoutMs(int recvTimeoutMs);
    void Clear();

private:
    void ParseBytes(uint8_t headerId, const std::vector<uint8_t> &headerValue, uint32_t &pos);
    void ParseByteArray(uint8_t headerId, const std::vector<uint8_t> &headerValue, uint32_t &pos, uint32_t dataLen);

    void AppendConnectionIdValue(std::vector<uint8_t> &retValue);
    void AppendByteValue(uint8_t headerId, std::vector<uint8_t> &retValue);
    void AppendByteArrayValue(uint8_t headerId, bool isHaveTail, std::vector<uint8_t> &retValue);

    int32_t ConvertToInt32(std::vector<uint8_t> &value);
    bool ConvertToUint32(std::vector<uint8_t> &value, uint32_t &length);
    void ConvertToVector(uint8_t value, std::vector<uint8_t> &retValue);
    void ConvertToVector(uint32_t value, std::vector<uint8_t> &retValue);
    void ConvertUnicodeToVector(std::string strs, std::vector<uint8_t> &retValue);
    std::string ConvertToString(std::vector<uint8_t> &value);

private:
    uint8_t responseCode_ = 0;
    std::mutex headerMapMutex_;
    std::map<uint8_t, std::vector<uint8_t> > obexHeaderMap_;
    int recvTimeoutMs_ = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_HEADER_H