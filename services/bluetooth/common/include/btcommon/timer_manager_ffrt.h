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

#ifndef TIMER_MANAGER_FFRT_H
#define TIMER_MANAGER_FFRT_H

#include <atomic>
#include "safe_map.h"
#include "ffrt_inner.h"

namespace utility {
class TimerManager {
public:
    static TimerManager *GetInstance();

    uint64_t StartTimer(std::function<void(void)> func, int delayMs, bool isPeriodic);
    void StopTimer(uint64_t timerId);
    bool IsTimerStarted(uint64_t timerId);
    void ShutDown(void);

private:
    TimerManager();
    ~TimerManager() = default;

    bool StartTimerInner(uint64_t timerId, std::function<void(void)> func, int delayMs, bool isPeriodic);

    const uint16_t MILLISEC_2_MICROSEC = 1000;
    ffrt::mutex queueMutex_ {};
    std::unique_ptr<ffrt::queue> queue_ { nullptr };  // locked by queueMutex_

    const uint64_t INVALID_TIMER_ID = 0;
    std::atomic_uint64_t timerCount_ = 1;
    OHOS::SafeMap<uint64_t, std::shared_ptr<ffrt::task_handle>> taskHandleMap_ {};
};
}  // namespace utility

#endif  // TIMER_MANAGER_FFRT_H