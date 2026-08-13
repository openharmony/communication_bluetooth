/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef HFP_HF_DEFINES_H
#define HFP_HF_DEFINES_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace bluetooth {

constexpr int HFP_HF_COMMAND_MTU = 512;
constexpr int HFP_HF_OUTGOING_CALL_ID = -1;
constexpr int HFP_HF_CALL_DEFAULT_ACTION = -1;
constexpr int HANDSFREECLIENT_AT_CMD_NREC = 15;
constexpr int HANDSFREECLIENT_AT_CMD_VENDOR_SPECIFIC_CMD = 16;
constexpr int HANDSFREECLIENT_AT_CMD_BIEV = 17;
constexpr int HF_CONNECTION_TIMEOUT_MS = 30000;  // 30s
constexpr int32_t TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID = 4005;
constexpr int OUTGOING_TIMEOUT_MS = 5000;  // 5s

// AG feature masks
constexpr uint32_t HFP_HF_AG_FEATURES_ECNR = 0x00000002;                  // Echo cancellation/noise reduction
constexpr uint32_t HFP_HF_AG_FEATURES_ENHANCED_CALL_STATUS = 0x00000040;  // Enhanced Call Status

// Flag to check for local NREC support
const bool HANDSFREECLIENT_NREC_SUPPORTED = true;

constexpr const char* DISCONNECTED = "Disconnected";
constexpr const char* CONNECTING = "Connecting";
constexpr const char* CONNECTED = "Connected";
constexpr const char* AUDIO_CONNECTED = "AudioConnected";

constexpr const char* AUDIO_PARAM_KEY = "hfp_extra";

typedef struct {
    uint16_t index {0};
    uint16_t dir {0};
    uint16_t status {0};
    uint16_t mode {0};
    uint16_t mprty {0};
    std::string number {""};
    uint16_t type {0};
} HfpHfCurrentCallData;

enum HfpHfVolumeType { HFP_HF_VOLUME_TYPE_SPK, HFP_HF_VOLUME_TYPE_MIC };

// network state
enum { HFP_HF_NETWORK_STATE_NOT_AVAILABLE, HFP_HF_NETWORK_STATE_AVAILABLE };

enum HfpHfConnectState {
    HFP_HF_STATE_DISCONNECTED = 0,
    HFP_HF_STATE_CONNECTING,
    HFP_HF_STATE_DISCONNECTING,
    HFP_HF_STATE_CONNECTED
};

enum HfpHfAudioState {
    HFP_HF_AUDIO_STATE_DISCONNECTED = HFP_HF_STATE_CONNECTED,
    HFP_HF_AUDIO_STATE_CONNECTING,
    HFP_HF_AUDIO_STATE_DISCONNECTING,
    HFP_HF_AUDIO_STATE_CONNECTED
};

enum {
    HFP_HF_INVALID_EVT = 0,

    // service start/stop
    HFP_HF_SERVICE_STARTUP_EVT = 1,
    HFP_HF_SERVICE_SHUTDOWN_EVT = 2,

    // service connect events
    HFP_HF_CONNECT_EVT = 10,
    HFP_HF_DISCONNECT_EVT = 11,
    HFP_HF_CONNECT_AUDIO_EVT = 20,
    HFP_HF_DISCONNECT_AUDIO_EVT = 21,
    HFP_HF_RETRY_CONNECT_AUDIO_EVT = 22,
    HFP_HF_REMOVE_STATE_MACHINE_EVT = 30,
    HFP_HF_INTERACTIVE_EVT,

    // service timeout events
    HFP_HF_CONNECTION_TIMEOUT_EVT,
    HFP_HF_DISCONNECT_TIMEOUT_EVT,
    HFP_HF_CONNECT_AUDIO_TIMEOUT_EVT,
    HFP_HF_DISCONNECT_AUDIO_TIMEOUT_EVT,

    HFP_HF_SEND_DTMF_EVT,
    HFP_HF_ACCEPT_CALL_EVT,
    HFP_HF_HOLD_CALL_EVT,
    HFP_HF_REJECT_CALL_EVT,
    HFP_HF_SEND_KEY_PRESSED,
    HFP_HF_HANDLE_INCOMING_CALL_EVT,
    HFP_HF_HANDLE_MULTI_CALL_EVT,
    HFP_HF_DIAL_LAST_NUMBER,
    HFP_HF_DIAL_MEMORY,
    HFP_HF_SEND_VOICE_TAG,
    HFP_HF_FINISH_CALL_EVT,
    HFP_HF_DIAL_CALL_EVT,
    HFP_HF_OPEN_VOICE_RECOGNITION_EVT,
    HFP_HF_CLOSE_VOICE_RECOGNITION_EVT,
    HFP_HF_SET_VOLUME_EVT,
    HFP_HF_BATTERY_LEVEL_CHANGED_EVT,
    HFP_HF_ENHANCED_DRIVER_SAFETY_CHANGED_EVT,
    HFP_HF_QUERY_CURRENT_CALLS,
    HFP_HF_SUBSCRIBER_INFO,

