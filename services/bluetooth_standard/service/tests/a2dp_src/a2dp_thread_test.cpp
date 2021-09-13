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

class A2dp_thread_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("A2dp_thread_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("A2dp_thread_test TearDownTestCase\n");
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
        service = new A2dpSrcService();
        service->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(PROFILE_NAME_A2DP_SRC))
            .Times(AtLeast(0))
            .WillRepeatedly(Return(service));

        config = A2dpCodecConfig::CreateCodec(A2DP_SOURCE_CODEC_INDEX_SBC, A2DP_CODEC_PRIORITY_DEFAULT);
        observer = new A2dpCodecEncoderObserver(0);
        deobserver = new A2dpCodecDecoderObserver();

        stub::A2dpService::GetInstance()->RegisterObserver(&obs);

        A2dpCodecThread::GetInstance()->StartA2dpCodecThread();
    }
    virtual void TearDown()
    {
        A2dpCodecThread::GetInstance()->StopA2dpCodecThread();
        delete config;

        if (service != nullptr) {
            service->GetContext()->Uninitialize();
            delete service;
        }
    }

protected:
    NiceMock<MockAvdtp> mockAvdtp_;
    stub::A2dpServiceObserver obs;
    A2dpEncoderInitPeerParams peerParams = {};
    A2dpCodecConfig *config = nullptr;
    A2dpEncoderObserver *observer = nullptr;
    A2dpDecoderObserver *deobserver = nullptr;
    A2dpSrcService *service = nullptr;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(A2dp_thread_test, PostMessage_1)
{
    utility::Message msg1(A2DP_PCM_ENCODED, A2DP_ROLE_SINK, nullptr);
    A2dpCodecThread::GetInstance()->PostMessage(msg1, peerParams, config, observer, nullptr);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    utility::Message msg2(A2DP_AUDIO_SUSPEND, 0, nullptr);
    A2dpCodecThread::GetInstance()->PostMessage(msg2, peerParams, config, nullptr, nullptr);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dp_thread_test, PostMessage_2)
{
    utility::Message msg1(A2DP_PCM_ENCODED, A2DP_ROLE_SINK, nullptr);
    A2dpCodecThread::GetInstance()->PostMessage(msg1, peerParams, config, observer, nullptr);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    utility::Message msg2(A2DP_AUDIO_RECONFIGURE, A2DP_ROLE_SINK, nullptr);
    A2dpCodecThread::GetInstance()->PostMessage(msg2, peerParams, nullptr, nullptr, nullptr);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dp_thread_test, PostMessage_3)
{
    utility::Message msg1(A2DP_PCM_ENCODED, A2DP_ROLE_SINK, nullptr);
    A2dpCodecThread::GetInstance()->PostMessage(msg1, peerParams, config, observer, nullptr);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    utility::Message msg2(A2DP_PCM_PUSH, 0, nullptr);
    A2dpCodecThread::GetInstance()->PostMessage(msg2, peerParams, nullptr, nullptr, nullptr);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}

TEST_F(A2dp_thread_test, PostMessage_4)
{
    utility::Message msg1(A2DP_FRAME_DECODED, A2DP_ROLE_SINK, nullptr);
    A2dpCodecThread::GetInstance()->PostMessage(msg1, peerParams, config, nullptr, deobserver);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);

    uint16_t packetSize = 10;
    uint8_t *frameData = (uint8_t *)malloc(packetSize);
    utility::Message msg2(A2DP_FRAME_READY, packetSize, frameData);
    A2dpCodecThread::GetInstance()->PostMessage(msg2, peerParams, nullptr, nullptr, nullptr);
    usleep(A2DP_SRC_TEST_TIMER_NORMAL);
}