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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gap_if.h"
#include "rfcomm.h"
#include "sdp.h"
#include "string.h"
#include <thread>

extern "C" {
#include "btm.h"
#include "btstack.h"
#include "log.h"
}

#include "pbap_mock.h"
#include "adapter_config.h"
#include "securec.h"

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

int SDP_ServiceSearchAttribute(const BtAddr *addr, const SdpUuid *uuidAry, SdpAttributeIdList attributeIdList,
    void *context,
    void (*ServiceSearchAttributeCb)(
        const BtAddr *addr, const SdpService *serviceAry, uint16_t serviceNum, void *context))
{
    LOG_INFO("%s enter", __FUNCTION__);

    auto maddr = std::make_unique<BtAddr>();
    uint8_t a1[6] = {127, 0, 0, 1, 0, 0};
    memcpy_s(maddr->addr, sizeof(maddr->addr), addr, sizeof(a1));

    // classid
    auto servicePointer = std::make_unique<SdpService>();
    auto classId = std::make_unique<BtUuid>();
    classId->uuid16 = 0x112F;
    servicePointer->classId = classId.get();

    // rfcommNo
    auto descriptor = std::make_unique<SdpProtocolDescriptor>();
    servicePointer->descriptorNumber = 1;
    descriptor->protocolUuid.uuid16 = 0x0003;
    descriptor->parameter[0].value = 15;
    servicePointer->descriptor = descriptor.get();

    // version no
    auto profileDescriptor = std::make_unique<SdpProfileDescriptor>();
    servicePointer->profileDescriptorNumber = 1;
    profileDescriptor->profileUuid.uuid16 = 0x1130;
    profileDescriptor->versionNumber = 0x0102;
    servicePointer->profileDescriptor = profileDescriptor.get();

    // attribute
    servicePointer->attributeNumber = 3;
    auto attribute = std::make_unique<SdpAttribute[]>(3);
    servicePointer->attribute = attribute.get();

    // Supported Repositories
    attribute[1].attributeId = 0x0314;
    auto res = std::make_unique<uint8_t>();
    *res = 0x01;
    attribute[1].attributeValue = res.get();

    // Supported feature
    attribute[2].attributeId = 0x0317;
    auto feature = std::make_unique<uint32_t>();
    *feature = 0x00000001;
    attribute[2].attributeValue = feature.get();

    ServiceSearchAttributeCb(maddr.get(), servicePointer.get(), 1, context);
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

int SDP_AddBrowseGroupList(uint32_t handle, const BtUuid *browseUuid, uint16_t browseUuidNumber)
{
    LOG_INFO("%s enter", __FUNCTION__);
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

int GAPIF_RegisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo, uint16_t securityMode)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

int GAPIF_DeregisterServiceSecurity(const BtAddr *addr, const GapServiceSecurityInfo *serviceInfo)
{
    LOG_INFO("%s enter", __FUNCTION__);
    return 0;
}

int GAPIF_RequestSecurity(const BtAddr *addr, const GapRequestSecurityParam *param)
{
    LOG_INFO("%s enter", __FUNCTION__);
    if (param && param->callback) {
        param->callback(0, param->info, param->context);
    }
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

int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
{
    return 0;
}

std::map<std::string, int> PbapProfileConfigMock::phoneBookConfigMap_;

bool PbapProfileConfigMock::GetValue(
    const std::string &addr, const std::string &section, const std::string &property, int &value)
{
    LOG_INFO("%s enter", __PRETTY_FUNCTION__);
    if (addr != "00:00:01:00:00:7F") {
        return false;
    }
    auto target = phoneBookConfigMap_.find(MakeKey(addr, section, property));
    if (target != phoneBookConfigMap_.end()) {
        value = target->second;
        return true;
    }
    return false;
}

bool PbapProfileConfigMock::SetValue(
    const std::string &addr, const std::string &section, const std::string &property, int &value)
{
    LOG_INFO("%s enter", __PRETTY_FUNCTION__);
    if (addr != "00:00:01:00:00:7F") {
        return false;
    }
    phoneBookConfigMap_[MakeKey(addr, section, property)] = value;
    return true;
}

std::string PbapProfileConfigMock::MakeKey(
    const std::string &addr, const std::string &section, const std::string &property)
{
    return addr + "_" + section + "_" + property;
}

bool PbapAdapterConfigMock::GetValueInt(const std::string &section, const std::string &property, int &value)
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

bool PbapAdapterConfigMock::GetValueBool(const std::string &section, const std::string &property, bool &value)
{
    if (property == bluetooth::PROPERTY_SRM_ENABLE) {
        value = true;
        return true;
    }
    return false;
}
