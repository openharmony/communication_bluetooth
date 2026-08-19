/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef SWITCH_DEPENDENCY_H
#define SWITCH_DEPENDENCY_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "base_def.h"
#include "bluetooth_common_event_subscriber.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace bluetooth {
class SwitchDependency;
class SystemAbilityStatusListener : public SystemAbilityStatusChangeStub {
public:
    explicit SystemAbilityStatusListener(std::weak_ptr<SwitchDependency> ptr);
    ~SystemAbilityStatusListener() = default;

    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    std::weak_ptr<SwitchDependency> ptr_;
};

class SwitchDependency : public std::enable_shared_from_this<SwitchDependency> {
public:
    using DependencyCallback = std::function<void(void)>;

    explicit SwitchDependency(DependencyCallback callback);
    ~SwitchDependency();
    int Init(void);

    void OnAddSystemAbility(int32_t systemAbilityId);
    void OnSystemParameterChanged(std::string key, std::string value);
private:
    void RegisterDataShareReadyCommonEvent(void);
    void RegisterBootCompleteCommonEvent(void);
    void OnDataShareReadyEvent(void);
    void CheckAllDependencySatisfied(void);
    std::string ToLogString(void);

    std::mutex isCallbackTriggeredMutex_ {};
    bool isCallbackTriggered_ { false };
    DependencyCallback dependencyCallback_ {};
    sptr<SystemAbilityStatusListener> systemAbilityStatusListener_ { nullptr };

    mutable std::mutex dependedSystemAbilityMapMutex_ {};
    std::map<int32_t, bool> dependedSystemAbilityMap_ {
        {POWER_MANAGER_SERVICE_ID, false},
// glass do not wait for the data share. can not modify the value of air plane due to no view of glass
#ifndef BT_MCU_PROXY_ENABLE
        {COMMON_EVENT_SERVICE_ID, false},
        {DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, false},
#endif
    };

    struct DependedSystemParam {
        std::string key = "";
        std::string expectValue = "";
        std::string defaultValue = "";
        bool isSatisfied = false;  // 该系统参数是否满足诉求
    };
    mutable std::mutex dependedSystemParamsVecMutex_ {};
    std::vector<DependedSystemParam> dependedSystemParamsVec_ {
#if !(defined(BT_MCU_PROXY_ENABLE) || defined(BLUETOOTH_WATCH_ENABLE) || defined(BT_USE_OPEN_STACK))
        // Qualcomm mpxx driver node; not used by open-stack UART boards (e.g. rk3568/BCM).
        {"vendor.setup_mpxx_dir_mode", "success", "false", false},
#endif // BT_MCU_PROXY_ENABLE / BLUETOOTH_WATCH_ENABLE / BT_USE_OPEN_STACK
    };

    std::atomic_bool isDataShareReady_ { false };
    std::shared_ptr<BluetoothCommonEventSubscriberEx> dataShareReadySubscribe_ { nullptr };
    std::shared_ptr<BluetoothCommonEventSubscriberEx> bootCompleteSubscribe_ { nullptr };

    BT_DISALLOW_COPY_AND_ASSIGN(SwitchDependency);
    BT_DISALLOW_MOVE_AND_ASSIGN(SwitchDependency);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // SWITCH_DEPENDENCY_H
