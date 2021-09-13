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
#include "map_mse_service.h"
#include "sdp.h"
#include "string.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "mse_mock.h"
#include "securec.h"
extern "C" {
#include "btm.h"
#include "btstack.h"
#include "log.h"
}

using namespace bluetooth;
static int retErr = 0;
static uint8_t frComno = 0;
static const uint8_t RFCOMNO_100 = 100;

MseMock::MseMock()
{}
MseMock::~MseMock()
{}
void MseMock::SetReturnErr(int err)
{
    retErr = err;
}

void MseMock::ReSetRfcomm()
{
    frComno = RFCOMNO_100;
}

std::map<std::string, int> MseMock::mapConfigMap_;
bool MseMock::GetValue(const std::string &addr, const std::string &section, const std::string &property, int &value)
{
    LOG_INFO("%s enter", __PRETTY_FUNCTION__);
    auto target = mapConfigMap_.find(MakeKey(addr, section, property));
    value = 0;
    if (target != mapConfigMap_.end()) {
        value = target->second;
    }
    return true;
}

bool MseMock::SetValue(const std::string &addr, const std::string &section, const std::string &property, int &value)
{
    LOG_INFO("%s enter", __PRETTY_FUNCTION__);
    mapConfigMap_[MakeKey(addr, section, property)] = value;
    return true;
}

bool MseMock::GetValueInt(const std::string &section, const std::string &property, int &value)
{
    if (property == bluetooth::PROPERTY_L2CAP_MTU) {
        value = 1024;
        return true;
    }
    if (property == bluetooth::PROPERTY_RFCOMM_MTU) {
        value = 512;
        return true;
    }
    return false;
}

bool MseMock::GetValueBool(const std::string &section, const std::string &property, bool &value)
{
    if (property == bluetooth::PROPERTY_SRM_ENABLE) {
        value = true;
        return true;
    }
    return false;
}

bool MseMock::RemoveProperty(const std::string &addr, const std::string &section, const std::string &property)
{
    LOG_INFO("%s enter", __PRETTY_FUNCTION__);
    mapConfigMap_.erase(MakeKey(addr, section, property));
    return true;
}

std::string MseMock::MakeKey(const std::string &addr, const std::string &section, const std::string &property)
{
    return addr + "_" + section + "_" + property;
}

int GAPIF_RegisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo, uint16_t securityMode)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

int GAPIF_DeregisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

uint8_t RFCOMM_AssignServerNum()
{
    LOG_INFO("%s enter", __FUNCTION__);
    LOG_INFO("RFCOMM: %d ", (int)frComno);
    return frComno++;
}

int RFCOMM_FreeServerNum(uint8_t scn)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

int SDP_ServiceSearchAttribute(const BtAddr *addr, const SdpUuid *uuidArray, SdpAttributeIdList attributeIdList,
    void *context,
    void (*ServiceSearchAttributeCb)(
        const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context))
{
    LOG_INFO("%s enter", __FUNCTION__);

    auto maddr = std::make_unique<BtAddr>();
    uint8_t a1[6] = {127, 0, 0, 1, 0, 0};
    memcpy_s(maddr->addr, sizeof(maddr->addr), addr, sizeof(a1));

    // classid
    auto servicePointer = std::make_unique<SdpService>();
    BtUuid *classId = new BtUuid;
    classId->uuid16 = 0x1133;
    servicePointer->classId = classId;

    // rfcommNo
    servicePointer->descriptorNumber = 2;
    auto descriptor = std::make_unique<SdpProtocolDescriptor[]>(2);
    descriptor[0].protocolUuid.type = 0x01;
    descriptor[0].protocolUuid.uuid16 = 0x0003;
    descriptor[0].parameterNumber = 1;
    descriptor[0].parameter[0].type = SDP_TYPE_UINT_8;
    descriptor[0].parameter[0].value = 200;
    descriptor[1].protocolUuid.type = 0x01;
    descriptor[1].protocolUuid.uuid16 = 0x0008;
    descriptor[1].parameterNumber = 0;
    servicePointer->descriptor = descriptor.get();

    // version no
    auto profileDescriptor = std::make_unique<SdpProfileDescriptor>();
    servicePointer->profileDescriptorNumber = 1;
    profileDescriptor->profileUuid.uuid16 = 0x1134;
    profileDescriptor->versionNumber = 0x0104;
    servicePointer->profileDescriptor = profileDescriptor.get();

    // attribute
    servicePointer->attributeNumber = 2;
    auto attribute = std::make_unique<SdpAttribute[]>(2);
    servicePointer->attribute = attribute.get();
    //psm
    attribute[0].attributeId = 0x0200;
    uint16_t* psmNumber = new uint16_t();
    *psmNumber = 0x1021;
    attribute[0].attributeValue = psmNumber;

    // Supported feauth
    attribute[1].attributeId = 0x0317;
    auto feauth = std::make_unique<uint32_t>();
    *feauth = 0x00FEFFFF;
    attribute[1].attributeValue = feauth.get();

    ServiceSearchAttributeCb(maddr.get(), servicePointer.get(), 1, context);
    delete classId;
    delete psmNumber;
    return 0;
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
    return 0 | retErr;
}

