/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef SYSTEM_ABILITY_SUBSCRIBER_H
#define SYSTEM_ABILITY_SUBSCRIBER_H

#include <set>
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace bluetooth {
class SystemAbilitySubscriber : public SystemAbilityStatusChangeStub {
public:
    using SystemAbilityCallback = std::function<void(int32_t systemAbilityId)>;

    SystemAbilitySubscriber(const std::set<int>& saId,
        SystemAbilityCallback onAddSystemAbilityCallback, SystemAbilityCallback onRemoveSystemAbilityCallback)
        : saId_(saId), onAddSystemAbilityCallback_(onAddSystemAbilityCallback),
        onRemoveSystemAbilityCallback_(onRemoveSystemAbilityCallback) {}
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
private:
    std::set<int32_t> saId_;
    SystemAbilityCallback onAddSystemAbilityCallback_;
    SystemAbilityCallback onRemoveSystemAbilityCallback_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // SYSTEM_ABILITY_SUBSCRIBER_H