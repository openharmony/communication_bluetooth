/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer A2DP profile interface (bt_av.h).
 */

#ifndef BT_AV_H
#define BT_AV_H

#include <cstdint>

#include "bt_types.h"

typedef enum {
    BTAV_A2DP_SRC_STATE_DISCONNECTED = 0,
    BTAV_A2DP_SRC_STATE_CONNECTING,
    BTAV_A2DP_SRC_STATE_CONNECTED,
    BTAV_A2DP_SRC_STATE_PLAYING,
} btav_source_state_t;

typedef enum {
    BTAV_A2DP_CODEC_INDEX_SOURCE_SBC = 0,
    BTAV_A2DP_CODEC_INDEX_SOURCE_AAC,
    BTAV_A2DP_CODEC_INDEX_SOURCE_APTX,
    BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_HD,
    BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC,
    BTAV_A2DP_CODEC_INDEX_SOURCE_MAX,
    BTAV_A2DP_CODEC_INDEX_SINK_MAX,
    BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_V2,
    BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST,
} btav_a2dp_codec_index_t;

typedef enum {
    BTAV_A2DP_CODEC_PRIORITY_DISABLED = -1,
    BTAV_A2DP_CODEC_PRIORITY_DEFAULT = 0,
    BTAV_A2DP_CODEC_PRIORITY_HIGHEST = 1000 * 1000,
} btav_a2dp_codec_priority_t;

typedef enum {
    BTAV_A2DP_CODEC_SAMPLE_RATE_NONE = 0x0,
    BTAV_A2DP_CODEC_SAMPLE_RATE_44100 = 0x1 << 0,
    BTAV_A2DP_CODEC_SAMPLE_RATE_48000 = 0x1 << 1,
    BTAV_A2DP_CODEC_SAMPLE_RATE_88200 = 0x1 << 2,
    BTAV_A2DP_CODEC_SAMPLE_RATE_96000 = 0x1 << 3,
    BTAV_A2DP_CODEC_SAMPLE_RATE_176400 = 0x1 << 4,
    BTAV_A2DP_CODEC_SAMPLE_RATE_192000 = 0x1 << 5,
    BTAV_A2DP_CODEC_SAMPLE_RATE_16000 = 0x1 << 6,
    BTAV_A2DP_CODEC_SAMPLE_RATE_24000 = 0x1 << 7,
    BTAV_A2DP_CODEC_SAMPLE_RATE_48000_96000 = 0x0A,
    BTAV_A2DP_CODEC_SAMPLE_RATE_44100_48000_96000 =
        BTAV_A2DP_CODEC_SAMPLE_RATE_44100 | BTAV_A2DP_CODEC_SAMPLE_RATE_48000 |
        BTAV_A2DP_CODEC_SAMPLE_RATE_96000,
} btav_a2dp_codec_sample_rate_t;

typedef enum {
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE = 0x0,
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 = 0x1 << 0,
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 = 0x1 << 1,
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32 = 0x1 << 2,
} btav_a2dp_codec_bits_per_sample_t;

typedef enum {
    BTAV_A2DP_CODEC_CHANNEL_MODE_NONE = 0x0,
    BTAV_A2DP_CODEC_CHANNEL_MODE_MONO = 0x1 << 0,
    BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO = 0x1 << 1,
} btav_a2dp_codec_channel_mode_t;

typedef struct {
    btav_a2dp_codec_index_t codec_type;
    btav_a2dp_codec_priority_t codec_priority;
    btav_a2dp_codec_sample_rate_t sample_rate;
    btav_a2dp_codec_bits_per_sample_t bits_per_sample;
    btav_a2dp_codec_channel_mode_t channel_mode;
    uint64_t codec_specific_1;
    uint64_t codec_specific_2;
    uint64_t codec_specific_3;
    uint64_t codec_specific_4;
} btav_a2dp_codec_config_t;

typedef struct {
    size_t size;
    int (*init)(...);
    void (*cleanup)(...);
    int (*config_codec)(...);
    int (*set_active_device)(...);
} btav_source_interface_t;

typedef struct {
    size_t size;
    int (*init)(...);
    void (*cleanup)(...);
    int (*config_codec)(...);
    int (*set_active_device)(...);
} btav_sink_interface_t;

#endif  // BT_AV_H
