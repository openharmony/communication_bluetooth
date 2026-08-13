/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

 #ifndef HID_DEVICE_DEFINES_H
#define HID_DEVICE_DEFINES_H

#include <cstdint>
#include <string>


namespace OHOS {
namespace bluetooth {
constexpr int HID_DEVICE_APP_STATE_NOT_REGISTERED = 0;
constexpr int HID_DEVICE_APP_STATE_REGISTERED = 1;

// Hid device event
constexpr int HID_DEVICE_INVALID_EVT = 0;
constexpr int HID_DEVICE_SERVICE_STARTUP_EVT = 1;
constexpr int HID_DEVICE_SERVICE_SHUTDOWN_EVT = 2;
constexpr int HID_DEVICE_CONNECT_EVT = 3;
constexpr int HID_DEVICE_DISCONNECT_EVT = 4;
constexpr int HID_DEVICE_SEND_REPORT_EVT = 5;
constexpr int HID_DEVICE_REPLY_REPORT_EVT = 6;
constexpr int HID_DEVICE_REPORT_ERROR_EVT = 7;
constexpr int HID_DEVICE_UNREGISTER_HID_DEVICE_EVT = 8;
constexpr int HID_DEVICE_AUTOUNREGISTER_HID_DEVICE_EVT = 9;
struct SendHidDeviceData {
    uint8_t type = 0;
    uint8_t param = 0;
    uint16_t dataSize = 0;
    uint8_t reportId = 0;
};

}  // namespace bluetooth
}
#endif  // HID_DEVICE_DEFINES_H
