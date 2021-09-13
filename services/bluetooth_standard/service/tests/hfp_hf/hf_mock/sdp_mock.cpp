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

#include "hf_mock.h"

using namespace bluetooth;
HfSdpMock *gSdpMocker = nullptr;

void registerSdpMocker(HfSdpMock *mocker)
{
    gSdpMocker = mocker;
}

HfSdpMock& GetSdpMocker()
{
    return *gSdpMocker;
}

uint32_t SDP_CreateServiceRecord()
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_CreateServiceRecord();
}

int SDP_RegisterServiceRecord(uint32_t handle)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_RegisterServiceRecord();
}

int SDP_DeregisterServiceRecord(uint32_t handle)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_DeregisterServiceRecord();
}

int SDP_DestroyServiceRecord(uint32_t handle)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_DestroyServiceRecord();
}

int SDP_AddServiceClassIdList(uint32_t handle, const BtUuid *classid, uint16_t classidNumber)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_AddServiceClassIdList();
}

int SDP_AddProtocolDescriptorList(uint32_t handle, const SdpProtocolDescriptor *descriptor, uint16_t descriptorNumber)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_AddProtocolDescriptorList();
}

int SDP_AddBluetoothProfileDescriptorList(
    uint32_t handle, const SdpProfileDescriptor *profileDescriptor, uint16_t profileDescriptorNumber)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_AddBluetoothProfileDescriptorList();
}

int SDP_AddBrowseGroupList(uint32_t handle, const BtUuid *browseUuid, uint16_t browseUuidNumber)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_AddBrowseGroupList();
}

int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_AddAttribute();
}

int SDP_ServiceSearchAttribute(const BtAddr *addr, const SdpUuid *uuidArray, SdpAttributeIdList attributeIdList,
    void *context,
    void (*ServiceSearchAttributeCb)(
        const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context))
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    (void)memcpy_s(&gSdpMocker->addr, sizeof(BtAddr), addr, sizeof(BtAddr));
    gSdpMocker->sdpCb = ServiceSearchAttributeCb;
    return gSdpMocker->SDP_ServiceSearchAttribute();
}

int SDP_AddServiceName(uint32_t handle, uint16_t baseAttributeId, const char *name, uint16_t nameLen)
{
    LOG_INFO("%s excute", __PRETTY_FUNCTION__);
    return gSdpMocker->SDP_AddServiceName();
}