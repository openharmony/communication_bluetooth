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

#include "avrcp_ct_mock.h"

using namespace testing;
using namespace bluetooth;

class avrcp_ct_sdp_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("avrcp_tg_sdp_test SetUpTestCase\n");

    }
    static void TearDownTestCase()
    {
        printf("avrcp_tg_sdp_test TearDownTestCase\n");

    }
    virtual void SetUp()
    {
        registerSdpMocker(&mockSdp_);
    }
    virtual void TearDown()
    {}

    StrictMock<MockSdp> mockSdp_;
};
void func(const BtAddr *btAddr, const uint32_t *handles, uint16_t handleArray, void *context)
{}

TEST_F(avrcp_ct_sdp_test, DeAvrcCtSdpManager_001)
{
    AvrcCtSdpManager sdpManager(0xFFFF);
    sdpManager.~AvrcCtSdpManager();
}

TEST_F(avrcp_ct_sdp_test, RegisterService_001)
{
    AvrcCtSdpManager Sdpmanager(0x0002);

    EXPECT_CALL(GetSdpMocker(), SDP_CreateServiceRecord).WillRepeatedly(Return(0x00));
    EXPECT_CALL(GetSdpMocker(), SDP_AddServiceClassIdList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddAdditionalProtocolDescriptorList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddBluetoothProfileDescriptorList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddAttribute).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddServiceName).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_AddBrowseGroupList).WillRepeatedly(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_RegisterServiceRecord).WillRepeatedly(Return(BT_NO_ERROR));

    EXPECT_EQ(Sdpmanager.RegisterService(), BT_NO_ERROR);

    EXPECT_CALL(GetSdpMocker(), SDP_DestroyServiceRecord).Times(1).WillOnce(Return(BT_NO_ERROR));
    EXPECT_CALL(GetSdpMocker(), SDP_DeregisterServiceRecord).Times(1).WillOnce(Return(BT_NO_ERROR));

    EXPECT_EQ(Sdpmanager.UnregisterService(), BT_NO_ERROR);
}

TEST_F(avrcp_ct_sdp_test, FindCtService_001)
{
    EXPECT_CALL(GetSdpMocker(), SDP_ServiceSearch).Times(1).WillOnce(Return(BT_NO_ERROR));
    AvrcCtSdpManager Sdpmanager(0x0000);
    RawAddress bda("00:00:00:00:00:01");

    EXPECT_EQ(Sdpmanager.FindTgService(bda, func), BT_NO_ERROR);
}