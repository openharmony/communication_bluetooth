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
 * Stub of the removed stack layer HFP AG profile interface (bt_hf.h).
 */

#ifndef BT_HF_H
#define BT_HF_H

#include <cstddef>

#include "bt_types.h"

typedef enum {
    BTHF_CONNECTION_STATE_DISCONNECTED = 0,
    BTHF_CONNECTION_STATE_CONNECTING,
    BTHF_CONNECTION_STATE_CONNECTED,
    BTHF_CONNECTION_STATE_SLC_CONNECTED,
    BTHF_CONNECTION_STATE_DISCONNECTING,
} bthf_connection_state_t;

typedef enum {
    BTHF_AUDIO_STATE_DISCONNECTED = 0,
    BTHF_AUDIO_STATE_CONNECTING,
    BTHF_AUDIO_STATE_CONNECTED,
    BTHF_AUDIO_STATE_DISCONNECTING,
} bthf_audio_state_t;

typedef enum {
    BTHF_VR_STATE_STOPPED = 0,
    BTHF_VR_STATE_STARTED,
} bthf_vr_state_t;

typedef enum {
    BTHF_NREC_STOP = 0,
    BTHF_NREC_START,
} bthf_nrec_t;

typedef enum {
    BTHF_VOLUME_TYPE_SPK = 0,
    BTHF_VOLUME_TYPE_MIC,
} bthf_volume_type_t;

typedef enum {
    BTHF_WBS_NO = 0,
    BTHF_WBS_YES,
    BTHF_WBS_SUPER,
} bthf_wbs_config_t;

typedef enum {
    BTHF_CALL_ACTION_CHLD_0 = 0,
    BTHF_CALL_ACTION_CHLD_1,
    BTHF_CALL_ACTION_CHLD_2,
    BTHF_CALL_ACTION_CHLD_3,
    BTHF_CALL_ACTION_CHLD_4,
    BTHF_CALL_ACTION_CHLD_1X,
    BTHF_CALL_ACTION_CHLD_2X,
} bthf_chld_type_t;

typedef enum {
    BTHF_CALL_STATE_ACTIVE = 0,
    BTHF_CALL_STATE_HELD,
    BTHF_CALL_STATE_DIALING,
    BTHF_CALL_STATE_ALERTING,
    BTHF_CALL_STATE_INCOMING,
    BTHF_CALL_STATE_WAITING,
    BTHF_CALL_STATE_IDLE,
    BTHF_CALL_STATE_DISCONNECTED,
} bthf_call_state_t;

typedef enum {
    BTHF_AT_RESPONSE_ERROR = 0,
    BTHF_AT_RESPONSE_OK,
} bthf_at_response_t;

typedef enum {
    BTHF_CALL_DIR_INCOMING = 0,
    BTHF_CALL_DIR_OUTGOING,
} bthf_call_direction_t;

typedef enum {
    BTHF_CALL_TYPE_VOICE = 0,
    BTHF_CALL_TYPE_DATA,
    BTHF_CALL_TYPE_FAX,
} bthf_call_mode_t;

typedef enum {
    BTHF_CALL_MPTY_TYPE_SINGLE = 0,
    BTHF_CALL_MPTY_TYPE_MULTI,
} bthf_call_mpty_type_t;

typedef enum {
    BTHF_CALL_ADDRTYPE_UNKNOWN = 0,
    BTHF_CALL_ADDRTYPE_INTERNATIONAL,
} bthf_call_addrtype_t;

typedef enum {
    BTHF_NETWORK_STATE_NOT_AVAILABLE = 0,
    BTHF_NETWORK_STATE_AVAILABLE,
} bthf_network_state_t;

typedef enum {
    BTHF_SERVICE_TYPE_HOME = 0,
    BTHF_SERVICE_TYPE_ROAMING,
} bthf_service_type_t;

typedef enum {
    BTHF_HF_IND_ENHANCED_DRIVER_SAFETY = 0,
    BTHF_HF_IND_BATTERY_LEVEL_STATUS,
} bthf_hf_ind_type_t;

typedef struct {
    size_t size;
    bt_status_t (*connection_state_cb)(bthf_connection_state_t state,
                                       const RawAddress *bd_addr);
    bt_status_t (*audio_state_cb)(bthf_audio_state_t state,
                                  const RawAddress *bd_addr);
    bt_status_t (*vr_cb)(bthf_vr_state_t state, const RawAddress *bd_addr);
    bt_status_t (*answer_call_cb)(const RawAddress *bd_addr);
    bt_status_t (*hangup_call_cb)(const RawAddress *bd_addr);
    bt_status_t (*volume_cb)(bthf_volume_type_t type, int volume,
                             const RawAddress *bd_addr);
    bt_status_t (*dial_call_cb)(char *number, const RawAddress *bd_addr);
    bt_status_t (*send_dtmf_cb)(char tone, const RawAddress *bd_addr);
    bt_status_t (*noise_reduction_cb)(bthf_nrec_t nrec,
                                      const RawAddress *bd_addr);
    bt_status_t (*at_response_cb)(const RawAddress *bd_addr);
    bt_status_t (*cw_cb)(bthf_call_state_t state, const RawAddress *bd_addr);
    bt_status_t (*call_ind_cb)(bthf_call_state_t state,
                               const RawAddress *bd_addr);
    bt_status_t (*call_setup_ind_cb)(bthf_call_state_t state,
                                     const RawAddress *bd_addr);
    bt_status_t (*call_held_ind_cb)(bthf_call_state_t state,
                                    const RawAddress *bd_addr);
    bt_status_t (*net_state_cb)(bthf_network_state_t state,
                                const RawAddress *bd_addr);
    bt_status_t (*call_ind_net_cb)(const RawAddress *bd_addr);
    bt_status_t (*signal_cb)(int signal, const RawAddress *bd_addr);
    bt_status_t (*roaming_cb)(int roaming, const RawAddress *bd_addr);
    bt_status_t (*battery_cb)(int battery, const RawAddress *bd_addr);
    bt_status_t (*call_volume_cb)(bthf_volume_type_t type, int volume,
                                  const RawAddress *bd_addr);
} bthf_callbacks_t;

