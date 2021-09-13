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

#ifndef STACK_MOCK_H
#define STACK_MOCK_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

extern "C" {
#include "rfcomm.h"
#include "sdp.h"
#include "btm.h"
#include "gap_if.h"
#include "l2cap_if.h"
}

typedef struct {
    BtAddr btaddr;
    SdpService serviceArray;
    uint16_t serviceNum;
} SdpSearchCbParam;

typedef struct {
    void *context;
    RFCOMM_EventCallback callBack;
} RfcommEventCb;

typedef struct {
    void *context;
    L2capService callBack;
} L2capEventCb;

typedef struct {
    void *context;
    void (*cb)(const BtAddr *addr, uint16_t lcid, int result, void *context);
} L2capConnectReqCb;

class Mock_Stack {
public:
    uint16_t handle;
    // RFCOMM
    MOCK_METHOD0(RFCOMM_AssignServerNum, uint8_t());
    MOCK_METHOD0(RFCOMM_RegisterServer, int());
    MOCK_METHOD0(RFCOMM_ConnectChannel, int());
    MOCK_METHOD0(RFCOMM_GetPeerAddress, BtAddr());
    MOCK_METHOD2(RFCOMM_Read, int(uint16_t handle, Packet **pkt));
    MOCK_METHOD0(RFCOMM_Write, int());
    MOCK_METHOD0(RFCOMM_DeregisterServer, int());
    // SDP
    MOCK_METHOD0(SDP_AddServiceClassIdList, int());
    MOCK_METHOD0(SDP_AddProtocolDescriptorList, int());
    MOCK_METHOD0(SDP_AddBluetoothProfileDescriptorList, int());
    MOCK_METHOD0(SDP_AddAttribute, int());
    MOCK_METHOD0(SDP_AddBrowseGroupList, int());
    MOCK_METHOD0(SDP_ServiceSearchAttribute, int());
    MOCK_METHOD0(SDP_RegisterServiceRecord, int());
    MOCK_METHOD0(SDP_DeregisterServiceRecord, int());
    MOCK_METHOD0(SDP_DestroyServiceRecord, int());
    MOCK_METHOD0(SDP_AddServiceName, int());
    // BTM

    // GAP
    // L2CAP
    MOCK_METHOD0(L2CIF_RegisterService, int());
    MOCK_METHOD0(L2CIF_DeregisterService, void());
    MOCK_METHOD0(L2CIF_ConnectReq, int());
    MOCK_METHOD0(L2CIF_ConnectRsp, void());
    MOCK_METHOD0(L2CIF_ConfigReq, int());
    MOCK_METHOD0(L2CIF_ConfigRsp, int());
    MOCK_METHOD0(L2CIF_DisconnectionReq, void());
    MOCK_METHOD0(L2CIF_DisconnectionRsp, void());
    MOCK_METHOD0(L2CIF_SendData, int());
};
#endif  // AG_MOCK_H
