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

#ifndef FREEZE_UTILS_H
#define FREEZE_UTILS_H

#include <mutex>
#include <string>
#include <map>
#include <vector>
#include "ffrt_inner.h"
#include "safe_map.h"
#include "system_ability_subscriber.h"

namespace OHOS {
namespace bluetooth {
#ifdef RES_SCHED_SUPPORT
class FreezeUtil {
public:
    FreezeUtil() = default;
    ~FreezeUtil();
    static FreezeUtil *GetInstance(void);
    void InitQueueTask();
    void CheckPeriodical();
    void SetFreezeState();
    void RequestActive(const int32_t pid, const int32_t uid, const std::string &reason);
    bool isReport(std::pair<int, int> appInfo);
    void ReportBtDataToRss(const int32_t pid, const int32_t uid, const bool &isTransfer, const std::string &reason);
    void ReportSppConnectState(
        int32_t pid, int32_t uid, const std::string &address, int id, const std::string &action);
private:
    void SubscribeRssSystemAbility();
    SafeMap<std::pair<int, int>, bool> hasDataReport_;
    std::shared_ptr<ffrt::queue> ffrtQueue_ = nullptr;
    sptr<SystemAbilitySubscriber> statusChangeListener_ { nullptr };
};
#endif
}  // namespace OHOS
}  // namespace bluetooth
#endif  // FREEZE_UTILS_H
