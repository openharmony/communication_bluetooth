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

#include <cstddef>
#include <cstdint>
#include <vector>

#include "bt_types.h"

enum BtavSourceState {
    BTAV_A2DP_SRC_STATE_DISCONNECTED = 0,
    BTAV_A2DP_SRC_STATE_CONNECTING,
    BTAV_A2DP_SRC_STATE_CONNECTED,
    BTAV_A2DP_SRC_STATE_PLAYING,
};

/* Connection state reported by the A2DP connection callback (bluedroid
 * system/include/hardware/bt_av.h). */
enum BtavConnectionState {
    BTAV_CONNECTION_STATE_DISCONNECTED = 0,
    BTAV_CONNECTION_STATE_CONNECTING,
    BTAV_CONNECTION_STATE_CONNECTED,
    BTAV_CONNECTION_STATE_DISCONNECTING,
};

/* Audio datapath state reported by the A2DP audio callback; LATENCY_CHANGED
 * is a service-layer extension of the bluedroid set. */
enum BtavAudioState {
    BTAV_AUDIO_STATE_REMOTE_SUSPEND = 0,
    BTAV_AUDIO_STATE_STOPPED,
    BTAV_AUDIO_STATE_STARTED,
    BTAV_AUDIO_STATE_LATENCY_CHANGED,
};

enum BtavA2dpCodecIndex {
    BTAV_A2DP_CODEC_INDEX_SOURCE_SBC = 0,
    BTAV_A2DP_CODEC_INDEX_SOURCE_AAC,
    BTAV_A2DP_CODEC_INDEX_SOURCE_APTX,
    BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_HD,
    BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC,
    BTAV_A2DP_CODEC_INDEX_SOURCE_MAX,
    BTAV_A2DP_CODEC_INDEX_SINK_MAX,
    BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_V2,
    BTAV_A2DP_CODEC_INDEX_SOURCE_L2HC_ST,
};

enum BtavA2dpCodecPriority {
    BTAV_A2DP_CODEC_PRIORITY_DISABLED = -1,
    BTAV_A2DP_CODEC_PRIORITY_DEFAULT = 0,
    BTAV_A2DP_CODEC_PRIORITY_HIGHEST = 1000 * 1000,
};

enum BtavA2dpCodecSampleRate {
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
};

enum BtavA2dpCodecBitsPerSample {
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE = 0x0,
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16 = 0x1 << 0,
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24 = 0x1 << 1,
    BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32 = 0x1 << 2,
};

enum BtavA2dpCodecChannelMode {
    BTAV_A2DP_CODEC_CHANNEL_MODE_NONE = 0x0,
    BTAV_A2DP_CODEC_CHANNEL_MODE_MONO = 0x1 << 0,
    BTAV_A2DP_CODEC_CHANNEL_MODE_STEREO = 0x1 << 1,
};

struct BtavA2dpCodecConfig {
    BtavA2dpCodecIndex codecType;
    BtavA2dpCodecPriority codecPriority;
    BtavA2dpCodecSampleRate sampleRate;
    BtavA2dpCodecBitsPerSample bitsPerSample;
    BtavA2dpCodecChannelMode channelMode;
    uint64_t codecSpecific1;
    uint64_t codecSpecific2;
    uint64_t codecSpecific3;
    uint64_t codecSpecific4;
};

/* A2DP source callbacks. */
typedef void (*BtavConnectionStateCallback)(const OHOS::bluetooth::RawAddress &bdAddr, BtavConnectionState state);
typedef void (*BtavAudioStateCallback)(const OHOS::bluetooth::RawAddress &bdAddr, BtavAudioState state);
typedef void (*BtavAudioSourceConfigCallback)(const OHOS::bluetooth::RawAddress &bdAddr, BtavA2dpCodecConfig codecConfig,
    std::vector<BtavA2dpCodecConfig> codecsLocalCapabilities,
    std::vector<BtavA2dpCodecConfig> codecsSelectableCapabilities);
typedef bool (*BtavMandatoryCodecPreferredCallback)(const OHOS::bluetooth::RawAddress &bdAddr);

/* Audio configuration callback of the A2DP sink profile; sampleRate in Hz,
 * channelCount 1 for mono and 2 for stereo. */
typedef void (*BtavAudioSinkConfigCallback)(const OHOS::bluetooth::RawAddress &bdAddr, uint32_t sampleRate,
    uint8_t channelCount);

struct BtavSourceCallbacks {
    size_t size;
    BtavConnectionStateCallback connectionStateCb;
    BtavAudioStateCallback audioStateCb;
    BtavAudioSourceConfigCallback audioConfigCb;
    BtavMandatoryCodecPreferredCallback mandatoryCodecPreferredCb;
};

/* A2DP source interface consumed by the service layer (a2dp_service.cpp). */
struct BtavSourceInterface {
    size_t size;
    BtStackStatus (*init)(BtavSourceCallbacks *callbacks, int maxConnectedAudioDevices,
        const std::vector<BtavA2dpCodecConfig> &codecPriorities,
        const std::vector<BtavA2dpCodecConfig> &offloadingPreference);
    void (*cleanup)(void);
    BtStackStatus (*connect)(const OHOS::bluetooth::RawAddress &bdAddr);
    BtStackStatus (*disconnect)(const OHOS::bluetooth::RawAddress &bdAddr);
    BtStackStatus (*setActiveDevice)(const OHOS::bluetooth::RawAddress &bdAddr);
    BtStackStatus (*configCodec)(const OHOS::bluetooth::RawAddress &bdAddr, std::vector<BtavA2dpCodecConfig> codecPreferences);
};

/* A2DP sink callbacks and interface consumed by the service layer
 * (native_a2dp_adapter.cpp); the interface members mirror the removed stack
 * layer av sink entry points. */
struct BtavSinkCallbacks {
    size_t size;
    BtavConnectionStateCallback connectionStateCb;
    BtavAudioStateCallback audioStateCb;
    BtavAudioSinkConfigCallback audioConfigCb;
};

struct BtavSinkInterface {
    size_t size;
    BtStackStatus (*init)(BtavSinkCallbacks *callbacks, int maxConnectedAudioDevices);
    BtStackStatus (*connect)(const OHOS::bluetooth::RawAddress &bdAddr);
    BtStackStatus (*disconnect)(const OHOS::bluetooth::RawAddress &bdAddr);
    void (*cleanup)(void);
    void (*setAudioFocusState)(int focusState);
    void (*setAudioTrackGain)(float gain);
    BtStackStatus (*setActiveDevice)(const OHOS::bluetooth::RawAddress &bdAddr);
};

#endif  // BT_AV_H
