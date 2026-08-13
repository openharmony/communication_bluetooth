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

#ifndef AVRCP_CT_DEFINE_H
#define AVRCP_CT_DEFINE_H

#define ACTIVE_DEVICE_CHANGE 3

namespace OHOS {
namespace bluetooth {

enum AvrcpCtConnectState {
    AVRCP_CT_STATE_DISCONNECTED = 0,
    AVRCP_CT_STATE_CONNECTING = 1,
    AVRCP_CT_STATE_CONNECTED = 2,
    AVRCP_CT_STATE_DISCONNECTING = 4,
};

typedef struct {
    bool remoteCtlConnected;
    bool browsingConnected;
} AvrcpCtConnectFlag;

enum AvrcpCtCmdMsg {
    AVRCP_MSG_PASSTHRU,
    AVRCP_MSG_SET_SHUFFLE,
    AVRCP_MSG_SET_REPEAT = 0x06,
};

enum AvrcpCtCmd {
    PASS_THRU_CMD_ID_PLAY = 0x44,
    PASS_THRU_CMD_ID_PAUSE = 0x46,
    PASS_THRU_CMD_ID_VOL_UP = 0x41,
    PASS_THRU_CMD_ID_VOL_DOWN = 0x42,
    PASS_THRU_CMD_ID_STOP = 0x45,
    PASS_THRU_CMD_ID_FF = 0x49,
    PASS_THRU_CMD_ID_REWIND = 0x48,
    PASS_THRU_CMD_ID_FORWARD = 0x4B,
    PASS_THRU_CMD_ID_BACKWARD = 0x4C,
};

enum AvrcpCtInteralMsg {
    STACK_CONNECT,
    STACK_DISCONNECT,
    DEVICE_STATE_ACTIVE,
    DEVICE_STATE_INACTIVE,
    AVRCP_CT_GET_COVER_ART_PSM_EVT,
    CLEANUP
};

enum {
    AVRCP_CT_CONNECT_EVT = 10,
    AVRCP_CT_DISCONNECT_EVT = 11,

    AVRCP_CT_SET_ABS_VOL_EVT = 20,
    AVRCP_CT_REGISTER_ABS_VOL_EVT = 21,
};

enum AvrcpCtProcessMsg {
    AVRCP_CT_MSG_PROCESS_TRACK_CHANGED = 205,
    AVRCP_CT_MSG_PROCESS_PLAY_POS_CHANGED = 206,
    AVRCP_CT_MSG_PROCESS_PLAY_STATUS_CHANGED = 207,
    AVRCP_CT_MSG_PROCESS_VOLUME_CHANGED_NOTIFICATION = 208,
    AVRCP_CT_MSG_PROCESS_SET_ABS_VOL_CMD = 203,
    AVRCP_CT_MSG_PROCESS_REGISTER_ABS_VOL_NOTIFICATION = 204,
    AVRCP_CT_MSG_PROCESS_ADDRESSED_PLAYER_CHANGED = 215,
    AVRCP_CT_MSG_PROCESS_SUPPORTED_APPLICATION_SETTINGS = 217,
    AVRCP_CT_MSG_PROCESS_CURRENT_APPLICATION_SETTINGS = 218,
    AVRCP_CT_MSG_PROCESS_AVAILABLE_PLAYER_CHANGED = 219,
    AVRCP_CT_MSG_PROCESS_RECEIVED_COVER_ART_PSM = 220,
    AVRCP_CT_MSG_PROCESS_IMAGE_DOWNLOADED = 400,
    AVRCP_CT_MSG_INTERNAL_ABS_VOL_TIMEOUT = 102,
    AVRCP_CT_MSG_PROCESS_PREPARE = 800,
};

enum {
    ABS_VOL_TIMEOUT_MILLIS = 1000,
    ABS_VOL_BASE = 127,
    VOLUME_CHANGE_IGNORED_TIMEDIFF_MS = 300,
};

enum NotificationRspType {
    NOTIFICATION_RSP_TYPE_INTERIM = 0x00,
    NOTIFICATION_RSP_TYPE_CHANGED = 0x01,
};

enum KeyState { KEY_STATE_PRESSED = 0, KEY_STATE_RELEASED = 1 };

enum AvrcpCtSetPlayerApp {
    AVRCP_EQUALIZER_STATUS = 0x01,
    AVRCP_REPEAT_STATUS = 0x02,
    AVRCP_SHUFFLE_STATUS = 0x03,
    AVRCP_SCAN_STATUS = 0x04,
};

// AVRCP Player Application Setting 值（对齐双框架 PlayerApplicationSettings.JNI_*）
enum AvrcpCtRepeatValue {
    AVRCP_REPEAT_OFF = 0x01,
    AVRCP_REPEAT_SINGLE_TRACK = 0x02,
    AVRCP_REPEAT_ALL_TRACK = 0x03,
    AVRCP_REPEAT_GROUP = 0x04,
};

enum AvrcpCtShuffleValue {
    AVRCP_SHUFFLE_OFF = 0x01,
    AVRCP_SHUFFLE_ALL_TRACK = 0x02,
    AVRCP_SHUFFLE_GROUP = 0x03,
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_CT_DEFINE_H
