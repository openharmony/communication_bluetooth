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

#include "a2dp_snk_test.h"
#include "a2dp_sbc_dynamic_lib_ctrl.h"
#include "sbc_constant.h"
#include "sbc_decoder.h"

using namespace testing;
using namespace bluetooth;
using namespace sbc;

class sbc_decoder_test : public testing::Test {
public:
    static void SetUpTestCase()
    {
        printf("sbc_decoder_test SetUpTestCase\n");
    }
    static void TearDownTestCase()
    {
        printf("sbc_decoder_test TearDownTestCase\n");
    }
    virtual void SetUp()
    {
        registerSnkAvdtpMock(&mockSnkAvdtp_);
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
        service = new A2dpSnkService();
        service->GetContext()->Initialize();
        EXPECT_CALL(psmMock_, GetProfileService(_)).Times(AtLeast(0)).WillRepeatedly(Return(service));
    }
    virtual void TearDown()
    {
        if (service != nullptr) {
            service->GetContext()->Uninitialize();
            delete service;
        }
    }
protected:
    NiceMock<MockSnkAvdtp> mockSnkAvdtp_;
    A2dpSnkService *service;
    StrictMock<ProfileServiceManagerInstanceMock> psminstanceMock_;
    StrictMock<ProfileServiceManagerMock> psmMock_;
    StrictMock<AdapterConfigInstanceMock> acinstanceMock_;
    StrictMock<AdapterConfigMock> acMock_;
};

TEST_F(sbc_decoder_test, SBCDecode_1)
{
    A2dpSBCDynamicLibCtrl codecWrapper(false);
    CODECSbcLib *codecSbcDecoderLib_ = codecWrapper.LoadCodecSbcLib();
    sbc::IDecoderBase *sbcDecoder = codecSbcDecoderLib_->sbcDecoder.createSbcDecode();

    CodecParam codecParam;
    codecParam.blocks = SBC_BLOCK16;
    codecParam.subbands = SBC_SUBBAND4;
    codecParam.channelMode = SBC_CHANNEL_MODE_MONO;
    codecParam.frequency = SBC_FREQ_44100;
    codecParam.allocation = SBC_ALLOCATION_SNR;
    codecParam.bitpool = 4;
    codecParam.endian = SBC_ENDIANESS_LE;

    uint8_t inData[] = {SBC_SYNCWORD, 48, 1, 55};
    uint8_t outData[] = {32};
    uint8_t *in = inData;
    uint8_t *out = outData;
    size_t iLength = 8;
    size_t oLength = 16;
    size_t *written = nullptr;

    ssize_t result = sbcDecoder->SBCDecode(codecParam, in, iLength, out, oLength, written);
    EXPECT_EQ(result, 8);
    codecSbcDecoderLib_->sbcDecoder.destroySbcDecode(sbcDecoder);
    codecWrapper.UnloadCodecSbcLib(codecSbcDecoderLib_);  
}

TEST_F(sbc_decoder_test, SBCDecode_2)
{
    A2dpSBCDynamicLibCtrl codecWrapper(false);
    CODECSbcLib *codecSbcDecoderLib_ = codecWrapper.LoadCodecSbcLib();
    sbc::IDecoderBase *sbcDecoder = codecSbcDecoderLib_->sbcDecoder.createSbcDecode();

    CodecParam codecParam;
    codecParam.blocks = SBC_BLOCK16;
    codecParam.subbands = SBC_SUBBAND8;
    codecParam.channelMode = SBC_CHANNEL_MODE_MONO;
    codecParam.frequency = SBC_FREQ_44100;
    codecParam.allocation = SBC_ALLOCATION_SNR;
    codecParam.bitpool = 8;
    codecParam.endian = SBC_ENDIANESS_LE;

    uint8_t inData[] = {SBC_SYNCWORD, 9, 1, 193, 1};
    uint8_t outData[] = {32};
    uint8_t *in = inData;
    uint8_t *out = outData;
    size_t iLength = 16;
    size_t oLength = 16;
    size_t *written = nullptr;

    ssize_t result = sbcDecoder->SBCDecode(codecParam, in, iLength, out, oLength, written);
    EXPECT_EQ(result, 13);
    codecSbcDecoderLib_->sbcDecoder.destroySbcDecode(sbcDecoder);
    codecWrapper.UnloadCodecSbcLib(codecSbcDecoderLib_);
}