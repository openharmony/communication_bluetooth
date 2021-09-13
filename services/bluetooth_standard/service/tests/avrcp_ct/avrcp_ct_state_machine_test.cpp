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

#include "avrcp_ct_mock.h"

using namespace testing;
using namespace bluetooth;

/// The name of state "CONNECTING".
#define AVRC_CT_SM_STATE_CONNECTING ("CONNECTING")
/// The name of state "CONNECTED".
#define AVRC_CT_SM_STATE_CONNECTED ("CONNECTED")
/// The name of state "DISCONNECTING".
#define AVRC_CT_SM_STATE_DISCONNECTING ("DISCONNECTING")
/// The name of state "PENDING".
#define AVRC_CT_SM_STATE_PENDING ("PENDING")
/// The name of state "BUSY".
#define AVRC_CT_SM_STATE_BUSY ("BUSY")
/// The name of state "CONTINUATION".
#define AVRC_CT_SM_STATE_CONTINUATION ("CONTINUATION")
/// The name of state "DISABLE".
#define AVRC_CT_SM_STATE_DISABLE ("DISABLE")

class AvrcpCtStateMachine_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("AvrcpCtStateMachine_test SetUpTestCase\n");

    }
    static void TearDownTestCase()
    {
        printf("AvrcpCtStateMachine_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerAvctpMocker(&mockAvctp_);
    }
    virtual void TearDown()
    {}

    NiceMock<MockAvctp> mockAvctp_;
};

TEST_F(AvrcpCtStateMachine_test, GetInstance_001)
{
    auto ins = AvrcCtStateMachineManager::GetInstance();

    EXPECT_TRUE(ins != nullptr);
}

TEST_F(AvrcpCtStateMachine_test, DeleteAvrcCtStateMachineManager_001)
{
    RawAddress bda("00:00:00:00:00:00");
    utility::Message msg(0x02);
    AvrcCtStateMachineManager::GetInstance()->~AvrcCtStateMachineManager();
    bool result = AvrcCtStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, AddControlStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:00");
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;
    AvrcCtConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, false, nullptr, nullptr);
    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq).WillRepeatedly(Return(BT_NO_ERROR));
    bool result = AvrcCtStateMachineManager::GetInstance()->AddControlStateMachine(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, AddBrowseStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:01");
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq).WillRepeatedly(Return(BT_NO_ERROR));
    bool result = AvrcCtStateMachineManager::GetInstance()->AddBrowseStateMachine(bda);

    EXPECT_TRUE(result);
}

TEST_F(AvrcpCtStateMachine_test, AddBrowseStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:60");
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq).WillRepeatedly(Return(BT_NO_ERROR));
    bool result = AvrcCtStateMachineManager::GetInstance()->AddBrowseStateMachine(bda);

    EXPECT_TRUE(result);
}

TEST_F(AvrcpCtStateMachine_test, AddBrowseStateMachine_003)
{
    RawAddress bda("00:00:00:00:00:02");
    bool result = AvrcCtStateMachineManager::GetInstance()->AddBrowseStateMachine(bda);

    EXPECT_TRUE(result);
}

TEST_F(AvrcpCtStateMachine_test, DeletePairOfStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:01");
    AvrcCtStateMachineManager::GetInstance()->DeletePairOfStateMachine(bda);
    utility::Message msg(0x02);
    bool result = AvrcCtStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, DeletePairOfStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:05");
    AvrcCtStateMachineManager::GetInstance()->DeletePairOfStateMachine(bda);
    utility::Message msg(0x02);
    bool result = AvrcCtStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, DeleteBrowseStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:01");
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;

    AvrcCtConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, false, nullptr, nullptr);
    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    ;
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq).WillRepeatedly(Return(BT_NO_ERROR));
    AvrcCtStateMachineManager::GetInstance()->AddControlStateMachine(bda);
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq).WillRepeatedly(Return(BT_NO_ERROR));
    AvrcCtStateMachineManager::GetInstance()->AddBrowseStateMachine(bda);
    AvrcCtStateMachineManager::GetInstance()->DeleteBrowseStateMachine(bda);
    utility::Message msg(0x02);
    bool result = AvrcCtStateMachineManager::GetInstance()->SendMessageToBrowseStateMachine(bda, msg);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, DeleteBrowseStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:05");

    AvrcCtStateMachineManager::GetInstance()->DeleteBrowseStateMachine(bda);

    utility::Message msg(0x02);
    bool result = AvrcCtStateMachineManager::GetInstance()->SendMessageToBrowseStateMachine(bda, msg);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, SendMessageToControlStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:06");
    utility::Message msg(0x02);
    bool result = AvrcCtStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, SendMessageToBrowseStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:07");
    utility::Message msg(0x02);
    bool result = AvrcCtStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, SendMessageToAllControlStateMachine_001)
{
    RawAddress bda1("00:00:00:00:00:10");
    RawAddress bda2("00:00:00:00:00:11");
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;
    AvrcCtConnectManager::GetInstance()->Add(
        bda1, connectId, role, controlMtu, browseMtu, companyId, uidCounter, false, nullptr, nullptr);
    AvrcCtConnectManager::GetInstance()->Add(
        bda2, connectId, role, controlMtu, browseMtu, companyId, uidCounter, false, nullptr, nullptr);
    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq).WillRepeatedly(Return(BT_NO_ERROR));
    AvrcCtStateMachineManager::GetInstance()->AddControlStateMachine(bda1);
    AvrcCtStateMachineManager::GetInstance()->AddBrowseStateMachine(bda1);
    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    AvrcCtStateMachineManager::GetInstance()->AddControlStateMachine(bda2);
    AvrcCtStateMachineManager::GetInstance()->AddBrowseStateMachine(bda2);

    utility::Message msg(0x02);
    AvrcCtStateMachineManager::GetInstance()->SendMessageToAllControlStateMachine(msg);
}

