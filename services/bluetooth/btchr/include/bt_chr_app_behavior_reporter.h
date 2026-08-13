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

#ifndef BT_CHR_APP_BEHAVIOR_REPORTER_H
#define BT_CHR_APP_BEHAVIOR_REPORTER_H

#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <memory>
#include "btcommon/timer_manager.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

namespace OHOS {
namespace EventFwk {
class CommonEventData;
class CommonEventSubscribeInfo;
}

namespace bluetooth {

class AppBehaviorCommonEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit AppBehaviorCommonEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo) {}
    ~AppBehaviorCommonEventSubscriber() {}
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
};

struct AppBehaviorData {
    std::string appName;
    int64_t appUid;
    int64_t appPid;
    int64_t businessType;
};

struct AppBehaviorDataInternal {
    std::string appName;
    int64_t appUid;
    int64_t appPid;
    int64_t businessType;
    int64_t triggerCount;
};

enum AppBehaviorType {
    GATT_TRANSFER = 0,
    SPP_TRANSFOR = 1,
};

bool IsSameAppBehavior(const AppBehaviorDataInternal& lhs, const AppBehaviorData& rhs);

class BtChrAppBehaviorReporter {
public:
    static BtChrAppBehaviorReporter& GetInstance();

    bool ReportAppBehavior(const AppBehaviorData& data);

private:
    BtChrAppBehaviorReporter() = default;
    ~BtChrAppBehaviorReporter();
    BtChrAppBehaviorReporter(const BtChrAppBehaviorReporter&) = delete;
    BtChrAppBehaviorReporter& operator=(const BtChrAppBehaviorReporter&) = delete;

    void InitQueueTask();
    void CheckPeriodical();
    void RegisterCommonEvent();
    void UnRegisterCommonEvent();
    void InitScreenAndChargingStatus();
    void HandleScreenStateChange(bool isScreenOn);
    void HandleChargingStateChange(bool isCharging);

    friend class AppBehaviorCommonEventSubscriber;

    std::shared_ptr<utility::Timer> reportTimer_;
    std::mutex reportMutex_;
    std::mutex dataMutex_;

    std::mutex dataQueueMutex_;
    std::vector<AppBehaviorDataInternal> behaviorDataQueue_;

    std::shared_ptr<AppBehaviorCommonEventSubscriber> subscriber_ = nullptr;
    bool isScreenOn_ = false;
    bool isCharging_ = false;
};

} // namespace bluetooth
} // namespace OHOS

#endif // BT_CHR_APP_BEHAVIOR_REPORTER_H
