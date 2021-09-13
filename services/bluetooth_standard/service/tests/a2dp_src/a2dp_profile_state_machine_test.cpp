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

#include "a2dp_src_test.h"

using namespace testing;
using namespace bluetooth;

class ObserverProfileUt : public A2dpProfileObserver {
public:
    ObserverProfileUt(){};

    ~ObserverProfileUt(){};

    void SetRole(uint8_t role){};

    void OnConnectStateChanged(const BtAddr &addr, int state, void *context){};

    void OnAudioStateChanged(const BtAddr &addr, int state, void *context){};

    void OnCodecStateChanged(const BtAddr &addr, A2dpSrcCodecStatus codecInfo, void *context){};

private:
    uint8_t role_;
};

ObserverProfileUt profileObserver_;

class a2dp_profile_state_machine_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_profile_state_machine_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_profile_state_machine_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerAvdtpMock(&mockAvdtp_);
        // Mock AdapterConfig
        registerNewMockAdapterConfig(&acinstanceMock_);
        EXPECT_CALL(acinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&acMock_));
        EXPECT_CALL(acMock_, GetValue(_, PROPERTY_MAX_CONNECTED_DEVICES, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(6), Return(true)));
        EXPECT_CALL(acMock_, GetValue(_, PROPERTY_CODEC_SBC, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(1), Return(true)));
        EXPECT_CALL(acMock_, GetValue(_, PROPERTY_CODEC_AAC, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<2>(2), Return(true)));
    }
    virtual void TearDown()
    {}

