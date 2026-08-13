/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef A2DP_HFP_RECOVER
#define A2DP_HFP_RECOVER

#include <string>
#include "btcommon/timer_manager.h"

namespace OHOS {
namespace bluetooth {

enum class RecoverType : int32_t {
    A2dpRecover,
    HfpRecover
};

class A2dpHfpRecover {
public:
    static A2dpHfpRecover& GetInstance();
    void SetTimerForRecoverA2dpService(const std::string& addr);
    void SetTimerForRecoverHfpService(const std::string& addr);

private:
    A2dpHfpRecover() = default;
    void A2dpOrHfpTimerCallback(RecoverType type, const std::string& addr);
    std::shared_ptr<utility::Timer> timerForRecoverA2dpService_ = nullptr;
    std::shared_ptr<utility::Timer> timerForRecoverHfpService_ = nullptr;
    std::mutex recoverLock_{};
    const uint16_t TIMER_FOR_RECOVER_A2DP_SERVICE_MS = 10000;
    const uint16_t TIMER_FOR_RECOVER_HFP_SERVICE_MS = 10000;
    const std::vector<int> recoverBlackList_ = {
        279,    // Freelace Pro
        333     // Freelace Pro 2
    };
};
}
}
#endif