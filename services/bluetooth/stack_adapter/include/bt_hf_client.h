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
 * Stub of the removed stack layer HFP HF profile interface (bt_hf_client.h).
 * Interface table layout and signatures mirror bluedroid
 * system/include/hardware/bt_hf_client.h.
 */

#ifndef BT_HF_CLIENT_H
#define BT_HF_CLIENT_H

#include <cstddef>

#include "bt_types.h"

typedef enum {
    BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED = 0,
    BTHF_CLIENT_CONNECTION_STATE_CONNECTING,
    BTHF_CLIENT_CONNECTION_STATE_CONNECTED,
    BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED,
    BTHF_CLIENT_CONNECTION_STATE_DISCONNECTING,
} bthf_client_connection_state_t;

typedef enum {
    BTHF_CLIENT_AUDIO_STATE_DISCONNECTED = 0,
    BTHF_CLIENT_AUDIO_STATE_CONNECTING,
    BTHF_CLIENT_AUDIO_STATE_CONNECTED,
    BTHF_CLIENT_AUDIO_STATE_DISCONNECTING,
} bthf_client_audio_state_t;

typedef enum {
    BTHF_CLIENT_VR_STATE_STOPPED = 0,
    BTHF_CLIENT_VR_STATE_STARTED,
} bthf_client_vr_state_t;

typedef enum {
    BTHF_CLIENT_CALL_STATE_ACTIVE = 0,
    BTHF_CLIENT_CALL_STATE_HELD,
    BTHF_CLIENT_CALL_STATE_DIALING,
    BTHF_CLIENT_CALL_STATE_ALERTING,
    BTHF_CLIENT_CALL_STATE_INCOMING,
    BTHF_CLIENT_CALL_STATE_WAITING,
    BTHF_CLIENT_CALL_STATE_HELD_BY_RESP_HOLD,
} bthf_client_call_state_t;

typedef enum {
    BTHF_CLIENT_CALL_ACTION_CHLD_0 = 0,
    BTHF_CLIENT_CALL_ACTION_CHLD_1,
    BTHF_CLIENT_CALL_ACTION_CHLD_2,
    BTHF_CLIENT_CALL_ACTION_CHLD_3,
    BTHF_CLIENT_CALL_ACTION_ATA,
    BTHF_CLIENT_CALL_ACTION_CHUP,
    BTHF_CLIENT_CALL_ACTION_BTRH_0,
    BTHF_CLIENT_CALL_ACTION_BTRH_1,
    BTHF_CLIENT_CALL_ACTION_BTRH_2,
} bthf_client_call_action_t;

typedef enum {
    BTHF_CLIENT_CMD_COMPLETE_OK = 0,
    BTHF_CLIENT_CMD_COMPLETE_ERROR,
    BTHF_CLIENT_CMD_COMPLETE_ERROR_CME,
} bthf_client_cmd_complete_t;

typedef enum {
    BTHF_CLIENT_NETWORK_STATE_NOT_AVAILABLE = 0,
    BTHF_CLIENT_NETWORK_STATE_AVAILABLE,
} bthf_client_network_state_t;

typedef enum {
    BTHF_CLIENT_SERVICE_TYPE_HOME = 0,
    BTHF_CLIENT_SERVICE_TYPE_ROAMING,
} bthf_client_service_type_t;

typedef enum {
    BTHF_CLIENT_CALL_NONE = 0,
    BTHF_CLIENT_CALL_ACTIVE,
    BTHF_CLIENT_CALL_HELD,
    BTHF_CLIENT_CALL_DIALING,
    BTHF_CLIENT_CALL_ALERTING,
    BTHF_CLIENT_CALL_INCOMING,
    BTHF_CLIENT_CALL_WAITING,
    BTHF_CLIENT_CALL_HELD_BY_RESP_HOLD,
} bthf_client_call_t;

typedef enum {
    BTHF_CLIENT_CALLSETUP_NONE = 0,
    BTHF_CLIENT_CALLSETUP_INCOMING,
    BTHF_CLIENT_CALLSETUP_DIALING,
    BTHF_CLIENT_CALLSETUP_ALERTING,
} bthf_client_callsetup_t;

typedef enum {
    BTHF_CLIENT_CALLHELD_NONE = 0,
    BTHF_CLIENT_CALLHELD_HOLD_AND_ACTIVE,
    BTHF_CLIENT_CALLHELD_HOLD,
} bthf_client_callheld_t;

typedef enum {
    BTHF_CLIENT_RESP_AND_HOLD_HELD = 0,
    BTHF_CLIENT_RESP_AND_HOLD_ACCEPT,
    BTHF_CLIENT_RESP_AND_HOLD_REJECT,
} bthf_client_resp_and_hold_t;

typedef enum {
    BTHF_CLIENT_CALL_DIR_INCOMING = 0,
    BTHF_CLIENT_CALL_DIR_OUTGOING,
} bthf_client_call_direction_t;

typedef enum {
    BTHF_CLIENT_CALL_MPTY_TYPE_SINGLE = 0,
    BTHF_CLIENT_CALL_MPTY_TYPE_MULTI,
} bthf_client_call_mpty_type_t;

