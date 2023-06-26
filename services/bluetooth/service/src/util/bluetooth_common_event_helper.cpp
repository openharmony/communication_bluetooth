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
#include "bt_def.h"
#include "log.h"

using namespace OHOS::EventFwk;

namespace OHOS {
namespace BluetoothHelper {
constexpr uint32_t FORMAT_PINCODE_LENGTH = 6;
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

bool BluetoothCommonEventHelper::PublishEvent(Want& want, const std::vector<std::string> &permissions)
{
    bool rtn;
    CommonEventData commonData;
    commonData.SetWant(want);

    if (permissions.size() > 0) {
        CommonEventPublishInfo publishInfo;
        publishInfo.SetSubscriberPermissions(permissions);
        rtn = CommonEventManager::PublishCommonEvent(commonData, publishInfo);
    } else {
        rtn = CommonEventManager::PublishCommonEvent(commonData);
    }
    
    HILOGI("send common event %{public}s, rtn:%{public}d", want.GetAction().c_str(), rtn);
    return rtn;
}

bool BluetoothCommonEventHelper::PublishBluetoothStateChangeEvent(int code,
    const std::vector<std::string> &permissions)
{
    return BluetoothCommonEventHelper::PublishEvent(COMMON_EVENT_BLUETOOTH_HOST_STATE_UPDATE, code, permissions);
}

void BluetoothCommonEventHelper::PublishPairReqEvent(const bluetooth::RawAddress& device, int reqType, int number)
{
    std::vector<std::string> permissions;
    permissions.emplace_back("ohos.permission.DISCOVER_BLUETOOTH");

    std::string pinCodeStr = std::to_string(number);
    if (reqType == bluetooth::PAIR_CONFIRM_TYPE_PASSKEY_DISPLAY || reqType == bluetooth::PAIR_CONFIRM_TYPE_NUMERIC) {
        while (pinCodeStr.length() < FORMAT_PINCODE_LENGTH) {
            pinCodeStr = "0" + pinCodeStr;
        }
    }

    Want want;
    want.SetAction(COMMON_EVENT_BLUETOOTH_REMOTEDEVICE_PAIRING_REQ);
    want.SetParam("deviceId", device.GetAddress());
    want.SetParam("pinType", reqType);
    want.SetParam("pinCode", pinCodeStr);
    
    BluetoothCommonEventHelper::PublishEvent(want, permissions);
}
}
}


