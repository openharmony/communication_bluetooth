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
#include "hfp_ag_profile.h"
#include "common_mock_all.h"
#include "ag_mock.h"

using namespace testing;
using namespace bluetooth;
class HfpAgProfileTest : public testing::Test {
public:
    std::unique_ptr<HfpAgProfile> agProfile{nullptr};

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

        // Mock PowerManager & PowerManagerInstance
        registerNewMockPowerManager(&pmInstanceMock_);
        EXPECT_CALL(pmInstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(ReturnRef(pmMock_));
        EXPECT_CALL(pmMock_, StatusUpdate(_,_,_)).Times(AtLeast(0)).WillRepeatedly(Return());

        // Mock BT Manager
        registerBtmMocker(&btmMock_);
        initBtmMock();

        // Mock Gap
        registerGapMocker(&gapMock_);
        initGapMock();

        // Mock Rfcomm
        registerRfcommMocker(&rfcommMock_);
        initRfcommMock();

        // Mock Sdp
        registerSdpMocker(&sdpMock_);
        initSdpMock();

        agProfile = std::make_unique<HfpAgProfile>(agAddr);
        agProfile->ProcessSlcEstablished();
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

    void initGapMock()
    {
        EXPECT_CALL(GetGapMocker(), GAPIF_RegisterServiceSecurity()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetGapMocker(), GAPIF_DeregisterServiceSecurity()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetGapMocker(), GAP_RequestSecurity()).Times(AnyNumber()).WillRepeatedly(Return(0));
    }

    void initRfcommMock()
    {
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_RegisterServer()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_DeregisterServer()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_FreeServerNum()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_AcceptConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_RejectConnection()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_AssignServerNum()).Times(AnyNumber()).WillRepeatedly(Return(1));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_ConnectChannel()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_DisconnectChannel()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Read(_, _)).Times(AnyNumber()).WillRepeatedly(Return(1));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_Write()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetRfcommMocker(), RFCOMM_GetPeerAddress()).Times(AnyNumber()).WillRepeatedly(Return(agBtAddr));
    }

    void initSdpMock()
    {
        EXPECT_CALL(GetSdpMocker(), SDP_CreateServiceRecord()).Times(AnyNumber()).WillRepeatedly(Return(1));
        EXPECT_CALL(GetSdpMocker(), SDP_RegisterServiceRecord()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_DeregisterServiceRecord()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_DestroyServiceRecord()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddServiceClassIdList()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddProtocolDescriptorList()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddBluetoothProfileDescriptorList())
            .Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddBrowseGroupList()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddAttribute()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_ServiceSearchAttribute()).Times(AnyNumber()).WillRepeatedly(Return(0));
        EXPECT_CALL(GetSdpMocker(), SDP_AddServiceName()).Times(AnyNumber()).WillRepeatedly(Return(0));
    }

private:
    StrictMock<AdapterManagerMock> amMock_;
    StrictMock<ProfileServiceManagerInstanceMock> psmInstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<PowerManagerInstanceMock> pmInstanceMock_;
    StrictMock<PowerManagerMock> pmMock_;
    StrictMock<AgBtmMock> btmMock_;
    StrictMock<AgGapMock> gapMock_;
    StrictMock<AgRfcommMock> rfcommMock_;
    StrictMock<AgSdpMock> sdpMock_;
    HfpAgService agService{};
};

TEST_F(HfpAgProfileTest, RingTimeout)
{
    agProfile->RingTimeout();
}

TEST_F(HfpAgProfileTest, RejectAudioConnection)
{
    int ret = agProfile->RejectAudioConnection();
    EXPECT_EQ(HFP_AG_SUCCESS, ret);
}

TEST_F(HfpAgProfileTest, SendSubscriberNumberInformation)
{
    int ret = agProfile->SendSubscriberNumberInformation(1, "999", 1);
    EXPECT_EQ(HFP_AG_SUCCESS, ret);
}

TEST_F(HfpAgProfileTest, SendResultCode_1)
{
    int ret = agProfile->SendResultCode(HFP_AG_RESULT_RING);
    EXPECT_EQ(HFP_AG_SUCCESS, ret);
}

TEST_F(HfpAgProfileTest, SendResultCode_2)
{
    int ret = agProfile->SendResultCode(HFP_AG_RESULT_NO_CARRIER);
    EXPECT_EQ(HFP_AG_SUCCESS, ret);
}

TEST_F(HfpAgProfileTest, SendResultCode_3)
{
    int ret = agProfile->SendResultCode(HFP_AG_RESULT_BUSY);
    EXPECT_EQ(HFP_AG_SUCCESS, ret);
}

TEST_F(HfpAgProfileTest, SendResultCode_4)
{
    int ret = agProfile->SendResultCode(HFP_AG_RESULT_NO_ANSWER);
    EXPECT_EQ(HFP_AG_SUCCESS, ret);
}

TEST_F(HfpAgProfileTest, SendResultCode_5)
{
    int ret = agProfile->SendResultCode(HFP_AG_RESULT_DELAYED);
    EXPECT_EQ(HFP_AG_SUCCESS, ret);
}

TEST_F(HfpAgProfileTest, SendResultCode_6)
{
    int ret = agProfile->SendResultCode(HFP_AG_RESULT_BLOCKLISTED);
    EXPECT_EQ(HFP_AG_SUCCESS, ret);
}

// Not support HFP_AG_RESULT_NO_DIALTONE, so return failure.
TEST_F(HfpAgProfileTest, SendResultCode_7)
{
    int ret = agProfile->SendResultCode(HFP_AG_RESULT_NO_DIALTONE);
    EXPECT_EQ(HFP_AG_FAILURE, ret);
}