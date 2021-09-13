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
#include "a2dp_encoder_sbc.h"
#include "a2dp_sbc_param_ctrl.h"

using namespace testing;
using namespace bluetooth;

class A2dpCodecEncoderObserverSbcTest : public A2dpEncoderObserver {
public:
    A2dpCodecEncoderObserverSbcTest(uint16_t streamHandle){};
    ~A2dpCodecEncoderObserverSbcTest()
    {}
    uint32_t Read(uint8_t **buf, uint32_t size) override
    {
        *buf = &buffer_;
        return 512;
    }
    // pktTimeStamp will be added in packet's head.
    bool EnqueuePacket(const Packet *packet, size_t frames, uint32_t bytes, uint32_t pktTimeStamp) const override
    {
        return 0;
    }

private:
    uint8_t buffer_ = 2;
};

class a2dp_encoder_sbc_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_encoder_sbc_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_encoder_sbc_test TearDownTestCase\n");
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

TEST_F(a2dp_encoder_sbc_test, UpdateEncoderParam_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = nullptr;
    A2dpSbcEncoder *encoder = new A2dpSbcEncoder(peerParams, tmp, observer);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 24, 134, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = 16;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_SBC_CHANNEL_MODE_MONO;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = 16;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    encoder->UpdateEncoderParam();

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_sbc_test, UpdateEncoderParam_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = nullptr;
    A2dpSbcEncoder *encoder = new A2dpSbcEncoder(peerParams, tmp, observer);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 34, 138, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = 32;
    userConfig.channelMode_ = A2DP_SBC_CHANNEL_MODE_STEREO;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = 32;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 34, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    encoder->UpdateEncoderParam();

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_sbc_test, A2dpSbcGetNumberOfBlocks_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = nullptr;
    A2dpSbcEncoder *encoder = new A2dpSbcEncoder(peerParams, tmp, observer);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 34, 18, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = 32;
    userConfig.channelMode_ = A2DP_SBC_CHANNEL_MODE_STEREO;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = 32;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 34, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    encoder->UpdateEncoderParam();

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_sbc_test, SendFrames_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = true;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 895;
    A2dpEncoderObserver *observer = new A2dpCodecEncoderObserverSbcTest(0);
    A2dpSbcEncoder *encoder = new A2dpSbcEncoder(peerParams, tmp, observer);

    uint64_t timeStampMs = 1;
    encoder->SendFrames(timeStampMs);
    encoder->SendFrames(timeStampMs);

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_sbc_test, SendFrames_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = false;
    peerParams->peerSupports3mbps = false;
    peerParams->peermtu = 0;
    A2dpEncoderObserver *observer = new A2dpCodecEncoderObserverSbcTest(0);
    A2dpSbcEncoder *encoder = new A2dpSbcEncoder(peerParams, tmp, observer);

    uint64_t timeStampMs = 1;
    encoder->SendFrames(timeStampMs);

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_encoder_sbc_test, SendFrames_3)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpEncoderInitPeerParams *peerParams = new A2dpEncoderInitPeerParams();
    peerParams->isPeerEdr = true;
    peerParams->peerSupports3mbps = true;
    peerParams->peermtu = 998;
    A2dpEncoderObserver *observer = new A2dpCodecEncoderObserverSbcTest(0);
    A2dpSbcEncoder *encoder = new A2dpSbcEncoder(peerParams, tmp, observer);

    uint64_t timeStampMs = 1;
    encoder->SendFrames(timeStampMs);

    delete encoder;
    encoder = nullptr;
    delete peerParams;
    peerParams = nullptr;
    delete tmp;
    tmp = nullptr;
}