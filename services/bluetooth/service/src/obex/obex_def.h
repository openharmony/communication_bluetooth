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

#ifndef OBEX_DEF_H
#define OBEX_DEF_H
#define RECV_TIMEOUT_DEFAULT_MS (5 * 1000)

#include <cstdint>

namespace OHOS {
namespace bluetooth {
const uint8_t HEAD_CONNECT_LEN = 7;
const uint8_t HEAD_DISCONNECT_LEN = 3;
const uint8_t HEAD_SETPATH_LEN = 5;
const uint8_t HEAD_ABORT_LEN = 3;
const uint8_t HEAD_DEFAULT_LEN = 3;
const uint8_t HEAD_BUFFER_LEN = 7;

const uint8_t HEAD_AND_RSPDATA_LEN = 6;
const uint8_t MIN_BUFFER_LEN = 20;

const int32_t MAX_PACKET_SIZE_INT = 0xFFFE;

const uint8_t OBEX_OPCODE_CONNECT = 0x80;
const uint8_t OBEX_OPCODE_DISCONNECT = 0x81;
const uint8_t OBEX_OPCODE_PUT = 0x02;
const uint8_t OBEX_OPCODE_PUT_FINAL = 0x82;
const uint8_t OBEX_OPCODE_GET = 0x03;
const uint8_t OBEX_OPCODE_GET_FINAL = 0x83;
const uint8_t OBEX_OPCODE_RESERVED = 0x04;
const uint8_t OBEX_OPCODE_RESERVED_FINAL = 0x84;
const uint8_t OBEX_OPCODE_SETPATH = 0x85;
const uint8_t OBEX_OPCODE_SESSION = 0x87;
const uint8_t OBEX_OPCODE_ABORT = 0xFF;
const uint8_t OBEX_OPCODE_UNIMPLEMENTED = 0x00;

enum ResponseCodes {
    OBEX_HTTP_CONTINUE = 0x90,
    OBEX_HTTP_OK = 0xA0,
    OBEX_HTTP_CREATED = 0xA1,
    OBEX_HTTP_ACCEPTED = 0xA2,
    OBEX_HTTP_NOT_AUTHORITATIVE = 0xA3,
    OBEX_HTTP_NO_CONTENT = 0xA4,
    OBEX_HTTP_RESET = 0xA5,
    OBEX_HTTP_PARTIAL = 0xA6,
    OBEX_HTTP_MULT_CHOICE = 0xB0,
    OBEX_HTTP_MOVED_PERM = 0xB1,
    OBEX_HTTP_MOVED_TEMP = 0xB2,
    OBEX_HTTP_SEE_OTHER = 0xB3,
    OBEX_HTTP_NOT_MODIFIED = 0xB4,
    OBEX_HTTP_USE_PROXY = 0xB5,
    OBEX_HTTP_BAD_REQUEST = 0xC0,
    OBEX_HTTP_UNAUTHORIZED = 0xC1,
    OBEX_HTTP_PAYMENT_REQUIRED = 0xC2,
    OBEX_HTTP_FORBIDDEN = 0xC3,
    OBEX_HTTP_NOT_FOUND = 0xC4,
    OBEX_HTTP_BAD_METHOD = 0xC5,
    OBEX_HTTP_NOT_ACCEPTABLE = 0xC6,
    OBEX_HTTP_PROXY_AUTH = 0xC7,
    OBEX_HTTP_TIMEOUT = 0xC8,
    OBEX_HTTP_CONFLICT = 0xC9,
    OBEX_HTTP_GONE = 0xCA,
    OBEX_HTTP_LENGTH_REQUIRED = 0xCB,
    OBEX_HTTP_PRECON_FAILED = 0xCC,
    OBEX_HTTP_ENTITY_TOO_LARGE = 0xCD,
    OBEX_HTTP_REQ_TOO_LARGE = 0xCE,
    OBEX_HTTP_UNSUPPORTED_TYPE = 0xCF,
    OBEX_HTTP_INTERNAL_ERROR = 0xD0,
    OBEX_HTTP_NOT_IMPLEMENTED = 0xD1,
    OBEX_HTTP_BAD_GATEWAY = 0xD2,
    OBEX_HTTP_UNAVAILABLE = 0xD3,
    OBEX_HTTP_GATEWAY_TIMEOUT = 0xD4,
    OBEX_HTTP_VERSION = 0xD5,
    OBEX_DATABASE_FULL = 0xE0,
    OBEX_DATABASE_LOCKED = 0xE1,
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_DEF_H