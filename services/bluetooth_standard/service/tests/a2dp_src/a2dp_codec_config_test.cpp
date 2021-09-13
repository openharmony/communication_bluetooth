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
#include "a2dp_decoder_sbc.h"
#include "a2dp_decoder_aac.h"
#include "a2dp_sbc_param_ctrl.h"

using namespace testing;
using namespace bluetooth;

class A2dpDecoderObserverTest : public A2dpDecoderObserver {
public:
    A2dpDecoderObserverTest(uint16_t streamHandle){};
    ~A2dpDecoderObserverTest(){};
    void DataAvailable(uint8_t *buf, uint32_t size) override{};
};

class A2dp_codec_config_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("A2dp_codec_config_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("A2dp_codec_config_test TearDownTestCase\n");
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

TEST_F(A2dp_codec_config_test, GetAudioBitsPerSample)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    uint32_t ret = tmp->GetAudioBitsPerSample();
    EXPECT_EQ(A2DP_SAMPLE_BITS_16, ret);
    delete tmp;
    tmp = nullptr;
}

TEST_F(A2dp_codec_config_test, GetUserConfig)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpCodecCapability ret = tmp->GetUserConfig();
    EXPECT_EQ((uint32_t)0, ret.bitsPerSample);
    EXPECT_EQ(0, (int)ret.channelMode_);
    EXPECT_EQ(1, (int)ret.codecIndex_);
    EXPECT_EQ(2001, (int)ret.codecPriority_);
    EXPECT_EQ(0, (int)ret.codecSpecific1);
    EXPECT_EQ(0, (int)ret.codecSpecific2);
    EXPECT_EQ(0, (int)ret.codecSpecific3);
    EXPECT_EQ(0, (int)ret.codecSpecific4);
    EXPECT_EQ(0, (int)ret.defaultPriority_);
    EXPECT_EQ(0, (int)ret.sampleRate_);
    delete tmp;
    tmp = nullptr;
}

TEST_F(A2dp_codec_config_test, isCodecConfigChanged_1)
{
    uint8_t lastCodecInfo[AVDT_CODEC_SIZE] = {0, 0, 0, 0};
    uint8_t newCodecInfo[AVDT_CODEC_SIZE] = {1, 1, 1, 1};
    uint8_t *newCodecInformation = newCodecInfo;
    bool ret = IsConfigChanged(lastCodecInfo, newCodecInformation);
    EXPECT_EQ(false, ret);
}

TEST_F(A2dp_codec_config_test, isCodecConfigChanged_2)
{
    uint8_t lastCodecInfo[AVDT_CODEC_SIZE] = {2, 2, 2, 2};
    uint8_t newCodecInfo[AVDT_CODEC_SIZE] = {2, 2, 2, 2};
    uint8_t *newCodecInformation = newCodecInfo;
    bool ret = IsConfigChanged(lastCodecInfo, newCodecInformation);
    EXPECT_EQ(false, ret);
}

TEST_F(A2dp_codec_config_test, isCodecConfigChanged_3)
{
    uint8_t lastCodecInfo[AVDT_CODEC_SIZE] = {3, 3, 3, 3};
    uint8_t newCodecInfo[AVDT_CODEC_SIZE] = {3, 3, 3, 3};
    uint8_t *newCodecInformation = newCodecInfo;
    bool ret = IsConfigChanged(lastCodecInfo, newCodecInformation);
    EXPECT_EQ(false, ret);
}

TEST_F(A2dp_codec_config_test, SetCodecUserConfig)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpCodecCapability userConfig{};
    A2dpCodecCapability audioConfig{};
    uint8_t peer = 0;
    uint8_t result = 1;
    bool restartConfig = false;
    bool ret = tmp->SetCodecUserConfig(userConfig, audioConfig, &peer, &result, &restartConfig);
    EXPECT_EQ(false, ret);
    delete tmp;
    tmp = nullptr;
}

TEST_F(A2dp_codec_config_test, CreateCodec)
{
    A2dpCodecConfig *tmp = new A2dpCodecConfigSbcSource(A2DP_CODEC_PRIORITY_DEFAULT);
    A2dpCodecIndex codecIndex = (A2dpCodecIndex)0;
    A2dpCodecPriority codecPriority = (A2dpCodecPriority)0;
    A2dpCodecConfig *ret = tmp->CreateCodec(codecIndex, codecPriority);
    EXPECT_EQ(nullptr, ret);
    delete tmp;
    tmp = nullptr;
}

