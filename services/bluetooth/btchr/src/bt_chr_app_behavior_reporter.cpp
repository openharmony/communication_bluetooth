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

#ifndef LOG_TAG
#define LOG_TAG "bt_chr_app_behavior_reporter"
#endif
#include "bt_chr_app_behavior_reporter.h"
#include "battery_srv_client.h"
#include "hisysevent.h"
#include "power_mgr_client.h"
#include "bt_chr_ue_manager.h"

namespace OHOS {
namespace bluetooth {

void AppBehaviorCommonEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    auto want = data.GetWant();
    std::string action = want.GetAction();
    auto& reporter = BtChrAppBehaviorReporter::GetInstance();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        reporter.HandleScreenStateChange(true);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF) {
        reporter.HandleScreenStateChange(false);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING) {
        reporter.HandleChargingStateChange(true);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING) {
        reporter.HandleChargingStateChange(false);
    }
}

static constexpr int64_t DELAY_INTERVAL = 30 * 60 * 1000; // 30min

bool IsSameAppBehavior(const AppBehaviorDataInternal& lhs, const AppBehaviorData& rhs) {
    return lhs.appName == rhs.appName &&
           lhs.appUid == rhs.appUid &&
           lhs.appPid == rhs.appPid &&
           lhs.businessType == rhs.businessType;
}

BtChrAppBehaviorReporter::~BtChrAppBehaviorReporter() {
    UnRegisterCommonEvent();
    if (reportTimer_ != nullptr) {
        reportTimer_->Stop();
        reportTimer_ = nullptr;
    }
}

BtChrAppBehaviorReporter& BtChrAppBehaviorReporter::GetInstance() {
    static BtChrAppBehaviorReporter instance;
    return instance;
}

void BtChrAppBehaviorReporter::CheckPeriodical() {
    if (behaviorDataQueue_.empty()) {
        return;
    }
    std::vector<AppBehaviorDataInternal> dataQueue;
    {
        std::lock_guard<std::mutex> lock(dataQueueMutex_);
        dataQueue = std::move(behaviorDataQueue_);
        behaviorDataQueue_.clear();
    }
    BtChrUeManager::GetInstance()->WriteAppWakeupUe(dataQueue);
}

bool BtChrAppBehaviorReporter::ReportAppBehavior(const AppBehaviorData& data) {
    {
        std::lock_guard<std::mutex> lock(reportMutex_);
        if (!reportTimer_) {
            reportTimer_ = std::make_shared<utility::Timer>(
                std::bind(&BtChrAppBehaviorReporter::CheckPeriodical, this));
            reportTimer_->Start(DELAY_INTERVAL, true);
        }
        if (subscriber_ == nullptr) {
            InitScreenAndChargingStatus();
            RegisterCommonEvent();
        }
    }
    {
        std::lock_guard<std::mutex> lock(dataMutex_);
        if (isScreenOn_ || isCharging_) {
            return false;
        }
    }
    std::lock_guard<std::mutex> lock(dataQueueMutex_);

    for (auto& queuedData : behaviorDataQueue_) {
        if (IsSameAppBehavior(queuedData, data)) {
            queuedData.triggerCount++;
            return true;
        }
    }

    AppBehaviorDataInternal newData;
    newData.appName = data.appName;
    newData.appUid = data.appUid;
    newData.appPid = data.appPid;
    newData.businessType = data.businessType;
    newData.triggerCount = 1;
    behaviorDataQueue_.push_back(newData);
    return true;
}

void BtChrAppBehaviorReporter::InitScreenAndChargingStatus()
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    isScreenOn_ = OHOS::PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
    auto& batterySrvClient = OHOS::PowerMgr::BatterySrvClient::GetInstance();
    PowerMgr::BatteryChargeState chargeState = batterySrvClient.GetChargingStatus();
    isCharging_ = (chargeState == PowerMgr::BatteryChargeState::CHARGE_STATE_ENABLE);
}

void BtChrAppBehaviorReporter::RegisterCommonEvent()
{
    if (subscriber_ != nullptr) {
        return;
    }
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_CHARGING);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_DISCHARGING);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscriber_ = std::make_shared<AppBehaviorCommonEventSubscriber>(subscribeInfo);
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void BtChrAppBehaviorReporter::UnRegisterCommonEvent()
{
    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
}

void BtChrAppBehaviorReporter::HandleScreenStateChange(bool isScreenOn)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    isScreenOn_ = isScreenOn;
}

void BtChrAppBehaviorReporter::HandleChargingStateChange(bool isCharging)
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    isCharging_ = isCharging;
}

} // namespace bluetooth
} // namespace OHOS
