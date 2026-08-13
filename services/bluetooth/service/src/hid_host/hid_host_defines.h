/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef HID_HOST_DEFINES_H
#define HID_HOST_DEFINES_H

#include <cstdint>
#include <string>

#include "gatt_data.h"

namespace OHOS {
namespace bluetooth {
constexpr int HID_HOST_STATE_DISCONNECTED = 0;
constexpr int HID_HOST_STATE_CONNECTING = 1;
constexpr int HID_HOST_STATE_DISCONNECTING = 2;
constexpr int HID_HOST_STATE_CONNECTED = 3;

constexpr int HID_HOST_SUCCESS = 0;
constexpr int HID_HOST_FAILURE = 1;

constexpr int HID_HOST_DEVICE_TYPE_UNKNOWN = 0;
constexpr int HID_HOST_DEVICE_TYPE_BREDR = 1;
constexpr int HID_HOST_DEVICE_TYPE_BLE = 2;

// Hid host event
constexpr int HID_HOST_INVALID_EVT = 0;
// Service start/stop
constexpr int HID_HOST_SERVICE_STARTUP_EVT = 1;
constexpr int HID_HOST_SERVICE_SHUTDOWN_EVT = 2;
// Device open/close
constexpr int HID_HOST_CONNECT_EVT = 3;
constexpr int HID_HOST_DISCONNECT_EVT = 4;
constexpr int HID_HOST_CONNECTION_STATE_CHANGED_EVT = 5;
constexpr int HID_HOST_VC_UNPLUG_EVT = 6;
constexpr int HID_HOST_SEND_DATA_EVT = 7;
constexpr int HID_HOST_SET_REPORT_EVT = 8;
constexpr int HID_HOST_GET_REPORT_EVT = 9;
constexpr int HID_HOST_INT_HANDSK_EVT = 10;

// Remove state machine
constexpr int HID_HOST_REMOVE_STATE_MACHINE_EVT = 13;
// Connection/disconnection timeout
constexpr int HID_HOST_CONNECTION_TIMEOUT_EVT = 14;
constexpr int HID_HOST_DISCONNECTION_TIMEOUT_EVT = 15;

struct SendHidData {
    uint8_t type = 0;
    uint8_t param = 0;
    uint16_t dataSize = 0;
    uint8_t reportId = 0;
};

}  // namespace bluetooth
}
#endif  // HID_HOST_DEFINES_H
