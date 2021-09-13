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

class a2dp_snk_service_connection_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_snk_service_connection_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_snk_service_connection_test TearDownTestCase\n");
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
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(a2dp_snk_service_connection_test, A2dpConnect_1)
{
    // add device1
    RawAddress rawAddr1("01:02:03:04:05:06");
    A2dpDeviceInfo *deviceInfo1 = new A2dpDeviceInfo(rawAddr1);

    deviceInfo1->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr1.GetAddress().c_str(), deviceInfo1);

    // add device2
    RawAddress rawAddr2("01:02:03:04:05:07");
    A2dpDeviceInfo *deviceInfo2 = new A2dpDeviceInfo(rawAddr2);

    deviceInfo2->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr2.GetAddress().c_str(), deviceInfo2);

    // add device3
    RawAddress rawAddr3("01:02:03:04:05:08");
    A2dpDeviceInfo *deviceInfo3 = new A2dpDeviceInfo(rawAddr3);

    deviceInfo3->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr3.GetAddress().c_str(), deviceInfo3);

    // add device4
    RawAddress rawAddr4("01:02:03:04:05:09");
    A2dpDeviceInfo *deviceInfo4 = new A2dpDeviceInfo(rawAddr4);

    deviceInfo4->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr4.GetAddress().c_str(), deviceInfo4);

    // add device5
    RawAddress rawAddr5("01:02:03:04:05:0A");
    A2dpDeviceInfo *deviceInfo5 = new A2dpDeviceInfo(rawAddr5);

    deviceInfo5->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr5.GetAddress().c_str(), deviceInfo5);

    // add device6
    RawAddress rawAddr6("01:02:03:04:05:0B");
    A2dpDeviceInfo *deviceInfo6 = new A2dpDeviceInfo(rawAddr6);

    deviceInfo6->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr6.GetAddress().c_str(), deviceInfo6);

    RawAddress rawAddr("01:02:03:04:05:0C");

    bool ret = service->ConnectManager().JudgeConnectedNum();
    EXPECT_EQ(false, ret);

    service->DeleteDeviceFromList(rawAddr1);
    service->DeleteDeviceFromList(rawAddr2);
    service->DeleteDeviceFromList(rawAddr3);
    service->DeleteDeviceFromList(rawAddr4);
    service->DeleteDeviceFromList(rawAddr5);
    service->DeleteDeviceFromList(rawAddr6);
}

TEST_F(a2dp_snk_service_connection_test, A2dpConnect_2)
{
    RawAddress rawAddr("01:02:03:04:05:0B");
    A2dpDeviceInfo *deviceInfo = new A2dpDeviceInfo(rawAddr);

    EXPECT_CALL(GetSnkAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));

    deviceInfo->SetConnectState(static_cast<int>(BTConnectState::CONNECTED));
    service->AddDeviceToList(rawAddr.GetAddress().c_str(), deviceInfo);

    bool ret = service->ConnectManager().A2dpConnect(rawAddr);
    EXPECT_EQ(true, ret);

    service->DeleteDeviceFromList(rawAddr);
}

TEST_F(a2dp_snk_service_connection_test, A2dpDisconnect_1)
{
    RawAddress rawAddr("01:02:03:04:05:0B");

    int state = static_cast<int>(BTConnectState::CONNECTED);
    service->ConnectManager().AddDevice(rawAddr, state);

    bool ret = service->ConnectManager().A2dpDisconnect(rawAddr);
    EXPECT_EQ(true, ret);
}
