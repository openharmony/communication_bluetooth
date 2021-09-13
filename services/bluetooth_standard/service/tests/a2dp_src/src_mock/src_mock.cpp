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

#include "a2dp_mock.h"

BtAddr btAddr;
typedef void (*ServiceSearchhAttributeCbT)(const BtAddr *, const SdpService *, uint16_t, void *);
typedef void (*ServiceSearchCbT)(const BtAddr *, const uint32_t *, uint16_t, void *);
ServiceSearchhAttributeCbT ServiceSearchAttributeCb;
int SDP_ServiceSearchAttribute(const BtAddr *addr, const SdpUuid *uuidArray, SdpAttributeIdList attributeIdList,
    void *context, ServiceSearchhAttributeCbT callback)
{
    LOG_INFO("%s enter", __FUNCTION__);
    btAddr = *addr;
    ServiceSearchAttributeCb = callback;
    LOG_INFO("%s end,addres[0][1][2]=%d,%d,%d", __FUNCTION__, btAddr.addr[0], btAddr.addr[1], btAddr.addr[2]);
    return 0;
}

int GAPIF_UnregisterServiceSecurity(GAP_ServiceConnectDirection direction, GAP_Service serviceId)
{
    LOG_INFO("%s excute", __FUNCTION__);
    return 0;
}

int GAPIF_RegisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo, uint16_t securityMode)
{
    LOG_INFO("%s excute", __FUNCTION__);
    return 0;
}

int GAPIF_DeregisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo)
{
    LOG_INFO("%s excute", __FUNCTION__);
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

/**
 * @brief Create sdp record
 */
uint32_t SDP_CreateServiceRecord()
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0x0018;
}
int SDP_DestroyServiceRecord(uint32_t handle)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}
/**
 * @brief Register sdp service
 */
int SDP_RegisterServiceRecord(uint32_t handle)
{
    LOG_INFO("%s enter", __FUNCTION__, handle);
    return 0;
}
/**
 * @brief Deregister sdp service
 */
int SDP_DeregisterServiceRecord(uint32_t handle)
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
 * @brief Add ServiceRecordState Attribute to record
 * @see Refer to charter 5.1.3 of Core 5.0
 */
int SDP_AddServiceRecordState(uint32_t handle, uint32_t state)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add ServiceID Attribute to record
 * @see Refer to charter 5.1.4 of Core 5.0
 */
int SDP_AddServiceId(uint32_t handle, BtUuid *serviceid)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add ProtocolDescriptorList Attribute to record
 * @see Refer to charter 5.1.5 of Core 5.0
 */
int SDP_AddProtocolDescriptorList(uint32_t handle, const SdpProtocolDescriptor *descriptor, uint16_t descriptorNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
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
 * @brief Add BrowseGroupList Attribute to record
 * @see Refer to charter 5.1.7 of Core 5.0
 */
int SDP_AddBrowseGroupList(uint32_t handle, BtUuid *browseUuid, uint16_t browseUuidNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add ServiceInfoTimeToLive Attribute to record
 * @see Refer to charter 5.1.9 of Core 5.0
 */
int SDP_AddServiceInfoTimeToLive(uint32_t handle, uint32_t value)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add ServiceAvailability Attribute to record
 * @see Refer to charter 5.1.10 of Core 5.0
 */
int SDP_AddServiceAvailability(uint32_t handle, uint8_t value)
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

/**
 * @brief Add DocumentationURL Attribute to record
 * @see Refer to charter 5.1.12 of Core 5.0
 */
int SDP_AddDocumentationUrl(uint32_t handle, uint8_t *url, uint16_t urlLen)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add ClientExecutableURL Attribute to record
 * @see Refer to charter 5.1.13 of Core 5.0
 */
int SDP_AddClientExecutableUrl(uint32_t handle, uint8_t *url, uint16_t urlLen)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add IconURL Attribute to record
 * @see Refer to charter 5.1.14 of Core 5.0
 */
int SDP_AddIconUrl(uint32_t handle, uint8_t *url, uint16_t urlLen)
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
    LOG_INFO("Service Name : %s", name);
    return 0;
}

/**
 * @brief Add ServiceDescription Attribute to record
 * @see Refer to charter 5.1.16 of Core 5.0
 */
int SDP_AddServiceDescription(uint32_t handle, uint16_t baseAttributeId, char *description, uint16_t descriptionLen)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add ProviderName Attribute to record
 * @see Refer to charter 5.1.17 of Core 5.0
 */
int SDP_AddProviderName(uint32_t handle, uint16_t baseAttributeId, char *name, uint16_t nameLen)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

/**
 * @brief Add Attribute undefined in Core 5.0 specification
 * @see Refer to charter 2.2 of Core 5.0
 */
int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
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

int SDP_ServiceSearch(const BtAddr *addr, const SdpUuid *uuidArray, void *context, ServiceSearchCbT callback)
{
    return 0;
}

int GAPIF_RequestSecurity(const BtAddr *addr, const GapRequestSecurityParam *param)
{
    return 0;
}
