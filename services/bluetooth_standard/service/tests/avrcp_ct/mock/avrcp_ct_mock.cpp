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

#include "avrcp_ct_mock.h"

using namespace testing;

MockSdp *mockSdp = nullptr;
MockGap *mockGap = nullptr;
MockAvctp *mockAvctp = nullptr;
AvctConnectParam g_connParam;

void registerGapMocker(MockGap *mocker)
{
    mockGap = mocker;
}
MockGap &GetGapMocker()
{
    return *mockGap;
}

void registerSdpMocker(MockSdp *mocker)
{
    mockSdp = mocker;
}
MockSdp &GetSdpMocker()
{
    return *mockSdp;
}

void registerAvctpMocker(MockAvctp *mocker)
{
    mockAvctp = mocker;
}
MockAvctp &GetAvctpMocker()
{
    return *mockAvctp;
}

AvctConnectParam GetAvctConnectParam()
{
    return g_connParam;
}

int GAPIF_RegisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo, uint16_t securityMode)
{
    return mockGap->GAPIF_RegisterServiceSecurity();
}

int GAPIF_DeregisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo)
{
    return mockGap->GAPIF_DeregisterServiceSecurity();
}

uint32_t SDP_CreateServiceRecord()
{
    return mockSdp->SDP_CreateServiceRecord();
}

int SDP_AddServiceClassIdList(uint32_t handle, const BtUuid *classid, uint16_t classidNumber)
{
    return mockSdp->SDP_AddServiceClassIdList();
}

int SDP_AddProtocolDescriptorList(uint32_t handle, const SdpProtocolDescriptor *descriptor, uint16_t descriptorNumber)
{
    return mockSdp->SDP_AddBluetoothProfileDescriptorList();
}
int SDP_AddAdditionalProtocolDescriptorList(
    uint32_t handle, const SdpAdditionalProtocolDescriptor *descriptorList, uint16_t descriptorListNumber)
{
    return mockSdp->SDP_AddAdditionalProtocolDescriptorList();
}

int SDP_AddBluetoothProfileDescriptorList(
    uint32_t handle, const SdpProfileDescriptor *profileDescriptor, uint16_t profileDescriptorNumber)
{
    return mockSdp->SDP_AddBluetoothProfileDescriptorList();
}

int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
{
    return mockSdp->SDP_AddAttribute();
}

int SDP_AddServiceName(uint32_t handle, uint16_t baseAttributeId, const char *name, uint16_t nameLen)
{
    return mockSdp->SDP_AddServiceName();
}

int SDP_RegisterServiceRecord(uint32_t handle)
{
    return mockSdp->SDP_RegisterServiceRecord();
}

int SDP_AddBrowseGroupList(uint32_t handle, const BtUuid *browseUuid, uint16_t browseUuidNumber)
{
    return mockSdp->SDP_AddBrowseGroupList();
}

int SDP_DestroyServiceRecord(uint32_t handle)
{
    return mockSdp->SDP_DestroyServiceRecord();
}

int SDP_DeregisterServiceRecord(uint32_t handle)
{
    return mockSdp->SDP_DeregisterServiceRecord();
}

int SDP_ServiceSearch(const BtAddr *addr, const SdpUuid *uuidArray, void *context,
    void (*ServiceSearchCb)(const BtAddr *addr, const uint32_t *handleArray, uint16_t handleNum, void *context))
{
    return mockSdp->SDP_ServiceSearch();
}

void AVCT_Register(uint16_t mtu, uint16_t mtuBr, uint16_t role)
{
    return mockAvctp->AVCT_Register();
}

uint16_t AVCT_ConnectReq(uint8_t *connId, const AvctConnectParam *connParam, const BtAddr *peerAddr)
{
    memcpy(&g_connParam, connParam, sizeof(AvctConnectParam));

    return mockAvctp->AVCT_ConnectReq();
}
void AVCT_Deregister()
{
    return mockAvctp->AVCT_Deregister();
}
uint16_t AVCT_DisconnectReq(uint8_t connId)
{
    return mockAvctp->AVCT_DisconnectReq();
}

uint16_t AVCT_BrConnectReq(uint8_t connId, uint8_t role)
{
    return mockAvctp->AVCT_BrConnectReq();
}

uint16_t AVCT_BrDisconnectReq(uint8_t connId)
{
    return mockAvctp->AVCT_BrDisconnectReq();
}

uint16_t AVCT_SendMsgReq(uint8_t connId, uint8_t label, uint8_t cr, const Packet *msg)
{
    return mockAvctp->AVCT_SendMsgReq();
}

uint16_t AVCT_BrSendMsgReq(uint8_t connId, uint8_t label, uint8_t cr, const Packet *msg)
{
    return mockAvctp->AVCT_SendMsgReq();
}

