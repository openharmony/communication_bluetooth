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

#include <memory.h>
#include "src/att/att_receive.h"
#include "src/att/att_common.h"
#include "include/att.h"
#include "buffer.h"
#include "packet.h"
#include "list.h"
#include "log.h"
#include "gap_if.h"
#include "mock/att_send_mock.h"

static AttClientSendDataCallback g_attSendDataCB;
static AttServerSendDataCallback g_attSendDataCB_s;
extern AsyncCallInfromation_2 asyncCallInfromation_2;

void AttSendRespCallback(uint16_t lcid, int result)
{
    AttConnectInfo *connect = new AttConnectInfo();
    uint16_t handle = lcid == 0 ? connect->retGattConnectHandle : lcid;

    g_attSendDataCB.attSendDataCB(handle, result, g_attSendDataCB.context);

    delete connect;
}

void AttSendRespCallback_server(uint16_t lcid, int result)
{

    AttConnectInfo *connect = new AttConnectInfo();

    g_attSendDataCB_s.attSendDataCB(connect->retGattConnectHandle, result, g_attSendDataCB.context);

    delete connect;
}

void  ATT_ReadByTypeRequest(
    uint16_t connectHandle, uint16_t startHandle, uint16_t endHandle, const BtUuid *uuid)
{
    if (connectHandle == 0x13 || connectHandle == 0) {
        return;
    } else {
        return;
    }
}
void ATT_ReadByGroupTypeRequest(
    uint16_t connectHandle, uint16_t startHandle, uint16_t endHandle, const BtUuid *uuid)
{
    if (connectHandle == 0x13 || connectHandle == 0) {
        return;
    } else {
        return;
    }
}
void ATT_ReadMultipleRequest(uint16_t connectHandle,const Buffer *handleList)
{}
void ATT_WriteCommand(uint16_t connectHandle, uint16_t attHandle,const Buffer *attValue)
{}
void ATT_PrepareWriteRequest(uint16_t connectHandle, AttReadBlobReqPrepareWriteValue attReadBlobObj, const Buffer *attValue)
{}
void ATT_HandleValueConfirmation(uint16_t connectHandle)
{}

void ATT_ExchangeMTURequest(uint16_t connectHandle, uint16_t clientRxMTU)
{
    std::unique_lock<std::mutex> lock_discisvc(asyncCallInfromation_2.mutex_);
    asyncCallInfromation_2.isCompleted_ = true;
    asyncCallInfromation_2.condition_.notify_all();
}

void ATT_ReadByGroupTypeRequest(uint16_t connectHandle, uint16_t startHandle, uint16_t endHandle, BtUuid *uuid)
{
    std::unique_lock<std::mutex> lock_discisvc(asyncCallInfromation_2.mutex_);
    asyncCallInfromation_2.isCompleted_ = true;
    asyncCallInfromation_2.condition_.notify_all();
}

void ATT_FindByTypeValueRequest(uint16_t connectHandle, const AttFindByTypeValueReq *attFindByTypePtreve, const Buffer *attValue)
{}

void ATT_ReadByTypeRequest(uint16_t connectHandle, uint16_t startHandle, uint16_t endHandle, BtUuid *uuid)
{
    std::unique_lock<std::mutex> lock_discisvc(asyncCallInfromation_2.mutex_);
    asyncCallInfromation_2.isCompleted_ = true;
    asyncCallInfromation_2.condition_.notify_all();
}

void ATT_FindInformationRequest(uint16_t connectHandle, uint16_t startHandle, uint16_t endHandle)
{}

void ATT_ReadRequest(uint16_t connectHandle, uint16_t attHandle)
{
    std::unique_lock<std::mutex> lock_discisvc(asyncCallInfromation_2.mutex_);
    asyncCallInfromation_2.isCompleted_ = true;
    asyncCallInfromation_2.condition_.notify_all();
}

void ATT_WriteRequest(uint16_t connectHandle, uint16_t attHandle, const Buffer *attValue)
{}

