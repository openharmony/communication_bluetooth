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

#include "classic_mock.h"
#include "sdp.h"
ClassicMock *g_sdpmocker;

SearchAttributeCb g_searchAttributeCb;
SearchAttributeCb GetSdpServiceSearchAttCb()
{
    return g_searchAttributeCb;
}

void registerSdpMock(ClassicMock *instance)
{
    g_sdpmocker = instance;
}
uint32_t SDP_CreateServiceRecord()
{
    return g_sdpmocker->SDP_CreateServiceRecord();
}
int SDP_AddServiceClassIdList(uint32_t handle, const BtUuid *classid, uint16_t classidNumber)
{
    return g_sdpmocker->SDP_AddServiceClassIdList();
}
int SDP_AddAttribute(
    uint32_t handle, uint16_t attributeId, SdpDataType type, void *attributeValue, uint16_t attributeValueLength)
{
    return g_sdpmocker->SDP_AddAttribute();
}
int SDP_RegisterServiceRecord(uint32_t handle)
{
    return g_sdpmocker->SDP_RegisterServiceRecord();
}

int SDP_DeregisterServiceRecord(uint32_t handle)
{
    return g_sdpmocker->SDP_DeregisterServiceRecord();
}
int SDP_DestroyServiceRecord(uint32_t handle)
{
    return g_sdpmocker->SDP_DestroyServiceRecord();
}
int SDP_ServiceSearchAttribute(const BtAddr *addr, const SdpUuid *uuidArray, SdpAttributeIdList attributeIdList,
    void *context,
    void (*ServiceSearchAttributeCb)(
        const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context))
{
    g_searchAttributeCb = ServiceSearchAttributeCb;
    return g_sdpmocker->SDP_ServiceSearchAttribute();
}