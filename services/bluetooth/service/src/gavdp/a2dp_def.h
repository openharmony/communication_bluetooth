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

#ifndef A2DP_DEF_H
#define A2DP_DEF_H

#include <cstddef>
#include <cstdint>

#include "bt_config.h"
#include "bt_def.h"

namespace OHOS {
namespace bluetooth {
/**
 * SBC Codec Specific Information Element
 */
#define A2DP_ROLE_SOURCE 0
#define A2DP_ROLE_SINK 1
#define A2DP_ROLE_INT 2
#define A2DP_ROLE_ACP 3

enum A2dpEvent {
    A2DP_INVALID_EVT = 0,
    A2DP_CONNECT_EVT,
    A2DP_DISCONNECT_EVT,
    A2DP_AVDTP_EVT,
    A2DP_SDP_EVT,
    A2DP_TIMEOUT_EVT,
};

enum A2dpConnectNum {
    A2DP_CONNECT_NUM_MAX = BLUETOOTH_MAX_DEFAULT_CONNECTIONS_NUM,
    A2DP_CONNECT_NUM_UN_MAX,
};

enum A2dpMessage {
    A2DP_MSG_CONNECT,
    A2DP_MSG_DISCONNECT,
    A2DP_MSG_CONNECT_TIMEOUT,
    A2DP_MSG_CONNECT_FORBIDDEN,
    A2DP_MSG_PROFILE_CONNECTED,
    A2DP_MSG_PROFILE_CONNECTING,
    A2DP_MSG_PROFILE_DISCONNECTED,
    A2DP_MSG_PROFILE_DISCONNECTING,
    A2DP_MSG_PROFILE_AUDIO_PLAY_START,
    A2DP_MSG_PROFILE_AUDIO_PLAY_SUSPEND,
    A2DP_MSG_PROFILE_AUDIO_PLAY_STOP,
    A2DP_MSG_PROFILE_CODEC_CHANGE,
};

enum A2dpHiechoStatus {
    A2DP_STATUS_INVALID = 0,
    A2DP_STATUS_ENABLE = 1,
    A2DP_STATUS_DISABLE_NO_SEIZE = 2,
    A2DP_DISABLE_SEIZE = 3,
};

enum A2dpServiceType {
    A2DP_SERVICE_INVALID,
    A2DP_SERVICE_IDLE,
    A2DP_SERVICE_BUSY,
    A2DP_SERVICE_EMPTY_STREAM,
};

enum A2dpCodecPriority {
    A2DP_CODEC_PRIORITY_DISABLED = -1,
    A2DP_CODEC_PRIORITY_DEFAULT = 0,
    A2DP_CODEC_PRIORITY_SBC = 1001,
    A2DP_CODEC_PRIORITY_AAC = 2001,
    A2DP_CODEC_PRIORITY_LDAC = 5001,
    A2DP_CODEC_PRIORITY_L2HCST = 8000,
    A2DP_CODEC_PRIORITY_L2HCV2 = 8001,
    A2DP_CODEC_PRIORITY_HIGHEST = 1000 * 1000
};

enum A2dpCodecSampleRate {
    A2DP_CODEC_SAMPLE_RATE_NONE = 0x0,
    A2DP_CODEC_SAMPLE_RATE_44100 = 0x1 << 0,
    A2DP_CODEC_SAMPLE_RATE_48000 = 0x1 << 1,
    A2DP_CODEC_SAMPLE_RATE_88200 = 0x1 << 2,
    A2DP_CODEC_SAMPLE_RATE_96000 = 0x1 << 3,
    A2DP_CODEC_SAMPLE_RATE_176400 = 0x1 << 4,
    A2DP_CODEC_SAMPLE_RATE_192000 = 0x1 << 5,
    A2DP_CODEC_SAMPLE_RATE_16000 = 0x1 << 6,
    A2DP_CODEC_SAMPLE_RATE_24000 = 0x1 << 7,
    A2DP_CODEC_SAMPLE_RATE_48000_96000 = 0x0A,
    A2DP_CODEC_SAMPLE_RATE_44100_48000_96000 =
        A2DP_CODEC_SAMPLE_RATE_44100 | A2DP_CODEC_SAMPLE_RATE_48000 | A2DP_CODEC_SAMPLE_RATE_96000,
};

enum A2dpCodecChannelMode {
    A2DP_CODEC_CHANNEL_MODE_NONE = 0x0,
    A2DP_CODEC_CHANNEL_MODE_MONO = 0x1 << 0,
    A2DP_CODEC_CHANNEL_MODE_STEREO = 0x1 << 1,
    A2DP_CODEC_CHANNEL_MODE_MONO_STEREO = 0x3,
};

enum CodecChangeAppScene {
    APP_TYPE_MUSIC = 0,
    APP_TYPE_VIDEO,
    APP_TYPE_SMALL_VIDEO,
    APP_TYPE_GAME,
    APP_TYPE_OTHER,
};

/*
 * DDM iware type
 * following iaware interface
 */
enum DdmIwareAppType {
    DDM_IWARE_MUSIC   = 7,
    DDM_IWARE_VIDEO   = 8,
    DDM_IWARE_GAME    = 9,
    DDM_IWARE_OTHERS  = 255,
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // A2DP_DEF_H