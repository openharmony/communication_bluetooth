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
#ifndef BLUETOOTH_COMMON_EVENT_SUBSCRIDER_H
#define BLUETOOTH_COMMON_EVENT_SUBSCRIDER_H

#include <string>

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

namespace OHOS {
namespace bluetooth {

using CommonEventSubscriber = OHOS::EventFwk::CommonEventSubscriber;
using CommonEventData = OHOS::EventFwk::CommonEventData;
using CommonEventSubscribeInfo = OHOS::EventFwk::CommonEventSubscribeInfo;

class BluetoothCommonEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit BluetoothCommonEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo) {}
    ~BluetoothCommonEventSubscriber() {}

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;

private:
};

class BluetoothCommonEventSubscriberEx : public OHOS::EventFwk::CommonEventSubscriber {
public:
    using CommonEventCallback = std::function<void(const OHOS::EventFwk::CommonEventData &data)>;

    BluetoothCommonEventSubscriberEx(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo,
        std::string action, CommonEventCallback callback)
        : CommonEventSubscriber(subscriberInfo), action_(action), callback_(callback) {}
    ~BluetoothCommonEventSubscriberEx() {}

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;

private:
    std::string action_;
    CommonEventCallback callback_;
};
} // namespace bluetooth
} // namespace OHOS

#endif //BLUETOOTH_COMMON_EVENT_SUBSCRIDER_H