/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <stdint.h>
#include "att/att_common.h"
#include "mock/gatt_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define READBYTYPERESPONSEFREE 1
#define READBYGROUPTYPERESPONSEFREE 2

void AttErrorResponse_1(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_2(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_3(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_4(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_5(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_6(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_7(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_8(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_9(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_10(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_11(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_12(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_13(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_14(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_15(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_16(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_17(const AttConnectInfo *connect, Buffer *buffer);

void AttErrorResponse_18(const AttConnectInfo *connect, Buffer *buffer);

void AttExchangeMTURequest(const AttConnectInfo *connect, Buffer *buffer);

void AttExchangeMTUResponse(AttConnectInfo *connect, Buffer *buffer);

void AttFindInformationRequest(AttConnectInfo *connect, Buffer *buffer);

void AttFindInformationResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttFindByTypeValueRequest(AttConnectInfo *connect, Buffer *buffer);

void AttFindByTypeValueResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttReadByTypeRequest(AttConnectInfo *connect, Buffer *buffer);

void AttReadByTypeResponse_1(const AttConnectInfo *connect, Buffer *buffer);

void AttReadByTypeResponse_2(const AttConnectInfo *connect, Buffer *buffer);

void AttReadByTypeResponse_3(const AttConnectInfo *connect, Buffer *buffer);

void AttReadRequest(AttConnectInfo *connect, Buffer *buffer);

void AttReadResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttReadResponse1(const AttConnectInfo *connect, Buffer *buffer);

void AttReadBlobRequest(AttConnectInfo *connect, Buffer *buffer);

void AttReadBlobResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttReadBlobResponse1(const AttConnectInfo *connect, Buffer *buffer);

void AttReadMultipleRequest(AttConnectInfo *connect, Buffer *buffer);

void AttReadMultipleResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttReadByGroupTypeRequest(AttConnectInfo *connect, Buffer *buffer);

void AttReadByGroupTypeResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttWriteRequest(AttConnectInfo *connect, Buffer *buffer);

void AttWriteResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttWriteCommand(const AttConnectInfo *connect, Buffer *buffer);

void AttSignedWriteCommand(const AttConnectInfo *connect, Buffer *buffer);

void AttPrepareWriteRequest(AttConnectInfo *connect, Buffer *buffer);

void AttPrepareWriteResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttPrepareWriteResponse1(const AttConnectInfo *connect, Buffer *buffer);

void AttExecuteWriteRequest(AttConnectInfo *connect, Buffer *buffer);

void AttExecuteWriteResponse(const AttConnectInfo *connect, Buffer *buffer);

void AttTransactionTimeOutId(const AttConnectInfo *connect, Buffer *buffer);

void AttNull(const AttConnectInfo *connect, Buffer *buffer);

void AttHandleValueNotification(const AttConnectInfo *connect, Buffer *buffer);

void AttHandleValueNotification1(const AttConnectInfo *connect, Buffer *buffer);

void AttHandleValueIndication(const AttConnectInfo *connect, Buffer *buffer);

void AttHandleValueConfirmation(AttConnectInfo *connect, Buffer *buffer);

#ifdef __cplusplus
}
#endif
