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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KSTACK, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HEARING_AID_DEFINES_H
#define HEARING_AID_DEFINES_H

#include <cstdint>
#include <string>

#include "log.h"

namespace OHOS {
namespace bluetooth {
constexpr int HEARING_AID_SUCCESS = 0;
constexpr int HEARING_AID_FAILURE = 1;
constexpr uint64_t HI_SYNC_ID_INVALID = 0;

enum HearingAidConnectState {
    HEARING_AID_STATE_DISCONNECTED = 0,
    HEARING_AID_STATE_CONNECTING,
    HEARING_AID_STATE_CONNECTED,
    HEARING_AID_STATE_DISCONNECTING
};

// state from bt stack callback
enum BTHearingAidStateConnectionState {
    BTHEARING_AID_STATE_DISCONNECTED = 0,
    BTHEARING_AID_STATE_CONNECTING,
    BTHEARING_AID_STATE_CONNECTED,
    BTHEARING_AID_STATE_DISCONNECTING,
    BTHEARING_AID_STATE_MAX
};

enum HearingAidEvent {
    HEARING_AID_SERVICE_STARTUP_EVT = 1,
    HEARING_AID_SERVICE_SHUTDOWN_EVT,
    HEARING_AID_SERVICE_SHUTDOWN_DONE_EVT,
    HEARING_AID_DISCONNECT_EVT,
    HEARING_AID_CONNECT_EVT,
    HEARING_AID_STACK_STATE_EVT,
    HEARING_AID_REMOVE_STATE_MACHINE_EVT,
    HEARING_AID_CONNECT_TIMEOUT_EVT,
    HEARING_AID_SET_VOLUME_BY_ADDR,
    HEARING_AID_SET_ACTIVE_DEVICE,
    HEARING_AID_DISABLE_EVT
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // HEARING_AID_DEFINES_H