typedef enum {
    BTHF_CLIENT_VOLUME_TYPE_SPK = 0,
    BTHF_CLIENT_VOLUME_TYPE_MIC,
} bthf_client_volume_type_t;

typedef enum {
    BTHF_CLIENT_SUBSCRIBER_SERVICE_TYPE_UNKNOWN = 0,
    BTHF_CLIENT_SUBSCRIBER_SERVICE_TYPE_VOICE,
    BTHF_CLIENT_SUBSCRIBER_SERVICE_TYPE_FAX,
} bthf_client_subscriber_service_type_t;

typedef enum {
    BTHF_CLIENT_IN_BAND_RING_NOT_PROVIDED = 0,
    BTHF_CLIENT_IN_BAND_RING_PROVIDED,
} bthf_client_in_band_ring_state_t;

typedef struct {
    size_t size;
    void (*connection_state_cb)(const RawAddress *bd_addr,
                                bthf_client_connection_state_t state,
                                unsigned int peer_feat, unsigned int chld_feat);
    void (*audio_state_cb)(const RawAddress *bd_addr, bthf_client_audio_state_t state);
    void (*vr_cmd_cb)(const RawAddress *bd_addr, bthf_client_vr_state_t state);
    void (*network_state_cb)(const RawAddress *bd_addr, bthf_client_network_state_t state);
    void (*network_roaming_cb)(const RawAddress *bd_addr, bthf_client_service_type_t type);
    void (*network_signal_cb)(const RawAddress *bd_addr, int signal);
    void (*battery_level_cb)(const RawAddress *bd_addr, int level);
    void (*current_operator_cb)(const RawAddress *bd_addr, const char *name);
    void (*call_cb)(const RawAddress *bd_addr, bthf_client_call_t call);
    void (*callsetup_cb)(const RawAddress *bd_addr, bthf_client_callsetup_t callsetup);
    void (*callheld_cb)(const RawAddress *bd_addr, bthf_client_callheld_t callheld);
    void (*resp_and_hold_cb)(const RawAddress *bd_addr,
                             bthf_client_resp_and_hold_t resp_and_hold);
    void (*clip_cb)(const RawAddress *bd_addr, const char *number);
    void (*call_waiting_cb)(const RawAddress *bd_addr, const char *number);
    void (*current_calls_cb)(const RawAddress *bd_addr, int index,
                             bthf_client_call_direction_t dir,
                             bthf_client_call_state_t state,
                             bthf_client_call_mpty_type_t mpty, const char *number);
    void (*volume_change_cb)(const RawAddress *bd_addr, bthf_client_volume_type_t type,
                             int volume);
    void (*cmd_complete_cb)(const RawAddress *bd_addr, bthf_client_cmd_complete_t type,
                            int cme);
    void (*subscriber_info_cb)(const RawAddress *bd_addr, const char *name,
                               bthf_client_subscriber_service_type_t type);
    void (*in_band_ring_cb)(const RawAddress *bd_addr,
                            bthf_client_in_band_ring_state_t in_band);
    void (*last_voice_tag_number_cb)(const RawAddress *bd_addr, const char *number);
    void (*ring_indication_cb)(const RawAddress *bd_addr);
    void (*unknown_event_cb)(const RawAddress *bd_addr, const char *event_string);
} bthf_client_callbacks_t;

typedef struct {
    size_t size;
    bt_status_t (*init)(bthf_client_callbacks_t *callbacks);
    bt_status_t (*connect)(const RawAddress *bd_addr);
    bt_status_t (*disconnect)(const RawAddress *bd_addr);
    bt_status_t (*connect_audio)(const RawAddress *bd_addr);
    bt_status_t (*disconnect_audio)(const RawAddress *bd_addr);
    bt_status_t (*start_voice_recognition)(const RawAddress *bd_addr);
    bt_status_t (*stop_voice_recognition)(const RawAddress *bd_addr);
    bt_status_t (*volume_control)(const RawAddress *bd_addr, bthf_client_volume_type_t type,
        int volume);
    bt_status_t (*dial)(const RawAddress *bd_addr, const char *number);
    bt_status_t (*dial_memory)(const RawAddress *bd_addr, int location);
    bt_status_t (*handle_call_action)(const RawAddress *bd_addr, bthf_client_call_action_t action,
        int idx);
    bt_status_t (*query_current_calls)(const RawAddress *bd_addr);
    bt_status_t (*query_current_operator_name)(const RawAddress *bd_addr);
    bt_status_t (*retrieve_subscriber_info)(const RawAddress *bd_addr);
    bt_status_t (*send_dtmf)(const RawAddress *bd_addr, char code);
    bt_status_t (*request_last_voice_tag_number)(const RawAddress *bd_addr);
    void (*cleanup)(void);
    bt_status_t (*send_at_cmd)(const RawAddress *bd_addr, int cmd, int val1, int val2,
        const char *arg);
    bt_status_t (*send_android_at)(const RawAddress *bd_addr, const char *arg);
} bthf_client_interface_t;

#endif  // BT_HF_CLIENT_H
