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

#ifndef HF_MOCK_H
#define HF_MOCK_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "hfp_hf_defines.h"
#include "btstack.h"
#include "log.h"
#include "btm.h"
#include "gap_if.h"
#include "rfcomm.h"
#include "sdp.h"
#include "securec.h"
using namespace bluetooth;
const BtAddr hfBtAddr = { { 2, 0, 0, 0, 0, 0}, 1};
const std::string hfAddr = "00:00:00:00:00:02";

class HfBtmMock {
public:
    MOCK_METHOD0(BTM_RegisterScoCallbacks, int());
    MOCK_METHOD0(BTM_DeregisterScoCallbacks, int());
    MOCK_METHOD0(BTM_WriteVoiceSetting, int());
    MOCK_METHOD0(BTM_IsSecureConnection, bool());
    MOCK_METHOD0(BTM_CreateEscoConnection, int());
    MOCK_METHOD0(BTM_DisconnectScoConnection, int());
    MOCK_METHOD0(BTM_AcceptEscoConnectionRequest, int());
    MOCK_METHOD0(BTM_CreateScoConnection, int());
    MOCK_METHOD0(BTM_AcceptScoConnectionRequest, int());
    MOCK_METHOD0(BTM_RejectScoConnectionRequest, int());
    MOCK_METHOD0(BTM_GetLocalSupportedCodecs, int());
    MOCK_METHOD0(BTM_IsControllerSupportEsco, bool());
    MOCK_METHOD0(BTM_AddLocalRfcommScnForLogging, void());
    MOCK_METHOD0(BTM_AddRemoteRfcommScnForLogging, void());
    MOCK_METHOD0(BTM_RemoveLocalRfcommScnChannelForLogging, void());
    MOCK_METHOD0(BTM_RemoveRemoteRfcommScnChannelForLogging, void());

    const BtmScoCallbacks *callbacks;
};

class HfGapMock {
public:
    MOCK_METHOD0(GAPIF_RegisterServiceSecurity, int());
    MOCK_METHOD0(GAPIF_DeregisterServiceSecurity, int());
    MOCK_METHOD0(GAP_RequestSecurity, int());
};

class HfRfcommMock {
public:
    MOCK_METHOD0(RFCOMM_RegisterServer, int());
    MOCK_METHOD0(RFCOMM_DeregisterServer, int());
    MOCK_METHOD0(RFCOMM_FreeServerNum, int());
    MOCK_METHOD0(RFCOMM_AcceptConnection, int());
    MOCK_METHOD0(RFCOMM_RejectConnection, int());
    MOCK_METHOD0(RFCOMM_AssignServerNum, uint8_t());
    MOCK_METHOD0(RFCOMM_ConnectChannel, int());
    MOCK_METHOD0(RFCOMM_DisconnectChannel, int());
    MOCK_METHOD2(RFCOMM_Read, int(uint16_t handle, Packet **pkt));
    MOCK_METHOD0(RFCOMM_Write, int());
    MOCK_METHOD0(RFCOMM_GetPeerAddress, BtAddr());

    void (*rfcommCb)(uint16_t handle, uint32_t eventId, const void *eventData, void *context);
};

class HfSdpMock {
public:
    MOCK_METHOD0(SDP_CreateServiceRecord, uint32_t());
    MOCK_METHOD0(SDP_RegisterServiceRecord, int());
    MOCK_METHOD0(SDP_DeregisterServiceRecord, int());
    MOCK_METHOD0(SDP_DestroyServiceRecord, int());
    MOCK_METHOD0(SDP_AddServiceClassIdList, int());
    MOCK_METHOD0(SDP_AddProtocolDescriptorList, int());
    MOCK_METHOD0(SDP_AddBluetoothProfileDescriptorList, int());
    MOCK_METHOD0(SDP_AddBrowseGroupList, int());
    MOCK_METHOD0(SDP_AddAttribute, int());
    MOCK_METHOD0(SDP_ServiceSearchAttribute, int());
    MOCK_METHOD0(SDP_AddServiceName, int());

    BtAddr addr;
    void (*sdpCb)(const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context);
};

void registerBtmMocker(HfBtmMock *mocker);
HfBtmMock& GetBtmMocker();

void registerGapMocker(HfGapMock *mocker);
HfGapMock& GetGapMocker();

void registerRfcommMocker(HfRfcommMock *mocker);
HfRfcommMock& GetRfcommMocker();

void registerSdpMocker(HfSdpMock *mocker);
HfSdpMock& GetSdpMocker();
#endif // HF_MOCK_H