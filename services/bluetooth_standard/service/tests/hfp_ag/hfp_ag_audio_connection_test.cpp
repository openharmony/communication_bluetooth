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

#include "hfp_ag_service.h"
#include "hfp_ag_audio_connection.h"
#include "common_mock_all.h"
#include "ag_mock.h"

using namespace testing;
using namespace bluetooth;
class HfpAgAudioTest : public testing::Test {
public:
    HfpAgAudioConnection agAudio{};

    void SetUp()
    {
        // Mock AdapterManager::GetState()
        registerNewMockAdapterManager(&amMock_);
        EXPECT_CALL(GetAdapterManagerMock(), AdapterManager_GetState()).Times(AtLeast(0))
            .WillRepeatedly(Return(STATE_TURN_ON));

        // Mock ProfileServiceManager
        registerNewMockProfileServiceManager(&psmInstanceMock_);
        EXPECT_CALL(psmInstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
        agService.GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(&agService));

        // Mock BT Manager
        registerBtmMocker(&btmMock_);
        initBtmMock();
    }

    void TearDown()
    {
        agService.GetContext()->Uninitialize();
    }

    void initBtmMock()
    {
        EXPECT_CALL(GetBtmMocker(), BTM_RegisterScoCallbacks()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_DeregisterScoCallbacks()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_WriteVoiceSetting()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_IsSecureConnection()).Times(AnyNumber()).WillRepeatedly(Return(false));
        EXPECT_CALL(GetBtmMocker(), BTM_CreateEscoConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_DisconnectScoConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_AcceptEscoConnectionRequest()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_CreateScoConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_AcceptScoConnectionRequest()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_RejectScoConnectionRequest()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_GetLocalSupportedCodecs()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetBtmMocker(), BTM_IsControllerSupportEsco()).Times(AnyNumber()).WillRepeatedly(Return(false));
    }

    void SetPremise_Start()
    {
        int ret = HfpAgAudioConnection::Register();
        EXPECT_EQ(0, ret);
    }

    void SetPremise_Stop()
    {
        int ret = HfpAgAudioConnection::Deregister();
        EXPECT_EQ(0, ret);
    }

private:
    StrictMock<AdapterManagerMock> amMock_;
    StrictMock<ProfileServiceManagerInstanceMock> psmInstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AgBtmMock> btmMock_;
    HfpAgService agService{};
};

TEST_F(HfpAgAudioTest, GetActiveDevice)
{
    HfpAgAudioConnection::SetActiveDevice(agAddr);
    usleep(50000);  // 50ms wait for service thread processing the message

    std::string addr = HfpAgAudioConnection::GetActiveDevice();
    EXPECT_EQ(agAddr, addr);
}

