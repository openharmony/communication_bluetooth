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

class a2dp_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerAvdtpMock(&mockAvdtp_);
        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
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
        snkservice = new A2dpSnkService();
        snkservice->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(PROFILE_NAME_A2DP_SINK))
            .Times(AtLeast(0))
            .WillRepeatedly(Return(snkservice));
        srcservice = new A2dpSrcService();
        srcservice->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(PROFILE_NAME_A2DP_SRC))
            .Times(AtLeast(0))
            .WillRepeatedly(Return(srcservice));
    }
    virtual void TearDown()
    {
        snkservice->GetContext()->Uninitialize();
        delete snkservice;
        snkservice = nullptr;
        srcservice->GetContext()->Uninitialize();
        delete srcservice;
        srcservice = nullptr;
    }

protected:
    NiceMock<MockAvdtp> mockAvdtp_;
    A2dpSnkService *snkservice;
    A2dpSrcService *srcservice;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(a2dp_test, GetConfigure)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));
    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();

    profile->Enable();
    A2dpProfilePeer *profilePeer = profile->FindOrCreatePeer(addr, peerSepType);
    uint8_t acpId = 10;
    uint8_t intId = 5;
    ConfigureStream ret;
    profilePeer->SetAcpSeid(acpId);
    profilePeer->SetIntSeid(intId);

    profile->SetConfigure(addr);
    ret = profile->GetConfigure();

    EXPECT_EQ(5, ret.intSeid);
    EXPECT_EQ(10, ret.acpSeid);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Connect_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->Enable();
    profile->FindOrCreatePeer(addr, peerSepType);
    profile->Connect(addr);
    int ret = profile->Connect(addr);

    EXPECT_EQ(0, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Connect_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->Enable();
    profile->FindOrCreatePeer(addr, peerSepType);
    profile->Disconnect(addr);
    int ret = profile->Connect(addr);

    EXPECT_EQ(0, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Disconnect_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->Enable();
    profile->FindOrCreatePeer(addr, peerSepType);
    int ret = profile->Disconnect(addr);

    EXPECT_EQ(0, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Disconnect_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    int ret = profile->Disconnect(addr);

    EXPECT_EQ(2, ret);
}

TEST_F(a2dp_test, Stop_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    uint16_t handle = 0;
    bool suspend = false;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, peerSepType);
    int ret = profile->Stop(handle, suspend);

    EXPECT_EQ(25, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Stop_2)
{
    uint16_t handle = 3;
    bool suspend = false;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();

    int ret = profile->Stop(handle, suspend);

    EXPECT_EQ(1, ret);
}

TEST_F(a2dp_test, Stop_3)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    uint16_t handle = 0;
    bool suspend = true;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, peerSepType);

    int ret = profile->Stop(handle, suspend);

    EXPECT_EQ(25, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Stop_4)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;
    uint16_t handle = 0;
    bool suspend = true;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SuspendReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    A2dpProfilePeer *peer = profile->FindOrCreatePeer(addr, peerSepType);
    peer->GetStateMachine()->InitState(A2DP_PROFILE_STREAMING);

    int ret = profile->Stop(handle, suspend);
    peer->GetStateMachine()->InitState(A2DP_PROFILE_IDLE);

    EXPECT_EQ(0, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Start_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->DeletePeer(addr);
    int ret = profile->Start(handle);

    EXPECT_EQ(1, ret);
}

TEST_F(a2dp_test, Start_3)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->Enable();
    profile->FindOrCreatePeer(addr, peerSepType);

    int ret = profile->Start(handle);

    EXPECT_EQ(0, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Close_1)
{
    uint16_t handle = 4;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();

    int ret = profile->Close(handle);

    EXPECT_EQ(1, ret);
}

TEST_F(a2dp_test, Close_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->Enable();
    A2dpProfilePeer *peer = profile->FindOrCreatePeer(addr, peerSepType);
    peer->GetStateMachine()->InitState(A2DP_PROFILE_STREAMING);

    int ret = profile->Close(handle);
    peer->GetStateMachine()->InitState(A2DP_PROFILE_IDLE);

    EXPECT_EQ(0, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Close_3)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();

    profile->Disable();
    A2dpProfilePeer *peer = profile->FindOrCreatePeer(addr, peerSepType);
    peer->GetStateMachine()->InitState(A2DP_PROFILE_CLOSING);

    int ret = profile->Close(handle);

    EXPECT_EQ(25, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Reconfig_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    uint16_t handle = 4;
    uint8_t p_codecInfos = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->DeletePeer(addr);
    int ret = profile->Reconfigure(handle, &p_codecInfos);

    EXPECT_EQ(1, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Reconfig_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    uint16_t handle = 0;
    uint8_t p_codecInfos = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->Enable();
    A2dpProfilePeer *peer = profile->FindOrCreatePeer(addr, peerSepType);
    peer->GetStateMachine()->InitState(A2DP_PROFILE_OPEN);

    int ret = profile->Reconfigure(handle, &p_codecInfos);

    EXPECT_EQ(0, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, Reconfig_3)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;
    uint16_t handle = 0;
    uint8_t p_codecInfos = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();

    profile->Disable();
    A2dpProfilePeer *peer = profile->FindOrCreatePeer(addr, peerSepType);
    peer->GetStateMachine()->InitState(A2DP_PROFILE_CLOSING);

    int ret = profile->Reconfigure(handle, &p_codecInfos);

    EXPECT_EQ(25, ret);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, SetRole)
{
    A2dpSnkProfile *profile = A2dpSnkProfile::GetInstance();

    EXPECT_EQ(A2DP_ROLE_SINK, profile->GetRole());
}

TEST_F(a2dp_test, SignalingTimeoutCallback_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();

    profile->Enable();
    profile->FindOrCreatePeer(addr, peerSepType);
    profile->SetActivePeer(addr);
    profile->GetActivePeer();

    A2dpProfilePeer *peer = profile->FindPeerByAddress(addr);
    peer->SignalingTimeoutCallback(A2DP_ROLE_SOURCE);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, SignalingTimeoutCallback_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();

    profile->FindOrCreatePeer(addr, peerSepType);
    profile->SetActivePeer(addr);

    A2dpProfilePeer *peer = profile->FindPeerByAddress(addr);
    peer->SetCurrentCmd(EVT_DISCONNECT_REQ);
    peer->SignalingTimeoutCallback(A2DP_ROLE_SOURCE);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    profile->DeletePeer(addr);
    profile->Disable();
}

TEST_F(a2dp_test, StopSignalingTimer)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, peerSepType);
    A2dpProfilePeer *peer = profile->FindPeerByAddress(addr);

    profile->Enable();
    peer->StopSignalingTimer();
    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, JudgeAllowedStreaming)
{
    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    bool ret = profile->JudgeAllowedStreaming();

    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_test, A2dpProfile)
{
    A2dpProfile *profile = new A2dpProfile(A2DP_ROLE_SOURCE);

    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t peerSepType = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    profile->Enable();
    profile->FindOrCreatePeer(addr, peerSepType);

    delete profile;
    profile = nullptr;
}

TEST_F(a2dp_test, Disable)
{
    A2dpProfile *profile = new A2dpProfile(A2DP_ROLE_SOURCE);

    profile->Disable();

    delete profile;
    profile = nullptr;
}

TEST_F(a2dp_test, GetProfileEnable)
{
    A2dpProfile *profile = new A2dpProfile(A2DP_ROLE_SOURCE);

    bool ret = profile->GetProfileEnable();

    EXPECT_EQ(false, ret);
    delete profile;
    profile = nullptr;
}

TEST_F(a2dp_test, CodecChangedNotify)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    A2dpProfile *profile = new A2dpProfile(A2DP_ROLE_SOURCE);
    ObserverProfile profileObserver_ = ObserverProfile(A2DP_ROLE_SOURCE);
    profile->RegisterObserver(&profileObserver_);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 15, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    uint8_t *resultCodecInformation = resultCodecInfo;

    A2dpProfilePeer *peer = profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);
    peer->GetCodecConfigure()->SetCodecConfig(peerCodeInformation, resultCodecInformation);

    profile->CodecChangedNotify(addr, NULL);

    profile->DeletePeer(addr);
    delete profile;
    profile = nullptr;
}

