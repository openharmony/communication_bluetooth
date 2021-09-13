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

class a2dp_src_service_statemachine_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_src_service_statemachine_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_src_service_statemachine_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        // Mock ProfileServiceManager
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
    A2dpSrcService *service;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(a2dp_src_service_statemachine_test, A2dpDisconnected_Dispatch1)
{
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_CONNECT_FORBIDDEN, A2DP_ROLE_SOURCE, &rawAddr);
    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnected state(name, mac);
    EXPECT_EQ(false, state.Dispatch(msg));
}

TEST_F(a2dp_src_service_statemachine_test, A2dpDisconnecting_Dispatch1)
{
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_DISCONNECT, A2DP_ROLE_SOURCE, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpDisconnecting state(name, mac);

    EXPECT_EQ(true, state.Dispatch(msg));
}

TEST_F(a2dp_src_service_statemachine_test, A2dpConnected_Dispatch1)
{
    RawAddress rawAddr("01:02:03:04:05:0B");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    utility::Message msg(A2DP_MSG_DISCONNECT, A2DP_ROLE_SOURCE, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnected state(name, mac);
    EXPECT_EQ(false, state.Dispatch(msg));

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(a2dp_src_service_statemachine_test, A2dpConnecting_Dispatch_1)
{
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_CONNECT_TIMEOUT, A2DP_ROLE_SOURCE, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    EXPECT_EQ(false, state.Dispatch(msg));
}

TEST_F(a2dp_src_service_statemachine_test, A2dpConnecting_Dispatch_2)
{
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_DISCONNECT, A2DP_ROLE_SOURCE, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    EXPECT_EQ(false, state.Dispatch(msg));
}

TEST_F(a2dp_src_service_statemachine_test, A2dpConnecting_Dispatch_3)
{
    RawAddress rawAddr("00:00:00:00:00:00");

    utility::Message msg(A2DP_MSG_CONNECT_FORBIDDEN, A2DP_ROLE_SOURCE, &rawAddr);

    utility::StateMachine mac;
    const std::string name = "abc";

    A2dpConnecting state(name, mac);
    EXPECT_EQ(false, state.Dispatch(msg));
}

TEST_F(a2dp_src_service_statemachine_test, OnConnectStateChanged)
{
    RawAddress rawAddr("01:02:03:04:05:06");

    BtAddr btAddr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = STREAM_CONNECTING;
    CallbackParameter context;
    context.role = A2DP_ROLE_ACP;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile connectChange;
    connectChange.OnConnectStateChanged(btAddr, state, (void *)&context);
}

TEST_F(a2dp_src_service_statemachine_test, OnAudioStateChanged_1)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo deviceInfo(rawAddr);

    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = A2DP_IS_PLAYING;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    ObserverProfile stateChange;
    stateChange.OnAudioStateChanged(btAddr, state, (void *)&context);
    EXPECT_STREQ("", service->GetActiveSinkDevice().GetAddress().c_str());
}

TEST_F(a2dp_src_service_statemachine_test, OnAudioStateChanged_2)
{
    RawAddress rawAddr("01:02:03:04:05:06");
    A2dpDeviceInfo deviceInfo(rawAddr);

    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);

    int state = A2DP_NOT_PLAYING;
    CallbackParameter context;
    context.role = A2DP_ROLE_INT;
    context.handle = 0;
    context.srcRole = true;

    service->UpdateActiveDevice(rawAddr);

    ObserverProfile stateChange;
    stateChange.OnAudioStateChanged(btAddr, state, (void *)&context);
    EXPECT_STREQ("01:02:03:04:05:06", service->GetActiveSinkDevice().GetAddress().c_str());
}

TEST_F(a2dp_src_service_statemachine_test, OnCodecStateChanged)
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
