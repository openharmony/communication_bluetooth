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

class A2dpAvdtp_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("A2dpAvdtp_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("A2dpAvdtp_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerAvdtpMock(&mockAvdtp_);
        EXPECT_CALL(GetAvdtpMocker(), AVDT_CreateStream).WillRepeatedly(Return(0));
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

TEST_F(A2dpAvdtp_test, GetHandle)
{
    A2dpStream *stream = new A2dpStream();
    uint16_t hdl = 0;

    stream->SetHandle(hdl);

    EXPECT_EQ(0, stream->GetHandle());

    delete stream;
    stream = nullptr;
    EXPECT_EQ(0, 0);
}

TEST_F(A2dpAvdtp_test, RegisterAvdtp)
{
    AvdtRegisterParam rcb = {0};
    rcb.role = A2DP_ROLE_SOURCE;

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);
    uint16_t ret = avdtp.RegisterAvdtp(rcb);
    EXPECT_EQ(0, ret);
}

TEST_F(A2dpAvdtp_test, ConnectReq_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    EXPECT_CALL(GetAvdtpMocker(), AVDT_ConnectReq).WillOnce(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    uint16_t ret = avdtp.ConnectReq(addr);
    EXPECT_EQ(0, ret);

    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ConnectReq_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_ConnectReq).WillOnce(Return(1));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    uint16_t ret = avdtp.ConnectReq(addr);
    EXPECT_EQ(1, ret);
}

TEST_F(A2dpAvdtp_test, DisconnectReq_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);
    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.DisconnectReq(addr);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, DisconnectReq_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DisconnectReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);
    A2dpSnkProfile::GetInstance()->Enable();

    uint16_t ret = avdtp.DisconnectReq(addr);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, DiscoverReq_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t maxNum = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DiscoverReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.DiscoverReq(addr, maxNum, label);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, DiscoverReq_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t maxNum = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DiscoverReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.DiscoverReq(addr, maxNum, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, GetCapabilityReq_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t seid = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetCapReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.GetCapabilityReq(addr, seid, label);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, GetCapabilityReq_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t seid = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetCapReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.GetCapabilityReq(addr, seid, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, GetAllCapabilityReq_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t seid = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetAllCapReq).WillOnce(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();

    A2dpSrcProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    uint16_t ret = avdtp.GetAllCapabilityReq(addr, seid, label);
    EXPECT_EQ(0, ret);
    sleep(A2DP_TEST_SIGNALLING_TIMEOUT);

    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, SetConfigureReq_1)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t seid = 0;
    AvdtSepConfig cfg;
    uint8_t label = 0;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SetConfigReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSnkProfile::GetInstance()->Enable();
    A2dpSnkProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    uint16_t ret = avdtp.SetConfigureReq(addr, handle, seid, cfg, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->DeletePeer(addr);
    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, SetConfigureReq_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t seid = 0;
    AvdtSepConfig cfg;
    uint8_t label = 0;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SetConfigReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);
    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.SetConfigureReq(addr, handle, seid, cfg, label);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, SetConfigureRsp_1)
{
    AvdtCatetory category;
    category.errCode = 0;
    category.category = 0;
    uint8_t label = 0;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SetConfigRsp).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);
    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.SetConfigureRsp(handle, label, category);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, SetConfigureRsp_2)
{
    AvdtCatetory category;
    category.errCode = 0;
    category.category = 0;
    uint8_t label = 0;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SetConfigRsp).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.SetConfigureRsp(handle, label, category);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, OpenReq_1)
{
    uint8_t label = 0;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_OpenReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.OpenReq(handle, label);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, OpenReq_2)
{
    uint8_t label = 0;
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_OpenReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    A2dpSrcProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    uint16_t ret = avdtp.OpenReq(handle, label);
    EXPECT_EQ(0, ret);

    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, OpenReq_3)
{
    uint8_t label = 0;
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_OpenReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.OpenReq(handle, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, CloseReq_1)
{
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.CloseReq(handle);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, CloseReq_2)
{
    uint16_t handle = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.CloseReq(handle);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, CloseRsp_1)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    uint8_t errCode = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseRsp).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();

    uint16_t ret = avdtp.CloseRsp(handle, label, errCode);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, CloseRsp_2)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    uint8_t errCode = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseRsp).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);
    A2dpSnkProfile::GetInstance()->Enable();
    A2dpSnkProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    uint16_t ret = avdtp.CloseRsp(handle, label, errCode);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->DeletePeer(addr);
    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, CloseRsp_3)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    uint8_t errCode = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_CloseRsp).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.CloseRsp(handle, label, errCode);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, AbortReq)
{
    uint16_t handle = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_AbortReq).WillOnce(Return(1));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    uint16_t ret = avdtp.AbortReq(handle, label);
    EXPECT_EQ(1, ret);
}

