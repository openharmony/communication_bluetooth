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

#include "a2dp_snk_test.h"

using namespace testing;
using namespace bluetooth;

class A2dp_service_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("A2dp_service_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("A2dp_service_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerSnkAvdtpMock(&mockSnkAvdtp_);
        EXPECT_CALL(GetSnkAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
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

        service = new A2dpSnkService();
        service->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(PROFILE_NAME_A2DP_SINK))
            .Times(AtLeast(0))
            .WillRepeatedly(Return(service));
        EXPECT_CALL(psmMock_, GetProfileService(PROFILE_NAME_A2DP_SRC))
            .Times(AtLeast(0))
            .WillRepeatedly(Return(nullptr));

        // Mock IProfileConfig
        registerNewMockProfileConfig(&pcinstanceMock_);
        EXPECT_CALL(pcinstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&pcMock_));
        EXPECT_CALL(pcMock_, GetValue(_, SECTION_CONNECTION_POLICIES, _, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<3>((int)BTStrategyType::CONNECTION_ALLOWED), Return(true)));
        EXPECT_CALL(pcMock_, GetValue(_, SECTION_CODE_CS_SUPPORT, PROPERTY_A2DP_SUPPORTS_OPTIONAL_CODECS, An<int &>()))
            .Times(AtLeast(0))
            .WillRepeatedly(DoAll(SetArgReferee<3>((int)A2DP_OPTIONAL_SUPPORT), Return(true)));

    }
    virtual void TearDown()
    {
        if (service != nullptr) {
            service->GetContext()->Uninitialize();
            delete service;
        }
    }

protected:
    NiceMock<MockSnkAvdtp> mockSnkAvdtp_;
    A2dpSnkService *service;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<ProfileConfigInstanceMock> pcinstanceMock_;
    StrictMock<ProfileConfigMock> pcMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(A2dp_service_test, Enable)
{
    EXPECT_CALL(GetSnkAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));
    service->Enable();
}

TEST_F(A2dp_service_test, Connect_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->Connect(rawAddr);
    EXPECT_EQ(RET_NO_ERROR, ret);
    usleep(A2DP_SNK_TEST_TIMER_NORMAL);

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, Disconnect_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");

    int ret = service->Disconnect(rawAddr);

    EXPECT_EQ(RET_BAD_STATUS, ret);
}

TEST_F(A2dp_service_test, Disconnect_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->Disconnect(rawAddr);
    EXPECT_EQ(RET_NO_ERROR, ret);
    usleep(A2DP_SNK_TEST_TIMER_NORMAL);

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, GetConnectDevices_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    std::list<RawAddress> devList;
    devList = service->GetConnectDevices();
    std::string ret = "";

    for (auto bdr : devList) {
        if (bdr.GetAddress().c_str() != NULL) {
            ret = bdr.GetAddress().c_str();
            break;
        }
    }

    EXPECT_EQ("01:02:03:04:05:06", ret);
    EXPECT_EQ(false, devList.empty());

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, GetConnectDevices_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    std::list<RawAddress> devList;
    devList = service->GetConnectDevices();

    EXPECT_EQ(true, devList.empty());

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, GetConnectState_2)
{
    int ret = service->GetConnectState();

    EXPECT_EQ(1, ret);
}

TEST_F(A2dp_service_test, GetProfile)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    service->GetProfile();
    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, GetDevicesByStates_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    std::vector<RawAddress> devList;
    std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    devList = service->GetDevicesByStates(states);

    EXPECT_STREQ(rawAddr.GetAddress().c_str(), devList.front().GetAddress().c_str());

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, GetDevicesByStates_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    std::vector<RawAddress> devList;
    std::vector<int> states = {static_cast<int>(BTConnectState::DISCONNECTED)};
    devList = service->GetDevicesByStates(states);

    EXPECT_EQ(true, devList.empty());

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, GetDevicesByStates_3)
{
    std::vector<RawAddress> devList;
    std::vector<int> states = {static_cast<int>(BTConnectState::CONNECTED)};
    devList = service->GetDevicesByStates(states);

    EXPECT_EQ(true, devList.empty());
}

TEST_F(A2dp_service_test, GetDeviceState_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret;
    ret = service->GetDeviceState(rawAddr);

    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTED), ret);

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, GetDeviceState_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");

    int ret;
    ret = service->GetDeviceState(rawAddr);

    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTED), ret);
}

TEST_F(A2dp_service_test, GetPlayingState_1)
{
    RawAddress rawAddr("01:02:03:04:05:08");

    int ret;
    ret = service->GetPlayingState(rawAddr);

    EXPECT_EQ(-3, ret);
}

TEST_F(A2dp_service_test, GetPlayingState_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetPlayingState(true);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret;
    ret = service->GetPlayingState(rawAddr);

    EXPECT_EQ(true, ret);

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, SetConnectStrategy_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    int strategy = 0;

    EXPECT_CALL(pcMock_, RemoveProperty(_, SECTION_CONNECTION_POLICIES, _)).WillOnce(Return(true));

    int ret = service->SetConnectStrategy(rawAddr, strategy);

    EXPECT_EQ(1, ret);
}

TEST_F(A2dp_service_test, SetConnectStrategy_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    int strategy = 1;

    EXPECT_CALL(pcMock_, SetValue(_, SECTION_CONNECTION_POLICIES, _, An<int &>())).WillOnce(Return(true));
    int ret = service->SetConnectStrategy(rawAddr, strategy);
    EXPECT_EQ(RET_NO_ERROR, ret);
    usleep(A2DP_SNK_TEST_TIMER_NORMAL);
}

