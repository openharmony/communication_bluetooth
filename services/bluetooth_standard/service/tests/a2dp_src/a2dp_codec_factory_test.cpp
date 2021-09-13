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

class a2dp_codec_factory_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("a2dp_codec_factory_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("a2dp_codec_factory_test TearDownTestCase\n");
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

TEST_F(a2dp_codec_factory_test, FindSourceCodec)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 3, 15, 15, 15, 14, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecConfig *ret = tmp->FindSourceCodec(peerCodeInformation);
    EXPECT_EQ(nullptr, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_codec_factory_test, FindSinkCodec)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 3, 15, 15, 15, 14, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    A2dpCodecConfig *ret = tmp->FindSinkCodec(peerCodeInformation);
    EXPECT_EQ(nullptr, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_codec_factory_test, SetCodecConfig)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 3, 15, 15, 15, 14, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;

    bool ret = tmp->SetCodecConfig(peerCodeInformation, nullptr);
    EXPECT_EQ(false, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_codec_factory_test, SetSinkCodecConfig_1)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 3, 15, 15, 15, 14, 15, 15, 15, 15};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 3, 15, 15, 15, 14, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    uint8_t *resultCodecInformation = resultCodecInfo;

    bool ret = tmp->SetSinkCodecConfig(peerCodeInformation, resultCodecInformation);
    EXPECT_EQ(false, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_codec_factory_test, SetSinkCodecConfig_2)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {8, 6, 0, 15, 15, 15, 14, 15, 15, 15, 15};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {8, 6, 3, 15, 15, 15, 14, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    uint8_t *resultCodecInformation = resultCodecInfo;

    bool ret = tmp->SetSinkCodecConfig(peerCodeInformation, resultCodecInformation);
    EXPECT_EQ(false, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_codec_factory_test, SetSinkCodecConfig_3)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    uint8_t *resultCodecInformation = resultCodecInfo;

    bool ret = tmp->SetSinkCodecConfig(peerCodeInformation, resultCodecInformation);
    EXPECT_EQ(true, ret);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_codec_factory_test, UpdateCodecPriority)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    uint8_t *resultCodecInformation = resultCodecInfo;

    tmp->SetSinkCodecConfig(peerCodeInformation, resultCodecInformation);

    A2dpCodecCapability userConfig{};
    userConfig.codecIndex_ = A2DP_SINK_CODEC_INDEX_MAX;
    uint8_t peerSinkCapabilities[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t *peerSinkCapabilitiesInfo = peerSinkCapabilities;
    bool restartConfig = false;
    tmp->SetCodecUserConfig(userConfig, peerSinkCapabilitiesInfo, resultCodecInformation, &restartConfig);

    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_codec_factory_test, SetCodecUserConfig)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    uint8_t *resultCodecInformation = resultCodecInfo;

    tmp->SetSinkCodecConfig(peerCodeInformation, resultCodecInformation);

    A2dpCodecCapability userConfig{};
    userConfig.codecIndex_ = (A2dpCodecIndex)0;
    uint8_t peerSinkCapabilities[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t *peerSinkCapabilitiesInfo = peerSinkCapabilities;
    bool restartConfig = false;
    bool ret = tmp->SetCodecUserConfig(userConfig, peerSinkCapabilitiesInfo, resultCodecInformation, &restartConfig);

    EXPECT_EQ(false, ret);
    delete tmp;
    tmp = nullptr;
}

TEST_F(a2dp_codec_factory_test, SetCodecAudioConfig)
{
    std::map<A2dpCodecIndex, A2dpCodecPriority> map;
    A2dpCodecFactory *tmp = new A2dpCodecFactory(map);

    uint8_t peerCodeInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t resultCodecInfo[AVDT_CODEC_SIZE] = {6, 6, 0, 18, 139, 15, 15, 15, 15, 15, 15};
    uint8_t *peerCodeInformation = peerCodeInfo;
    uint8_t *resultCodecInformation = resultCodecInfo;
    A2dpCodecCapability audioConfig{};
    bool restartConfig = false;

    tmp->SetSinkCodecConfig(peerCodeInformation, resultCodecInformation);
    bool ret = tmp->SetCodecAudioConfig(audioConfig, peerCodeInformation, resultCodecInformation, &restartConfig);

    EXPECT_EQ(true, ret);
    delete tmp;
    tmp = nullptr;
}