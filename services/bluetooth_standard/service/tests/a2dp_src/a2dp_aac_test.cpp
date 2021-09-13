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
#include "a2dp_aac_param_ctrl.h"

using namespace testing;
using namespace bluetooth;

class a2dp_aac_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_aac_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_aac_test TearDownTestCase\n");
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

TEST_F(a2dp_aac_test, SetSampleRateUser_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_AAC_SAMPLE_RATE_OCTET1_44100;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_16;
    userConfig.channelMode_ = A2DP_AAC_CHANNEL_MODE_OCTET2_SINGLE;
    A2dpCodecCapability audioConfig{};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInfomation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInfomation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetSampleRateUser_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 244, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_AAC_SAMPLE_RATE_OCTET2_48000;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_32;
    userConfig.channelMode_ = A2DP_AAC_CHANNEL_MODE_OCTET2_DOUBLE;
    A2dpCodecCapability audioConfig{};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 240, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetSampleRateUser_3)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_24;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetSampleRateUser_4)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
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

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetSampleRateCommonCapa_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 16, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = 3;
    userConfig.bitsPerSample = 8;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = 1;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 16, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetSampleRateCommonCapa_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 128, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = 3;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = 32768;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 128, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetChannelModeUser_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
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

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetChannelModeUser_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 244, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_AAC_SAMPLE_RATE_OCTET2_48000;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_32;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = A2DP_AAC_CHANNEL_MODE_OCTET2_DOUBLE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 240, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetChannelModeUser_3)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 1, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_24;
    userConfig.channelMode_ = 1;
    userConfig.codecSpecific1 = 1;
    userConfig.codecSpecific2 = 1;
    userConfig.codecSpecific3 = 1;
    userConfig.codecSpecific4 = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = 1;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetCodecConfig_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {7, 0, 2, 0, 0, 1, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_24;
    userConfig.channelMode_ = 1;
    userConfig.codecSpecific1 = 1;
    userConfig.codecSpecific2 = 1;
    userConfig.codecSpecific3 = 1;
    userConfig.codecSpecific4 = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = 1;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(false, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetCodecConfig_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 1, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_24;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = 1;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetPeerCodecCapabilities_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {7, 0, 2, 0, 0, 1, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_24;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = 1;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    bool ret = tmp->SetPeerCodecCapabilities(peerCodeInformation);
    EXPECT_EQ(false, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetPeerCodecCapabilities_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 1, 8, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_24;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = 1;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    bool ret = tmp->SetPeerCodecCapabilities(peerCodeInformation);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, SetPeerCodecCapabilities_3)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigAacSink(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_24;
    userConfig.channelMode_ = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.channelMode_ = 1;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 0, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    bool ret = tmp->SetPeerCodecCapabilities(peerCodeInformation);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_aac_test, GetBitrateAac_1)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {7, 0, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetBitrateAac(peerCodeInformation);
    EXPECT_EQ(-1, ret);
}

TEST_F(a2dp_aac_test, GetBitrateAac_2)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetBitrateAac(peerCodeInformation);
    EXPECT_EQ(15, ret);
}

TEST_F(a2dp_aac_test, GetSampleRateAac_1)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {7, 0, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetSampleRateAac(peerCodeInformation);
    EXPECT_EQ(-1, ret);
}

TEST_F(a2dp_aac_test, GetSampleRateAac_2)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 0, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetSampleRateAac(peerCodeInformation);
    EXPECT_EQ(-1, ret);
}

TEST_F(a2dp_aac_test, GetTrackChannelModeAac_1)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {7, 0, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetTrackChannelModeAac(peerCodeInformation);
    EXPECT_EQ(-1, ret);
}

TEST_F(a2dp_aac_test, GetTrackChannelModeAac_2)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetTrackChannelModeAac(peerCodeInformation);
    EXPECT_EQ(4, ret);
}

TEST_F(a2dp_aac_test, GetTrackChannelCountAac_1)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {7, 0, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetTrackChannelCountAac(peerCodeInformation);
    EXPECT_EQ(-1, ret);
}

TEST_F(a2dp_aac_test, GetTrackChannelCountAac_2)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetTrackChannelCountAac(peerCodeInformation);
    EXPECT_EQ(2, ret);
}

TEST_F(a2dp_aac_test, ParseInfoAac_1)
{
    int ret = GetTrackChannelCountAac(nullptr);
    EXPECT_EQ(-1, ret);
}

TEST_F(a2dp_aac_test, ParseInfoAac_2)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 16, 2, 0, 255, 244, 0, 0, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    int ret = GetTrackChannelCountAac(peerCodeInformation);
    EXPECT_EQ(-1, ret);
}


TEST_F(a2dp_aac_test, isCodecConfigChangedAac_1)
{
    uint8_t lastCodecInfo[AVDT_CODEC_SIZE] = {7, 0, 2, 0, 255, 4, 0, 0, 15, 15, 15};
    uint8_t newCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 4, 0, 0, 15, 15, 15};
    uint8_t *lastCodecInformation = lastCodecInfo;
    uint8_t *newCodecInformation = newCodecInfo;

    bool ret = IsAacConfigChanged(lastCodecInformation, newCodecInformation);
    EXPECT_EQ(false, ret);
}

TEST_F(a2dp_aac_test, isCodecConfigChangedAac_2)
{
    uint8_t lastCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 4, 0, 0, 15, 15, 15};
    uint8_t newCodecInfo[AVDT_CODEC_SIZE] = {7, 0, 2, 0, 255, 4, 0, 0, 15, 15, 15};
    uint8_t *lastCodecInformation = lastCodecInfo;
    uint8_t *newCodecInformation = newCodecInfo;

    bool ret = IsAacConfigChanged(lastCodecInformation, newCodecInformation);
    EXPECT_EQ(false, ret);
}

TEST_F(a2dp_aac_test, isCodecConfigChangedAac_3)
{
    uint8_t lastCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 4, 0, 0, 15, 15, 15};
    uint8_t newCodecInfo[AVDT_CODEC_SIZE] = {8, 0, 2, 0, 255, 4, 0, 0, 15, 15, 15};
    uint8_t *lastCodecInformation = lastCodecInfo;
    uint8_t *newCodecInformation = newCodecInfo;

    bool ret = IsAacConfigChanged(lastCodecInformation, newCodecInformation);
    EXPECT_EQ(true, ret);
}