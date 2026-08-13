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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_system_ability_sub"
#endif

#include "system_ability_subscriber.h"

#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {
void SystemAbilitySubscriber::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbilityId(%{public}d) is added", systemAbilityId);
    if (saId_.find(systemAbilityId) != saId_.end()) {
        onAddSystemAbilityCallback_(systemAbilityId);
    }
}

void SystemAbilitySubscriber::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbilityId(%{public}d) is removed", systemAbilityId);
    if (saId_.find(systemAbilityId) != saId_.end()) {
        onRemoveSystemAbilityCallback_(systemAbilityId);
    }
}
}  // namespace bluetooth
}