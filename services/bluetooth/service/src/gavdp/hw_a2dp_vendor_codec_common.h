/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef HW_A2DP_VENDOR_CODEC_COMMON_H
#define HW_A2DP_VENDOR_CODEC_COMMON_H

#include <cstdint>

/* Huawei vendor A2DP codec "USER" value placeholders. These mirror the
 * closed-source HAL's vendor-specific codec values and are opaque to the
 * migrated service layer (only used as opaque numeric identifiers). */

/* Codec priority USER values */
#define A2DP_CODEC_PRIORITY_DISABLED_USER 0
#define A2DP_CODEC_PRIORITY_DEFAULT_USER 1
#define A2DP_CODEC_PRIORITY_SBC_USER 2
#define A2DP_CODEC_PRIORITY_AAC_USER 3
#define A2DP_CODEC_PRIORITY_LDAC_USER 4
#define A2DP_CODEC_PRIORITY_L2HCST_USER 5
#define A2DP_CODEC_PRIORITY_L2HCV2_USER 6
#define A2DP_CODEC_PRIORITY_HIGHEST_USER 7

/* Codec type USER values */
#define A2DP_CODEC_TYPE_SBC_USER 0x01
#define A2DP_CODEC_TYPE_AAC_USER 0x02
#define A2DP_CODEC_TYPE_LDAC_USER 0x03
#define A2DP_CODEC_TYPE_L2HCST_USER 0x04
#define A2DP_CODEC_TYPE_L2HCV2_USER 0x05

/* Sample rate USER values */
#define A2DP_SBC_SAMPLE_RATE_16000_USER 0x01
#define A2DP_SBC_SAMPLE_RATE_44100_USER 0x02
#define A2DP_SBC_SAMPLE_RATE_48000_USER 0x04
#define A2DP_L2HCV2_SAMPLE_RATE_48000_USER 0x08
#define A2DP_L2HCV2_SAMPLE_RATE_96000_USER 0x10
#define A2DP_L2HCV2_SAMPLE_RATE_ALL_MSK_USER 0x18

/* Bits per sample USER values */
#define A2DP_SAMPLE_BITS_16_USER 0x01
#define A2DP_SAMPLE_BITS_24_USER 0x02
#define A2DP_SAMPLE_BITS_32_USER 0x04

/* Channel mode USER values */
#define A2DP_SBC_CHANNEL_MODE_MONO_USER 0x01
#define A2DP_SBC_CHANNEL_MODE_STEREO_USER 0x02
#define A2DP_SBC_CHANNEL_MODE_MONO_STEREO_USER 0x03

/* L2HC V2 bit rate identifiers (used with A2dpSrcCodecInfo::codecSpecific4) */
#define A2DP_L2HCV2_BIT_RATE_320K 0
#define A2DP_L2HCV2_BIT_RATE_640K 1
#define A2DP_L2HCV2_BIT_RATE_960K 2
#define A2DP_L2HCV2_BIT_RATE_1500K 3
#define A2DP_L2HCV2_BIT_RATE_2300K 4
#define A2DP_L2HCV2_BIT_RATE_AUTO_RATE 0xFF

/* Vendor codec common USER rate values */
namespace CodecCommon {
enum L2hcUserRate : int64_t {
    L2HCV2_USER_RATE_LOW = 0,
    L2HCV2_USER_RATE_MID = 1,
    L2HCV2_USER_RATE_HIGH = 2,
    L2HCV2_USER_RATE_2MBPS = 3,
    L2HCV2_USER_RATE_LOSSLESS = 4,
    L2HCV2_USER_AUTO_RATE = 0xFF,
    L2HCST_USER_RATE_96K = 0,
    L2HCST_USER_RATE_128K = 1,
    L2HCST_USER_RATE_192K = 2,
    L2HCST_USER_RATE_256K = 3,
    L2HCST_USER_RATE_320K = 4,
    L2HCST_USER_RATE_480K = 5,
    L2HCST_USER_RATE_640K = 6,
    L2HCST_USER_RATE_960K = 7,
    L2HCST_USER_AUTO_RATE = 0xFF,
};
}  // namespace CodecCommon

#endif  // HW_A2DP_VENDOR_CODEC_COMMON_H
