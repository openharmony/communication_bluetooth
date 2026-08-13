/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef COMMON_TIMER_H
#define COMMON_TIMER_H

#include "../../../service/src/base/base_def.h"
#include <atomic>
#include <mutex>
#include <functional>

namespace utility {
class Timer {
public:
    /**
     * @brief Construct a new Timer object
     *
     * @param callback Timer's callback function.
     * @since 6
     */
    explicit Timer(const std::function<void()> &callback);

    /**
     * @brief Destroy the Timer object
     *
     * @since 6
     */
    ~Timer();

    /**
     * @brief Start Running Timer.
     *
     * @param ms Countdown time.
     * @param isPeriodic Timer isPeriodic.
     * @return Success set timer return true, else return false.
     * @since 6
     */
    bool Start(int ms, bool isPeriodic = false);

    /**
     * @brief Stop Running Timer.
     *
     * @since 6
     */
    void Stop();

    /**
     * @brief: Check whether the timer is started.
     * @return true if the timer is started, else return false.
     */
    bool IsStarted(void);

private:
    std::atomic_bool isPeriodic_ {false};
    std::function<void()> callback_ {};

    const uint64_t INVALID_TIMER_ID = 0;
    uint64_t timerId_ {INVALID_TIMER_ID};

    BT_DISALLOW_COPY_AND_ASSIGN(Timer);
};

/**
 * @brief 关闭所有定时器
 * @details 此函数用于关闭所有正在运行的定时器，仅用于DT测试用例使用
 */
void CloseAllTimer(void);
}  // namespace utility

#endif  // COMMON_TIMER_H
