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
#include "bluetooth_timer.h"
#include "common_timer_errors.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
BluetoothTimer *BluetoothTimer::GetInstance()
{
    static BluetoothTimer instance;
    return &instance;
}

BluetoothTimer::BluetoothTimer() : timer_(std::make_unique<OHOS::Utils::Timer>("BluetoothTimer"))
{
    timer_->Setup();
}

BluetoothTimer::~BluetoothTimer()
{
    if (timer_) {
        timer_->Shutdown(true);
        timer_ = nullptr;
    }
}

void BluetoothTimer::Register(const TimerCallback &callback, uint32_t &outTimerId, uint32_t interval)
{
    if (timer_ == nullptr) {
        HILOGE("timer_ is nullptr");
        return;
    }

    uint32_t ret = timer_->Register(callback, interval, true); // one shot timer
    if (ret == OHOS::Utils::TIMER_ERR_DEAL_FAILED) {
        HILOGE("Register timer failed");
        return;
    }

    outTimerId = ret;
}

void BluetoothTimer::UnRegister(uint32_t timerId)
{
    if (timerId == 0) {
        HILOGE("timerId is 0, no register timer");
        return;
    }

    if (timer_ == nullptr) {
        HILOGE("timer_ is nullptr");
        return;
    }

    timer_->Unregister(timerId);
}
}  // namespace NFC
}  // namespace OHOS