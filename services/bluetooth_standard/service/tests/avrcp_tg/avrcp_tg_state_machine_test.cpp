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

#include "avrcp_tg_mock.h"

using namespace testing;
using namespace bluetooth;

class AvrcpTgStateMachine_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("AvrcpTgStateMachine_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("AvrcpTgStateMachine_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerAvctpMocker(&mockAvctp_);
    }
    virtual void TearDown()
    {}
    NiceMock<MockAvctp> mockAvctp_;
};

void AddConnInfo(RawAddress bda)
{
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;
    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq()).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq()).Times(AtLeast(0));

    EXPECT_CALL(GetAvctpMocker(), AVCT_Deregister).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrDisconnectReq).Times(AtLeast(0));
    AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
    AvrcTgConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, nullptr, nullptr);
}

void AddConnInfo(RawAddress bda, RawAddress bda2)
{
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;
    EXPECT_CALL(GetAvctpMocker(), AVCT_Register).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_ConnectReq()).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrConnectReq()).Times(AtLeast(0));

    EXPECT_CALL(GetAvctpMocker(), AVCT_Deregister).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).Times(AtLeast(0));
    EXPECT_CALL(GetAvctpMocker(), AVCT_BrDisconnectReq).Times(AtLeast(0));
    AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
    AvrcTgConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, nullptr, nullptr);
    AvrcTgConnectManager::GetInstance()->Add(
        bda2, connectId, role, controlMtu, browseMtu, companyId, uidCounter, nullptr, nullptr);
}

TEST_F(AvrcpTgStateMachine_test, GetInstance_001)
{
    auto ins = AvrcTgStateMachineManager::GetInstance();

    EXPECT_TRUE(ins != nullptr);
}

TEST_F(AvrcpTgStateMachine_test, DeleteAvrcTgStateMachineManager_001)
{
    RawAddress bda("00:00:00:00:00:00");
    utility::Message msg(0x02);
    AvrcTgStateMachineManager::GetInstance()->~AvrcTgStateMachineManager();
    EXPECT_FALSE(AvrcTgStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg));
}

TEST_F(AvrcpTgStateMachine_test, AddControlStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    EXPECT_EQ(RET_NO_ERROR, AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda));

    AvrcTgStateMachineManager::GetInstance()->~AvrcTgStateMachineManager();
}

TEST_F(AvrcpTgStateMachine_test, AddBrowseStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    EXPECT_EQ(RET_BAD_PARAM, AvrcTgStateMachineManager::GetInstance()->AddBrowseStateMachine(bda));
}

TEST_F(AvrcpTgStateMachine_test, AddBrowseStateMachine_003)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda);
    EXPECT_EQ(RET_NO_ERROR, AvrcTgStateMachineManager::GetInstance()->AddBrowseStateMachine(bda));
    AvrcTgStateMachineManager::GetInstance()->~AvrcTgStateMachineManager();
}

TEST_F(AvrcpTgStateMachine_test, DeletePairOfStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda);
    AvrcTgStateMachineManager::GetInstance()->DeletePairOfStateMachine(bda);
    utility::Message msg(0x02);
    EXPECT_FALSE(AvrcTgStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg));
}

TEST_F(AvrcpTgStateMachine_test, DeletePairOfStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    AvrcTgStateMachineManager::GetInstance()->DeletePairOfStateMachine(bda);
    utility::Message msg(0x02);
    EXPECT_FALSE(AvrcTgStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg));
}

TEST_F(AvrcpTgStateMachine_test, DeleteBrowseStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda);
    AvrcTgStateMachineManager::GetInstance()->AddBrowseStateMachine(bda);
    AvrcTgStateMachineManager::GetInstance()->DeleteBrowseStateMachine(bda);

    utility::Message msg(0x02);
    EXPECT_FALSE(AvrcTgStateMachineManager::GetInstance()->SendMessageToBrowseStateMachine(bda, msg));
    AvrcTgStateMachineManager::GetInstance()->~AvrcTgStateMachineManager();
}

TEST_F(AvrcpTgStateMachine_test, DeleteBrowseStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    AvrcTgStateMachineManager::GetInstance()->DeleteBrowseStateMachine(bda);
    utility::Message msg(0x02);
    EXPECT_FALSE(AvrcTgStateMachineManager::GetInstance()->SendMessageToBrowseStateMachine(bda, msg));
}

TEST_F(AvrcpTgStateMachine_test, SendMessageToControlStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    utility::Message msg(0x02);
    EXPECT_FALSE(AvrcTgStateMachineManager::GetInstance()->SendMessageToControlStateMachine(bda, msg));
}

TEST_F(AvrcpTgStateMachine_test, SendMessageToBrowseStateMachine_002)
{
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    utility::Message msg(0x02);
    EXPECT_FALSE(AvrcTgStateMachineManager::GetInstance()->SendMessageToBrowseStateMachine(bda, msg));
}

TEST_F(AvrcpTgStateMachine_test, SendMessageToAllControlStateMachine_001)
{
    RawAddress bda1("00:00:00:00:00:10");
    RawAddress bda2("00:00:00:00:00:11");
    AddConnInfo(bda1,bda2);
    AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda1);
    AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda2);

    utility::Message msg(0x02);
    AvrcTgStateMachineManager::GetInstance()->SendMessageToAllControlStateMachine(msg);
    AvrcTgStateMachineManager::GetInstance()->~AvrcTgStateMachineManager();
}

