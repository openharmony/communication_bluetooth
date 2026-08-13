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

#ifndef WATCH_DEFINES_H
#define WATCH_DEFINES_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace bluetooth {

    constexpr const char* APPLE_IAP_UUID = "00000000-deca-fade-deca-deafdecacafe";

    constexpr const char* EVENT_BLUETOOTH_REMOTEDEVICE_DISCONNECT_REASON =
        "usual.event.bluetooth.remotedevice.DISCONNECT_REASON";
    constexpr const char* EVENT_BLUETOOTH_HOST_HFP_CONNECTION_STATE =
        "usual.event.bluetooth.HFP_HF_CONNECTION_STATE";
    constexpr const char* EVENT_BLUETOOTH_HOST_CONNECT_HFP =
        "usual.event.bluetooth.CONNECT_HFP_HF";
    constexpr const char* EVENT_PHONE_CONNECT_STATE_CHANGE =
        "usual.event.bluetooth.PHONE_CONNECT_STATE_CHANGE";
    constexpr const char* ENTER_SYSTEM_STR_MODE = "usual.event.ENTER_SYSTEM_STR_MODE";
    constexpr const char* EXIT_SYSTEM_STR_MODE = "usual.event.EXIT_SYSTEM_STR_MODE";
    constexpr const char* PARAM_KEY_DISCONNECTION_REASON = "reason";
    constexpr const char* PARAM_KEY_CONNECTION_STATE = "state";
    constexpr const char* PARAM_KEY_PHONE_CONNECTION_STATE = "phone_connect_state";

    constexpr const char* STATUS_SETTING_MODE_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
    constexpr const char* CONTROL_POINT_KEY = "control_point";
    constexpr const char* NOTIFICATION_SOURCE_KEY = "notification_source";
    constexpr const char* DATA_SOURCE_KEY = "data_source";
    constexpr const char* CONTROL_POINT_UUID = "69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9";
    constexpr const char* NOTIFICATION_SOURCE_UUID = "9FBF120D-6301-42D9-8C58-25E699A21DBD";
    constexpr const char* DATA_SOURCE_UUID = "22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB";
    constexpr const char* SETTINGS_DATA_BASE_URI = "dataability:///com.ohos.settingsdata.DataAbility";
    constexpr const char* STATUS_SETTING_CHR_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=pair_dotting_process";
    constexpr const char* STATUS_SETTING_CHR_KEY_URI = "pair_dotting_process";
    constexpr const char* STATUS_SETTING_HEALTH_CONNECTION_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&"
        "key=hw_bluetooth_connection_status";
    constexpr const char* STATUS_SETTING_SECONDARY_PHONE_URI =
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&"
        "key=oobe_back_up_connect_scene";
    constexpr const char* STATUS_SETTING_HEALTH_CONNECTION_KEY_URI = "hw_bluetooth_connection_status";
    constexpr const char* STATUS_SETTING_SECONDARY_PHONE_URI_KEY_URI = "oobe_back_up_connect_scene";
    constexpr const char* HEALTH_CONNECTED = "2";
    constexpr const char* SECONDARY_PHONE_PAIR = "0";
    constexpr const char* SECONDARY_PHONE_UNKNOW = "-1";
    constexpr const char* EVENT_COMMON_LOCAL_DISCONNECT = "usual.event.bluetooth.COMMON_EVENT_DISCONNECT_BT";

    /* Bluetooth permissions */
    constexpr const char* MANAGE_BLUETOOTH = "ohos.permission.MANAGE_BLUETOOTH";
    constexpr const char* RECEIVER_STARTUP_COMPLETED = "ohos.permission.RECEIVER_STARTUP_COMPLETED";
    constexpr const char* MANAGE_SETTINGS = "ohos.permission.MANAGE_SETTINGS";

    constexpr int BITMASK = 0x1F00;
    constexpr int IS_PHONE_CREATE_CONNECT = 1000;
    constexpr int HID_DATA_LEN = 80;

    /* voip call state */
    constexpr int VOIP_CALL_INCOMMING = 0;
    constexpr int VOIP_CALL_ACCEPT = 1;
    /* ACL disconnect reason */
    constexpr int PIN_OR_KEY_MISSING = 6;
    constexpr int ACL_CONNECTION_TIMEOUT = 8;

    constexpr int MIN_NOT_TIMEOUT_RSSI = -65;

    /* Connection device type */
    constexpr int AUDIO_VIDEO = 0x0400;
    constexpr int COMPUTER = 0x0100;
    constexpr int PHONE = 0x0200;

    /* Message delay time */
    // delay time of connect hf if watch create connection
    constexpr int DELAY_TIME_CONNECT_HF_WATCH_CC = 1000;
    // delay time of connect hf if phone create connection
    constexpr int DELAY_TIME_CONNECT_HF_PHONE_CC = 3000;
    constexpr int DELAY_TIME_ROUTE_SCO = 3000;
    constexpr int DELAY_TIME_DISCONNECT_HF = 2000;
    constexpr int DELAY_TIME_HEALTH_DISCONNECT_CONNECT_HF = 10000;
    constexpr int DELAY_TIME_CONNECTION_TIMEOUT = 30000;
    constexpr int DELAY_TIME_CHR_BT_DISCONNECT_TIMEOUT = 1000;
    constexpr int DELAY_TIME_DISCONNECT_SCO_TIMEOUT = (10 * 60 * 1000);
    constexpr int DELAY_TIME_SAIS_ADDED_TIMEOUT = 5000;
    inline constexpr  uint8_t g_hidData[] = {
        0x05, 0x0C, /* Usage Page (Consumer), */
        0x09, 0x01, /* Usage (Consumer Control), */
        0xA1, 0x01, /* Collection (Application), */
        0x85, 0x03, /* Report_ID(3), */
        0x05, 0x0C, /* Usage Page (Consumer), */
        0x15, 0x00, /* Logical Minimum (0), */
        0x25, 0x01, /* Logical Maximum (1), */
        0x95, 0x07, /* Report Count (7), */
        0x75, 0x01, /* Report Size (1), */
        0x09, 0xB6, /* Usage (Scan Previous Track), */
        0x09, 0xB5, /* Usage (Scan Next Track), */
        0x09, 0xB7, /* Usage (Stop), */
        0x09, 0xCD, /* Usage (Play/Pause), */
        0x09, 0xE2, /* Usage (Mute), */
        0x09, 0xE9, /* Usage (Volume Increment) */
        0x09, 0xEA, /* Usage (Volume Decrement) */
        0x81, 0x02, /* Input (Data, Variable, Absolute), Bit Field */
        0x95, 0x01, /* Report Count (1), */
        0x75, 0x01, /* Report Size (1), */
        0x81, 0x03, /* Input (Data, Variable, Absolute), Bit Field */
        0xC0, /* End Collection, */
        0x05, 0x01, /* Usage Page (Generic Desktop), */
        0x09, 0x02, /* Usage (Mouse), */
        0xA1, 0x01, /* Collection (Application), */
        0x85, 0x02, /* Report_ID(2), */
        0x09, 0x01, /* Usage (Pointer), */
        0xA1, 0x00, /* Collection (Physical), */
        0x05, 0x09, /* Usage Page (Button), */
        0x19, 0x01, /* Usage Minimum (Button 01), */
        0x29, 0x05, /* Usage Maximum (Button 05), */
        0x15, 0x00, /* Logical Minimum (0), */
        0x25, 0x01, /* Logical Maximum (1), */
        0x95, 0x05, /* Report Count (5), */
        0x75, 0x01, /* Report Size (1), */
        0x81, 0x02, /* Input (Data, Variable, Absolute), Bit Field */
        0x95, 0x01, /* Report Count (1), */
        0x75, 0x03, /* Report Size (3), */
        0x81, 0x03, /* Input (Data, Variable, Absolute), Bit Field */
        0x05, 0x01, /* Usage Page (Generic Desktop), */
        0x09, 0x30, /* Usage (X), */
        0x09, 0x31, /* Usage (Y), */
        0x09, 0x38, /* Usage (Wheel), */
        0x15, 0xD8, /* Logical Minimum (-40), */
        0x25, 0x32, /* Logical Maximum (50), */
        0x75, 0x08, /* Report Size (8), */
        0x95, 0x03, /* Report Count (3), */
        0x81, 0x06, /* Input (Data, Variable, Absolute), Bit Field */
        0xC0, /* End Collection, */
        0xC0, /* End Collection, */
        0x05, 0x0D, /* Usage Page (Digitizer), */
        0x09, 0x04, /* Usage (Touch Screen), */
        0xA1, 0x01, /* Collection (Application), */
        0x85, 0xAA, /* Report_ID(170), */
        0x09, 0x22, /* Usage (Finger), */
        0xA1, 0x00, /* Collection (Physical), */
        0x09, 0x42, /* Usage (Tip Switch), */
        0x15, 0x00, /* Logical Minimum (0), */
        0x25, 0x01, /* Logical Maximum (1), */
        0x75, 0x01, /* Report Size (1), */
        0x95, 0x01, /* Report Count (1), */
        0x81, 0x02, /* Input (Data, Variable, Absolute), Bit Field */
        0x09, 0x32, /* Usage (In Range), */
        0x15, 0x00, /* Logical Minimum (0), */
        0x25, 0x01, /* Logical Maximum (1), */
        0x81, 0x02, /* Input (Data, Variable, Absolute), Bit Field */
        0x09, 0x51, /* Usage (Contact identifier), */
        0x75, 0x05, /* Report Size (5), */
        0x95, 0x01, /* Report Count (1), */
        0x16, 0x00, 0x00, /* Logical Minimum (0), */
        0x26, 0x10, 0x00, /* Logical Maximum (16), */
        0x81, 0x02, /* Input (Data, Variable, Absolute), Bit Field */
        0x09, 0x47, /* Usage (Touch Valid), */
        0x75, 0x01, /* Report Size (1), */
        0x95, 0x01, /* Report Count (1), */
        0x15, 0x00, /* Logical Minimum (0), */
        0x25, 0x01, /* Logical Maximum (1), */
        0x81, 0x02, /* Input (Data, Variable, Absolute), Bit Field */
        0x05, 0x01, /* Usage Page (Generic Desktop), */
        0x09, 0x30, /* Usage (X), */
        0x75, 0x10, /* Report Size (16), */
        0x95, 0x01, /* Report Count (1), */
        0x55, 0x0D, /* Unit Exponent (13), */
        0x65, 0x33, /* Unit (English Linear: inch=3, Slug=0, Second=0, Fahrenheit=0, Ampere=0, Candela=0), */
        0x35, 0x00, /* Physical Minimum (0), */
        0x46, 0x37, 0x04, /* Physical Maximum (1079), */
        0x26, 0x37, 0x04, /* Logical Maximum (1079), */
        0x81, 0x02, /* Input (Data, Variable, Absolute), Bit Field */
        0x09, 0x31, /* Usage (Y), */
        0x75, 0x10, /* Report Size (16), */
        0x95, 0x01, /* Report Count (1), */
        0x55, 0x0D, /* Unit Exponent (13), */
        0x65, 0x33, /* Unit (English Linear: inch=3, Slug=0, Second=0, Fahrenheit=0, Ampere=0, Candela=0), */
        0x35, 0x00, /* Physical Minimum (0), */
        0x46, 0x5F, 0x09, /* Physical Maximum (2399), */
        0x26, 0x5F, 0x09, /* Logical Maximum (2399), */
        0x81, 0x02, /* Input (Data, Variable, Absolute), Bit Field */
        0x05, 0x0D, /* Usage Page (Digitizer), */
        0x09, 0x55, /* Usage (Contact count mamimum), */
        0x25, 0x08, /* Logical Maximum (8), */
        0x75, 0x08, /* Report Size (8), */
        0x95, 0x01, /* Report Count (1), */
        0xB1, 0x02, /* Fearure (Data, Value, Absolute, Non-volatile, Bit Field), */
        0xC0, /* End Collection */
        0xC0, /* End Collection */
    };

    enum class BrHidControlAction {
        BR_HID_CONTROL_ACTION_CMD_VOLUME_INCREMENT = 0,
        BR_HID_CONTROL_ACTION_CMD_VOLUME_DECREMENT,
        BR_HID_CONTROL_ACTION_CMD_SLIDE_UP,
        BR_HID_CONTROL_ACTION_CMD_SLIDE_DOWN,
        BR_HID_CONTROL_ACTION_CMD_SLIDE_RIGHT,
        BR_HID_CONTROL_ACTION_CMD_SLIDE_LEFT,
        BR_HID_CONTROL_ACTION_CMD_DOUBLE_CLICK,
        BR_HID_CONTROL_ACTION_CMD_SINGLE_CLICK,
        BR_HID_CONTROL_ACTION_CMD_MAX,
    };

    enum class HidConsumerControlUsageBitField {
        HID_CONSUMER_CONTROL_USAGE_BIT_SCAN_PREVIOUS_TRACK = 0,
        HID_CONSUMER_CONTROL_USAGE_BIT_SCAN_NEXT_TRACK,
        HID_CONSUMER_CONTROL_USAGE_BIT_STOP,
        HID_CONSUMER_CONTROL_USAGE_BIT_PLAY_PAUSE,
        HID_CONSUMER_CONTROL_USAGE_BIT_MUTE,
        HID_CONSUMER_CONTROL_USAGE_BIT_VOLUME_INCREMENT,
        HID_CONSUMER_CONTROL_USAGE_BIT_VOLUME_DECREMENT,
    };

    typedef struct {
        uint8_t tipSwitch : 1;
        uint8_t inRange : 1;
        uint8_t contactIdentifier : 5;
        uint8_t touchValid : 1;
        uint16_t x;
        uint16_t y;
    } __attribute__((packed)) HidTouchScreenUsAge;

    typedef struct {
        uint8_t reportId;
        HidTouchScreenUsAge hidTouchScreenUsAgeData;
    } HidTouchScreen;

    typedef struct {
        uint8_t invalidData : 1;
        uint8_t dataType : 7;
    } __attribute__((packed)) HidConsumerControlUsAge;

    typedef struct {
        uint8_t reportId;
        HidConsumerControlUsAge hidConsumerControlUsAgeData;
    } HidConsumerControl;

    enum BluetoothSwitchStatus {
        BLUETOOTH_SWITCH_STATE_OFF = 0,
        BLUETOOTH_SWITCH_STATE_ON,
        BLUETOOTH_SWITCH_STATE_HALF,
    };

    constexpr const char* NAME = "HID Keyboard";
    constexpr const char* DESCRIPTION = "HID";
    constexpr const char* PROVIDER = "Huawei";
    constexpr const uint16_t BAND_MODE = 0;
    constexpr const uint16_t HEADPHONE_MODE = 1;
    constexpr const uint16_t WAKE_TIME = 6000;
    constexpr const int DEFAULT_VAL = 0;
    constexpr const int COMPARA_DEFAULT_VAL = -1;
    constexpr const uint8_t g_subclass = 0x80;  // SUBCLASS1_MOUSE
    constexpr const uint16_t g_accessLatency = 11250;
    constexpr const int32_t WATCH_MCU_PLAY_ID = 0x7FFFFFFF;
    constexpr const uint8_t HW_A2DP_OFFLOAD_HARDWARE_ENCODING = 2;
    constexpr const char *g_bluetoothSwitchStateProperty = "persist.bluetooth.switch_enable";
    constexpr const char *g_bluetoothWatchStrModeEnable = "persist.bluetooth.watch_str_enable";
    constexpr const char *g_bluetoothHalfOpenProperty = "persist.bluetooth.watch_half_property";
    constexpr const char *COLLABORATION_SERVICE = "persist.bluetooth.collaboration_service";
    constexpr const char *g_watchStrMode = "persist.watch_str_mode";
    constexpr const char *g_stateOff = "0";
    constexpr const char *g_stateOn = "1";
    constexpr const char *g_stateHalf = "2";
    constexpr const uint8_t MCU_START_PLAY = 0;
    constexpr const uint8_t MCU_STOP_PLAY = 1;
    constexpr const uint8_t MCU_CHECK_DEVICE = 3;
    constexpr const char* HID_MAP_UPDATE = "persist.bluetooth.watch_hid_map_update_state";

    constexpr const uint8_t HID_REPORT_ID = 0xAA;
    constexpr const uint8_t HID_CONSUMER_CONTROL_REPORT_ID = 0x03;
    constexpr const uint16_t HID_SLIDE_END_AXIS_Y = 1000;
    constexpr const uint8_t HID_CLICK_EVENT_TIMES = 2;
    constexpr const uint16_t HID_PLAY_PAUSE_AXIS = 500;
    constexpr const uint8_t BR_HID_RECONNECTED_COUNT_MAX = 5;
}  // namespace bluetooth
}
#endif  // WATCH_DEFINES_H