TEST_F(A2dp_service_test, SetConnectStrategy_3)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    int strategy = 2;

    EXPECT_CALL(pcMock_, SetValue(_, SECTION_CONNECTION_POLICIES, _, An<int &>())).WillOnce(Return(true));
    int ret = service->SetConnectStrategy(rawAddr, strategy);

    EXPECT_EQ(RET_NO_ERROR, ret);
}

TEST_F(A2dp_service_test, SetConnectStrategy_4)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    int strategy = 3;

    int ret = service->SetConnectStrategy(rawAddr, strategy);
    EXPECT_EQ(RET_BAD_STATUS, ret);
}

TEST_F(A2dp_service_test, GetConnectStrategy)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    bool ret;
    ret = service->GetConnectStrategy(rawAddr);

    EXPECT_EQ(true, ret);
}

TEST_F(A2dp_service_test, RegisterObserver)
{
    IA2dpObserver *observer = nullptr;
    service->RegisterObserver(observer);
}

TEST_F(A2dp_service_test, DeregisterObserver)
{
    IA2dpObserver *observer = nullptr;
    service->DeregisterObserver(observer);
}

TEST_F(A2dp_service_test, GetMaxConnectNum)
{
    int ret = service->GetMaxConnectNum();
    EXPECT_EQ(6, ret);
}

TEST_F(A2dp_service_test, ProcessFramkCb)
{
    int state = 0;
    RawAddress rawAddr("01:02:03:04:05:06");

    service->ProcessConnectFrameworkCallback(state, rawAddr);
}

TEST_F(A2dp_service_test, GetDeviceFromList_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    A2dpDeviceInfo *ret;

    ret = service->GetDeviceFromList(rawAddr);

    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTED), ret->GetConnectState());

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, GetDeviceFromList_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");

    A2dpDeviceInfo *ret;

    ret = service->GetDeviceFromList(rawAddr);

    EXPECT_EQ(nullptr, ret);
}

TEST_F(A2dp_service_test, GetDeviceList)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTED), service->GetDeviceList().begin()->second->GetConnectState());

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, DeleteDeviceFromList)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTED), service->GetDeviceList().begin()->second->GetConnectState());

    service->DeleteDeviceFromList(rawAddr);

    EXPECT_EQ(true, service->GetDeviceList().empty());

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(A2dp_service_test, A2dpDeviceInfo_GetDevice)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    RawAddress ret = RawAddress::ConvertToString(deviceInfo->GetDevice().addr);

    EXPECT_STREQ(rawAddr.GetAddress().c_str(), ret.GetAddress().c_str());

    delete deviceInfo;
    deviceInfo = nullptr;
}

TEST_F(A2dp_service_test, A2dpDeviceInfo_SetStateMachine)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    std::string ret = deviceInfo->GetStateMachine()->GetState()->Name();

    EXPECT_EQ("A2dpServiceDisconnected", ret);

    delete deviceInfo;
    deviceInfo = nullptr;
}

TEST_F(A2dp_service_test, A2dpDeviceInfo_GetPlayingState)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetPlayingState(true);

    bool ret;
    ret = deviceInfo->GetPlayingState();

    EXPECT_EQ(true, ret);

    delete deviceInfo;
    deviceInfo = nullptr;
}

TEST_F(A2dp_service_test, A2dpDeviceInfo_SetHandle)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetHandle(1);

    int ret;
    ret = deviceInfo->GetHandle();

    EXPECT_EQ(true, ret);

    delete deviceInfo;
    deviceInfo = nullptr;
}

TEST_F(A2dp_service_test, SwitchOptionalCodecs)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    bool isEnable = true;
    service->SwitchOptionalCodecs(rawAddr, isEnable);
}

TEST_F(A2dp_service_test, GetOptionalCodecsSupportState)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    int ret = service->GetOptionalCodecsSupportState(rawAddr);
    EXPECT_EQ(1, ret);
}

TEST_F(A2dp_service_test, StartPlaying_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");

    int ret = service->StartPlaying(rawAddr);

    EXPECT_EQ(RET_BAD_STATUS, ret);
}

TEST_F(A2dp_service_test, SuspendPlaying_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");

    int ret = service->SuspendPlaying(rawAddr);

    EXPECT_EQ(RET_BAD_STATUS, ret);
}

TEST_F(A2dp_service_test, SuspendPlaying_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    int handle = 20;

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetHandle(handle);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->SuspendPlaying(rawAddr);

    EXPECT_EQ(RET_NO_ERROR, ret);
}

TEST_F(A2dp_service_test, StopPlaying_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");

    int ret = service->StopPlaying(rawAddr);

    EXPECT_EQ(RET_BAD_STATUS, ret);
}

TEST_F(A2dp_service_test, StopPlaying_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    int handle = 20;

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetHandle(handle);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int ret = service->StopPlaying(rawAddr);

    EXPECT_EQ(RET_NO_ERROR, ret);
}

TEST_F(A2dp_service_test, UpdateOptCodecStatus)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    A2dpSrcCodecStatus codecStatus;
    A2dpSrcCodecInfo info{0};
    info.codecType = A2DP_CODEC_TYPE_SBC_USER;
    codecStatus.codecInfoConfirmedCap.push_back(info);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetCodecStatus(codecStatus);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    EXPECT_CALL(pcMock_, SetValue(_, SECTION_CODE_CS_SUPPORT, PROPERTY_A2DP_SUPPORTS_OPTIONAL_CODECS, An<int &>()))
        .WillOnce(Return(true));

    service->UpdateOptCodecStatus(rawAddr);
}

TEST_F(A2dp_service_test, CheckDisable)
{
    service->Enable();
    usleep(A2DP_SNK_TEST_TIMER_NORMAL);

    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    service->Disable();
    usleep(A2DP_SNK_TEST_TIMER_NORMAL);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
    service->CheckDisable();
}