TEST_F(AvrcpTgStateMachine_test, SendMessageToAllBrowseStateMachine_001)
{
    RawAddress bda1("00:00:00:00:00:10");
    RawAddress bda2("00:00:00:00:00:11");
    AddConnInfo(bda1, bda2);
    AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda1);
    AvrcTgStateMachineManager::GetInstance()->AddBrowseStateMachine(bda1);
    AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda2);
    AvrcTgStateMachineManager::GetInstance()->AddBrowseStateMachine(bda2);

    utility::Message msg(0x02);
    AvrcTgStateMachineManager::GetInstance()->SendMessageToAllBrowseStateMachine(msg);
    AvrcTgStateMachineManager::GetInstance()->~AvrcTgStateMachineManager();
}

TEST_F(AvrcpTgStateMachine_test, IsControlConnectingState_001)
{
    AvrcTgStateMachineManager::GetInstance()->ShutDown();
    RawAddress bda("00:00:00:00:00:11");
    AddConnInfo(bda);
    AvrcTgStateMachineManager::GetInstance()->AddControlStateMachine(bda);

    EXPECT_CALL(GetAvctpMocker(), AVCT_SendMsgReq).WillRepeatedly(Return(0));

    bool result = AvrcTgStateMachineManager::GetInstance()->IsControlConnectingState(bda);

    EXPECT_TRUE(result);
    AvrcTgStateMachineManager::GetInstance()->~AvrcTgStateMachineManager();
}

TEST_F(AvrcpTgStateMachine_test, IsControlConnectedState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcTgStateMachineManager::GetInstance()->IsControlConnectedState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpTgStateMachine_test, IsControlContinuationState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcTgStateMachineManager::GetInstance()->IsControlContinuationState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpTgStateMachine_test, IsControlDisconnectingState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcTgStateMachineManager::GetInstance()->IsControlDisconnectingState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpTgStateMachine_test, IsControlShutDownState_001)
{
    RawAddress bda("00:00:00:00:00:10");
    bool result = AvrcTgStateMachineManager::GetInstance()->IsControlDisableState(bda);

    EXPECT_FALSE(result);
}

TEST_F(AvrcpTgStateMachine_test, ControlStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:10");
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;
    AvrcTgConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, nullptr, nullptr);
    AvrcTgConnectManager::GetInstance()->SetActiveDevice(bda.GetAddress());
    AvrcTgStateMachineManager *smManager = AvrcTgStateMachineManager::GetInstance();
    smManager->AddControlStateMachine(bda);

    utility::Message msgConned(AVRC_TG_SM_EVENT_TO_CONNECTED_STATE);
    smManager->SendMessageToControlStateMachine(bda, msgConned);

    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).WillOnce(Return(0));
    utility::Message msgDis(AVRC_TG_SM_EVENT_TO_DISCONNECTING_STATE);
    smManager->SendMessageToControlStateMachine(bda, msgDis);

    smManager->DeletePairOfStateMachine(bda);
    smManager->AddControlStateMachine(bda);
    EXPECT_CALL(GetAvctpMocker(), AVCT_Deregister).Times(1);
    EXPECT_CALL(GetAvctpMocker(), AVCT_DisconnectReq).WillOnce(Return(0));
    utility::Message msgshut(AVRC_TG_SM_EVENT_TO_DISABLE_STATE);
    smManager->SendMessageToControlStateMachine(bda, msgshut);

    utility::Message msgde(200);
    smManager->SendMessageToControlStateMachine(bda, msgde);
    smManager->~AvrcTgStateMachineManager();
    AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
}

TEST_F(AvrcpTgStateMachine_test, BrowseStateMachine_001)
{
    RawAddress bda("00:00:00:00:00:10");
    uint8_t connectId = 0x05;
    uint8_t role = 0x01;
    uint16_t controlMtu = 0x0050;
    uint16_t browseMtu = 0x0030;
    uint32_t companyId = 0x001958;
    uint32_t uidCounter = 0x00000001;
    AvrcTgConnectManager::GetInstance()->Add(
        bda, connectId, role, controlMtu, browseMtu, companyId, uidCounter, nullptr, nullptr);
    AvrcTgConnectManager::GetInstance()->SetActiveDevice(bda.GetAddress());
    AvrcTgStateMachineManager *smManager = AvrcTgStateMachineManager::GetInstance();
    smManager->AddControlStateMachine(bda);
    smManager->AddBrowseStateMachine(bda);

    utility::Message brmsgConned(AVRC_TG_SM_EVENT_TO_CONNECTED_STATE);
    smManager->SendMessageToBrowseStateMachine(bda, brmsgConned);

    EXPECT_CALL(GetAvctpMocker(), AVCT_BrDisconnectReq).WillOnce(Return(0));
    utility::Message brmsgDis(AVRC_TG_SM_EVENT_TO_DISCONNECTING_STATE);
    smManager->SendMessageToBrowseStateMachine(bda, brmsgDis);

    utility::Message brmsgshut(AVRC_TG_SM_EVENT_TO_DISABLE_STATE);
    smManager->SendMessageToBrowseStateMachine(bda, brmsgshut);
    smManager->~AvrcTgStateMachineManager();
    AvrcTgConnectManager::GetInstance()->~AvrcTgConnectManager();
}
