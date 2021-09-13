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

#include "mock/att_receive_mock.h"
#include <memory.h>
#include <stdlib.h>
#include "buffer.h"
#include "packet.h"
#include "list.h"
#include "allocator.h"
#include "log.h"
#include "att/att_common.h"

static void *AllocatorMalloc(size_t size)
{
    return malloc(size);
}

static void *AllocatorCalloc(size_t size)
{
    return calloc(1, size);
}
static void AllocatorFree(void *ptr){};

const Allocator MEM_MALLOC = {
    .alloc = AllocatorMalloc,
    .free = AllocatorFree,
};
const Allocator MEM_CALLOC = {
    .alloc = AllocatorCalloc,
    .free = AllocatorFree,
};

void AttErrorResponse_1(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_1;
    AttError_1 attErrorTObj;
    attErrorTObj.reqOpcode = READ_BY_GROUP_TYPE_REQUEST;
    attErrorTObj.errorCode = ATT_ATTRIBUTE_NOT_FOUND;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_2(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_2;
    AttError_2 attErrorTObj;
    attErrorTObj.reqOpcode = READ_BY_GROUP_TYPE_REQUEST;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_3(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_3;
    AttError_3 attErrorTObj;
    attErrorTObj.reqOpcode = READ_BY_TYPE_REQUEST;
    attErrorTObj.errorCode = ATT_ATTRIBUTE_NOT_FOUND;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();
    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_4(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_4;
    AttError_4 attErrorTObj;
    attErrorTObj.reqOpcode = FIND_BY_TYPE_VALUE_REQUEST;
    attErrorTObj.errorCode = ATT_ATTRIBUTE_NOT_FOUND;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_5(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_5;
    AttError_5 attErrorTObj;
    attErrorTObj.reqOpcode = FIND_BY_TYPE_VALUE_REQUEST;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_6(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = FIND_INFORMATION_REQUEST;
    attErrorTObj.errorCode = ATT_ATTRIBUTE_NOT_FOUND;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_7(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_7;
    AttError_7 attErrorTObj;
    attErrorTObj.reqOpcode = FIND_INFORMATION_REQUEST;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_8(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_8;
    AttError_8 attErrorTObj;
    attErrorTObj.reqOpcode = READ_REQUEST;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_9(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_9;
    AttError_9 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_10(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = READ_REQUEST;
    attErrorTObj.errorCode = ATT_ATTRIBUTE_NOT_FOUND;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_11(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    attErrorTObj.errorCode = ATT_ATTRIBUTE_NOT_FOUND;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_12(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    attErrorTObj.errorCode = ATT_INVALID_HANDLE;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_13(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    attErrorTObj.errorCode = ATT_READ_NOT_PERMITTED;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_14(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    attErrorTObj.errorCode = ATT_WRITE_NOT_PERMITTED;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_15(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    attErrorTObj.errorCode = ATT_INSUFFICIENT_AUTHENTICATION;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_16(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    attErrorTObj.errorCode = ATT_INVALID_OFFSET;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_17(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    attErrorTObj.errorCode = ATT_INSUFFICIENT_ENCRYPTION;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttErrorResponse_18(const AttConnectInfo *connect, Buffer *buffer)
{
    typedef struct {
        uint8_t reqOpcode;
        uint16_t attHandleInError;
        uint8_t errorCode;
    } AttError_6;
    AttError_6 attErrorTObj;
    attErrorTObj.reqOpcode = WRITE_REQUEST;
    attErrorTObj.errorCode = ATT_INSUFFICIENT_ENCRYPTION;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();
    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_ERROR_RESPONSE_ID, &attErrorTObj, NULL, attClientDataCallback->context);
}

void AttExchangeMTURequest(const AttConnectInfo *connect, Buffer *buffer)
{
    AttExchangeMTUType attExchangeMTUObj;
    uint8_t *data = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;

    attServerDataCallback = AttGetATTServerCallback();

    data = (uint8_t *)BufferPtr(buffer);
    attExchangeMTUObj.mtuSize = ((uint16_t *)data)[0];
    LOG_INFO("%s enter, callback para : mtuSize = %d", __FUNCTION__, attExchangeMTUObj.mtuSize);

    attServerDataCallback->attServerCallback(connect->retGattConnectHandle,
        ATT_EXCHANGE_MTU_REQUEST_ID,
        &attExchangeMTUObj,
        NULL,
        attServerDataCallback->context);
}

void AttExchangeMTUResponse(AttConnectInfo *connect, Buffer *buffer)
{
    AttExchangeMTUType attExchangeMTUObj;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    uint8_t *data = (uint8_t *)BufferPtr(buffer);
    attExchangeMTUObj.mtuSize = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(connect->retGattConnectHandle,
        ATT_EXCHANGE_MTU_RESPONSE_ID,
        &attExchangeMTUObj,
        NULL,
        attClientDataCallback->context);
}

void AttFindInformationRequest(AttConnectInfo *connect, Buffer *buffer)
{
    AttFind attFindObj;
    uint8_t *data = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    data = (uint8_t *)BufferPtr(buffer);
    attFindObj.findInformationRequest.startHandle = ((uint16_t *)data)[0];

    LOG_INFO("%s enter, callback para : startHandle = %d, endHandle = %d",
        __FUNCTION__,
        attFindObj.findInformationRequest.startHandle);

    attServerDataCallback->attServerCallback(connect->retGattConnectHandle,
        ATT_FIND_INFORMATION_REQUEST_ID,
        &attFindObj,
        NULL,
        attServerDataCallback->context);
}

void AttFindInformationResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttFind attFindObj;
    AttHandleUuid handleUuidPairs[8] = {0};
    attFindObj.findInforRsponse.handleUuidPairs = handleUuidPairs;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attFindObj.findInforRsponse.pairNum = 1;
    attFindObj.findInforRsponse.format = 0x02;
    attFindObj.findInforRsponse.handleUuidPairs->attHandle = 0xFFFF;

    attClientDataCallback->attClientCallback(connecthandle_,
        ATT_FIND_INFORMATION_RESPONSE_ID,
        &attFindObj,
        NULL,
        attClientDataCallback->context);
}

void AttFindByTypeValueRequest(AttConnectInfo *connect, Buffer *buffer)
{
    AttFind attFindObj;
    Buffer *bufferNew = NULL;
    uint8_t *data = NULL;
    size_t buffSize;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    buffSize = BufferGetSize(buffer);
    data = (uint8_t *)BufferPtr(buffer);
    attFindObj.findByTypeValueRequest.handleRange.startHandle = ((uint16_t *)data)[0];
    attFindObj.findByTypeValueRequest.handleRange.endHandle = GATT_UT_ENDHANDLE_31;
    LOG_INFO("%s enter, callback para : startHandle = %d, endHandle = %d",
        __FUNCTION__,
        attFindObj.findByTypeValueRequest.handleRange.startHandle,
        attFindObj.findByTypeValueRequest.handleRange.endHandle);

    if (buffSize - GATT_UT_THREE_UINT16_LENGTH > 0) {
        bufferNew = BufferSliceMalloc(buffer, GATT_UT_THREE_UINT16_LENGTH, buffSize - GATT_UT_THREE_UINT16_LENGTH);
        attServerDataCallback->attServerCallback(connect->retGattConnectHandle,
            ATT_FIND_BY_TYPE_VALUE_REQUEST_ID,
            &attFindObj,
            bufferNew,
            attServerDataCallback->context);
        BufferFree(bufferNew);
    } else {
        attServerDataCallback->attServerCallback(connect->retGattConnectHandle,
            ATT_FIND_BY_TYPE_VALUE_REQUEST_ID,
            &attFindObj,
            NULL,
            attServerDataCallback->context);
    }
}

void AttFindByTypeValueResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttFind attFindObj;
    AttHandleInfo handleInfoList[8] = {0};
    attFindObj.findByTypeValueResponse.handleInfoList = handleInfoList;
    attFindObj.findByTypeValueResponse.listNum = 1;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    for (uint16_t i = 0; i < 1; i++) {
        (attFindObj.findByTypeValueResponse.handleInfoList + i)->attHandle = 0xFFFF;
        (attFindObj.findByTypeValueResponse.handleInfoList + i)->groupEndHandle = 0xFFFF;
    }

    attClientDataCallback->attClientCallback(connecthandle_,
        ATT_FIND_BY_TYPE_VALUE_RESPONSE_ID,
        &attFindObj,
        NULL,
        attClientDataCallback->context);
}

void AttReadByTypeRequest(AttConnectInfo *connect, Buffer *buffer)
{
    AttRead attReadObj;
    uint8_t *data = NULL;
    BtUuid classId_test;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t type = ((uint16_t *)data)[0];
    attReadObj.readHandleRangeUuid.handleRange.startHandle = ((uint16_t *)data)[1];
    attReadObj.readHandleRangeUuid.handleRange.endHandle = ((uint16_t *)data)[GATT_UT_UINT16_LENGTH];
    switch (type) {
        case 0x02:
            classId_test.type = GATT_UT_UINT16_LENGTH;
            classId_test.uuid128[0] = 1;
            classId_test.uuid16 = 0x2802;
            classId_test.uuid32 = 0x2802;
            attReadObj.readHandleRangeUuid.uuid = &classId_test;
            break;
        case 0x03:
            classId_test.type = 1;
            classId_test.uuid128[0] = 1;
            classId_test.uuid16 = 0x2803;
            classId_test.uuid32 = 0x2803;
            attReadObj.readHandleRangeUuid.uuid = &classId_test;
            break;
        case 0x01:
            classId_test.type = 0x01;
            classId_test.uuid128[0] = 1;
            classId_test.uuid16 = ((uint16_t *)data)[GATT_UT_OFFSET_3];
            classId_test.uuid32 = ((uint16_t *)data)[GATT_UT_OFFSET_3];
            attReadObj.readHandleRangeUuid.uuid = &classId_test;
            break;
        default:
            break;
    }
    LOG_INFO("%s enter, callback para : startHandle = %d, endHandle = %d",
        __FUNCTION__,
        attReadObj.readHandleRangeUuid.handleRange.startHandle,
        attReadObj.readHandleRangeUuid.handleRange.endHandle);

    attServerDataCallback->attServerCallback(
        connect->retGattConnectHandle, ATT_READ_BY_TYPE_REQUEST_ID, &attReadObj, NULL, attServerDataCallback->context);

    MEM_MALLOC.free(attReadObj.readHandleRangeUuid.uuid);
}

void AttReadByTypeResponse_1(const AttConnectInfo *connect, Buffer *buffer)
{
    AttRead attReadObj;
    uint8_t mockList[7] = {0x03, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    AttReadByTypeRspDataList valueList[8] = {0};
    attReadObj.readHandleListNum.valueList = valueList;
    AttClientDataCallback *attClientDataCallback = AttGetATTClientCallback();
    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attReadObj.readHandleListNum.len = 0x07;  // DISCOVER_CHARACTERISTIC_LENGTH_16BIT
    attReadObj.readHandleListNum.valueNum = 0x0001;
    (attReadObj.readHandleListNum.valueList)->attHandle.attHandle = 0xFFFD;
    (attReadObj.readHandleListNum.valueList)->attributeValue = mockList;

    attClientDataCallback = AttGetATTClientCallback();
    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_READ_BY_TYPE_RESPONSE_ID, &attReadObj, NULL, attClientDataCallback->context);
}

void AttReadByTypeResponse_2(const AttConnectInfo *connect, Buffer *buffer)
{
    AttRead attReadObj;
    uint8_t mockList[7] = {0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    AttReadByTypeRspDataList valueList[8] = {0};
    attReadObj.readHandleListNum.valueList = valueList;
    AttClientDataCallback *attClientDataCallback = AttGetATTClientCallback();

    attReadObj.readHandleListNum.len = 0x08;  // FIND_INCLUDE_SERVICE_LENGTH_16BIT
    attReadObj.readHandleListNum.valueNum = 0x0001;
    (attReadObj.readHandleListNum.valueList)->attHandle.attHandle = 0xFFFC;
    (attReadObj.readHandleListNum.valueList)->attributeValue = mockList;

    attClientDataCallback = AttGetATTClientCallback();
    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_READ_BY_TYPE_RESPONSE_ID, &attReadObj, NULL, attClientDataCallback->context);
}

void AttReadByTypeResponse_3(const AttConnectInfo *connect, Buffer *buffer)
{
    AttRead attReadObj;
    uint8_t mockList[7] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    AttReadByTypeRspDataList valueList[6] = {0};
    attReadObj.readHandleListNum.valueList = valueList;
    AttClientDataCallback *attClientDataCallback = AttGetATTClientCallback();

    attReadObj.readHandleListNum.len = 0x06;  // FIND_INCLUDE_SERVICE_LENGTH_128BIT
    attReadObj.readHandleListNum.valueNum = 0x0001;
    (attReadObj.readHandleListNum.valueList)->attHandle.attHandle = 0x0000;
    (attReadObj.readHandleListNum.valueList)->attributeValue = mockList;

    attClientDataCallback = AttGetATTClientCallback();
    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_READ_BY_TYPE_RESPONSE_ID, &attReadObj, NULL, attClientDataCallback->context);
}

void AttReadRequest(AttConnectInfo *connect, Buffer *buffer)
{
    AttRead attReadObj;
    uint8_t *data = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    data = (uint8_t *)BufferPtr(buffer);
    attReadObj.readHandle.attHandle = ((uint16_t *)data)[0];

    LOG_INFO("%s enter, callback para : attHandle = %d", __FUNCTION__, attReadObj.readHandle.attHandle);

    attServerDataCallback->attServerCallback(
        connect->retGattConnectHandle, ATT_READ_REQUEST_ID, &attReadObj, NULL, attServerDataCallback->context);
}

void AttReadResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_READ_RESPONSE_ID, NULL, buffer, attClientDataCallback->context);
}

void AttReadResponse1(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_READ_RESPONSE_ID, NULL, buffer, attClientDataCallback->context);
}

void AttReadBlobRequest(AttConnectInfo *connect, Buffer *buffer)
{
    AttRead attReadObj;
    uint8_t *data = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    data = (uint8_t *)BufferPtr(buffer);
    attReadObj.readBlob.attHandle = ((uint16_t *)data)[0];
    attReadObj.readBlob.offset = ((uint16_t *)(data + GATT_UT_UINT16_LENGTH))[0];

    LOG_INFO("%s enter, callback para : attHandle = %d, offset = %d",
        __FUNCTION__,
        attReadObj.readBlob.attHandle,
        attReadObj.readBlob.offset);

    attServerDataCallback->attServerCallback(
        connect->retGattConnectHandle, ATT_READ_BLOB_REQUEST_ID, &attReadObj, NULL, attServerDataCallback->context);
}

void AttReadBlobResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(
        connect->retGattConnectHandle, ATT_READ_BLOB_RESPONSE_ID, NULL, buffer, attClientDataCallback->context);
}

void AttReadBlobResponse1(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();
    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_READ_BLOB_RESPONSE_ID, NULL, buffer, attClientDataCallback->context);
}

void AttReadMultipleRequest(AttConnectInfo *connect, Buffer *buffer)
{
    LOG_INFO("%s enter", __FUNCTION__);
    AttWrite attWriteObj;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    attServerDataCallback->attServerCallback(
        connect->retGattConnectHandle, ATT_READ_MULTIPLE_REQUEST_ID,
        &attWriteObj, buffer, attServerDataCallback->context);
}

void AttReadMultipleResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();
    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_READ_MULTIPLE_RESPONSE_ID, NULL, buffer, attClientDataCallback->context);
}

void AttReadByGroupTypeRequest(AttConnectInfo *connect, Buffer *buffer)
{
    LOG_INFO("%s enter", __FUNCTION__);
    AttRead attReadObj;
    uint8_t *data = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    data = (uint8_t *)BufferPtr(buffer);
    attReadObj.readGroupRequest.handleRange.startHandle = ((uint16_t *)data)[0];
    attReadObj.readGroupRequest.handleRange.endHandle = GATT_UT_OFFSET_3;
    attReadObj.readGroupRequest.uuid = MEM_MALLOC.alloc(sizeof(BtUuid));
    LOG_INFO("%s enter, callback para : startHandle = %d, endHandle = %d",
        __FUNCTION__,
        attReadObj.readGroupRequest.handleRange.startHandle,
        attReadObj.readGroupRequest.handleRange.endHandle);

    attServerDataCallback->attServerCallback(connect->retGattConnectHandle,
        ATT_READ_BY_GROUP_TYPE_REQUEST_ID,
        &attReadObj,
        NULL,
        attServerDataCallback->context);
    MEM_MALLOC.free(attReadObj.readGroupRequest.uuid);
}

void AttReadByGroupTypeResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttRead attReadObj;
    uint8_t mockList[1] = {4};
    AttReadGoupAttributeData attributeData[GATT_UT_ARRAY_LENGTH_8] = {0};
    AttClientDataCallback *attClientDataCallback = NULL;
    for (int i = 0; i < GATT_UT_ARRAY_LENGTH_8; i++) {
        attributeData[i].attributeValue = mockList;
    }

    attReadObj.readGroupResponse.attributeData = attributeData;

    attClientDataCallback = AttGetATTClientCallback();
    attReadObj.readGroupResponse.num = 0x0001;
    attReadObj.readGroupResponse.length = 0x06;
    uint8_t offset = 0;
    uint8_t value[16] = {0};
    uint8_t uuidLen = 0x06 - sizeof(uint16_t) - sizeof(uint16_t);

    for (int i = 0; i < 1; i++) {
        (attReadObj.readGroupResponse.attributeData + i)->groupEndHandle = 0xFFFF;
        (attReadObj.readGroupResponse.attributeData + i)->attHandle = 0xFFFB;
        memcpy((attReadObj.readGroupResponse.attributeData + i)->attributeValue, (value + offset), uuidLen);
        offset = offset + uuidLen;
    }

    attClientDataCallback->attClientCallback(connect->retGattConnectHandle,
        ATT_READ_BY_GROUP_TYPE_RESPONSE_ID,
        &attReadObj,
        NULL,
        attClientDataCallback->context);
}

void AttWriteRequest(AttConnectInfo *connect, Buffer *buffer)
{
    LOG_INFO("%s enter", __FUNCTION__);
    size_t buffSize;
    AttWrite attWriteObj;
    uint8_t *data = NULL;
    Buffer *bufferNew = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    buffSize = BufferGetSize(buffer);
    data = (uint8_t *)BufferPtr(buffer);
    attWriteObj.writeRequest.attHandle = ((uint16_t *)data)[0];
    bufferNew = BufferSliceMalloc(buffer, GATT_UT_UINT16_LENGTH, buffSize - GATT_UT_UINT16_LENGTH);
    LOG_INFO("%s enter, callback para : attHandle = %d", __FUNCTION__, attWriteObj.writeRequest.attHandle);

    attServerDataCallback->attServerCallback(
        connect->retGattConnectHandle, ATT_WRITE_REQUEST_ID, &attWriteObj, bufferNew, attServerDataCallback->context);

    BufferFree(bufferNew);
}

void AttWriteResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();
    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_WRITE_RESPONSE_ID, NULL, buffer, attClientDataCallback->context);
}

void AttWriteCommand(const AttConnectInfo *connect, Buffer *buffer)
{
    LOG_INFO("%s enter", __FUNCTION__);
    size_t buffSize;
    AttWrite attWriteObj;
    uint8_t *data = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    buffSize = BufferGetSize(buffer);
    data = (uint8_t *)BufferPtr(buffer);
    attWriteObj.writeCommand.attHandle = ((uint16_t *)data)[0];
    Buffer *bufferNew = BufferSliceMalloc(buffer, GATT_UT_UINT16_LENGTH, buffSize - GATT_UT_UINT16_LENGTH);
    LOG_INFO("%s enter, callback para : attHandle = %d", __FUNCTION__, attWriteObj.writeCommand.attHandle);
    attServerDataCallback->attServerCallback(
        connect->retGattConnectHandle, ATT_WRITE_COMMAND_ID, &attWriteObj, bufferNew, attServerDataCallback->context);
    BufferFree(bufferNew);
}

void AttSignedWriteCommand(const AttConnectInfo *connect, Buffer *buffer)
{
    LOG_INFO("%s enter", __FUNCTION__);
    size_t buffSize;
    AttWrite attWriteObj;
    uint8_t *data = NULL;
    uint16_t attValueLen;
    Buffer *bufferNew = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    buffSize = BufferGetSize(buffer);
    data = (uint8_t *)BufferPtr(buffer);
    attWriteObj.signedWriteCommand.attHandleValueObj.attHandle = ((uint16_t *)data)[0];
    attValueLen = buffSize - GATT_UT_UINT16_LENGTH - 12;
    bufferNew = BufferSliceMalloc(buffer, GATT_UT_UINT16_LENGTH, attValueLen);
    LOG_INFO("%s enter, callback para : attHandle = %d",
        __FUNCTION__,
        attWriteObj.signedWriteCommand.attHandleValueObj.attHandle);

    attServerDataCallback->attServerCallback(connect->retGattConnectHandle,
        ATT_SIGNED_WRITE_COMMAND_ID,
        &attWriteObj,
        bufferNew,
        attServerDataCallback->context);

    BufferFree(bufferNew);
}

void AttPrepareWriteRequest(AttConnectInfo *connect, Buffer *buffer)
{
    LOG_INFO("%s enter", __FUNCTION__);
    size_t buffSize;
    AttWrite attWriteObj;
    uint8_t *data = NULL;
    Buffer *bufferNew = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    buffSize = BufferGetSize(buffer);
    data = (uint8_t *)BufferPtr(buffer);
    attWriteObj.prepareWrite.handleValue.attHandle = ((uint16_t *)data)[0];
    attWriteObj.prepareWrite.offset = GATT_UT_OFFSET_3;
    bufferNew = BufferSliceMalloc(buffer, GATT_UT_OFFSET_4, buffSize - GATT_UT_OFFSET_4);
    LOG_INFO("%s enter, callback para : attHandle = %d, offset = %d",
        __FUNCTION__,
        attWriteObj.prepareWrite.handleValue.attHandle,
        attWriteObj.prepareWrite.offset);

    attServerDataCallback->attServerCallback(connect->retGattConnectHandle,
        ATT_PREPARE_WRITE_REQUEST_ID,
        &attWriteObj,
        bufferNew,
        attServerDataCallback->context);

    BufferFree(bufferNew);
}

void AttPrepareWriteResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttWrite attWriteObj;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    attClientDataCallback->attClientCallback(connect->retGattConnectHandle,
        ATT_PREPARE_WRITE_RESPONSE_ID,
        &attWriteObj,
        NULL,
        attClientDataCallback->context);
}

void AttPrepareWriteResponse1(const AttConnectInfo *connect, Buffer *buffer)
{
    AttWrite attWriteObj;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();
    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(connecthandle_,
        ATT_PREPARE_WRITE_RESPONSE_ID,
        &attWriteObj,
        NULL,
        attClientDataCallback->context);
}

void AttExecuteWriteRequest(AttConnectInfo *connect, Buffer *buffer)
{
    LOG_INFO("%s enter", __FUNCTION__);

    AttWrite attWriteObj;
    uint8_t *data = NULL;
    AttServerDataCallback *attServerDataCallback = NULL;
    attServerDataCallback = AttGetATTServerCallback();
    data = (uint8_t *)BufferPtr(buffer);
    attWriteObj.excuteWrite.flag = data[0];

    LOG_INFO("%s enter, callback para : flag = %d", __FUNCTION__, attWriteObj.excuteWrite.flag);

    attServerDataCallback->attServerCallback(connect->retGattConnectHandle,
        ATT_EXECUTE_WRITE_REQUEST_ID,
        &attWriteObj,
        NULL,
        attServerDataCallback->context);
}

void AttExecuteWriteResponse(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_EXECUTE_WRITE_RESPONSE_ID, NULL, buffer, attClientDataCallback->context);
}

void AttTransactionTimeOutId(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, ATT_TRANSACTION_TIME_OUT_ID, NULL, buffer, attClientDataCallback->context);
}