    // stack sdp events
    HFP_HF_SDP_DISCOVERY_RESULT_SUCCESS = 98,
    HFP_HF_SDP_DISCOVERY_RESULT_FAIL = 99,

    // stack events
    HFP_HF_CONNECTED_EVT = 110,
    HFP_HF_DISCONNECTED_EVT = 111,
    HFP_HF_CONNECTING_EVT,
    HFP_HF_DISCONNECTING_EVT,

    // service level connection established event
    HFP_HF_SETUP_CODEC_CVSD = 198,
    HFP_HF_SLC_ESTABLISHED_EVT = 199,

    // stack audio events
    HFP_HF_AUDIO_CONNECTING_EVT = 201,
    HFP_HF_AUDIO_CONNECTED_EVT = 220,
    HFP_HF_AUDIO_DISCONNECTED_EVT,
    HFP_HF_AUDIO_CONNECTED_MSBC_EVT,

    HFP_HF_SEND_AT_COMMAND_EVT
};

enum {
    HFP_HF_TYPE_NONE,
    HFP_HF_TYPE_NETWORK_STATE,
    HFP_HF_TYPE_NETWORK_ROAM,
    HFP_HF_TYPE_NETWORK_SIGNAL,
    HFP_HF_TYPE_BATTERY_LEVEL,
    HFP_HF_TYPE_CURRENT_OPERATOR,
    HFP_HF_TYPE_CALL_STATE,
    HFP_HF_TYPE_CALL_SETUP_STATE,
    HFP_HF_TYPE_CALL_HELD_STATE,
    HFP_HF_TYPE_HOLD_RESULT,
    HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION,
    HFP_HF_TYPE_CALL_WAITING,
    HFP_HF_TYPE_CURRENT_CALLS,
    HFP_HF_TYPE_SET_VOLUME,
    HFP_HF_TYPE_AT_CMD_RESULT,
    HFP_HF_TYPE_SUBSCRIBER_NUMBER,
    HFP_HF_TYPE_INBAND_RING,
    HFP_HF_TYPE_LAST_NUMBER,
    HFP_HF_TYPE_RING_ALERT,
    HFP_HF_TYPE_UNKNOWN,
    HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE,
    HFP_HF_TYPE_VOICE_RECOGNITION_CHANGED
};

enum {
    HFP_HF_AT_RESULT_OK = 0,
    HFP_HF_AT_RESULT_ERROR,
    HFP_HF_AT_RESULT_NO_CARRIER,
    HFP_HF_AT_RESULT_BUSY,
    HFP_HF_AT_RESULT_NO_ANSWER,
    HFP_HF_AT_RESULT_DELAYED,
    HFP_HF_AT_RESULT_BLOCKLISTED,
    HFP_HF_AT_RESULT_CME,
};

enum HfpHfAcceptCallAction {
    HFP_HF_ACCEPT_CALL_ACTION_NONE = 0,
    HFP_HF_ACCEPT_CALL_ACTION_HOLD = 1,
    HFP_HF_ACCEPT_CALL_ACTION_FINISH = 2
};

enum class HfpHfHandleIncomingCalAction {
    HFP_HF_HOLD_INCOMING_ACTION = 0,
    HFP_HF_ACCEPT_HOLD_ACTION = 1,
    HFP_HF_REJECT_HOLD_ACTION = 2
};

enum HfpHfChldAction {
    HFP_HF_CHLD_ACTION_0 = 0, /* Releases all held calls. */
    HFP_HF_CHLD_ACTION_1 = 1, /* Releases all active calls and accepts the other call.
                                 Releases specified active call only (<idx>). */
    HFP_HF_CHLD_ACTION_2 = 2, /* Places all active calls on hold and accepts the other call.
                                 Request private consultation mode with specified call (<idx>). */
    HFP_HF_CHLD_ACTION_3 = 3, /* Adds a held call to the conversation. */
    HFP_HF_CHLD_ACTION_4 = 4  /* Explicit Call Transfer. */
};

enum HfpHfMptyType { HFP_HF_MPTY_TYPE_SINGLE = 0, HFP_HF_MPTY_TYPE_MULTI };

enum HfpHfDirectionType { HFP_HF_DIRECTION_TYPE_OUTGOING = 0, HFP_HF_DIRECTION_TYPE_INCOMING };

enum { HFP_HF_VR_STATE_CLOSED, HFP_HF_VR_STATE_OPENED };

}  // namespace bluetooth
}  // namespace OHOS
#endif // HFP_HF_DEFINES_H