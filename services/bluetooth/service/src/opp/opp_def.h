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

#ifndef OPP_DEF_H
#define OPP_DEF_H

#include <cstdint>
#include <string>
#include "log.h"
#include "btcommon/message.h"

namespace OHOS {
namespace bluetooth {

const std::string OPP_SERVICE_NAME = "OBEX Object Push";
constexpr int32_t SDP_OPP_SERVER_VERSION = 0x0102;
constexpr int32_t OPP_RFCOMM_CHANNEL = 12;
constexpr int32_t OPP_L2CAP_PSM = 0x1023;
constexpr uint16_t OPP_UUID16 = 0X1105;

constexpr int OPP_TRANSFER_USER_ACCEPT = 0;
constexpr int OPP_TRANSFER_AUTO_ACCEPT = 1;

constexpr int OPP_TRANSFER_DIRECTION_OUTBOUND = 0;
constexpr int OPP_TRANSFER_DIRECTION_INBOUND = 1;

constexpr int OPP_OBEX_STATUS_IDLE = 0;
constexpr int OPP_OBEX_STATUS_WAITING_CONTINUE = 1;
constexpr int OPP_OBEX_STATUS_WAITING_END = 3;

constexpr int INCOMING_CONNECT_TIMEOUT_MS = 10000;
constexpr int INCOMING_FILE_TIMEOUT_MS = 50000;

constexpr int OPP_TRANSFER_CONFIRM_PENDING = 0;
constexpr int OPP_TRANSFER_CONFIRM_ACCEPT = 1;
constexpr int OPP_TRANSFER_CONFIRM_REJECT = 2;
constexpr uint16_t OPP_MAX_PSM = 0XFFFF;

constexpr int32_t UNKNOWN_OBEX_VALUE = -1;
constexpr int TRANSFER_DEFAULT_COUNT = 1;

const uint8_t OPP_SUPPORTED_FORMATS_LIST_DATA[] = {
    0x08, 0x01 /* vCard 2.1 */,
    0x08, 0x02 /* vCard 3.0 */,
    0x08, 0x03 /* vCal 1.0 */,
    0x08, 0x04 /* iCal 2.0 */,
    0x08, 0xFF /* Any type of object */
};

constexpr int OPP_TRANSFER_STATUS_PENDING = 0;
constexpr int OPP_TRANSFER_STATUS_RUNNING = 1;
constexpr int OPP_TRANSFER_STATUS_FINISH = 2;

constexpr int OPP_TRANSFER_OPERATION_UNKNOWN = 0;
constexpr int OPP_TRANSFER_OPERATION_ACCEPT = 1;
constexpr int OPP_TRANSFER_OPERATION_FORBIDDEN = 2;

// The transfer rusult success.
constexpr int OPP_TRANSFER_RESULT_SUCCESS = 0;
// The transfer result file type is not supported.
constexpr int OPP_TRANSFER_RESULT_ERROR_UNSUPPORTED_TYPE = 1;
// The transfer result peer device cannot process the request.
constexpr int OPP_TRANSFER_RESULT_ERROR_BAD_REQUEST = 2;
// The transfer result peer device not accept the request.
constexpr int OPP_TRANSFER_RESULT_ERROR_NOT_ACCEPTABLE = 3;
// The transfer result cancels the ongoing file transfer process
constexpr int OPP_TRANSFER_RESULT_ERROR_CANCELED = 4;
// The transfer result failed to connect to the peer device.
constexpr int OPP_TRANSFER_RESULT_ERROR_CONNECTION_FAILED = 5;
// The transfer result local canceled file transfer
constexpr int OPP_TRANSFER_RESULT_ERROR_TRANSFER_FAILED = 6;
// The transfer result unknown error
constexpr int OPP_TRANSFER_RESULT_ERROR_UNKNOWN = 7;
// The transfer result length error
constexpr int OPP_TRANSFER_RESULT_ERROR_LENGTH = 8;
// The transfer rusult success disconnect.
constexpr int OPP_TRANSFER_RESULT_SUCCESS_DISCONNECT = 9;

constexpr int WAIT_TIME = 100;
constexpr int NOTICE_INTERVAL_TIME = 1;
constexpr int TRANSFER_ID_MAX = 30;
constexpr int TRANSFER_FILE_COUNT_MAX = 300;
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OPP_DEF_H