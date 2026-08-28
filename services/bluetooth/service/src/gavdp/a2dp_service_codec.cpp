/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_a2dp"
#endif

#include "a2dp_service_codec.h"
#include "log.h"
#include "a2dp_def.h"
#include "bluetooth_audio_manager.h"
#include "hw_a2dp_vendor_codec_common.h"

namespace OHOS {
namespace bluetooth {

static bool ConvertAudioConfigCodecPriority(BtavA2dpCodecConfig codecConfig, A2dpSrcCodecInfo &codecInfo)
{
    A2dpCodecPriority a2dpCodecPriority = static_cast<A2dpCodecPriority>(codecConfig.codecPriority);
    switch (a2dpCodecPriority) {
        case A2DP_CODEC_PRIORITY_DISABLED:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_DISABLED_USER;
            break;
        case A2DP_CODEC_PRIORITY_DEFAULT:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_DEFAULT_USER;
            break;
        case A2DP_CODEC_PRIORITY_HIGHEST:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_HIGHEST_USER;
            break;
        case A2DP_CODEC_PRIORITY_L2HCV2:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_L2HCV2_USER;
            break;
        case A2DP_CODEC_PRIORITY_L2HCST:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_L2HCST_USER;
            break;
        case A2DP_CODEC_PRIORITY_LDAC:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_LDAC_USER;
            break;
        case A2DP_CODEC_PRIORITY_AAC:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_AAC_USER;
            break;
        case A2DP_CODEC_PRIORITY_SBC:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_SBC_USER;
            break;
        default:
            codecInfo.codecPriority = A2DP_CODEC_PRIORITY_DEFAULT_USER;
            HILOGE("codecPriority is error param");
            break;
    }
    return true;
}

static bool ConvertAudioConfigCodecType(BtavA2dpCodecConfig codecConfig, A2dpSrcCodecInfo &codecInfo)
{
    switch (codecConfig.codecType) {
        case BTAV_A2DP_CODEC_INDEX_SOURCE_SBC:
            codecInfo.codecType = A2DP_CODEC_TYPE_SBC_USER;
            break;
        case BTAV_A2DP_CODEC_INDEX_SOURCE_AAC:
            codecInfo.codecType = A2DP_CODEC_TYPE_AAC_USER;
            break;
        case BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC:
            codecInfo.codecType = A2DP_CODEC_TYPE_LDAC_USER;
            break;
        case BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_V2:
            codecInfo.codecType = A2DP_CODEC_TYPE_L2HCV2_USER;
            break;
        case BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST:
            codecInfo.codecType = A2DP_CODEC_TYPE_L2HCST_USER;
            break;
        default:
            HILOGE("codecType is error param");
            return false;
    }
    return true;
}

static bool ConvertAudioConfigCodecSampleRate(BtavA2dpCodecConfig codecConfig, A2dpSrcCodecInfo &codecInfo)
{
    A2dpCodecSampleRate a2dpCodecSampleRate = static_cast<A2dpCodecSampleRate>(codecConfig.sampleRate);
    switch (a2dpCodecSampleRate) {
        case A2DP_CODEC_SAMPLE_RATE_96000:
            codecInfo.sampleRate = A2DP_L2HCV2_SAMPLE_RATE_96000_USER;
            break;
        case A2DP_CODEC_SAMPLE_RATE_48000:
            codecInfo.sampleRate = A2DP_SBC_SAMPLE_RATE_48000_USER;
            if (codecConfig.codecType == BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_V2) {
                codecInfo.sampleRate = A2DP_L2HCV2_SAMPLE_RATE_48000_USER;
            }
            break;
        case A2DP_CODEC_SAMPLE_RATE_44100:
            codecInfo.sampleRate = A2DP_SBC_SAMPLE_RATE_44100_USER;
            break;
        case A2DP_CODEC_SAMPLE_RATE_16000:
            codecInfo.sampleRate = A2DP_SBC_SAMPLE_RATE_16000_USER;
            break;
        case A2DP_CODEC_SAMPLE_RATE_48000_96000:
            codecInfo.sampleRate = A2DP_L2HCV2_SAMPLE_RATE_ALL_MSK_USER;
            break;
        case A2DP_CODEC_SAMPLE_RATE_44100_48000_96000:
            codecInfo.sampleRate = A2DP_SBC_SAMPLE_RATE_44100_USER | A2DP_L2HCV2_SAMPLE_RATE_48000_USER |
                                   A2DP_L2HCV2_SAMPLE_RATE_96000_USER;
            break;
        default:
            HILOGE("sampleRate is error param");
            return false;
    }
    return true;
}

static bool ConvertAudioConfigCodecBitsPer(BtavA2dpCodecConfig codecConfig, A2dpSrcCodecInfo &codecInfo)
{
    switch (codecConfig.bitsPerSample) {
        case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16:
            codecInfo.bitsPerSample = A2DP_SAMPLE_BITS_16_USER;
            break;
        case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24:
            codecInfo.bitsPerSample = A2DP_SAMPLE_BITS_24_USER;
            break;
        case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32:
            codecInfo.bitsPerSample = A2DP_SAMPLE_BITS_32_USER;
            break;
        default:
            if (codecConfig.bitsPerSample ==
                (BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24)) {
                codecInfo.bitsPerSample = A2DP_SAMPLE_BITS_16_USER | A2DP_SAMPLE_BITS_24_USER;
                return true;
            }
            if (codecConfig.bitsPerSample ==
                (BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 | BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 |
                BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32)) {
                codecInfo.bitsPerSample =
                    A2DP_SAMPLE_BITS_16_USER | A2DP_SAMPLE_BITS_24_USER | A2DP_SAMPLE_BITS_32_USER;
                return true;
            }
            HILOGE("bitsPerSample is error param");
            return false;
    }
    return true;
}

static bool ConvertAudioConfigCodecChannelMode(BtavA2dpCodecConfig codecConfig, A2dpSrcCodecInfo &codecInfo)
{
    A2dpCodecChannelMode a2dpCodecChannelMode = static_cast<A2dpCodecChannelMode>(codecConfig.channelMode);
    switch (a2dpCodecChannelMode) {
        case A2DP_CODEC_CHANNEL_MODE_STEREO:
            codecInfo.channelMode = A2DP_SBC_CHANNEL_MODE_STEREO_USER;
            break;
        case A2DP_CODEC_CHANNEL_MODE_MONO:
            codecInfo.channelMode = A2DP_SBC_CHANNEL_MODE_MONO_USER;
            break;
        case A2DP_CODEC_CHANNEL_MODE_MONO_STEREO:
            codecInfo.channelMode = A2DP_SBC_CHANNEL_MODE_MONO_STEREO_USER;
            break;
        default:
            HILOGE("channelMode is error param");
            return false;
    }
    return true;
}

bool ConvertAudioConfigCodec(BtavA2dpCodecConfig codecConfig, A2dpSrcCodecInfo &codecInfo)
{
    HILOGD("codecPriority:%{public}d, codecType:%{public}d, sampleRate:%{public}d,"
        "bitPerSample:%{public}d, channelMode:%{public}d",
        codecConfig.codecPriority, codecConfig.codecType, codecConfig.sampleRate,
        codecConfig.bitsPerSample, codecConfig.channelMode);
    codecInfo.codecPriority = static_cast<uint32_t>(codecConfig.codecPriority);
    bool ret = true;
    ret = ConvertAudioConfigCodecPriority(codecConfig, codecInfo);
    if (!ret) {
        return false;
    }
    ret = ConvertAudioConfigCodecType(codecConfig, codecInfo);
    if (!ret) {
        return false;
    }
    ret = ConvertAudioConfigCodecSampleRate(codecConfig, codecInfo);
    if (!ret) {
        return false;
    }
    ret = ConvertAudioConfigCodecBitsPer(codecConfig, codecInfo);
    if (!ret) {
        return false;
    }
    ret = ConvertAudioConfigCodecChannelMode(codecConfig, codecInfo);
    codecInfo.codecSpecific3 = static_cast<uint64_t>(codecConfig.codecSpecific3);
    codecInfo.codecSpecific4 = static_cast<uint64_t>(codecConfig.codecSpecific4);
    if (!ret) {
        return false;
    } else {
        return true;
    }
}

bool ConvertL2hcV2Bitrate(BtavA2dpCodecConfig &codecConfig, const A2dpSrcCodecInfo &codecInfo)
{
    switch (codecInfo.codecSpecific4) {
        case A2DP_L2HCV2_BIT_RATE_320K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCV2_USER_RATE_LOW;
            break;
        case A2DP_L2HCV2_BIT_RATE_640K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCV2_USER_RATE_MID;
            break;
        case A2DP_L2HCV2_BIT_RATE_960K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCV2_USER_RATE_HIGH;
            break;
        case A2DP_L2HCV2_BIT_RATE_AUTO_RATE:
            codecConfig.codecSpecific4 = CodecCommon::L2HCV2_USER_AUTO_RATE;
            break;
        case A2DP_L2HCV2_BIT_RATE_1500K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCV2_USER_RATE_LOSSLESS;
            break;
        case A2DP_L2HCV2_BIT_RATE_2300K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCV2_USER_RATE_2MBPS;
            break;
        default:
            HILOGE("L2HCV2 bitrate is error param");
            return false;
    }
    return true;
}

bool ConvertL2hcStBitrate(BtavA2dpCodecConfig &codecConfig, const A2dpSrcCodecInfo &codecInfo)
{
    switch (codecInfo.codecSpecific4) {
        case A2DP_L2HCST_BIT_RATE_96K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_RATE_96K;
            break;
        case A2DP_L2HCST_BIT_RATE_128K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_RATE_128K;
            break;
        case A2DP_L2HCST_BIT_RATE_192K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_RATE_192K;
            break;
        case A2DP_L2HCST_BIT_RATE_256K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_RATE_256K;
            break;
        case A2DP_L2HCST_BIT_RATE_320K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_RATE_320K;
            break;
        case A2DP_L2HCST_BIT_RATE_480K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_RATE_480K;
            break;
        case A2DP_L2HCST_BIT_RATE_640K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_RATE_640K;
            break;
        case A2DP_L2HCST_BIT_RATE_960K:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_RATE_960K;
            break;
        case A2DP_L2HCST_BIT_RATE_AUTO_RATE:
            codecConfig.codecSpecific4 = CodecCommon::L2HCST_USER_AUTO_RATE;
            break;
        default:
            HILOGE("L2HCST bitrate is error param");
            return false;
    }
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS