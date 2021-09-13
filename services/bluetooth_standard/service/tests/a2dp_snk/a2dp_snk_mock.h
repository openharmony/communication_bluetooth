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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log.h"

#include "avdtp.h"
#include "btm.h"
#include "btstack.h"
#include "gap_if.h"
#include "log.h"
#include "sdp.h"

using namespace testing;

class MockSnkAvdtp {
public:
    MOCK_METHOD0(AVDT_AbortReq, int());
    MOCK_METHOD0(AVDT_AbortRsp, int());
    MOCK_METHOD0(AVDT_CreateStream, int());
    MOCK_METHOD0(AVDT_RemoveStream, int());
    MOCK_METHOD0(AVDT_DiscoverReq, int());
    MOCK_METHOD0(AVDT_DiscoverRsp, int());
    MOCK_METHOD0(AVDT_GetCapReq, int());
    MOCK_METHOD0(AVDT_GetCapRsp, int());
    MOCK_METHOD0(AVDT_GetAllCapReq, int());
    MOCK_METHOD0(AVDT_GetAllCapRsp, int());
    MOCK_METHOD0(AVDT_DelayReq, int());
    MOCK_METHOD0(AVDT_DelayRsp, int());
    MOCK_METHOD0(AVDT_OpenReq, int());
    MOCK_METHOD0(AVDT_OpenRsp, int());
    MOCK_METHOD0(AVDT_SetConfigReq, int());
    MOCK_METHOD0(AVDT_SetConfigRsp, int());
    MOCK_METHOD0(AVDT_GetConfigReq, int());
    MOCK_METHOD0(AVDT_GetConfigRsp, int());
    MOCK_METHOD0(AVDT_StartReq, int());
    MOCK_METHOD0(AVDT_StartRsp, int());
    MOCK_METHOD0(AVDT_SuspendReq, int());
    MOCK_METHOD0(AVDT_SuspendRsp, int());
    MOCK_METHOD0(AVDT_CloseReq, int());
    MOCK_METHOD0(AVDT_CloseRsp, int());
    MOCK_METHOD0(AVDT_ReconfigReq, int());
    MOCK_METHOD0(AVDT_ReconfigRsp, int());
    MOCK_METHOD0(AVDT_WriteReq, int());
    MOCK_METHOD0(AVDT_ConnectRsp, int());
    MOCK_METHOD0(AVDT_ConnectReq, int());
    MOCK_METHOD0(AVDT_DisconnectReq, int());
    MOCK_METHOD0(AVDT_DisconnectRsp, int());
    MOCK_METHOD0(AVDT_GetL2capChannel, int());
    MOCK_METHOD0(AVDT_RegisterLocalSEP, int());
};

void registerSnkAvdtpMock(MockSnkAvdtp *instance);
MockSnkAvdtp &GetSnkAvdtpMocker();