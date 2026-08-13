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

#ifndef SYSTEM_ABILITY_PROCESSER_H
#define SYSTEM_ABILITY_PROCESSER_H

#include "base_def.h"
#include "bluetooth_common_event_subscriber.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief  Defines the interfaces.
 */
class SystemAbilityStatusChange : public SystemAbilityStatusChangeStub {
public:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
};

class SystemAbilityProcesser {
public:
    /**
     * @brief Get the Instance of the SystemAbilityProcesser.
     *
     * @return Returns the Instance of the SystemAbilityProcesser.
     */
    static SystemAbilityProcesser *GetInstance();

    /**
     * @brief Initialize system ability processer.
     *
     */
    static void Initialize();

    /**
     * @brief Uninitialize system ability processer.
     *
     */
    static void Uninitialize();

    /**
     * @brief Start the module of the system ability processer.
     */
    void Start();

    /**
     * @brief Stop the module of the system ability processer.
     */
    void Stop();

private:
    /**
     * @brief Construct a new SystemAbilityProcesser object.
     */
    SystemAbilityProcesser();

    sptr<SystemAbilityStatusChange> statusChangeListener_ { nullptr };

    BT_DISALLOW_COPY_AND_ASSIGN(SystemAbilityProcesser);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // SYSTEM_ABILITY_PROCESSER_H