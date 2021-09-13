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
#include "a2dp_codec_config.h"
#include "a2dp_encoder_aac.h"

using namespace testing;
using namespace bluetooth;

class A2dpCodecEncoderObserverAacTest : public A2dpEncoderObserver {
public:
    A2dpCodecEncoderObserverAacTest(uint16_t streamHandle)
    {
        flag_ = streamHandle;
    };
    ~A2dpCodecEncoderObserverAacTest()
    {}

    uint32_t Read(uint8_t **buf, uint32_t size) override
    {
        *buf = &buffer_;
        if (flag_ == 0)
            return 0;
        return 1;
    }

    // pktTimeStamp will be added in packet's head.
    bool EnqueuePacket(const Packet *packet, size_t frames, uint32_t bytes, uint32_t pktTimeStamp) const override
    {
        return 0;
    }

private:
    uint8_t buffer_ = 2;
    int flag_ = 0;
};

class a2dp_encoder_aac_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_encoder_aac_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_encoder_aac_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
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

TEST_F(a2dp_encoder_aac_test, ResetFeedingState)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = nullptr;
    A2dpAacEncoder *encoder = new A2dpAacEncoder(peerParams, tmp, observer);

    encoder->ResetFeedingState();

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_aac_test, UpdateEncoderParam_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = nullptr;
    A2dpAacEncoder *encoder = new A2dpAacEncoder(peerParams, tmp, observer);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 11, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_AAC_SAMPLE_RATE_OCTET1_44100;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_16;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = A2DP_AAC_CHANNEL_MODE_OCTET2_SINGLE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    encoder->UpdateEncoderParam();

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_aac_test, UpdateEncoderParam_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = nullptr;
    A2dpAacEncoder *encoder = new A2dpAacEncoder(peerParams, tmp, observer);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 11, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_AAC_SAMPLE_RATE_OCTET2_48000;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_16;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = A2DP_AAC_CHANNEL_MODE_OCTET2_SINGLE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    encoder->UpdateEncoderParam();

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_aac_test, UpdateEncoderParam_3)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = nullptr;
    A2dpAacEncoder *encoder = new A2dpAacEncoder(peerParams, tmp, observer);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 11, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = 3;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_AAC_CHANNEL_MODE_OCTET2_DOUBLE;
    A2dpCodecCapability audioConfig{};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    encoder->UpdateEncoderParam();

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_aac_test, SendFrames_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = new A2dpCodecEncoderObserverAacTest(1);
    A2dpAacEncoder *encoder = new A2dpAacEncoder(peerParams, tmp, observer);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 11, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_AAC_SAMPLE_RATE_OCTET1_44100;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_16;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = A2DP_AAC_CHANNEL_MODE_OCTET2_SINGLE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;
    uint64_t timeStampUs = 1000;

    tmp->SetCodecUserConfig(userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    encoder->UpdateEncoderParam();
    encoder->SendFrames(timeStampUs);
    encoder->SendFrames(timeStampUs);

    delete encoder;
    encoder = nullptr;
    delete observer;
    observer = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_aac_test, SendFrames_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = new A2dpCodecEncoderObserverAacTest(0);
    A2dpAacEncoder *encoder = new A2dpAacEncoder(peerParams, tmp, observer);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 11, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_AAC_SAMPLE_RATE_OCTET1_44100;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_16;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = A2DP_AAC_CHANNEL_MODE_OCTET2_SINGLE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;
    uint64_t timeStampUs = 1000;

    tmp->SetCodecUserConfig(userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    encoder->UpdateEncoderParam();
    encoder->SendFrames(timeStampUs);
    encoder->SendFrames(timeStampUs);

    delete encoder;
    encoder = nullptr;
    delete observer;
    observer = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}
