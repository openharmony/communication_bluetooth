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

#ifndef PAN_DEFINES_H
#define PAN_DEFINES_H

#include <cstdint>
#include <string>

#include "log.h"

namespace OHOS {
namespace Bluetooth {
constexpr int PAN_SUCCESS = 0;
constexpr int PAN_FAILURE = 1;

constexpr int PAN_STATE_DISCONNECTED = 0;
constexpr int PAN_STATE_CONNECTING = 1;
constexpr int PAN_STATE_DISCONNECTING = 2;
constexpr int PAN_STATE_CONNECTED = 3;
constexpr int PAN_STATE_NETWORK_ESTABLISHED = 4;

constexpr int PAN_SERVICE_STARTUP_EVT = 1;
constexpr int PAN_SERVICE_SHUTDOWN_EVT = 2;
constexpr int PAN_DISCONNECT_EVT = 3;
constexpr int PAN_CONTROL_STATE_CHANGE_EVT = 4;
constexpr int PAN_DISCONNECT_ALL_EVT = 5;
constexpr int PAN_DISCONNECTED_IND_EVT = 6;
constexpr int PAN_CONNECTING_IND_EVT = 7;
constexpr int PAN_CONNECTED_IND_EVT = 8;
constexpr int PAN_DISCONNECTING_IND_EVT = 9;
constexpr int PAN_REMOVE_STATE_MACHINE_EVT = 10;
constexpr int PAN_CONNECT_EVT = 11;
constexpr int PAN_CONNECT_TIMEOUT_EVT = 12;
constexpr int PAN_GET_IP_SUCCESS_EVT = 13;
constexpr int PAN_GET_IP_FAIL_EVT = 14;

const char PAN_NETWORK_IPV4_ADDRESS[] = { "192.168.44.1" };
constexpr int PAN_NETWORK_IPV4_PREFIX_LENGTH = 24;
constexpr int MAX_MASK_LENGTH = 32;

constexpr int INVALID_ROLE = 0;
constexpr int LOCAL_NAP_ROLE = 1;
constexpr int LOCAL_PANU_ROLE = 2;
constexpr int REMOTE_NAP_ROLE = 1;
constexpr int REMOTE_PANU_ROLE = 2;

constexpr int IPV4 = 0;
constexpr int IPV6 = 1;

constexpr int LINK_UP_BAND_WIDTH_KBPS = 24 * 1000;
constexpr int LINK_DOWN_BAND_WIDTH_KBPS = 24 * 1000;
constexpr int NET_SCORE = 69;
constexpr int MAX_PREFIX_LENGTH = 32;
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PAN_DEFINES_H