/**
 * @brief Register sdp service
 */
int SDP_RegisterServiceRecord(uint32_t handle)
{
    LOG_INFO("%s enter", __FUNCTION__, handle);
    return 0 | retErr;
}

/**
 * @brief Deregister sdp service
 */
int SDP_DeregisterServiceRecord(uint32_t handle)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

/**
 * @brief Add ServiceClassIDList Attribute to record
 * @see Refer to charter 5.1.2 of Core 5.0
 */
int SDP_AddServiceClassIdList(uint32_t handle, const BtUuid *classid, uint16_t classidNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);

    return 0 | retErr;
}

/**
 * @brief Add ServiceID Attribute to record
 * @see Refer to charter 5.1.4 of Core 5.0
 */
int SDP_AddServiceId(uint32_t handle, BtUuid *serviceid)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

/**
 * @brief Add ProtocolDescriptorList Attribute to record
 * @see Refer to charter 5.1.5 of Core 5.0
 */
int SDP_AddProtocolDescriptorList(uint32_t handle, const SdpProtocolDescriptor *descriptor, uint16_t descriptorNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

/**
 * @brief Add AdditionalProtocolDescriptorList Attribute to record
 * @see Refer to charter 5.1.6 of Core 5.0
 */
int SDP_AddAdditionalProtocolDescriptorList(
    uint32_t handle, const SdpAdditionalProtocolDescriptor *descriptorList, uint16_t descriptorListNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

/**
 * @brief Add BluetoothProfileDescriptorList Attribute to record
 * @see Refer to charter 5.1.11 of Core 5.0
 */
int SDP_AddBluetoothProfileDescriptorList(
    uint32_t handle, const SdpProfileDescriptor *profileDescriptor, uint16_t profileDescriptorNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

/**
 * @brief Add ServiceName Attribute to record
 * @see Refer to charter 5.1.15 of Core 5.0
 */
int SDP_AddServiceName(uint32_t handle, uint16_t baseAttributeId, const char *name, uint16_t nameLen)
{
    LOG_INFO("%s enter", __FUNCTION__);
    LOG_INFO("Service Name : %s", name);
    return 0 | retErr;
}

/**
 * @brief Add ServiceDescription Attribute to record
 * @see Refer to charter 5.1.16 of Core 5.0
 */
int SDP_AddServiceDescription(uint32_t handle, uint16_t baseAttributeId, char *description, uint16_t descriptionLen)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

/**
 * @brief Add Attribute undefined in Core 5.0 specification
 * @see Refer to charter 2.2 of Core 5.0
 */
int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0 | retErr;
}

/**
 * @brief Add BrowseGroupList Attribute to record
 * @see Refer to charter 5.1.7 of Core 5.0
 */
int SDP_AddBrowseGroupList(uint32_t handle, const BtUuid *browseUuid, uint16_t browseUuidNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
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

void BTM_AddLocalL2capPsmForLogging(uint8_t module, uint16_t psm)
{
}

void BTM_AddRemoteL2capPsmForLogging(uint8_t module, uint16_t psm, const BtAddr *remoteAddr)
{
}

void BTM_RemoveLocalL2capPsmForLogging(uint8_t module, uint16_t psm)
{
}

void BTM_RemoveRemoteL2capPsmForLogging(uint8_t module, uint16_t psm, const BtAddr *remoteAddr)
{
}

void BTM_AddLocalRfcommScnForLogging(uint8_t module, uint8_t scn)
{
}

void BTM_AddRemoteRfcommScnForLogging(uint8_t module, uint8_t scn, const BtAddr *remoteAddr)
{
}

void BTM_RemoveLocalRfcommScnChannelForLogging(uint8_t module, uint8_t scn)
{
}

void BTM_RemoveRemoteRfcommScnChannelForLogging(uint8_t module, uint8_t scn, const BtAddr *remoteAddr)
{
}