TEST_F(A2dpAvdtp_test, SuspendReq_1)
{
    uint16_t handle = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SuspendReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);
    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.SuspendReq(handle, label);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, SuspendReq_2)
{
    uint16_t handle = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SuspendReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);
    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.SuspendReq(handle, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, SuspendReq_3)
{
    uint16_t handle = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SuspendReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.SuspendReq(handle, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, StartReq_1)
{
    uint16_t handle = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_StartReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.StartReq(handle, label);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, StartReq_2)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_StartReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSnkProfile::GetInstance()->Enable();
    A2dpSnkProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    uint16_t ret = avdtp.StartReq(handle, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->DeletePeer(addr);
    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, StartReq_3)
{
    uint16_t handle = 0;
    uint8_t label = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_StartReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.StartReq(handle, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, SuspendRsp)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    uint16_t errHandle = 0;
    uint8_t errCode = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SuspendRsp).WillOnce(Return(1));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    uint16_t ret = avdtp.SuspendRsp(handle, label, errHandle, errCode);
    EXPECT_EQ(1, ret);
}

TEST_F(A2dpAvdtp_test, StartRsp)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    uint16_t errHandle = 0;
    uint8_t errCode = 0;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_StartRsp).WillOnce(Return(1));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    uint16_t ret = avdtp.StartRsp(handle, label, errHandle, errCode);
    EXPECT_EQ(1, ret);
}

TEST_F(A2dpAvdtp_test, ReconfigReq_1)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    AvdtSepConfig cfg = {0};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigReq).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.ReconfigureReq(handle, cfg, label);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, ReconfigReq_2)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    AvdtSepConfig cfg = {0};

    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSnkProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.ReconfigureReq(handle, cfg, label);
    EXPECT_EQ(0, ret);

    A2dpSnkProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, ReconfigureRsp_1)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    AvdtCatetory category;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigRsp).WillOnce(Return(1));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.ReconfigureRsp(handle, label, category);
    EXPECT_EQ(1, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, ReconfigureRsp_2)
{
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint16_t handle = 0;
    uint8_t label = 0;
    AvdtCatetory category;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigRsp).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SOURCE);

    A2dpSrcProfile::GetInstance()->Enable();
    A2dpSrcProfile::GetInstance()->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    uint16_t ret = avdtp.ReconfigureRsp(handle, label, category);
    EXPECT_EQ(0, ret);

    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, ReconfigureRsp_3)
{
    uint16_t handle = 0;
    uint8_t label = 0;
    AvdtCatetory category;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_ReconfigRsp).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_RegisterLocalSEP).WillRepeatedly(Return(0));

    A2dpAvdtp avdtp(A2DP_ROLE_SINK);

    A2dpSrcProfile::GetInstance()->Enable();
    uint16_t ret = avdtp.ReconfigureRsp(handle, label, category);
    EXPECT_EQ(0, ret);

    A2dpSrcProfile::GetInstance()->Disable();
}

