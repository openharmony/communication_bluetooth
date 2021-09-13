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

#include "di_mock.h"
using namespace bluetooth;
using namespace testing;
SDPMock *g_sdpmock;
XmlMock *g_xmlmock;
IContextCallbackMock *g_callback = new IContextCallbackMock();
void DIServiceTest::SetUpTestCase()
{}
void DIServiceTest::TearDownTestCase()
{
    delete g_callback;
    g_callback = nullptr;
}
void DIServiceTest::SetUp()
{
    diService_ = new DIService();
    g_sdpmock = new SDPMock();
    g_xmlmock = new XmlMock();
    diService_->GetContext()->RegisterCallback(*g_callback);
}
void DIServiceTest::TearDown()
{
    delete diService_;
    diService_ = nullptr;
    delete g_sdpmock;
    g_sdpmock = nullptr;
    delete g_xmlmock;
    g_xmlmock = nullptr;
}

TEST_F(DIServiceTest, Enable_case001)
{
    EXPECT_CALL(*g_callback, OnEnable(PROFILE_NAME_DI, false)).Times(1);
    EXPECT_CALL(*g_sdpmock, SDP_CreateServiceRecord()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_xmlmock, Load()).Times(2).WillRepeatedly(Return(false));
    diService_->Enable();
    EXPECT_CALL(*g_callback, OnDisable(PROFILE_NAME_DI, true)).Times(1);
    EXPECT_CALL(*g_sdpmock, SDP_DeregisterServiceRecord()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_sdpmock, SDP_DestroyServiceRecord()).Times(1).WillOnce(Return(BT_NO_ERROR));
    diService_->Disable();
}

TEST_F(DIServiceTest, Enable_case002)
{
    EXPECT_CALL(*g_callback, OnEnable(PROFILE_NAME_DI, false)).Times(1);
    EXPECT_CALL(*g_sdpmock, SDP_CreateServiceRecord()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_sdpmock, SDP_AddServiceClassIdList()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_sdpmock, SDP_AddAttribute()).Times(6).WillRepeatedly(Return(1));
    EXPECT_CALL(*g_sdpmock, SDP_RegisterServiceRecord()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_xmlmock, Load()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(6).WillRepeatedly(Return(false));
    diService_->Enable();
    EXPECT_CALL(*g_callback, OnDisable(PROFILE_NAME_DI, true)).Times(1);
    EXPECT_CALL(*g_sdpmock, SDP_DeregisterServiceRecord()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_sdpmock, SDP_DestroyServiceRecord()).Times(1).WillOnce(Return(BT_NO_ERROR));
    diService_->Disable();
}

TEST_F(DIServiceTest, Disable_case001)
{
    EXPECT_CALL(*g_callback, OnEnable(PROFILE_NAME_DI, true)).Times(1);
    EXPECT_CALL(*g_sdpmock, SDP_CreateServiceRecord()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_sdpmock, SDP_AddServiceClassIdList()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_sdpmock, SDP_AddAttribute()).Times(6).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_sdpmock, SDP_RegisterServiceRecord()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_xmlmock, Load()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(6).WillRepeatedly(Return(true));
    diService_->Enable();
    EXPECT_CALL(*g_callback, OnDisable(PROFILE_NAME_DI, false)).Times(1);
    EXPECT_CALL(*g_sdpmock, SDP_DeregisterServiceRecord()).Times(1).WillOnce(Return(1));
    diService_->Disable();
}
TEST_F(DIServiceTest, Disable_case002)
{
    EXPECT_CALL(*g_callback, OnEnable(PROFILE_NAME_DI, true)).Times(1);
    EXPECT_CALL(*g_sdpmock, SDP_CreateServiceRecord()).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*g_sdpmock, SDP_AddServiceClassIdList()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_sdpmock, SDP_AddAttribute()).Times(6).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_sdpmock, SDP_RegisterServiceRecord()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_xmlmock, Load()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*g_xmlmock, GetValue()).Times(6).WillRepeatedly(Return(true));
    diService_->Enable();
    EXPECT_CALL(*g_callback, OnDisable(PROFILE_NAME_DI, false)).Times(1);
    EXPECT_CALL(*g_sdpmock, SDP_DeregisterServiceRecord()).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(*g_sdpmock, SDP_DestroyServiceRecord()).Times(1).WillOnce(Return(1));
    diService_->Disable();
}

TEST_F(DIServiceTest, Connect_Disconnect)
{
    RawAddress device("01:00:00:00:00:00");
    diService_->Connect(device);
    diService_->Disconnect(device);
}

TEST_F(DIServiceTest, GetConnectDevices_GetConnectState_GetMaxConnectNum)
{
    diService_->GetConnectDevices();
    diService_->GetConnectState();
    diService_->GetMaxConnectNum();
}