void AttNull(const AttConnectInfo *connect, Buffer *buffer)
{
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(
        connecthandle_, 0x01, NULL, buffer, attClientDataCallback->context);
}

void AttHandleValueNotification(const AttConnectInfo *connect, Buffer *buffer)
{
    AttHandleValue AttHandleValueObj;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();
    attClientDataCallback->attClientCallback(connect->retGattConnectHandle,
        ATT_HANDLE_VALUE_NOTIFICATION_ID,
        &AttHandleValueObj,
        NULL,
        attClientDataCallback->context);
}

void AttHandleValueNotification1(const AttConnectInfo *connect, Buffer *buffer)
{
    AttHandleValue AttHandleValueObj;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();

    uint8_t *data = NULL;
    data = (uint8_t *)BufferPtr(buffer);
    uint16_t connecthandle_ = ((uint16_t *)data)[0];

    attClientDataCallback->attClientCallback(connecthandle_,
        ATT_HANDLE_VALUE_NOTIFICATION_ID,
        &AttHandleValueObj,
        NULL,
        attClientDataCallback->context);
}

void AttHandleValueIndication(const AttConnectInfo *connect, Buffer *buffer)
{
    AttHandleValue AttHandleValueObj;
    AttClientDataCallback *attClientDataCallback = NULL;

    attClientDataCallback = AttGetATTClientCallback();
    attClientDataCallback->attClientCallback(connect->retGattConnectHandle,
        ATT_HANDLE_VALUE_INDICATION_ID,
        &AttHandleValueObj,
        NULL,
        attClientDataCallback->context);
}

void AttHandleValueConfirmation(AttConnectInfo *connect, Buffer *buffer)
{
    AttServerDataCallback *attServerDataCallback = NULL;
    AttWrite attWriteObj;

    attServerDataCallback = AttGetATTServerCallback();

    attServerDataCallback->attServerCallback(
        connect->retGattConnectHandle, ATT_HANDLE_VALUE_CONFIRMATION_ID,
        &attWriteObj, NULL, attServerDataCallback->context);
}