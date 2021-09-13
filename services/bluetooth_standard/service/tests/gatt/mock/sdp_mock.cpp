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

#include "include/sdp.h"

uint32_t SDP_CreateServiceRecord()
{
    return 1;
}

int SDP_RegisterServiceRecord(uint32_t handle)
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

int SDP_AddServiceClassIdList(uint32_t handle, const BtUuid *classid, uint16_t classidNumber)
{
    return 0;
}

int SDP_AddProtocolDescriptorList(uint32_t handle, const SdpProtocolDescriptor *descriptor, uint16_t descriptorNumber)
{
    return 0;
}

int SDP_AddBluetoothProfileDescriptorList(
    uint32_t handle, const SdpProfileDescriptor *profileDescriptor, uint16_t profileDescriptorNumber)
{
    return 0;
}

int SDP_AddBrowseGroupList(uint32_t handle, const BtUuid *browseUuid, uint16_t browseUuidNumber)
{
    return 0;
}

int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
{
    return 0;
}

int SDP_ServiceSearchAttribute(const BtAddr *addr, const SdpUuid *uuidArray, SdpAttributeIdList attributeIdList,
    void *context,
    void (*ServiceSearchAttributeCb)(
        const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context))
{
    return 0;
}