TEST_F(a2dp_test, SetUserCodecConfigure)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpSrcCodecInfo info;
    A2dpProfile *profile = new A2dpProfile(A2DP_ROLE_SOURCE);

    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    profile->SetUserCodecConfigure(addr, info);

    profile->DeletePeer(addr);
    delete profile;
    profile = nullptr;
}

TEST_F(a2dp_test, NotifyEncoder)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpProfile *profile = GetProfileInstance(A2DP_ROLE_SINK);

    A2dpProfilePeer *peer = profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    profile->NotifyEncoder(addr);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, NotifyDecoder)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint16_t handle = 0;
    Packet *pkt = PacketMalloc(0, 0, 5);
    uint32_t timeStamp = 0;
    uint8_t pt = 0;
    uint16_t streamHandle = 0;
    A2dpProfile *profile = GetProfileInstance(A2DP_ROLE_SINK);

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    A2dpProfilePeer *peer = profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);
    peer->GetStateMachine()->InitState(A2DP_PROFILE_STREAMING);
    peer->EnableOptionalCodec(false);

    profile->NotifyDecoder(addr);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, SendDelay)
{
    A2dpProfile *profile = GetProfileInstance(A2DP_ROLE_SINK);
    uint16_t handle = 1;
    uint16_t delayValue = 2;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DelayReq).WillOnce(Return(0));
    profile->SendDelay(handle, delayValue);
}

TEST_F(a2dp_test, CloseRsp)
{
    A2dpProfile *profile = GetProfileInstance(A2DP_ROLE_SINK);
    uint16_t handle = 0;
    uint16_t delayValue = 2;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseRsp).WillOnce(Return(0));
    profile->CloseRsp(handle, delayValue);

    profile->DeletePeer(addr);
}

TEST_F(a2dp_test, WriteStream)
{
    A2dpProfile *profile = GetProfileInstance(A2DP_ROLE_SINK);
    uint16_t handle = 0;
    uint8_t config = 0;
    uint8_t bitpool = 150;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    EXPECT_CALL(GetAvdtpMocker(), AVDT_WriteReq).WillOnce(Return(0));
    profile->WriteStream(handle, config, bitpool);

    profile->DeletePeer(addr);
}