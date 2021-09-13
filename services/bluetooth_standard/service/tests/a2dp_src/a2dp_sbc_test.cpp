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

class a2dp_sbc_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_sbc_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_sbc_test TearDownTestCase\n");
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

TEST_F(a2dp_sbc_test, SetPeerCodecCapabilities)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 15, 15, 15, 14, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    bool ret = tmp->SetPeerCodecCapabilities(peerCodeInformation);
    EXPECT_EQ(false, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetSampleRateUser_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 32, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SBC_SAMPLE_RATE_44100;
    A2dpCodecCapability audioConfig{};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 32, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetSampleRateUser_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SBC_SAMPLE_RATE_48000;
    A2dpCodecCapability audioConfig{};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetSampleRateCommonCapa)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetSampleRateAudioConfig_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 32, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SBC_SAMPLE_RATE_44100;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 32, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetSampleRateAudioConfig_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SBC_SAMPLE_RATE_48000;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetBitsPerSampleAudioConfig)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SBC_SAMPLE_RATE_48000;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_16;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 15, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetChannelModeUser_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 24, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_SBC_CHANNEL_MODE_MONO;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 24, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetChannelModeUser_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 1, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_SBC_CHANNEL_MODE_JOINT_STEREO;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 1, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetChannelModeAudioConfig_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 23, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    audioConfig.channelMode_ = A2DP_SBC_CHANNEL_MODE_MONO;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 23, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetChannelModeAudioConfig_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 23, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    audioConfig.channelMode_ = A2DP_SBC_CHANNEL_MODE_JOINT_STEREO;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 23, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetChannelModeAudioConfig_3)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    audioConfig.channelMode_ = A2DP_SBC_CHANNEL_MODE_JOINT_STEREO;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 16, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetBlockLength_1)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 34, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    audioConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 34, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetBlockLength_2)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 66, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    userConfig.codecSpecific1 = 1;
    userConfig.codecSpecific2 = 1;
    userConfig.codecSpecific3 = 1;
    userConfig.codecSpecific4 = 1;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    audioConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 66, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInformation = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, SetBlockLength_3)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecCapability userConfig{};
    userConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    userConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    userConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    A2dpCodecCapability audioConfig{};
    audioConfig.sampleRate_ = A2DP_SAMPLE_RATE_NONE;
    audioConfig.bitsPerSample = A2DP_SAMPLE_BITS_NONE;
    audioConfig.channelMode_ = A2DP_CHANNEL_MODE_NONE;
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *resultCodecInforamtion = resultCodecInfo;
    bool restartConfig = false;

    bool ret = tmp->SetCodecUserConfig(
        userConfig, audioConfig, peerCodeInformation, resultCodecInforamtion, &restartConfig);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_sbc_test, GetMinBitpoolSbc)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    int ret = GetMinBitpoolSbc(peerCodeInformation);
    EXPECT_EQ(15, ret);
}

TEST_F(a2dp_sbc_test, GetMaxBitpoolSbc)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    int ret = GetMaxBitpoolSbc(peerCodeInformation);
    EXPECT_EQ(15, ret);
}

TEST_F(a2dp_sbc_test, GetSampleRateSbc)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    int ret = GetSampleRateSbc(peerCodeInformation);
    EXPECT_EQ(16, ret);
}

TEST_F(a2dp_sbc_test, GetTrackChannelModeSbc)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    int ret = GetTrackChannelModeSbc(peerCodeInformation);
    EXPECT_EQ(2, ret);
}

TEST_F(a2dp_sbc_test, GetTrackChannelCountSbc)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 130, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    int ret = GetTrackChannelCountSbc(peerCodeInformation);
    EXPECT_EQ(2, ret);
}

TEST_F(a2dp_sbc_test, GetNumberOfBlocksSbc_1)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 16, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    int ret = GetNumberOfBlocksSbc(peerCodeInformation);
    EXPECT_EQ(16, ret);
}

TEST_F(a2dp_sbc_test, isCodecConfigChangedSbc_1)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    bool ret = IsSbcConfigChanged(peerCodeInformation, nullptr);
    EXPECT_EQ(false, ret);
}

TEST_F(a2dp_sbc_test, isCodecConfigChangedSbc_2)
{
    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 8, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    bool ret = IsSbcConfigChanged(peerCodeInformation, peerCodeInformation);
    EXPECT_EQ(true, ret);
}