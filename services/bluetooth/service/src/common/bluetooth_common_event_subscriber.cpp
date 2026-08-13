/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_event_subscriber"
#endif

#include "bluetooth_common_event_subscriber.h"
#ifdef EDM_SERVICE_ENABLE
#include "bluetooth_edm_manager.h"
#endif
#ifdef FEATURE_MOUSE_FAST_CONNECTION
#include "pc_power_manager.h"
#endif

#include "log.h"
#include "common_util.h"
#include "hitrace_meter.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
#ifdef EDM_SERVICE_ENABLE
static const char *const BLUETOOTH_WHITELIST_CHANGED_EVENT = "com.ohos.edm.bluetoothdeviceschanged";
static const char *const BLUETOOTH_DENYLIST_CHANGED_EVENT = "com.ohos.edm.disallowedbluetoothdeviceschanged";
static const char *const BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT = "usual.event.EDM_CONFIG_CHANGED";
#endif

void BluetoothCommonEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto want = data.GetWant();
    std::string action = want.GetAction();
#ifdef EDM_SERVICE_ENABLE
    if ((action == BLUETOOTH_WHITELIST_CHANGED_EVENT) || (action == BLUETOOTH_DENYLIST_CHANGED_EVENT) ||
        (action == BLUETOOTH_ACCOUNT_DENYLIST_CHANGED_EVENT)) {
        BluetoothEdmManager::GetInstance().OnEdmListChanged(action);
    }
#endif
#ifdef FEATURE_MOUSE_FAST_CONNECTION
    #ifdef PC_FEATURE_FORCESLEEP_SUPPORT
        // pc逻辑
        if (action == COMMON_EVENT_POWER_MANAGER_STATE_CHANGED) {
            std::string eventData = data.GetData();
            PowerMgrHelper::GetInstance()->OnPowerStateChanged(data.GetCode(), eventData);
        }
    #endif
    // pc&tablet逻辑
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED ||
        action == EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED) {
        PowerMgrHelper::GetInstance()->OnPowerConnectStateChanged(action);
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        PowerMgrHelper::GetInstance()->OnScreenStateChanged(action);
    }
#endif
}

void BluetoothCommonEventSubscriberEx::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action_ == action) {
        HILOGI("BluetoothCommonEvent::OnReceiveEvent %{public}s", action.c_str());
        callback_(data);
    }
}

}  // namespace bluetooth
}  // namespace OHOS