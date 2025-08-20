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

#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_event_module"
#endif

#include "taihe_event_module.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
int TaiheUtils::GetProfileConnectionState(int state)
{
    int32_t profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
    switch (state) {
        case static_cast<int32_t>(BTConnectState::CONNECTING):
            HILOGD("STATE_CONNECTING(1)");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::CONNECTED):
            HILOGD("STATE_CONNECTED(2)");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTED;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTING):
            HILOGD("STATE_DISCONNECTING(3)");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTED):
            HILOGD("STATE_DISCONNECTED(0)");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
            break;
        default:
            break;
    }
    return profileConnectionState;
}
} // namespace Bluetooth
} // namespace OHOS