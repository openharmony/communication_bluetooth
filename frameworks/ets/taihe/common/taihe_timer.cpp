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
#define LOG_TAG "bt_taihe_timer"
#endif

#include "taihe_timer.h"

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "common_timer_errors.h"

namespace OHOS {
namespace Bluetooth {

TaiheTimer *TaiheTimer::GetInstance(void)
{
    static TaiheTimer instance;
    return &instance;
}

TaiheTimer::TaiheTimer() : timer_(std::make_unique<OHOS::Utils::Timer>("NapiBtTimer"))
{
    timer_->Setup();
}

TaiheTimer::~TaiheTimer()
{
    if (timer_) {
        timer_->Shutdown();
    }
}

int TaiheTimer::Register(const TimerCallback& callback, uint32_t &outTimerId, uint32_t interval)
{
    if (timer_ == nullptr) {
        HILOGE("timer_ is nulptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    uint32_t ret = timer_->Register(callback, interval, true); // only support once timer now
    if (ret == OHOS::Utils::TIMER_ERR_DEAL_FAILED) {
        HILOGE("Register timer failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    outTimerId = ret;
    HILOGI("timerId: %{public}u", outTimerId);
    return BT_NO_ERROR;
}

void TaiheTimer::Unregister(uint32_t timerId)
{
    if (timerId == 0) {
        HILOGE("timerId is 0, no registered timer");
        return;
    }

    HILOGI("timerId: %{public}d", timerId);
    if (timer_ == nullptr) {
        HILOGE("timer_ is nulptr");
        return;
    }

    timer_->Unregister(timerId);
}
}  // namespace Bluetooth
}  // namespace OHOS