TEST_F(AvrcpCtStateMachine_test, SendMessageToAllBrowseStateMachine_001)
{
    utility::Message msg(0x02);
    AvrcCtStateMachineManager::GetInstance()->SendMessageToAllBrowseStateMachine(msg);
}

TEST_F(AvrcpCtStateMachine_test, IsControlConnectingState_001)
{
    RawAddress bda("00:00:00:00:00:11");
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;
    AvrcCtConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, false, nullptr, nullptr);
    AvrcCtStateMachineManager *smManager = AvrcCtStateMachineManager::GetInstance();
    smManager->AddControlStateMachine(bda);

    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));

    bool result = AvrcCtStateMachineManager::GetInstance()->IsControlConnectingState(bda);

    EXPECT_TRUE(result);
}

TEST_F(AvrcpCtStateMachine_test, IsControlConnectedState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcCtStateMachineManager::GetInstance()->IsControlConnectedState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, IsControlPendingState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcCtStateMachineManager::GetInstance()->IsControlPendingState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, IsControlContinuationState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcCtStateMachineManager::GetInstance()->IsControlContinuationState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, IsControlDisconnectingState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcCtStateMachineManager::GetInstance()->IsControlDisconnectingState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, IsControlDisableState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcCtStateMachineManager::GetInstance()->IsControlDisableState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpCtStateMachine_test, InitState_001)
{
    RawAddress bda("00:00:00:00:00:10");

    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq).WillRepeatedly(Return(0));
    AvrcCtConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, false, nullptr, nullptr);
    AvrcCtStateMachineManager::GetInstance()->AddControlStateMachine(bda);
    AvrcCtStateMachineManager::GetInstance()->AddBrowseStateMachine(bda);
    AvrcCtStateMachineManager *smManager = AvrcCtStateMachineManager::GetInstance();
    utility::Message msgConned(AVRC_CT_SM_EVENT_TO_CONNECTED_STATE);
    smManager->SendMessageToControlStateMachine(bda, msgConned);
    EXPECT_CALL(GetAvctpMocker(), AVCT_Deregister).Times(1);
    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).WillRepeatedly(Return(0));
    utility::Message msgDis(AVRC_CT_SM_EVENT_TO_DISCONNECTING_STATE);
    smManager->SendMessageToControlStateMachine(bda, msgDis);
    utility::Message msgshut(AVRC_CT_SM_EVENT_TO_DISABLE_STATE);
    smManager->SendMessageToControlStateMachine(bda, msgshut);
    utility::Message msgde(200);
    smManager->SendMessageToControlStateMachine(bda, msgde);

    EXPECT_CALL(GetAvctpMocker(), AVCT_BrDisconnectReq);
    utility::Message brmsgConned(AVRC_CT_SM_EVENT_TO_CONNECTED_STATE);
    smManager->SendMessageToBrowseStateMachine(bda, brmsgConned);
    utility::Message brmsgDis(AVRC_CT_SM_EVENT_TO_DISCONNECTING_STATE);

    smManager->SendMessageToBrowseStateMachine(bda, brmsgDis);
    utility::Message brmsgshut(AVRC_CT_SM_EVENT_TO_DISABLE_STATE);
    smManager->SendMessageToBrowseStateMachine(bda, brmsgshut);
    smManager->IsBrowsePendingState(bda);
}