void AttCallBackCopyToCommon(attSendDataCallback AttSendDataCB, void *context)
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attSendDataCB.attSendDataCB = AttSendDataCB;
    g_attSendDataCB.context = context;

    return;
}

void ATT_ClientSendDataRegister(attSendDataCallback AttSendDataCB, void *context)
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attSendDataCB.attSendDataCB = AttSendDataCB;
    g_attSendDataCB.context = context;

    AttCallBackCopyToCommon(AttSendDataCB, context);

    return;
}

void ATT_ServerSendDataRegister(attSendDataCallback AttSendDataCB, void *context)
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attSendDataCB_s.attSendDataCB = AttSendDataCB;
    g_attSendDataCB_s.context = context;

    AttCallBackCopyToCommon(AttSendDataCB, context);

    return;
}

void AttCallBackDelectCopyToCommon()
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attSendDataCB.attSendDataCB = NULL;
    g_attSendDataCB.context = NULL;

    return;
}

void ATT_ClientSendDataDeRegister()
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attSendDataCB.attSendDataCB = NULL;
    g_attSendDataCB.context = NULL;

    AttCallBackDelectCopyToCommon();

    return;
}

void ATT_ServerSendDataDeRegister()
{
    LOG_INFO("%s enter", __FUNCTION__);

    g_attSendDataCB_s.attSendDataCB = NULL;
    g_attSendDataCB_s.context = NULL;

    AttCallBackDelectCopyToCommon();

    return;
}

void ATT_ErrorResponse(uint16_t connectHandle, const AttError *ATTErrorPtr)
{
    std::unique_lock<std::mutex> lock_discisvc(asyncCallInfromation_2.mutex_);
    asyncCallInfromation_2.isCompleted_ = true;
    asyncCallInfromation_2.condition_.notify_all();
}

void ATT_ExchangeMTUResponse(uint16_t connectHandle, uint16_t serverRxMTU)
{}

void ATT_ReadByGroupTypeResponse(
    uint16_t connectHandle, uint8_t length,const AttReadGoupAttributeData *serviceList, uint16_t serviceNum)
{}

void ATT_FindByTypeValueResponse(uint16_t connectHandle, const AttHandleInfo *handleInfoList, uint16_t listNum)
{}

void ATT_ReadByTypeResponse(
    uint16_t connectHandle, uint8_t length, const AttReadByTypeRspDataList *valueList, uint16_t attrValueNum)
{}

void ATT_FindInformationResponse(
    uint16_t connectHandle, uint8_t format, AttHandleUuid *handleUUIDPairs, uint16_t pairNum)
{}

void ATT_ReadResponse(uint16_t connectHandle, const Buffer *attValue)
{}

void ATT_PrepareWriteResponse(uint16_t connectHandle, AttReadBlobReqPrepareWriteValue attReadBlobObj,const Buffer *attValue)
{}

void ATT_HandleValueNotification(uint16_t connectHandle, uint16_t attHandle, const Buffer *attValue)
{}

void ATT_HandleValueIndication(uint16_t connectHandle, uint16_t attHandle, const Buffer *attValue)
{}

void ATT_WriteResponse(uint16_t connectHandle)
{}

void ATT_ExecuteWriteResponse(uint16_t connectHandle)
{}

void AssembleDataPackage(uint8_t *dest, uint8_t *offset, uint8_t *src, uint8_t size)
{
    memcpy((dest + *offset), src, size);
    *offset = *offset + size;
}

void ATT_SignedWriteCommand(uint16_t connectHandle, uint16_t attHandle,const Buffer *attValue)
{}

void ATT_ExecuteWriteRequest(uint16_t connectHandle, uint8_t flag)
{}

void ATT_ReadBlobRequest(uint16_t connectHandle, uint16_t attHandle, uint16_t offset)
{}

void ATT_ReadBlobResponse(uint16_t connectHandle, const Buffer *attReadBlobResObj)
{}