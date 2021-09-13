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

class a2dp_snk_service_statemachine_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_snk_service_statemachine_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_snk_service_statemachine_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerSnkAvdtpMock(&mockSnkAvdtp_);
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
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(service));

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
            service = nullptr;
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

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnected_Dispatch1)
{
    EXPECT_CALL(GetSnkAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));

    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_CONNECT, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnected state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnected_Dispatch2)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_DISCONNECT, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnected state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnected_Dispatch3)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CONNECTED, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnected state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnected_Dispatch4)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CONNECTING, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnected state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnected_Dispatch5)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTED, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnected state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnected_Dispatch6)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTING, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnected state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnected_Dispatch7)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CODEC_CHANGE, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnected state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnecting_Dispatch1)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_CONNECT, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnecting_Dispatch2)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_DISCONNECT, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnecting_Dispatch3)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CONNECTED, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnecting_Dispatch4)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CONNECTING, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnecting_Dispatch5)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTED, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnecting_Dispatch6)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTING, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpDisconnecting_Dispatch7)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CODEC_CHANGE, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_CONNECT, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_CONNECT_TIMEOUT, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch4)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_CONNECTED, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch5)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_CONNECTING, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch6)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTED, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch7)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTING, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch8)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_AUDIO_PLAY_START, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch9)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_AUDIO_PLAY_START, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch10)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_AUDIO_PLAY_SUSPEND, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch11)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_AUDIO_PLAY_STOP, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch12)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetPlayingState(true);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_AUDIO_PLAY_STOP, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch13)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetPlayingState(true);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_PROFILE_CODEC_CHANGE, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnected_Dispatch14)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    deviceInfo->SetPlayingState(true);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    int invalidState = 11;
    utility::Message msg(invalidState, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    bool ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnecting_Dispatch1)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_CONNECT, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnecting_Dispatch4)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CONNECTED, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnecting_Dispatch5)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CONNECTING, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnecting_Dispatch6)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTED, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnecting_Dispatch7)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_DISCONNECTING, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, A2dpConnecting_Dispatch8)
{
    bool ret = false;
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_PROFILE_CODEC_CHANGE, A2DP_ROLE_SINK, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    ret = state.Dispatch(msg);
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_snk_service_statemachine_test, OnConnectStateChanged1)
{
    A2dpDeviceInfo *deviceInfo;
    RawAddress rawAddr("01:02:03:04:05:06");

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = STREAM_CONNECTING;
    CallbackParameter context;
    context.role = A2DP_ROLE_ACP;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnConnectStateChanged(btAddr, state, (void *)&context);

    deviceInfo = service->GetDeviceFromList(rawAddr);
    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTING), deviceInfo->GetConnectState());
}

TEST_F(a2dp_snk_service_statemachine_test, OnConnectStateChanged2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = STREAM_CONNECTING;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnConnectStateChanged(btAddr, state, (void *)&context);

    int ret = service->GetDeviceFromList(rawAddr)->GetConnectState();
    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTING), ret);
}

TEST_F(a2dp_snk_service_statemachine_test, OnConnectStateChanged3)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = STREAM_CONNECT;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnConnectStateChanged(btAddr, state, (void *)&context);

    int ret = service->GetDeviceFromList(rawAddr)->GetConnectState();
    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTED), ret);
}

TEST_F(a2dp_snk_service_statemachine_test, OnConnectStateChanged4)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = STREAM_DISCONNECT;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnConnectStateChanged(btAddr, state, (void *)&context);

    int ret = service->GetDeviceFromList(rawAddr)->GetConnectState();
    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTED), ret);
}

TEST_F(a2dp_snk_service_statemachine_test, OnConnectStateChanged5)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = STREAM_DISCONNECTING;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnConnectStateChanged(btAddr, state, (void *)&context);

    int ret = service->GetDeviceFromList(rawAddr)->GetConnectState();
    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTING), ret);
}

TEST_F(a2dp_snk_service_statemachine_test, OnConnectStateChanged6)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = STREAM_CONNECT_FAILED;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnConnectStateChanged(btAddr, state, (void *)&context);

    int ret = service->GetDeviceFromList(rawAddr)->GetConnectState();
    EXPECT_EQ(static_cast<int>(BTConnectState::DISCONNECTED), ret);
}

TEST_F(a2dp_snk_service_statemachine_test, OnConnectStateChanged7)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = STREAM_TIMEOUT;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnConnectStateChanged(btAddr, state, (void *)&context);

    int ret = service->GetDeviceFromList(rawAddr)->GetConnectState();
    EXPECT_EQ(static_cast<int>(BTConnectState::CONNECTING), ret);
}

TEST_F(a2dp_snk_service_statemachine_test, OnAudioStateChanged1)
{
    RawAddress rawAddr("01:02:03:04:05:0B");
    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = A2DP_MSG_PROFILE_AUDIO_PLAY_START;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnAudioStateChanged(btAddr, state, (void *)&context);

    EXPECT_EQ(nullptr, service->GetDeviceFromList(rawAddr));
}

TEST_F(a2dp_snk_service_statemachine_test, OnAudioStateChanged2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = A2DP_MSG_PROFILE_AUDIO_PLAY_START;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnAudioStateChanged(btAddr, state, (void *)&context);
}

TEST_F(a2dp_snk_service_statemachine_test, OnAudioStateChanged3)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = A2DP_MSG_PROFILE_AUDIO_PLAY_SUSPEND;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnAudioStateChanged(btAddr, state, (void *)&context);
}

TEST_F(a2dp_snk_service_statemachine_test, OnAudioStateChanged4)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = A2DP_MSG_PROFILE_AUDIO_PLAY_STOP;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnAudioStateChanged(btAddr, state, (void *)&context);
}

TEST_F(a2dp_snk_service_statemachine_test, OnAudioStateChanged5)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = A2DP_MSG_PROFILE_CODEC_CHANGE;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnAudioStateChanged(btAddr, state, (void *)&context);
}

TEST_F(a2dp_snk_service_statemachine_test, OnCodecStateChanged1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo deviceInfo(rawAddr);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    A2dpSrcCodecStatus codecInfo;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnCodecStateChanged(btAddr, codecInfo, (void *)&context);

    EXPECT_EQ(nullptr, service->GetDeviceFromList(rawAddr));
}

TEST_F(a2dp_snk_service_statemachine_test, OnCodecStateChanged2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);
    deviceInfo->SetPlayingState(true);
    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTING));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    A2dpSrcCodecStatus codecInfo;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnCodecStateChanged(btAddr, codecInfo, (void *)&context);
}
