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
#include "common_mock_all.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "errno.h"
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include "map_mce_types.h"
#include "mce_mock.h"

#include "sdp.h"
#include "gap_if.h"
#include "rfcomm.h"

extern "C" {
#include "btm.h"
#include "btstack.h"
#include "log.h"
}

BtAddr btAddr;
int gapCtrl = MCE_TEST_CTRL_RESPONES_OK;
int sdpCtrl = MCE_TEST_CTRL_RESPONES_OK;
int callbackExcuteStatus;
int callbackConnectionStatus;

void MapMceMnsTransTestCallback::OnTransportIncomingDisconnected(const std::string &btAddr)
{}
void MapMceMnsTransTestCallback::OnTransportConnected(ObexTransport &transport)
{}
void MapMceMnsTransTestCallback::OnTransportDisconnected(ObexTransport &transport)
{}
void MapMceMnsTransTestCallback::OnTransportDataBusy(ObexTransport &transport, uint8_t isBusy)
{}
void MapMceMnsTransTestCallback::OnTransportDataAvailable(ObexTransport &transport, ObexPacket &obexPacket)
{}
void MapMceMnsTransTestCallback::OnTransportError(ObexTransport &transport, int errCd)
{}
void MapMceMnsTransTestCallback::OnTransportConnectIncoming(ObexIncomingConnect &incomingConnect)
{}

void (*ServiceSearchCb)(const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context);

int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
{
    return 0;
}

/**
 * @brief Register sdp service
 */
int SDP_RegisterServiceRecord(uint32_t handle)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

int SDP_ServiceSearchAttribute(const BtAddr *addr, const SdpUuid *uuidArray, SdpAttributeIdList attributeIdList,
    void *context,
    void (*ServiceSearchAttributeCb)(
        const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context))
{
    LOG_INFO("%s enter", __FUNCTION__);
    btAddr = *addr;
    ServiceSearchCb = ServiceSearchAttributeCb;
    LOG_INFO("%s end,addres[0][1][2]=%d,%d,%d", __FUNCTION__, btAddr.addr[0], btAddr.addr[1], btAddr.addr[2]);
    return 0;
}

/**
 * @brief Add AdditionalProtocolDescriptorList Attribute to record
 * @see Refer to charter 5.1.6 of Core 5.0
 */
