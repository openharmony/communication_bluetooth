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

#include "stack_mock.h"

Mock_Stack *g_stackmocker = NULL;
RFCOMM_EventCallback g_RegisterServerCb = NULL;
SdpSearchCbParam g_sdpCb_param = {0};
RfcommEventCb g_rfcCb = {0};
L2capEventCb g_l2cCb = {0};
L2capService g_svc;
L2capConnectReqCb g_l2cConReqCb = {0};

// RFCOMM
uint8_t RFCOMM_AssignServerNum()
{
    return g_stackmocker->RFCOMM_AssignServerNum();
}

int RFCOMM_RegisterServer(uint8_t scn, uint16_t mtu, uint32_t eventMask, RFCOMM_EventCallback callback, void *context)
{
    g_rfcCb.callBack = callback;
    g_rfcCb.context = context;
    return g_stackmocker->RFCOMM_RegisterServer();
}

int RFCOMM_DeregisterServer(uint8_t scn, bool isRemoveCallback)
{
    return g_stackmocker->RFCOMM_DeregisterServer();
}

int RFCOMM_FreeServerNum(uint8_t scn)
{
    return RFCOMM_SUCCESS;
}

int RFCOMM_ConnectChannel(const RfcommConnectReqInfo *reqInfo, uint16_t *handle)
{
    g_rfcCb.callBack = reqInfo->callback;
    g_rfcCb.context = reqInfo->context;
    return g_stackmocker->RFCOMM_ConnectChannel();
}

int RFCOMM_DisconnectChannel(uint16_t handle)
{
    return RFCOMM_SUCCESS;
}

int RFCOMM_AcceptConnection(uint16_t handle)
{
    return RFCOMM_SUCCESS;
}

int RFCOMM_RejectConnection(uint16_t handle)
{
    return RFCOMM_SUCCESS;
}

uint16_t RFCOMM_GetRecvMTU(uint16_t handle)
{
    return RFCOMM_SUCCESS;
}

int RFCOMM_Read(uint16_t handle, Packet **pkt)
{
    return g_stackmocker->RFCOMM_Read(handle, pkt);
}

int RFCOMM_Write(uint16_t handle, Packet *pkt)
{
    return g_stackmocker->RFCOMM_Write();
}

BtAddr RFCOMM_GetPeerAddress(uint16_t handle)
{
    return g_stackmocker->RFCOMM_GetPeerAddress();
}

// SDP
uint32_t SDP_CreateServiceRecord()
{
    return 0;
}

int SDP_RegisterServiceRecord(uint32_t handle)
{
    return g_stackmocker->SDP_RegisterServiceRecord();
}

int SDP_AddServiceClassIdList(uint32_t handle, const BtUuid *classid, uint16_t classidNumber)
{
    return g_stackmocker->SDP_AddServiceClassIdList();
}

int SDP_AddProtocolDescriptorList(uint32_t handle, const SdpProtocolDescriptor *descriptor, uint16_t descriptorNumber)
{
    return g_stackmocker->SDP_AddProtocolDescriptorList();
}

int SDP_AddBluetoothProfileDescriptorList(
    uint32_t handle, const SdpProfileDescriptor *profileDescriptor, uint16_t profileDescriptorNumber)
{
    return g_stackmocker->SDP_AddBluetoothProfileDescriptorList();
}

int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
{
    return g_stackmocker->SDP_AddAttribute();
}

int SDP_AddBrowseGroupList(uint32_t handle, const BtUuid *browseUuid, uint16_t browseUuidNumber)
{
    return g_stackmocker->SDP_AddBrowseGroupList();
}

int SDP_ServiceSearchAttribute(const BtAddr *addr, const SdpUuid *uuidArray, SdpAttributeIdList attributeIdList,
    void *context,
    void (*ServiceSearchAttributeCb)(
        const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context))
{
    int ret = g_stackmocker->SDP_ServiceSearchAttribute();
    if (ret == 0) {
        g_sdpCb_param.serviceArray.descriptorNumber = 1;
        g_sdpCb_param.serviceArray.descriptor->protocolUuid.uuid16 = UUID_PROTOCOL_RFCOMM;
        g_sdpCb_param.serviceArray.descriptor->parameter->type = SDP_TYPE_UINT_8;
        g_sdpCb_param.serviceArray.descriptor->parameter->value = 1;
        ServiceSearchAttributeCb(&g_sdpCb_param.btaddr, &g_sdpCb_param.serviceArray, g_sdpCb_param.serviceNum, context);
    }

    return ret;
}

int SDP_DeregisterServiceRecord(uint32_t handle)
{
    return g_stackmocker->SDP_DeregisterServiceRecord();
}

int SDP_DestroyServiceRecord(uint32_t handle)
{
    return g_stackmocker->SDP_DestroyServiceRecord();
}

int SDP_AddServiceName(uint32_t handle, uint16_t baseAttributeId, const char *name, uint16_t nameLen)
{
    return g_stackmocker->SDP_AddServiceName();
}

// GAP
int GAPIF_RegisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo, uint16_t securityMode)
{
    return GAP_SUCCESS;
}

int GAPIF_DeregisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo)
{
    return GAP_SUCCESS;
}

// L2CAP
int L2CIF_RegisterService(uint16_t lpsm, const L2capService *svc, void *context, void (*cb)(uint16_t lpsm, int result))
{
    memcpy(&g_svc, svc, sizeof(L2capService));
    g_l2cCb.callBack = g_svc;
    g_l2cCb.context = context;
    return g_stackmocker->L2CIF_RegisterService();
}

void L2CIF_DeregisterService(uint16_t lpsm, void (*cb)(uint16_t lpsm, int result))
{
    return g_stackmocker->L2CIF_DeregisterService();
}

int L2CIF_ConnectReq(const BtAddr *addr, uint16_t lpsm, uint16_t rpsm, void *context,
    void (*cb)(const BtAddr *addr, uint16_t lcid, int result, void *context))
{
    g_l2cConReqCb.context = context;
    g_l2cConReqCb.cb = cb;
    return g_stackmocker->L2CIF_ConnectReq();
}

void L2CIF_ConnectRsp(
    uint16_t lcid, uint8_t id, uint16_t result, uint16_t status, void (*cb)(uint16_t lcid, int result))
{
    return g_stackmocker->L2CIF_ConnectRsp();
}

int L2CIF_ConfigReq(uint16_t lcid, const L2capConfigInfo *cfg, void (*cb)(uint16_t lcid, int result))
{
    return g_stackmocker->L2CIF_ConfigReq();
}

int L2CIF_ConfigRsp(
    uint16_t lcid, uint8_t id, const L2capConfigInfo *cfg, uint16_t result, void (*cb)(uint16_t lcid, int result))
{
    return g_stackmocker->L2CIF_ConfigRsp();
}

void L2CIF_DisconnectionReq(uint16_t lcid, void (*cb)(uint16_t lcid, int result))
{
    return g_stackmocker->L2CIF_DisconnectionReq();
}

void L2CIF_DisconnectionRsp(uint16_t lcid, uint8_t id, void (*cb)(uint16_t lcid, int result))
{
    return g_stackmocker->L2CIF_DisconnectionRsp();
}

int L2CIF_SendData(uint16_t lcid, const Packet *pkt, void (*cb)(uint16_t lcid, int result))
{
    return g_stackmocker->L2CIF_SendData();
}