TEST_F(HfpAgAudioTest, ConnectByCvsd_1)
{
    SetPremise_Start();

    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(true, true, HFP_AG_CODEC_CVSD);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(0, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, ConnectByCvsd_2)
{
    SetPremise_Start();

    EXPECT_CALL(GetBtmMocker(), BTM_IsSecureConnection()).Times(AnyNumber()).WillRepeatedly(Return(true));
    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(true, true, HFP_AG_CODEC_CVSD);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(0, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, AcceptAudioConnection_MSBC1)
{
    SetPremise_Start();

    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(true, true, HFP_AG_CODEC_MSBC);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(0, ret);

    ret = agAudio.AcceptAudioConnection();
    EXPECT_EQ(0, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, AcceptAudioConnection_MSBC2)
{
    SetPremise_Start();

    EXPECT_CALL(GetBtmMocker(), BTM_IsSecureConnection()).Times(AnyNumber()).WillRepeatedly(Return(false));
    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(true, true, HFP_AG_CODEC_MSBC);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(0, ret);

    ret = agAudio.AcceptAudioConnection();
    EXPECT_EQ(0, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, AcceptAudioConnection_MSBC3)
{
    SetPremise_Start();

    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(false, false, HFP_AG_CODEC_MSBC);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(BT_BAD_PARAM, ret);

    ret = agAudio.AcceptAudioConnection();
    EXPECT_EQ(BT_BAD_PARAM, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, AcceptAudioConnection_CVSD1)
{
    SetPremise_Start();

    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(true, true, HFP_AG_CODEC_CVSD);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(0, ret);

    ret = agAudio.AcceptAudioConnection();
    EXPECT_EQ(0, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, AcceptAudioConnection_CVSD2)
{
    SetPremise_Start();

    EXPECT_CALL(GetBtmMocker(), BTM_IsSecureConnection()).Times(AnyNumber()).WillRepeatedly(Return(true));
    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(true, true, HFP_AG_CODEC_CVSD);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(0, ret);

    ret = agAudio.AcceptAudioConnection();
    EXPECT_EQ(0, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, AcceptAudioConnection_CVSD3)
{
    SetPremise_Start();

    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(false, false, HFP_AG_CODEC_CVSD);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(0, ret);

    ret = agAudio.AcceptAudioConnection();
    EXPECT_EQ(0, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, AcceptAudioConnection_NONE)
{
    SetPremise_Start();

    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(true, true, HFP_AG_CODEC_NONE);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(BT_BAD_PARAM, ret);

    ret = agAudio.AcceptAudioConnection();
    EXPECT_EQ(BT_BAD_PARAM, ret);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, OnConnectRequest)
{
    SetPremise_Start();

    BtmScoConnectionRequestParam param[1];
    param[0].addr = &agBtAddr;
    param[0].linkType = LINK_TYPE_SCO;
    GetBtmMocker().callbacks->scoConnectionRequest(param, nullptr);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, OnConnectCompleted1)
{
    SetPremise_Start();

    BtmScoConnectionRequestParam param1[1];
    param1[0].addr = &agBtAddr;
    param1[0].linkType = LINK_TYPE_SCO;
    GetBtmMocker().callbacks->scoConnectionRequest(param1, nullptr);

    BtmScoConnectionCompleteParam param2[1];
    param2[0].addr = &agBtAddr;
    param2[0].connectionHandle = 1;
    param2[0].status = 0;
    GetBtmMocker().callbacks->scoConnectionComplete(param2, nullptr);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, OnConnectCompleted2)
{
    SetPremise_Start();

    agAudio.SetActiveDevice(agAddr);
    agAudio.SetRemoteAddr(agAddr);
    agAudio.SetSupportFeatures(true, true, HFP_AG_CODEC_CVSD);
    int ret = agAudio.ConnectAudio();
    EXPECT_EQ(0, ret);

    BtmScoConnectionCompleteParam param2[1];
    param2[0].addr = &agBtAddr;
    param2[0].connectionHandle = 1;
    param2[0].status = 1;
    GetBtmMocker().callbacks->scoConnectionComplete(param2, nullptr);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, OnConnectCompleted3)
{
    SetPremise_Start();

    BtmScoConnectionRequestParam param1[1];
    param1[0].addr = &agBtAddr;
    param1[0].linkType = LINK_TYPE_SCO;
    GetBtmMocker().callbacks->scoConnectionRequest(param1, nullptr);

    BtmScoConnectionCompleteParam param2[1];
    param2[0].addr = &agBtAddr;
    param2[0].connectionHandle = 1;
    param2[0].status = 1;
    GetBtmMocker().callbacks->scoConnectionComplete(param2, nullptr);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, OnDisconnectCompleted1)
{
    SetPremise_Start();

    BtmScoConnectionRequestParam param1[1];
    param1[0].addr = &agBtAddr;
    param1[0].linkType = LINK_TYPE_SCO;
    GetBtmMocker().callbacks->scoConnectionRequest(param1, nullptr);

    BtmScoConnectionCompleteParam param2[1];
    param2[0].addr = &agBtAddr;
    param2[0].connectionHandle = 1;
    param2[0].status = 1;
    GetBtmMocker().callbacks->scoConnectionComplete(param2, nullptr);

    BtmScoDisconnectionCompleteParam param3[1];
    param3[0].reason = 0;
    param3[0].connectionHandle = 1;
    param3[0].status = 0;
    GetBtmMocker().callbacks->scoDisconnectionComplete(param3, nullptr);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, OnDisconnectCompleted2)
{
    SetPremise_Start();

    BtmScoConnectionRequestParam param1[1];
    param1[0].addr = &agBtAddr;
    param1[0].linkType = LINK_TYPE_SCO;
    GetBtmMocker().callbacks->scoConnectionRequest(param1, nullptr);

    BtmScoConnectionCompleteParam param2[1];
    param2[0].addr = &agBtAddr;
    param2[0].connectionHandle = 1;
    param2[0].status = 1;
    GetBtmMocker().callbacks->scoConnectionComplete(param2, nullptr);

    BtmScoDisconnectionCompleteParam param3[1];
    param3[0].reason = 0;
    param3[0].connectionHandle = 1;
    param3[0].status = 1;
    GetBtmMocker().callbacks->scoDisconnectionComplete(param3, nullptr);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, OnConnectionChanged)
{
    SetPremise_Start();

    BtmScoConnectionChangedParam param[1];
    param[0].connectionHandle = 1;
    param[0].retransmissionWindow = 0;
    param[0].rxPacketLength = 1;
    param[0].transmissionInterval = 1;
    param[0].txPacketLength = 1;
    GetBtmMocker().callbacks->scoConnectionChanged(param, nullptr);

    SetPremise_Stop();
}

TEST_F(HfpAgAudioTest, OnWriteVoiceSettingCompleted)
{
    SetPremise_Start();

    GetBtmMocker().callbacks->writeVoiceSettingComplete(1, nullptr);

    SetPremise_Stop();
}