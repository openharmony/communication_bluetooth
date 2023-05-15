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

#include "bluetooth_common_event_helper.h"
#include "common_event.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "log.h"

using namespace OHOS::EventFwk;

namespace OHOS {
namespace BluetoothHelper {
bool BluetoothCommonEventHelper::PublishEvent(const std::string &eventAction,int code,
    const std::vector<std::string> &permissions)
{
    Want want;
    want.SetAction(eventAction);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    if (permissions.size() > 0) {
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissions);
        if (!CommonEventManager::PublishCommonEvent(commonData, publishInfo)) {
            HILOGE("failed to send common event with permission!");
            return false;
        }
        HILOGI("send common event with permission!");
        return true;
    }
    if (!CommonEventManager::PublishCommonEvent(commonData)) {
        HILOGE("failed to send common event!");
        return false;
    }
    return true;
}

bool BluetoothCommonEventHelper::PublishBluetoothStateChangeEvent(int code,
    const std::vector<std::string> &permissions)
{
    return BluetoothCommonEventHelper::PublishEvent(COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE , code, permissions);
}
}
}


