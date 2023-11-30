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

#ifndef BLUETOOTH_TIMER_H
#define BLUETOOTH_TIMER_H

#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace OHOS {
namespace Bluetooth {

class BluetoothTimer {
public:
    using TimerCallback = std::function<void()>;
    static constexpr uint32_t DEFAULT_TIMEROUT = 10000; // ms

    static BluetoothTimer *GetInstance(void);

    BluetoothTimer();
    ~BluetoothTimer();

    void Register(const TimerCallback &callback, uint32_t &outTimerId, uint32_t interval = DEFAULT_TIMEROUT);
    void UnRegister(uint32_t timerId);

private:
    std::unique_ptr<Utils::Timer> timer_{nullptr};
};

}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
#endif