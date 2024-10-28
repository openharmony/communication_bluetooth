/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SERVICE_PROFILE_INTERFACE_CODE_H
#define BLUETOOTH_SERVICE_PROFILE_INTERFACE_CODE_H

#include "ipc_types.h"

/* SAID: 1130 */
namespace OHOS {
namespace Bluetooth {
enum BluetoothA2dpSinkObserverInterfaceCode {
    BT_A2DP_SINK_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    // The last code, if you want to add a new code, please add it before this
    BT_A2DP_SINK_OBSERVER_BUTT
};

enum BluetoothA2dpSinkInterfaceCode {
    BT_A2DP_SINK_CONNECT = 0,
    BT_A2DP_SINK_DISCONNECT,
    BT_A2DP_SINK_REGISTER_OBSERVER,
    BT_A2DP_SINK_DEREGISTER_OBSERVER,
    BT_A2DP_SINK_GET_DEVICE_BY_STATES,
    BT_A2DP_SINK_GET_DEVICE_STATE,
    BT_A2DP_SINK_GET_PLAYING_STATE,
    BT_A2DP_SINK_SET_CONNECT_STRATEGY,
    BT_A2DP_SINK_GET_CONNECT_STRATEGY,
    BT_A2DP_SINK_SEND_DELAY,
    // The last code, if you want to add a new code, please add it before this
    BT_A2DP_SINK_BUTT
};

enum BluetoothA2dpSourceObserverInterfaceCode {
    BT_A2DP_SRC_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    BT_A2DP_SRC_OBSERVER_PLAYING_STATUS_CHANGED,
    BT_A2DP_SRC_OBSERVER_CONFIGURATION_CHANGED,
    BT_A2DP_SRC_OBSERVER_MEDIASTACK_CHANGED,
    BT_A2DP_SRC_OBSERVER_VIRTUALDEVICE_CHANGED,
    BT_A2DP_SRC_OBSERVER_CAPTURE_CONNECTION_STATE_CHANGED,
    // The last code, if you want to add a new code, please add it before this
    BT_A2DP_SRC_OBSERVER_BUTT
};

enum BluetoothA2dpSrcInterfaceCode {
    BT_A2DP_SRC_CONNECT = 0,
    BT_A2DP_SRC_DISCONNECT,
    BT_A2DP_SRC_REGISTER_OBSERVER,
    BT_A2DP_SRC_DEREGISTER_OBSERVER,
    BT_A2DP_SRC_GET_DEVICE_BY_STATES,
    BT_A2DP_SRC_GET_DEVICE_STATE,
    BT_A2DP_SRC_GET_PLAYING_STATE,
    BT_A2DP_SRC_SET_CONNECT_STRATEGY,
    BT_A2DP_SRC_GET_CONNECT_STRATEGY,
    BT_A2DP_SRC_SET_ACTIVE_SINK_DEVICE,
    BT_A2DP_SRC_GET_ACTIVE_SINK_DEVICE,
    BT_A2DP_SRC_GET_CODEC_STATUS,
    BT_A2DP_SRC_SET_CODEC_PREFERENCE,
    BT_A2DP_SRC_SWITCH_OPTIONAL_CODECS,
    BT_A2DP_SRC_GET_OPTIONAL_CODECS_SUPPORT_STATE,
    BT_A2DP_SRC_START_PLAYING,
    BT_A2DP_SRC_SUSPEND_PLAYING,
    BT_A2DP_SRC_STOP_PLAYING,
    BT_A2DP_SRC_SET_AUDIO_CONFIGURE,
    BT_A2DP_SRC_WRITE_FRAME,
    BT_A2DP_SRC_GET_RENDER_POSITION,
    BT_A2DP_SRC_GET_CODEC_PREFERENCE,
    BT_A2DP_SRC_OFFLOAD_START_PLAYING,
    BT_A2DP_SRC_OFFLOAD_STOP_PLAYING,
    BT_A2DP_SRC_OFFLOAD_SESSION_REQUEST,
    BT_A2DP_SRC_OFFLOAD_GET_CODEC_STATUS,
    BT_A2DP_SRC_ENABLE_AUTO_PLAY,
    BT_A2DP_SRC_DISABLE_AUTO_PLAY,
    BT_A2DP_SRC_GET_AUTO_PLAY_DISABLED_DURATION,
    BT_A2DP_SRC_SET_VIRTUAL_DEVICE,
    BT_A2DP_SRC_GET_VIRTUALDEVICE_LIST,
    // The last code, if you want to add a new code, please add it before this
    BT_A2DP_SRC_BUTT
};

enum class BluetoothAvrcpCtObserverInterfaceCode {
    AVRCP_CT_CONNECTION_STATE_CHANGED = 0,
    AVRCP_CT_PRESS_BUTTON,
    AVRCP_CT_RELEASE_BUTTON,
    AVRCP_CT_SET_BROWSED_PLAYER,
    AVRCP_CT_GET_CAPABILITIES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_CURRENT_VALUE,
    AVRCP_CT_SET_PLAYER_APP_SETTING_CURRENT_VALUE,
    AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT,
    AVRCP_CT_GET_PLAYER_APP_SETTING_VALUE_TEXT,
    AVRCP_CT_ELEMENT_ATTRIBUTRES,
    AVRCP_CT_GET_PLAY_STATUS,
    AVRCP_CT_PLAY_ITEM,
    AVRCP_CT_GET_TOTAL_NUMBER_OF_ITEMS,
    AVRCP_CT_GET_ITEM_ATTRIBUTES,
    AVRCP_CT_SET_ABSOLUTE_VOLUME,
    AVRCP_CT_PLAYBACK_STATUS_CHANGED,
    AVRCP_CT_TRACK_CHANGED,
    AVRCP_CT_TRACK_REACHED_END,
    AVRCP_CT_TRACK_REACHED_START,
    AVRCP_CT_PLAYBACK_POS_CHANGED,
    AVRCP_CT_PLAY_APP_SETTING_CHANGED,
    AVRCP_CT_NOW_PLAYING_CONTENT_CHANGED,
    AVRCP_CT_AVAILABLE_PLAYER_CHANGED,
    AVRCP_CT_ADDRESSED_PLAYER_CHANGED,
    AVRCP_CT_UID_CHANGED,
    AVRCP_CT_VOLUME_CHANGED,
    AVRCP_CT_GET_MEDIA_PLAYERS,
    AVRCP_CT_GET_FOLDER_ITEMS,
    // The last code, if you want to add a new code, please add it before this
    AVRCP_CT_OBSERVER_BUTT
};

enum class BluetoothAvrcpCtInterfaceCode {
    AVRCP_CT_REGISTER_OBSERVER = 0,
    AVRCP_CT_UNREGISTER_OBSERVER,
    AVRCP_CT_GET_CONNECTED_DEVICES,
    AVRCP_CT_GET_DEVICES_BY_STATES,
    AVRCP_CT_GET_DEVICE_STATE,
    AVRCP_CT_CONNECT,
    AVRCP_CT_DISCONNECT,
    AVRCP_CT_PRESS_BUTTON,
    AVRCP_CT_RELEASE_BUTTON,
    AVRCP_CT_GET_UNIT_INFO,
    AVRCP_CT_GET_SUB_UNIT_INFO,
    AVRCP_CT_GET_SUPPORTED_COMPANIES,
    AVRCP_CT_GET_SUPPORTED_EVENTS,
    AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES,
    AVRCP_CT_GET_PLAYER_APP_SETTING_CURRENT_VALUE,
    AVRCP_CT_SET_PLAYER_APP_SETTING_CURRENT_VALUE,
    AVRCP_CT_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT,
    AVRCP_CT_GET_PLAYER_APP_SETTING_VALUES_TEXT,
    AVRCP_CT_GET_ELEMENT_ATTRIBUTES,
    AVRCP_CT_GET_PLAYER_STATUS,
    AVRCP_CT_PLAY_ITEM,
    AVRCP_CT_GET_FOLDER_ITEMS,
    AVRCP_CT_GET_TOTAL_NUMBER_OF_ITEMS,
    AVRCP_CT_SET_ABSOLUTE_VOLUME,
    AVRCP_CT_ENABLE_NOTIFICATION,
    AVRCP_CT_DISABLE_NOTIFICATION,
    AVRCP_CT_GET_ITEM_ATTRIBUTES,
    AVRCP_CT_SET_BROWSERED_PLAYER,
    AVRCP_CT_MEDIA_PLAYER_LIST,
    // The last code, if you want to add a new code, please add it before this
    AVRCP_CT_BUTT
};

enum BluetoothAvrcpTgObserverInterfaceCode {
    BT_AVRCP_TG_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    // The last code, if you want to add a new code, please add it before this
    BT_AVRCP_TG_OBSERVER_BUTT
};

enum BluetoothAvrcpTgInterfaceCode {
    BT_AVRCP_TG_REGISTER_OBSERVER = 0,
    BT_AVRCP_TG_UNREGISTER_OBSERVER,
    BT_AVRCP_TG_SET_ACTIVE_DEVICE,
    BT_AVRCP_TG_CONNECT,
    BT_AVRCP_TG_DISCONNECT,
    BT_AVRCP_TG_GET_CONNECTED_DEVICES,
    BT_AVRCP_TG_GET_DEVICES_BY_STATES,
    BT_AVRCP_TG_GET_DEVICE_STATE,
    BT_AVRCP_TG_NOTIFY_PLAYBACK_STATUS_CHANGED,
    BT_AVRCP_TG_NOTIFY_TRACK_CHANGED,
    BT_AVRCP_TG_NOTIFY_TRACK_REACHED_END,
    BT_AVRCP_TG_NOTIFY_TRACK_REACHED_START,
    BT_AVRCP_TG_NOTIFY_PLAYBACK_POS_CHANGED,
    BT_AVRCP_TG_NOTIFY_PLAYER_APP_SETTING_CHANGED,
    BT_AVRCP_TG_NOTIFY_NOWPLAYING_CONTENT_CHANGED,
    BT_AVRCP_TG_NOTIFY_AVAILABLE_PLAYERS_CHANGED,
    BT_AVRCP_TG_NOTIFY_ADDRESSED_PLAYER_CHANGED,
    BT_AVRCP_TG_NOTIFY_UID_CHANGED,
    BT_AVRCP_TG_NOTIFY_VOLUME_CHANGED,
    BT_AVRCP_TG_SET_DEVICE_ABSOLUTE_VOLUME,
    BT_AVRCP_TG_SET_DEVICE_ABS_VOLUME_ABILITY,
    BT_AVRCP_TG_GET_DEVICE_ABS_VOLUME_ABILITY,
    // The last code, if you want to add a new code, please add it before this
    BT_AVRCP_TG_BUTT
};

enum BluetoothHfpAgObserverInterfaceCode {
    BT_HFP_AG_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    BT_HFP_AG_OBSERVER_SCO_STATE_CHANGED,
    BT_HFP_AG_OBSERVER_ACTIVE_DEVICE_CHANGED,
    BT_HFP_AG_OBSERVER_HF_ENHANCED_DRIVER_SAFETY_CHANGED,
    BT_HFP_AG_OBSERVER_HFP_STACK_CHANGED,
    BT_HFP_AG_OBSERVER_VIRTUALDEVICE_CHANGED,
    // The last code, if you want to add a new code, please add it before this
    BT_HFP_AG_OBSERVER_BUTT
};

enum BluetoothHfpAgInterfaceCode {
    BT_HFP_AG_GET_CONNECT_DEVICES = 0,
    BT_HFP_AG_GET_DEVICES_BY_STATES,
    BT_HFP_AG_GET_DEVICE_STATE,
    BT_HFP_AG_CONNECT,
    BT_HFP_AG_DISCONNECT,
    BT_HFP_AG_GET_SCO_STATE,
    BT_HFP_AG_CONNECT_SCO,
    BT_HFP_AG_DISCONNECT_SCO,
    BT_HFP_AG_PHONE_STATE_CHANGED,
    BT_HFP_AG_CLCC_RESPONSE,
    BT_HFP_AG_OPEN_VOICE_RECOGNITION,
    BT_HFP_AG_CLOSE_VOICE_RECOGNITION,
    BT_HFP_AG_SET_ACTIVE_DEVICE,
    BT_HFP_AG_INTO_MOCK,
    BT_HFP_AG_SEND_NO_CARRIER,
    BT_HFP_AG_GET_ACTIVE_DEVICE,
    BT_HFP_AG_REGISTER_OBSERVER,
    BT_HFP_AG_DEREGISTER_OBSERVER,
    BT_HFP_AG_SET_CONNECT_STRATEGY,
    BT_HFP_AG_GET_CONNECT_STRATEGY,
    BT_HFP_AG_CONNECT_SCO_EX,
    BT_HFP_AG_DISCONNECT_SCO_EX,
    BT_HFP_AG_IS_IN_BAND_RINGING_ENABLE,
    BT_HFP_AG_CALL_DETAILS_CHANGED,
    BT_HFP_AG_IS_VGS_SUPPORTED,
    BT_HFP_AG_CALL_LOG,
    BT_HFP_AG_GET_VIRTUALDEVICE_LIST,
    BT_HFP_AG_UPDATE_VIRTUALDEVICE,
    // The last code, if you want to add a new code, please add it before this
    BT_HFP_AG_BUTT
};

enum BluetoothHfpHfObserverInterfaceCode {
    BT_HFP_HF_OBSERVER_CONNECTION_STATE_CHANGED = 0,
    BT_HFP_HF_OBSERVER_SCO_STATE_CHANGED,
    BT_HFP_HF_OBSERVER_CALL_CHANGED,
    BT_HFP_HF_OBSERVER_SIGNAL_STRENGTH_CHANGED,
    BT_HFP_HF_OBSERVER_REGISTRATION_STATUS_CHANGED,
    BT_HFP_HF_OBSERVER_ROAMING_STATUS_CHANGED,
    BT_HFP_HF_OBSERVER_OPERATOR_SELECTION_CHANGED,
    BT_HFP_HF_OBSERVER_SUBSCRIBER_NUMBER_CHANGED,
    BT_HFP_HF_OBSERVER_VOICE_RECOGNITION_STATUS_CHANGED,
    BT_HFP_HF_OBSERVER_IN_BAND_RING_TONE_CHANGED,
    // The last code, if you want to add a new code, please add it before this
    BT_HFP_HF_OBSERVER_BUTT
};

enum BluetoothHfpHfInterfaceCode {
    BT_HFP_HF_CONNECT_SCO = 0,
    BT_HFP_HF_DISCONNECT_SCO,
    BT_HFP_HF_GET_DEVICES_BY_STATES,
    BT_HFP_HF_GET_DEVICE_STATE,
    BT_HFP_HF_GET_SCO_STATE,
    BT_HFP_HF_SEND_DTMF_TONE,
    BT_HFP_HF_CONNECT,
    BT_HFP_HF_DISCONNECT,
    BT_HFP_HF_OPEN_VOICE_RECOGNITION,
    BT_HFP_HF_CLOSE_VOICE_RECOGNITION,
    BT_HFP_HF_GET_CURRENT_CALL_LIST,
    BT_HFP_HF_ACCEPT_INCOMING_CALL,
    BT_HFP_HF_HOLD_ACTIVE_CALL,
    BT_HFP_HF_REJECT_INCOMING_CALL,
    BT_HFP_HF_HANDLE_INCOMING_CALL,
    BT_HFP_HF_HANDLE_MULLTI_CALL,
    BT_HFP_HF_DIAL_LAST_NUMBER,
    BT_HFP_HF_DIAL_MEMORY,
    BT_HFP_HF_FINISH_ATIVE_CALL,
    BT_HFP_HF_START_DIAL,
    BT_HFP_HF_REGISTER_OBSERVER,
    BT_HFP_HF_DEREGISTER_OBSERVER,
    BT_HFP_HF_SEND_VOICE_TAG,
    BT_HFP_HF_SEND_KEY_PRESSED,
    // The last code, if you want to add a new code, please add it before this
    BT_HFP_HF_BUTT
};

#define SEND_IPC_REQUEST_RETURN_RESULT(code, data, reply, option, result)                   \
    do {                                                                                    \
        sptr<IRemoteObject> remote = Remote();                                              \
        if (remote == nullptr) {                                                            \
            HILOGE("remote is nullptr.");                                                   \
            return (result);                                                                \
        }                                                                                   \
        int ret = remote->SendRequest((code), (data), (reply), (option));                   \
        if (ret != BT_NO_ERROR) {                                                           \
            HILOGE("IPC send failed, ret(%{public}d), code(%{public}d)", ret, (code));      \
            return (result);                                                                \
        }                                                                                   \
    } while (0)

#define SEND_IPC_REQUEST_RETURN(code, data, reply, option)                                  \
    do {                                                                                    \
        sptr<IRemoteObject> remote = Remote();                                              \
        if (remote == nullptr) {                                                            \
            HILOGE("remote is nullptr.");                                                   \
            return;                                                                         \
        }                                                                                   \
        int ret = remote->SendRequest((code), (data), (reply), (option));                   \
        if (ret != BT_NO_ERROR) {                                                           \
            HILOGE("IPC send failed, ret(%{public}d), code(%{public}d)", ret, (code));      \
            return;                                                                         \
        }                                                                                   \
    } while (0)

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_SERVICE_PROFILE_INTERFACE_CODE_H