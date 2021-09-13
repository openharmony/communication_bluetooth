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

#ifndef AVRCP_TG_MOCK_H
#define AVRCP_TG_MOCK_H

#include "avrcp_tg_test.h"

using namespace testing;
using namespace bluetooth;

class MockGap {
public:
    MOCK_METHOD0(GAPIF_RegisterServiceSecurity, int());
    MOCK_METHOD0(GAPIF_DeregisterServiceSecurity, int());
};

class MockSdp {
public:
    MOCK_METHOD0(SDP_CreateServiceRecord, uint32_t());
    MOCK_METHOD0(SDP_AddServiceClassIdList, int());
    MOCK_METHOD0(SDP_AddProtocolDescriptorList, int());
    MOCK_METHOD0(SDP_AddAdditionalProtocolDescriptorList, int());
    MOCK_METHOD0(SDP_AddBluetoothProfileDescriptorList, int());
    MOCK_METHOD0(SDP_AddAttribute, int());
    MOCK_METHOD0(SDP_AddServiceName, int());
    MOCK_METHOD0(SDP_AddBrowseGroupList, int());
    MOCK_METHOD0(SDP_RegisterServiceRecord, int());
    MOCK_METHOD0(SDP_DestroyServiceRecord, int());
    MOCK_METHOD0(SDP_DeregisterServiceRecord, int());
    MOCK_METHOD0(SDP_ServiceSearch, int());
};

class MockAvctp {
public:
    MOCK_METHOD0(AVCT_Register, void());
    MOCK_METHOD0(AVCT_ConnectReq, uint16_t());
    MOCK_METHOD0(AVCT_DisconnectReq, uint16_t());
    MOCK_METHOD0(AVCT_Deregister, void());
    MOCK_METHOD0(AVCT_BrConnectReq, uint16_t());
    MOCK_METHOD0(AVCT_BrDisconnectReq, uint16_t());
    MOCK_METHOD0(AVCT_SendMsgReq, uint16_t());
    MOCK_METHOD0(AVCT_BrSendMsgReq, uint16_t());
    MOCK_METHOD0(AVCT_BrGetPeerMtu, uint16_t());
    MOCK_METHOD0(AVCT_GetPeerMtu, uint16_t());
};

void registerGapMocker(MockGap *mocker);
MockGap &GetGapMocker();

void registerSdpMocker(MockSdp *mocker);
MockSdp &GetSdpMocker();

void registerAvctpMocker(MockAvctp *mocker);
MockAvctp &GetAvctpMocker();

AvctConnectParam GetAvctConnectParam();
#endif