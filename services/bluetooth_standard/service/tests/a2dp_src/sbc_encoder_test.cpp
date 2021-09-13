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
#include "a2dp_sbc_dynamic_lib_ctrl.h"
#include "sbc_constant.h"
#include "sbc_encoder.h"

using namespace testing;
using namespace bluetooth;
using namespace sbc;

class sbc_encoder_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("sbc_encoder_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("sbc_encoder_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerNewMockProfileServiceManager(&psminstanceMock_);
        EXPECT_CALL(psminstanceMock_, GetInstance()).Times(AtLeast(0)).WillRepeatedly(Return(&psmMock_));
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

TEST_F(sbc_encoder_test, SBCEncode_1)
{
    A2dpSBCDynamicLibCtrl codecWrapper = A2dpSBCDynamicLibCtrl(true);
    CODECSbcLib *codecSbcEncoderLib_ = codecWrapper.LoadCodecSbcLib();
    sbc::IEncoderBase *encoder = codecSbcEncoderLib_->sbcEncoder.createSbcEncode();
    CodecParam codecParam;
    codecParam.blocks = SBC_BLOCK16;
    codecParam.subbands = SBC_SUBBAND4;
    codecParam.channelMode = SBC_CHANNEL_MODE_DUAL_CHANNEL;
    codecParam.allocation = SBC_ALLOCATION_SNR;
    codecParam.bitpool = 4;
    codecParam.endian = SBC_ENDIANESS_LE;
    codecParam.frequency = SBC_FREQ_16000;

    uint8_t inData[] = {64};
    uint8_t outData[] = {32};
    uint8_t *in = inData;
    uint8_t *out = outData;
    size_t iLength = 8;
    size_t oLength = 16;
    size_t *written = nullptr;

    ssize_t result = encoder->SBCEncode(codecParam, in, iLength, out, oLength, written);
    EXPECT_EQ(result, SBC_ERROR_INVALID_FRAME);
    codecSbcEncoderLib_->sbcEncoder.destroySbcEncode(encoder);
    codecWrapper.UnloadCodecSbcLib(codecSbcEncoderLib_);
}

TEST_F(sbc_encoder_test, SBCEncode_2)
{
    A2dpSBCDynamicLibCtrl codecWrapper = A2dpSBCDynamicLibCtrl(true);
    CODECSbcLib *codecSbcEncoderLib_ = codecWrapper.LoadCodecSbcLib();
    sbc::IEncoderBase *encoder = codecSbcEncoderLib_->sbcEncoder.createSbcEncode();
    CodecParam codecParam;
    codecParam.blocks = SBC_BLOCK16;
    codecParam.subbands = SBC_SUBBAND4;
    codecParam.channelMode = SBC_CHANNEL_MODE_JOINT_STEREO;
    codecParam.allocation = SBC_ALLOCATION_SNR;
    codecParam.bitpool = 3;
    codecParam.endian = SBC_ENDIANESS_LE;
    codecParam.frequency = SBC_FREQ_16000;

    uint8_t inData[] = {64};
    uint8_t outData[] = {32};
    uint8_t *in = inData;
    uint8_t *out = outData;
    size_t iLength = 8;
    size_t oLength = 16;
    size_t *written = nullptr;

    ssize_t result = encoder->SBCEncode(codecParam, in, iLength, out, oLength, written);
    EXPECT_EQ(result, 256);
    codecSbcEncoderLib_->sbcEncoder.destroySbcEncode(encoder);
    codecWrapper.UnloadCodecSbcLib(codecSbcEncoderLib_);
}

TEST_F(sbc_encoder_test, SBCEncode_3)
{
    A2dpSBCDynamicLibCtrl codecWrapper = A2dpSBCDynamicLibCtrl(true);
    CODECSbcLib *codecSbcEncoderLib_ = codecWrapper.LoadCodecSbcLib();
    sbc::IEncoderBase *encoder = codecSbcEncoderLib_->sbcEncoder.createSbcEncode();
    CodecParam codecParam;
    codecParam.blocks = SBC_BLOCK16;
    codecParam.subbands = SBC_SUBBAND8;
    codecParam.channelMode = SBC_CHANNEL_MODE_MONO;
    codecParam.allocation = SBC_ALLOCATION_SNR;
    codecParam.bitpool = 4;
    codecParam.endian = SBC_ENDIANESS_LE;
    codecParam.frequency = SBC_FREQ_16000;

    uint8_t inData[] = {64};
    uint8_t outData[] = {32};
    uint8_t *in = inData;
    uint8_t *out = outData;
    size_t iLength = 8;
    size_t oLength = 16;
    size_t *written = nullptr;

    ssize_t result = encoder->SBCEncode(codecParam, in, iLength, out, oLength, written);
    EXPECT_EQ(result, 256);
    codecSbcEncoderLib_->sbcEncoder.destroySbcEncode(encoder);
    codecWrapper.UnloadCodecSbcLib(codecSbcEncoderLib_);
}

TEST_F(sbc_encoder_test, SBCEncode_4)
{
    A2dpSBCDynamicLibCtrl codecWrapper = A2dpSBCDynamicLibCtrl(true);
    CODECSbcLib *codecSbcEncoderLib_ = codecWrapper.LoadCodecSbcLib();
    sbc::IEncoderBase *encoder = codecSbcEncoderLib_->sbcEncoder.createSbcEncode();
    CodecParam codecParam;
    codecParam.blocks = SBC_BLOCK4;
    codecParam.subbands = SBC_SUBBAND8;
    codecParam.channelMode = SBC_CHANNEL_MODE_DUAL_CHANNEL;
    codecParam.allocation = SBC_ALLOCATION_SNR;
    codecParam.bitpool = 4;
    codecParam.endian = SBC_ENDIANESS_LE;
    codecParam.frequency = SBC_FREQ_16000;

    uint8_t inData[] = {64};
    uint8_t outData[] = {32};
    uint8_t *in = inData;
    uint8_t *out = outData;
    size_t iLength = 8;
    size_t oLength = 16;
    size_t *written = nullptr;

    ssize_t result = encoder->SBCEncode(codecParam, in, iLength, out, oLength, written);
    EXPECT_EQ(result, 128);
    codecSbcEncoderLib_->sbcEncoder.destroySbcEncode(encoder);
    codecWrapper.UnloadCodecSbcLib(codecSbcEncoderLib_);
}