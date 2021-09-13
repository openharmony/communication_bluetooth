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

class a2dp_src_service_connection_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_src_service_connection_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_src_service_connection_test TearDownTestCase\n");
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

TEST_F(a2dp_src_service_connection_test, A2dpConnect_1)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    service->Enable();
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    RawAddress rawAddr("01:02:03:04:05:0B");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    bool ret = service->ConnectManager().A2dpConnect(rawAddr);
    EXPECT_EQ(true, ret);

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(0));
    service->Disable();
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    BtAddr addr = {{0x0B, 0x05, 0x04, 0x03, 0x02, 0x01}, 0x00};
    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
}

TEST_F(a2dp_src_service_connection_test, A2dpDisconnect)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    RawAddress rawAddr("01:02:03:04:05:0B");

    service->Enable();
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    bool ret = service->ConnectManager().A2dpDisconnect(rawAddr);
    EXPECT_EQ(false, ret);

    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::DISCONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    service->Disable();
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(a2dp_src_service_connection_test, AddDevice)
{
    EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    service->Enable();
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    RawAddress rawAddr("01:02:03:04:05:0B");
    int state = static_cast<int>(BTConnectState::CONNECTED);
    service->ConnectManager().AddDevice(rawAddr, state);

    service->DeleteDeviceFromList(rawAddr);
    service->Disable();
}

TEST_F(a2dp_src_service_connection_test, JudgeConnectedNum_1)
{
    RawAddress rawAddr("01:02:03:04:05:0C");

    bool ret = service->ConnectManager().JudgeConnectedNum();
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_src_service_connection_test, JudgeConnectedNum_2)
{
    RawAddress rawAddr("01:02:03:04:05:0B");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    bool ret = service->ConnectManager().JudgeConnectedNum();
    EXPECT_EQ(true, ret);
}

TEST_F(a2dp_src_service_connection_test, JudgeConnectExit)
{
    A2dpSnkService *serviceSnk;
    serviceSnk = new A2dpSnkService();
    serviceSnk->GetContext()->Initialize();
    EXPECT_CALL(psmMock_, GetProfileService(PROFILE_NAME_A2DP_SINK))
        .Times(AtLeast(0))
        .WillRepeatedly(Return(serviceSnk));

    RawAddress rawAddr("01:02:03:04:05:0B");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    uint8_t role = A2DP_ROLE_SOURCE;

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    serviceSnk->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    bool ret = service->ConnectManager().JudgeConnectExit(rawAddr, role);
    EXPECT_EQ(true, ret);

    serviceSnk->GetContext()->Uninitialize();
    delete serviceSnk;
}