typedef struct {
    size_t size;
    bt_status_t (*init)(bthf_callbacks_t *callbacks);
    void (*cleanup)(void);
    bt_status_t (*connect)(const RawAddress *bd_addr);
    bt_status_t (*disconnect)(const RawAddress *bd_addr);
    bt_status_t (*audio_connect)(const RawAddress *bd_addr);
    bt_status_t (*audio_disconnect)(const RawAddress *bd_addr);
    bt_status_t (*start_voice_recognition)(const RawAddress *bd_addr);
    bt_status_t (*stop_voice_recognition)(const RawAddress *bd_addr);
    bt_status_t (*volume_control)(const RawAddress *bd_addr,
                                  bthf_volume_type_t type, int volume);
    bt_status_t (*notify_battery_level)(const RawAddress *bd_addr, int level);
    bt_status_t (*set_sco_allowed)(const RawAddress *bd_addr, bool value);
} bthf_interface_t;

/*
 * The service layer refers to the HFP AG types with the qualified name
 * ::bluetooth::headset::*, so re-export the global types into that
 * namespace with using declarations.
 */
namespace bluetooth {
namespace headset {

using ::bthf_connection_state_t;
using ::BTHF_CONNECTION_STATE_DISCONNECTED;
using ::BTHF_CONNECTION_STATE_CONNECTING;
using ::BTHF_CONNECTION_STATE_CONNECTED;
using ::BTHF_CONNECTION_STATE_SLC_CONNECTED;
using ::BTHF_CONNECTION_STATE_DISCONNECTING;

using ::bthf_audio_state_t;
using ::BTHF_AUDIO_STATE_DISCONNECTED;
using ::BTHF_AUDIO_STATE_CONNECTING;
using ::BTHF_AUDIO_STATE_CONNECTED;
using ::BTHF_AUDIO_STATE_DISCONNECTING;

using ::bthf_vr_state_t;
using ::BTHF_VR_STATE_STOPPED;
using ::BTHF_VR_STATE_STARTED;

using ::bthf_nrec_t;
using ::BTHF_NREC_STOP;
using ::BTHF_NREC_START;

using ::bthf_volume_type_t;
using ::BTHF_VOLUME_TYPE_SPK;
using ::BTHF_VOLUME_TYPE_MIC;

using ::bthf_wbs_config_t;
using ::BTHF_WBS_NO;
using ::BTHF_WBS_YES;
using ::BTHF_WBS_SUPER;

using ::bthf_chld_type_t;
using ::BTHF_CALL_ACTION_CHLD_0;
using ::BTHF_CALL_ACTION_CHLD_1;
using ::BTHF_CALL_ACTION_CHLD_2;
using ::BTHF_CALL_ACTION_CHLD_3;
using ::BTHF_CALL_ACTION_CHLD_4;
using ::BTHF_CALL_ACTION_CHLD_1X;
using ::BTHF_CALL_ACTION_CHLD_2X;

using ::bthf_call_state_t;
using ::BTHF_CALL_STATE_ACTIVE;
using ::BTHF_CALL_STATE_HELD;
using ::BTHF_CALL_STATE_DIALING;
using ::BTHF_CALL_STATE_ALERTING;
using ::BTHF_CALL_STATE_INCOMING;
using ::BTHF_CALL_STATE_WAITING;
using ::BTHF_CALL_STATE_IDLE;
using ::BTHF_CALL_STATE_DISCONNECTED;

using ::bthf_at_response_t;
using ::BTHF_AT_RESPONSE_ERROR;
using ::BTHF_AT_RESPONSE_OK;

using ::bthf_call_direction_t;
using ::BTHF_CALL_DIR_INCOMING;
using ::BTHF_CALL_DIR_OUTGOING;

using ::bthf_call_mode_t;
using ::BTHF_CALL_TYPE_VOICE;
using ::BTHF_CALL_TYPE_DATA;
using ::BTHF_CALL_TYPE_FAX;

using ::bthf_call_mpty_type_t;
using ::BTHF_CALL_MPTY_TYPE_SINGLE;
using ::BTHF_CALL_MPTY_TYPE_MULTI;

using ::bthf_call_addrtype_t;
using ::BTHF_CALL_ADDRTYPE_UNKNOWN;
using ::BTHF_CALL_ADDRTYPE_INTERNATIONAL;

using ::bthf_network_state_t;
using ::BTHF_NETWORK_STATE_NOT_AVAILABLE;
using ::BTHF_NETWORK_STATE_AVAILABLE;

using ::bthf_service_type_t;
using ::BTHF_SERVICE_TYPE_HOME;
using ::BTHF_SERVICE_TYPE_ROAMING;

using ::bthf_hf_ind_type_t;
using ::BTHF_HF_IND_ENHANCED_DRIVER_SAFETY;
using ::BTHF_HF_IND_BATTERY_LEVEL_STATUS;

}  // namespace headset
}  // namespace bluetooth

#endif  // BT_HF_H