protected:
    NiceMock<MockAvdtp> mockAvdtp_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(a2dp_profile_state_machine_test, A2dpStateConfigure_1)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_OpenReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_OPEN_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateConfigure state(name, mac);

    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateConfigure_2)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_DISCONNECT_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateConfigure state(name, mac);

    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateConfigure_3)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;

    utility::Message message(EVT_OPEN_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);
    A2dpStateConfigure state(name, mac);

    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateConfigure_4)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 1;

    utility::Message message(EVT_DISCONNECT_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);
    A2dpStateConfigure state(name, mac);

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(msgInfo.a2dpMsg.stream.addr, A2DP_ROLE_SINK);
    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
    profile->DeletePeer(msgInfo.a2dpMsg.stream.addr);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateConfigure_5)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_TIME_OUT, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateConfigure state(name, mac);

    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateConfigure_6)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_SETCONFIG_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateConfigure state(name, mac);

    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);
    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpening_1)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_DISCONNECT_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpening state(name, mac);

    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpening_2)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 1;

    utility::Message message(EVT_DISCONNECT_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(msgInfo.a2dpMsg.stream.addr, A2DP_ROLE_SINK);

    A2dpStateOpening state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
    profile->DeletePeer(msgInfo.a2dpMsg.stream.addr);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpening_3)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_StartReq).WillRepeatedly(Return(0));
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 1;
    msgInfo.a2dpMsg.stream.errCode = 0;

    utility::Message message(EVT_OPEN_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(msgInfo.a2dpMsg.stream.addr, A2DP_ROLE_SINK);

    A2dpStateOpening state(name, mac);
    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
    profile->DeletePeer(msgInfo.a2dpMsg.stream.addr);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpening_4)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;
    msgInfo.a2dpMsg.stream.errCode = 1;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 0;

    utility::Message message(EVT_OPEN_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(msgInfo.a2dpMsg.stream.addr, A2DP_ROLE_SOURCE);

    A2dpStateOpening state(name, mac);
    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
    profile->DeletePeer(msgInfo.a2dpMsg.stream.addr);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpening_5)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_TIME_OUT, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpening state(name, mac);

    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpening_6)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_CLOSE_TRANS_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpening state(name, mac);
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_1)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_StartRsp).WillOnce(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_START_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_2)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseRsp).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;

    utility::Message message(EVT_CLOSE_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);

    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_3)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 1;
    msgInfo.a2dpMsg.stream.errCode = 0;

    utility::Message message(EVT_DISCONNECT_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_4)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_CLOSE_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);
    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_5)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(1));

    utility::Message message(EVT_DISCONNECT_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_6)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_StartReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_START_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);
    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_7)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_RECONFIG_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";
    A2dpSrcProfile::GetInstance()->Enable();
    A2dpStateOpen state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_8)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_START_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_9)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_TIME_OUT, 0, &msgInfo);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_10)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigRsp).WillOnce(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_RECONFIG_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";
    A2dpStateOpen state(name, mac);

    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    A2dpSrcProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    bool ret = state.Dispatch(message);
    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_11)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillRepeatedly(Return(0));

    utility::Message message(EVT_RECONFIG_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";
    A2dpStateOpen state(name, mac);

    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 15, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    uint8_t *resultCodecInformation = resultCodecInfo;

    A2dpProfilePeer *peer = A2dpSrcProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);
    peer->GetCodecConfigure()->SetCodecConfig(peerCodeInformation, resultCodecInformation);

    bool ret = state.Dispatch(message);
    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen_12)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_CLOSE_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";
    A2dpStateOpen state(name, mac);

    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    A2dpSrcProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    bool ret = state.Dispatch(message);
    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming_1)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_SuspendRsp).WillOnce(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_SUSPEND_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateStreaming state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming_2)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseRsp).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_CLOSE_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateStreaming state(name, mac);
    A2dpSrcProfile::GetInstance()->Enable();
    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming_3)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;

    utility::Message message(EVT_DISCONNECT_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateStreaming state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming_4)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_CLOSE_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";
    A2dpSrcProfile::GetInstance()->Enable();
    A2dpStateStreaming state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming_5)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_SuspendReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SOURCE;

    utility::Message message(EVT_SUSPEND_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";
    A2dpSrcProfile::GetInstance()->Enable();
    A2dpStateStreaming state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming_6)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x00, 0x00, 0x00}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 1;
    msgInfo.a2dpMsg.stream.errCode = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x00, 0x00, 0x00}, 0x00};

    utility::Message message(EVT_SUSPEND_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateStreaming state(name, mac);
    A2dpSnkProfile::GetInstance()->RegisterObserver(&profileObserver_);
    A2dpProfilePeer *peer =
        A2dpSnkProfile::GetInstance()->FindOrCreatePeer(msgInfo.a2dpMsg.stream.addr, A2DP_ROLE_SINK);
    peer->SetRestart(true);

    bool ret = state.Dispatch(message);
    A2dpSnkProfile::GetInstance()->DeletePeer(addr);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming_7)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(1));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;
    msgInfo.a2dpMsg.stream.addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 1;
    msgInfo.a2dpMsg.stream.errCode = 0;

    utility::Message message(EVT_DISCONNECT_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateStreaming state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming_8)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;
    msgInfo.a2dpMsg.stream.addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 1;
    msgInfo.a2dpMsg.stream.errCode = 0;
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));

    utility::Message message(EVT_CLOSE_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";
    A2dpStateStreaming state(name, mac);

    BtAddr addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x00};
    A2dpSnkProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    bool ret = state.Dispatch(message);
    EXPECT_EQ(true, ret);

    A2dpSnkProfile::GetInstance()->DeletePeer(addr);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateClosing_1)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;

    utility::Message message(EVT_CLOSE_TRANS_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateClosing state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateClosing_2)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;

    utility::Message message(EVT_DISCONNECT_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateClosing state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateClosing_3)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = A2DP_ROLE_SINK;

    utility::Message message(EVT_DISCONNECT_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateClosing state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_1)
{
    utility::Message message(EVT_SUSPEND_CFM, 0, nullptr);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(false, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_2)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_ConnectReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SNK;
    msgInfo.a2dpMsg.connectInfo.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    utility::Message message(EVT_CONNECT_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_3)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SRC;
    msgInfo.a2dpMsg.connectInfo.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.connectInfo.errCode = 1;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    utility::Message message(EVT_DISCOVER_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_4)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SRC;
    msgInfo.a2dpMsg.connectInfo.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.connectInfo.errCode = 0;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    utility::Message message(EVT_DISCOVER_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_5)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetAllCapReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SRC;
    msgInfo.a2dpMsg.connectInfo.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.connectInfo.errCode = 0;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.stream.acpSeid = 1;

    utility::Message message(EVT_GET_ALLCAP_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_6)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetCapReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SRC;
    msgInfo.a2dpMsg.connectInfo.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.connectInfo.errCode = 0;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.stream.acpSeid = 1;

    utility::Message message(EVT_GET_CAP_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_7)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_SetConfigReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SRC;
    msgInfo.a2dpMsg.connectInfo.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.connectInfo.errCode = 0;
    msgInfo.a2dpMsg.configStream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.configStream.acpSeid = 1;
    msgInfo.a2dpMsg.configStream.intSeid = 1;

    utility::Message message(EVT_SETCONFIG_REQ, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_8)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SRC;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    utility::Message message(EVT_SDP_CFM, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_10)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SNK;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    msgInfo.a2dpMsg.stream.handle = 0;

    utility::Message message(EVT_DISCONNECT_IND, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);
    A2dpSrcProfile::GetInstance()->RegisterObserver(&profileObserver_);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_11)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SRC;
    msgInfo.a2dpMsg.stream.addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    utility::Message message(EVT_TIME_OUT, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateIdle_12)
{
    A2dpAvdtMsg msgInfo = {0};
    msgInfo.role = AVDT_ROLE_SRC;

    utility::Message message(EVT_SDP_DISC, 0, &msgInfo);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateIdle state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateConfigure)
{
    utility::Message message(EVT_SUSPEND_CFM, 0, nullptr);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateConfigure state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(false, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpening)
{
    utility::Message message(EVT_SUSPEND_CFM, 0, nullptr);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpening state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(false, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateOpen)
{
    utility::Message message(EVT_SUSPEND_CFM, 0, nullptr);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateOpen state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(false, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateStreaming)
{
    utility::Message message(EVT_SUSPEND_CFM, 0, nullptr);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateStreaming state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(false, ret);
}

TEST_F(a2dp_profile_state_machine_test, A2dpStateClosing)
{
    utility::Message message(EVT_SUSPEND_CFM, 0, nullptr);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpStateClosing state(name, mac);

    bool ret = state.Dispatch(message);

    EXPECT_EQ(false, ret);
}