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
#define LOG_TAG "bt_common_timer"
#endif

#include "btcommon/timer_manager.h"
#include "btcommon/timer_manager_ffrt.h"

#include "bluetooth_log.h"
#include "securec.h"
#include "ffrt_inner.h"
#include "safe_map.h"

namespace utility {
TimerManager::TimerManager()
{
    queue_ = std::make_unique<ffrt::queue>("bt_service_timer");
}

TimerManager *TimerManager::GetInstance()
{
    static TimerManager instance;
    return &instance;
}

bool TimerManager::StartTimerInner(uint64_t timerId, std::function<void(void)> func, int delayMs, bool isPeriodic)
{
    auto ffrtFunc = [this, timerId, func, delayMs, isPeriodic]() {
        do {
            std::shared_ptr<ffrt::task_handle> handlePtr = nullptr;
            if (!taskHandleMap_.Find(timerId, handlePtr)) {
                return;
            }
            // 处理周期定时器
            if (isPeriodic && StartTimerInner(timerId, func, delayMs, isPeriodic)) {
                break;
            }
            // 单次定时器触发超时后，清理资源
            taskHandleMap_.Erase(timerId);
        } while (0);
        // 执行定时器超时函数
        func();
    };

    std::lock_guard<ffrt::mutex> lock(queueMutex_);
    ffrt::task_attr taskAttr;
    taskAttr.name("bt_service_timer").delay(static_cast<uint64_t>(delayMs) * MILLISEC_2_MICROSEC);
    if (queue_) {
        auto handle = queue_->submit_h(ffrtFunc, taskAttr);
        if (handle == nullptr) {
            HILOGE("ffrt queue submit failed");
            return false;
        }
        auto handlePtr = std::make_shared<ffrt::task_handle>(std::move(handle));
        taskHandleMap_.EnsureInsert(timerId, handlePtr);
    }
    return true;
}

uint64_t TimerManager::StartTimer(std::function<void(void)> func, int delayMs, bool isPeriodic)
{
    uint64_t timerId = timerCount_++;
    bool ret = StartTimerInner(timerId, func, delayMs, isPeriodic);
    return ret ? timerId : INVALID_TIMER_ID;
}

void TimerManager::StopTimer(uint64_t timerId)
{
    std::lock_guard<ffrt::mutex> lock(queueMutex_);
    std::shared_ptr<ffrt::task_handle> handlePtr = nullptr;
    taskHandleMap_.Find(timerId, handlePtr);
    if (queue_ && handlePtr) {
        queue_->cancel(*handlePtr);
    }
    taskHandleMap_.Erase(timerId);
}

bool TimerManager::IsTimerStarted(uint64_t timerId)
{
    std::shared_ptr<ffrt::task_handle> handlePtr = nullptr;
    return taskHandleMap_.Find(timerId, handlePtr) && handlePtr != nullptr;
}

void TimerManager::ShutDown(void)
{
    std::lock_guard<ffrt::mutex> lock(queueMutex_);
    taskHandleMap_.Clear();
    queue_ = nullptr;
}

void CloseAllTimer(void)
{
    TimerManager::GetInstance()->ShutDown();
}

Timer::Timer(const std::function<void()> &callback) : isPeriodic_(false), callback_(callback)
{}

Timer::~Timer()
{
    Stop();
}

bool Timer::Start(int ms, bool isPeriodic)
{
    if (IsStarted()) {
        HILOGD("timer is started");
        return false;
    }

    timerId_ = TimerManager::GetInstance()->StartTimer(callback_, ms, isPeriodic);
    if (timerId_ == INVALID_TIMER_ID) {
        HILOGE("start timer failed");
        return false;
    }
    return true;
}

void Timer::Stop()
{
    if (!IsStarted()) {
        HILOGD("timer is stopped");
        return;
    }

    TimerManager::GetInstance()->StopTimer(timerId_);
}

bool Timer::IsStarted(void)
{
    return TimerManager::GetInstance()->IsTimerStarted(timerId_);
}
}  // namespace utility