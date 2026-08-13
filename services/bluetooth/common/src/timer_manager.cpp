/*
 * Copyright (C) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_common_timer"
#endif

#include "btcommon/timer_manager.h"
#include "bluetooth_log.h"
#include "securec.h"
#include "common_timer_errors.h"
#include "timer.h"

namespace utility {
class TimerManager {
public:
    TimerManager() : timerManager_("bt_service_timer", -1)
    {
        timerManager_.Setup();
    }
    ~TimerManager()
    {
        timerManager_.Shutdown();
    }

    static TimerManager &GetInstance()
    {
        static TimerManager instance;
        return instance;
    }

    void ShutDown(void)
    {
        timerManager_.Shutdown();
    }

private:
    friend class utility::Timer;

    OHOS::Utils::Timer timerManager_;
};

void CloseAllTimer(void)
{
    TimerManager.GetInstance().ShutDown();
}

Timer::Timer(const std::function<void()> &callback) : isPeriodic_(false), timerId_(INVALID_TIMER_ID)
{
    callback_ = [this, callback]() {
        // Clear timerId if timer trigger and the timer is not periodic timer.
        if (!isPeriodic_) {
            timerId_ = INVALID_TIMER_ID;
        }
        callback();
    };
}

Timer::~Timer()
{
    Stop();
}

bool Timer::Start(int ms, bool isPeriodic)
{
    if (IsStarted()) {
        HILOGD("btServiceTimer(%{public}u) is started", timerId_.load());
        return false;
    }

    uint32_t ret = TimerManager::GetInstance().timerManager_.Register(callback_, ms, !isPeriodic);
    if (ret == OHOS::Utils::TIMER_ERR_DEAL_FAILED) {
        return false;
    }

    // If Register success, 'ret' is 'timerId'.
    HILOGD("btServiceTimer(%{public}u) start", ret);
    timerId_ = ret;
    isPeriodic_ = isPeriodic;
    return true;
}

void Timer::Stop()
{
    uint32_t timerId = timerId_.load();
    if (timerId == INVALID_TIMER_ID) {
        HILOGD("btServiceTimer(%{public}u) has stoped", timerId_.load());
        return;
    }
    timerId_ = INVALID_TIMER_ID;  // Clear timerId_ means the timer is stooped.

    if (timerId != INVALID_TIMER_ID) {
        HILOGD("btServiceTimer(%{public}u) stop", timerId);
        TimerManager::GetInstance().timerManager_.Unregister(timerId);
    }
}

bool Timer::IsStarted(void)
{
    return timerId_.load() != INVALID_TIMER_ID;
}
}  // namespace utility