TEST_F(A2dpAvdtp_test, ProcAvdtpEvent_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_DISCONNECT_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SINK;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpConfigureInd_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CONFIG_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DiscoverReq).WillOnce(Return(0));
    EXPECT_CALL(GetAvdtpMocker(), AVDT_SetConfigRsp).WillOnce(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpConfigureInd_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CONFIG_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SINK;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_SetConfigRsp).WillOnce(Return(0));

    A2dpSnkProfile *profile = A2dpSnkProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpOpenInd_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_OPEN_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpOpenInd_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_OPEN_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SINK;

    A2dpSnkProfile *profile = A2dpSnkProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpOpenInd_3)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_OPEN_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SINK;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpStartInd)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_START_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpSuspendInd)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_SUSPEND_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpReconfigureInd_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_RECONFIG_IND_EVT;
    AvdtCtrlData data = {};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpReconfigureInd_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_RECONFIG_IND_EVT;
    AvdtCtrlData data = {};
    uint8_t role = A2DP_ROLE_SINK;

    A2dpSnkProfile *profile = A2dpSnkProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpReconfigureInd_3)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_RECONFIG_IND_EVT;
    AvdtCtrlData data {};
    uint8_t role = A2DP_ROLE_SINK;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpCloseInd_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CLOSE_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SINK;

    A2dpSnkProfile *profile = A2dpSnkProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    data.closeInd.label = 0;
    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpCloseInd_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CLOSE_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    data.closeInd.label = 0;
    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpCloseChannelInd_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CLOSE_TRANS_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpCloseChannelInd_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CLOSE_TRANS_IND_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpConnectCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CONNECT_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_DiscoverReq).WillOnce(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpDisconnectCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_DISCONNECT_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SINK;

    A2dpSnkProfile *profile = A2dpSnkProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpDiscoverCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_DISCOVER_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetCapReq).WillOnce(Return(0));

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpDiscoverCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_DISCOVER_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetCapReq).WillRepeatedly(Return(0));

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpGetCapabilityCFM_3)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_GETCAP_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetCapReq).WillRepeatedly(Return(0));

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpGetALLCapCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_GET_ALLCAP_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    EXPECT_CALL(GetAvdtpMocker(), AVDT_GetAllCapReq).WillRepeatedly(Return(0));

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpConfigureCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CONFIG_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);
    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    A2dpSrcProfile::GetInstance()->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpConfigureCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CONFIG_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpGetConfigureCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_GETCONFIG_CFM_EVT;
    AvdtCtrlData data {};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpGetConfigureCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_GETCONFIG_CFM_EVT;
    AvdtCtrlData data {};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpReconfigureCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_RECONFIG_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);
    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpReconfigureCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_RECONFIG_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpOpenCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_OPEN_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);
    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpOpenCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_OPEN_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpStartCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_START_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpStartCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_START_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpSuspendCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_SUSPEND_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);
    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpSuspendCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_SUSPEND_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpStreamCloseCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CLOSE_CFM_EVT;
    AvdtCtrlData data {};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SINK);

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpStreamCloseCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_CLOSE_CFM_EVT;
    AvdtCtrlData data {};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpAbortCFM_1)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_ABORT_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpSrcProfile *profile = A2dpSrcProfile::GetInstance();
    profile->FindOrCreatePeer(addr, A2DP_ROLE_SOURCE);
    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
    profile->DeletePeer(addr);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpAbortCFM_2)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = AVDT_ABORT_CFM_EVT;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SOURCE;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dpAvdtp_test, ParseAvdtpCallbackContent)
{
    uint16_t handle = 0;
    BtAddr addr = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, 0x00};
    uint8_t event = 31;
    AvdtCtrlData data = {0};
    uint8_t role = A2DP_ROLE_SINK;

    A2dpAvdtp::ProcAvdtpEvent(handle, &addr, event, &data, role);
}