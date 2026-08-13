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

#ifndef BAS_DEFINES_H
#define BAS_DEFINES_H

#include <cstdint>
#include <string>

#include "gatt_data.h"

namespace OHOS {
namespace bluetooth {
constexpr int BAS_STATE_DISCONNECTED = 0;
constexpr int BAS_STATE_CONNECTING = 1;
constexpr int BAS_STATE_DISCONNECTING = 2;
constexpr int BAS_STATE_CONNECTED = 3;

constexpr int BAS_SUCCESS = 0;
constexpr int BAS_FAILURE = 1;

constexpr int BAS_DEVICE_TYPE_UNKNOWN = 0;
constexpr int BAS_DEVICE_TYPE_BREDR = 1;
constexpr int BAS_DEVICE_TYPE_BLE = 2;

constexpr int BAS_INVALID_EVT = 0;
constexpr int BAS_SERVICE_STARTUP_EVT = 1;
constexpr int BAS_SERVICE_SHUTDOWN_EVT = 2;
constexpr int BAS_CONNECT_EVT = 3;
constexpr int BAS_DISCONNECT_EVT = 4;
constexpr int BAS_CONNECTION_STATE_CHANGED_EVT = 5;
constexpr int BAS_REMOVE_STATE_MACHINE_EVT = 6;
constexpr int BAS_CONNECTION_TIMEOUT_EVT = 7;
constexpr int BAS_DISCONNECTION_TIMEOUT_EVT = 8;
constexpr int BAS_READ_BATTERY_EVT = 9;
constexpr int BAS_SERVICE_DISCOVERED_EVT = 10;
constexpr int BAS_BATTERY_LEVEL_READ_EVT = 11;
constexpr int BAS_BATTERY_LEVEL_CHANGED_EVT = 12;
constexpr int BAS_INVALID_BATTERY_LEVEL = -1;
constexpr int BAS_MAX_BATTERY_LEVEL = 100;

struct BasBatteryData {
    int batteryLevel = BAS_INVALID_BATTERY_LEVEL;
};

}  // namespace bluetooth
}
#endif  // BAS_DEFINES_H