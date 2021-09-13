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

class A2dp_src_service_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("A2dp_src_service_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("A2dp_src_service_test TearDownTestCase\n");
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

        service = new A2dpSrcService();
        service->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(PROFILE_NAME_A2DP_SRC))
            .Times(AtLeast(0))
            .WillRepeatedly(Return(service));       
        
    }
    virtual void TearDown()
    {
        if (service != nullptr) {
            service->GetContext()->Uninitialize();
            delete service;
        }
    }

protected:
    NiceMock<MockAvdtp> mockAvdtp_;
    A2dpSrcService *service;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(A2dp_src_service_test, StartPlaying_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");

    int ret = service->StartPlaying(rawAddr);

    EXPECT_EQ(RET_BAD_STATUS, ret);
}

TEST_F(A2dp_src_service_test, StartPlaying_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    int handle = 21;

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetHandle(handle);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->StartPlaying(rawAddr);

    EXPECT_EQ(AVDT_BAD_PARAMS, ret);
}

TEST_F(A2dp_src_service_test, SuspendPlaying_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    int handle = 21;

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetHandle(handle);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->SuspendPlaying(rawAddr);
    EXPECT_EQ(RET_NO_ERROR, ret);
}

TEST_F(A2dp_src_service_test, SuspendPlaying_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    int handle = 21;

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetHandle(handle);
    deviceInfo->SetPlayingState(true);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->SuspendPlaying(rawAddr);
    EXPECT_EQ(AVDT_BAD_PARAMS, ret);
}

TEST_F(A2dp_src_service_test, StopPlaying_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    int handle = 22;

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetHandle(handle);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->StopPlaying(rawAddr);
    EXPECT_EQ(RET_NO_ERROR, ret);
}

TEST_F(A2dp_src_service_test, StopPlaying_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    int handle = 22;

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetHandle(handle);
    deviceInfo->SetPlayingState(true);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->StopPlaying(rawAddr);
    EXPECT_EQ(AVDT_BAD_PARAMS, ret);
}

TEST_F(A2dp_src_service_test, GetSnkProfile)
{
    IProfileA2dp::GetSnkProfile();
}

TEST_F(A2dp_src_service_test, GetSrcProfile)
{
    IProfileA2dp::GetSrcProfile();
}

TEST_F(A2dp_src_service_test, SetActiveSinkDevice)
{
    RawAddress rawAddr1("00:00:00:00:00:01");
    A2dpDeviceInfo *deviceInfo1 = new A2dpDeviceInfo(rawAddr1);
    deviceInfo1->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr1.GetAddress().c_str(), deviceInfo1);
    service->UpdateActiveDevice(rawAddr1);

    RawAddress rawAddr2("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo2 = new A2dpDeviceInfo(rawAddr2);
    deviceInfo2->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr2.GetAddress().c_str(), deviceInfo2);

    service->SetActiveSinkDevice(rawAddr2);
    EXPECT_STREQ(rawAddr2.GetAddress().c_str(), service->GetActiveSinkDevice().GetAddress().c_str());
}

TEST_F(A2dp_src_service_test, SetCodecPreference)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    A2dpSrcCodecStatus codecStatus;
    A2dpSrcCodecInfo info{0};
    info.codecType = 1;
    info.sampleRate = 1;
    info.bitsPerSample = 1;
    info.channelMode = 1;
    codecStatus.codecInfoConfirmedCap.push_back(info);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetCodecStatus(codecStatus);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->SetCodecPreference(rawAddr, info);
    EXPECT_EQ(RET_BAD_PARAM, ret);

    codecStatus.codecInfo = info;
    deviceInfo->SetCodecStatus(codecStatus);
    ret = service->SetCodecPreference(rawAddr, info);
    EXPECT_EQ(RET_NO_ERROR, ret);

    A2dpSrcCodecInfo info2{0};
    info2.codecType = 1;
    info2.sampleRate = 3;
    info2.bitsPerSample = 3;
    info2.channelMode = 3;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);
    A2dpProfile *profile = GetProfileInstance(A2DP_ROLE_SOURCE);
    A2dpProfilePeer *peer = profile->FindOrCreatePeer(btAddr, A2DP_ROLE_SOURCE);
    peer->EnableOptionalCodec(false);

    ret = service->SetCodecPreference(rawAddr, info2);
    EXPECT_EQ(RET_BAD_PARAM, ret);

    profile->DeletePeer(btAddr);
}

TEST_F(A2dp_src_service_test, SetAudioConfigure)
{
    BtAddr addr = {{0x06, 0x05, 0x04, 0x03, 0x02, 0x01}, 0x00};
    RawAddress rawAddr("01:02:03:04:05:06");

    service->SetAudioConfigure(rawAddr, 0, 0, 0);
}