TEST_F(A2dp_codec_config_test, GetSourceCodecIndex_1)
{
    uint8_t codecInfo[AVDT_CODEC_SIZE] = {2, 2, 2};
    uint8_t *codecInformation = codecInfo;
    A2dpCodecIndex ret = GetSourceCodecIndex(codecInformation);
    EXPECT_EQ(A2DP_SOURCE_CODEC_INDEX_AAC, ret);
}

TEST_F(A2dp_codec_config_test, GetSourceCodecIndex_2)
{
    uint8_t codecInfo[AVDT_CODEC_SIZE] = {5, 5, 5};
    uint8_t *codecInformation = codecInfo;
    A2dpCodecIndex ret = GetSourceCodecIndex(codecInformation);
    EXPECT_EQ(A2DP_SOURCE_CODEC_INDEX_MAX, ret);
}

TEST_F(A2dp_codec_config_test, GetSinkCodecIndex_1)
{
    uint8_t codecInfo[AVDT_CODEC_SIZE] = {2, 2, 2};
    uint8_t *codecInformation = codecInfo;
    A2dpCodecIndex ret = GetSinkCodecIndex(codecInformation);
    EXPECT_EQ(A2DP_SINK_CODEC_INDEX_AAC, ret);
}

TEST_F(A2dp_codec_config_test, GetSinkCodecIndex_2)
{
    uint8_t codecInfo[AVDT_CODEC_SIZE] = {5, 5, 5};
    uint8_t *codecInformation = codecInfo;
    A2dpCodecIndex ret = GetSinkCodecIndex(codecInformation);
    EXPECT_EQ(A2DP_SINK_CODEC_INDEX_MAX, ret);
}

TEST_F(A2dp_codec_config_test, IsPeerSinkCodecValid_1)
{
    uint8_t codecInfo[AVDT_CODEC_SIZE] = {8, 8, 2};
    uint8_t *codecInformation = codecInfo;
    bool ret = IsPeerSinkCodecValid(codecInformation);
    EXPECT_EQ(true, ret);
}

TEST_F(A2dp_codec_config_test, IsPeerSinkCodecValid_2)
{
    uint8_t codecInfo[AVDT_CODEC_SIZE] = {3, 3, 3};
    uint8_t *codecInformation = codecInfo;
    bool ret = IsPeerSinkCodecValid(codecInformation);
    EXPECT_EQ(false, ret);
}

TEST_F(A2dp_codec_config_test, IsPeerSourceCodecValid_1)
{
    uint8_t codecInfo[AVDT_CODEC_SIZE] = {8, 8, 2};
    uint8_t *codecInformation = codecInfo;
    bool ret = IsPeerSourceCodecValid(codecInformation);
    EXPECT_EQ(true, ret);
}

TEST_F(A2dp_codec_config_test, IsPeerSourceCodecValid_2)
{
    uint8_t codecInfo[AVDT_CODEC_SIZE] = {3, 3, 3};
    uint8_t *codecInformation = codecInfo;
    bool ret = IsPeerSourceCodecValid(codecInformation);
    EXPECT_EQ(false, ret);
}

TEST_F(A2dp_codec_config_test, A2dpSbcDecoder)
{
    A2dpDecoderObserver *observer = nullptr;
    A2dpSbcDecoder *decoder = new A2dpSbcDecoder(observer);
    uint8_t data[3] = {0, 0, 0};
    uint8_t *dataInfo = data;
    uint16_t size = 0;

    bool ret = decoder->DecodePacket(dataInfo, size);
    EXPECT_EQ(true, ret);
    delete decoder;
    decoder = nullptr;
}

TEST_F(A2dp_codec_config_test, A2dpAacDecoder)
{
    A2dpDecoderObserver *observer = nullptr;
    A2dpAacDecoder *decoder = new A2dpAacDecoder(observer);
    uint8_t data[3] = {0, 0, 0};
    uint8_t *dataInfo = data;
    uint16_t size = 1;

    bool ret = decoder->DecodePacket(dataInfo, size);
    EXPECT_EQ(true, ret);

    delete decoder;
    decoder = nullptr;
}