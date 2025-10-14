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

#ifndef TAIHE_TIMER_H
#define TAIHE_TIMER_H

#include <memory>
#include "timer.h"

namespace OHOS {
namespace Bluetooth {
class TaiheTimer {
public:
    using TimerCallback = std::function<void ()>;
    static constexpr uint32_t DEFAULT_TIMEOUT = 10000; // 10s

    static TaiheTimer *GetInstance(void);

    TaiheTimer();
    ~TaiheTimer();

    int Register(const TimerCallback& callback, uint32_t &outTimerId, uint32_t interval = DEFAULT_TIMEOUT);
    void Unregister(uint32_t timerId);
private:
    std::unique_ptr<OHOS::Utils::Timer> timer_ {nullptr};
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif // TAIHE_TIMER_H