int SDP_AddAdditionalProtocolDescriptorList(
    uint32_t handle, const SdpAdditionalProtocolDescriptor *descriptorList, uint16_t descriptorListNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add Attribute, which value is sequence, undefined in Core 5.0 specification
 * @see Refer to charter 2.2 of Core 5.0
 */
int SDP_AddSequenceAttribute(
    uint32_t handle, uint16_t attributeId, const uint8_t *attributeValue, uint16_t attributeValueLength)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Create sdp record
 */
uint32_t SDP_CreateServiceRecord()
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add ServiceClassIDList Attribute to record
 * @see Refer to charter 5.1.2 of Core 5.0
 */
int SDP_AddServiceClassIdList(uint32_t handle, const BtUuid *classid, uint16_t classidNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add ServiceName Attribute to record
 * @see Refer to charter 5.1.15 of Core 5.0
 */
int SDP_AddServiceName(uint32_t handle, uint16_t baseAttributeId, const char *name, uint16_t nameLen)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add BluetoothProfileDescriptorList Attribute to record
 * @see Refer to charter 5.1.11 of Core 5.0
 */
int SDP_AddBluetoothProfileDescriptorList(
    uint32_t handle, const SdpProfileDescriptor *profileDescriptor, uint16_t profileDescriptorNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

uint8_t RFCOMM_AssignServerNum()
{
    return 0x0f;
}

int RFCOMM_FreeServerNum(uint8_t scn)
{
    return 0;
}

int SDP_AddProtocolDescriptorList(uint32_t handle, const SdpProtocolDescriptor *descriptor, uint16_t descriptorNumber)
{
    return 0;
}

int SDP_DeregisterServiceRecord(uint32_t handle)
{
    return 0;
}

int SDP_DestroyServiceRecord(uint32_t handle)
{
    return 0;
}

int GAPIF_UnregisterServiceSecurity(GAP_ServiceConnectDirection direction, GAP_Service serviceId)
{
    LOG_INFO("%s excute", __FUNCTION__);
    return 0;
}

int TEST_MasServiceSdpSearchCallback(uint8_t attributeId, uint32_t rfcomNo)
{
    LOG_INFO("%s enter", __FUNCTION__);

    SdpService serviceAry;
    SdpAttribute sequceAttArry[3];
    char testname[] = "testinstance";
    int featrues = 0x003F7FFF;
    int messageType = 0xEEFFEEFF;

    // attributeValue2 is 1 or 0   MCE_MNS_GOEP_L2CAP_PSM_VALUE
    // attributeId is 25
    sequceAttArry[0].attributeId = 0x0315;  // MAP_MCE_INSTANCE_ATTRIBUTE_ID
    sequceAttArry[0].attributeValue = &attributeId;
    sequceAttArry[0].attributeValueLength = 1;

    sequceAttArry[1].attributeId = 0x0317;                   // MAP_MCE_SUPPORTED_FEATURES_ATTRIBUTE_ID
    sequceAttArry[1].attributeValue = (uint8_t *)&featrues;  // MAP_MCE_SUPPORTED_FEATURES_V14 is 0x003F7FFF
    sequceAttArry[1].attributeValueLength = 4;

    sequceAttArry[2].attributeId = 0x0316;  // MAP_MCE_SUPPORTED_MESSAGE_TYPE_ATTRIBUTE_ID
    sequceAttArry[2].attributeValue = &messageType;
    sequceAttArry[2].attributeValueLength = 4;

    serviceAry.attribute = sequceAttArry;
    serviceAry.attributeNumber = 3;

    // attributeId is 0x0200 //MAP_MCE_GOEP_L2CAP_PSM_ATTRIBUTE_ID
    // attributeValue is attributeValue;
    // attributeValueLength is 2
    // attributeId is 0x0200 //MAP_MCE_GOEP_L2CAP_PSM_ATTRIBUTE_ID
    // attributeValue is attributeValue;
    // attributeValueLength is 2

    SdpProtocolDescriptor addProDescrip[2];
    // reserved addProDescrip(0).protocolUuid.uuid16 is 0x0100; // UUID_PROTOCOL_L2CAP
    // MCE_PROTOCOL_DESCRIPTOR_L2CAP_UUID
    addProDescrip[0].protocolUuid.uuid16 = UUID_PROTOCOL_RFCOMM;  // MCE_PROTOCOL_DESCRIPTOR_RFCOMM_UUID
    addProDescrip[0].parameter[0].type = SDP_TYPE_UINT_16;
    addProDescrip[0].parameter[0].value = rfcomNo;
    serviceAry.descriptor = addProDescrip;
    serviceAry.descriptorNumber = 1;
    serviceAry.serviceName = testname;

    if (sdpCtrl == MCE_TEST_CTRL_RESPONES_OK) {
        if (ServiceSearchCb != nullptr) {
            ServiceSearchCb(&btAddr, &serviceAry, 1, 0);
            LOG_INFO("%s success,rfcomNo = %d", __FUNCTION__, rfcomNo);
        }
    } else {
        ServiceSearchCb(&btAddr, nullptr, 0, 0);
        LOG_INFO("%s ng", __FUNCTION__);
    }

    LOG_INFO("%s end,rfcomNo = %d", __FUNCTION__, rfcomNo);
    return BT_NO_ERROR;
}

int GAPIF_RegisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo, uint16_t securityMode)
{
    LOG_INFO("%s excute", __FUNCTION__);
    if (gapCtrl == MCE_TEST_CTRL_RESPONES_OK) {
        return BT_NO_ERROR;
    } else {
        return BT_OPERATION_FAILED;
    }
}

int GAPIF_DeregisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo)
{
    LOG_INFO("%s excute", __FUNCTION__);
    return 0;
}

int GAPIF_RequestSecurity(const BtAddr *addr, const GapRequestSecurityParam *param)
{
    return 0;
}

int BTM_Initialize()
{
    LOG_INFO("[%s]\n", __func__);
    return BT_NO_ERROR;
}

int BTM_Close()
{
    return BT_NO_ERROR;
}

int BTM_Enable(int controller)
{
    return BT_NO_ERROR;
}

int BTM_Disable(int controller)
{
    return BT_NO_ERROR;
}

bool MapMceAdapterConfigMock::GetValueInt(const std::string &section, const std::string &property, int &value)
{
    if (property == bluetooth::PROPERTY_L2CAP_MTU) {
        value = 1024;
        return true;
    }
    if (property == bluetooth::PROPERTY_RFCOMM_MTU) {
        value = 512;
        return true;
    }
    if (property == bluetooth::PROPERTY_MAP_MAX_OF_GET_UREAD) {
        value = 100;
        return true;
    }
    return false;
}

bool MapMceAdapterConfigMock::GetValueBool(const std::string &section, const std::string &property, bool &value)
{
    if (property == bluetooth::PROPERTY_SRM_ENABLE) {
        value = true;
        return true;
    }
    return false;
}

void BTM_AddLocalL2capPsmForLogging(uint8_t module, uint16_t psm)
{}
void BTM_AddRemoteL2capPsmForLogging(uint8_t module, uint16_t psm, const BtAddr *remoteAddr)
{}
void BTM_RemoveLocalL2capPsmForLogging(uint8_t module, uint16_t psm)
{}
void BTM_RemoveRemoteL2capPsmForLogging(uint8_t module, uint16_t psm, const BtAddr *remoteAddr)
{}
void BTM_AddLocalRfcommScnForLogging(uint8_t module, uint8_t scn)
{}
void BTM_AddRemoteRfcommScnForLogging(uint8_t module, uint8_t scn, const BtAddr *remoteAddr)
{}
void BTM_RemoveLocalRfcommScnChannelForLogging(uint8_t module, uint8_t scn)
{}
void BTM_RemoveRemoteRfcommScnChannelForLogging(uint8_t module, uint8_t scn, const BtAddr *remoteAddr)
{}
