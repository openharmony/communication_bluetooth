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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_switch"
#endif

#include "switch_dependency.h"

#include "bluetooth_datashare_utils.h"
#include "bluetooth_errorcode.h"
#include "iservice_registry.h"
#include "log.h"
#include "thread_util.h"
#include "parameter.h"
#include "parameters.h"

namespace OHOS {
namespace bluetooth {
namespace {
// 由于WatchParameter函数仅能传递C类型函数，从稳定性角度考虑，C类型函数回调C++类型对象，保存一个全局weak_ptr使用。
std::weak_ptr<SwitchDependency> g_switchDependencyWptr;

void SystemParameterWatchCallback(const char *key, const char *value, void *context)
{
    if (key == nullptr || value == nullptr) {
        HILOGE("Invalid key or value");
        return;
    }
    HILOGI("system parameter change, key = %{public}s, value = %{public}s", key, value);
    auto switchDependencySptr = g_switchDependencyWptr.lock();
    if (!switchDependencySptr) {
        HILOGE("g_switchDependencyWptr is nullptr");
        return;
    }
    switchDependencySptr->OnSystemParameterChanged(key, value);
}
}

SwitchDependency::SwitchDependency(DependencyCallback callback)
{
    dependencyCallback_ = [this, callback]() {
        {
            std::lock_guard<std::mutex> lock(isCallbackTriggeredMutex_);
            if (isCallbackTriggered_) {
                HILOGW("switch dependency callback is triggered");
                return;
            }
            isCallbackTriggered_ = true;
        }
        if (callback) {
            callback();
        }
    };
}

SwitchDependency::~SwitchDependency()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("failed to get samgrProxy");
        return;
    }
    if (systemAbilityStatusListener_ == nullptr) {
        HILOGE("systemAbilityStatusListener_ is nullptr!");
        return;
    }
    std::lock_guard<std::mutex> lock(dependedSystemAbilityMapMutex_);
    for (auto [systemAbilityId, _] : dependedSystemAbilityMap_) {
        samgrProxy->UnSubscribeSystemAbility(systemAbilityId, systemAbilityStatusListener_);
    }

    if (dataShareReadySubscribe_) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(dataShareReadySubscribe_);
    }
    if (bootCompleteSubscribe_) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(bootCompleteSubscribe_);
    }
}

int SwitchDependency::Init(void)
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("failed to get samgrProxy");
        return -1;
    }

    systemAbilityStatusListener_ = new SystemAbilityStatusListener(shared_from_this());
    if (systemAbilityStatusListener_ == nullptr) {
        HILOGE("systemAbilityStatusListener_ is nullptr!");
        return -1;
    }

    g_switchDependencyWptr = weak_from_this();
    {
        std::lock_guard<std::mutex> lock(dependedSystemParamsVecMutex_);
        for (auto &sysParam : dependedSystemParamsVec_) {
            std::string value = OHOS::system::GetParameter(sysParam.key, sysParam.defaultValue);
            if (value == sysParam.expectValue) {
                HILOGI("%{public}s is ready", sysParam.key.c_str());
                sysParam.isSatisfied = true;
            } else {
                HILOGI("%{public}s is not ready, attempt watch it", sysParam.key.c_str());
                int ret = WatchParameter(sysParam.key.c_str(), SystemParameterWatchCallback, nullptr);
                if (ret != 0) {
                    HILOGE("WatchParameter failed, ret(%{public}d)", ret);
                }
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(dependedSystemAbilityMapMutex_);
        for (auto [systemAbilityId, _] : dependedSystemAbilityMap_) {
            int32_t ret = samgrProxy->SubscribeSystemAbility(systemAbilityId, systemAbilityStatusListener_);
            if (ret != ERR_OK) {
                HILOGE("subscribe systemAbilityId(%{public}d) failed!", systemAbilityId);
                systemAbilityStatusListener_ = nullptr;
                return -1;
            }
        }
    }
    return 0;
}

void SwitchDependency::RegisterDataShareReadyCommonEvent(void)
{
    uint32_t coreEventPriority = 1;
    EventFwk::MatchingSkills matchingSkills;
    std::string commonEventDatashareReady = "usual.event.DATA_SHARE_READY";
    matchingSkills.AddEvent(commonEventDatashareReady);

    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(coreEventPriority);

    auto func = [ptr = weak_from_this()](const OHOS::EventFwk::CommonEventData &data) {
        // Work in CommonEvent thread, need post task first.
        DoInAdapterManagerThread([ptr]() {
            auto switchDependencyPtr = ptr.lock();
            CHECK_AND_RETURN_LOG(switchDependencyPtr, "switchDependencyPtr is nullptr");
            switchDependencyPtr->OnDataShareReadyEvent();
        });
    };
    dataShareReadySubscribe_ =
        std::make_shared<BluetoothCommonEventSubscriberEx>(subscribeInfo, commonEventDatashareReady, func);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(dataShareReadySubscribe_)) {
        HILOGE("Subscribe DATA_SHARE_READY common event failed");
        // no need return
    }
}

void SwitchDependency::RegisterBootCompleteCommonEvent(void)
{
    uint32_t coreEventPriority = 1;
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(coreEventPriority);

    auto func = [ptr = weak_from_this()](const OHOS::EventFwk::CommonEventData &data) {
        // Work in CommonEvent thread, need post task first.
        DoInAdapterManagerThread([ptr]() {
            auto switchDependencyPtr = ptr.lock();
            CHECK_AND_RETURN_LOG(switchDependencyPtr, "switchDependencyPtr is nullptr");
            // 系统已BOOT成功，默认驱动加载完毕
            switchDependencyPtr->OnSystemParameterChanged("vendor.setup_mpxx_dir_mode", "success");
        });
    };
    bootCompleteSubscribe_ =
        std::make_shared<BluetoothCommonEventSubscriberEx>(
            subscribeInfo, EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED, func);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(bootCompleteSubscribe_)) {
        HILOGE("Subscribe COMMON_EVENT_BOOT_COMPLETED common event failed");
        // no need return
    }
}

void SwitchDependency::CheckAllDependencySatisfied(void)
{
    HILOGI("%{public}s", ToLogString().c_str());

    bool isAllDependencyOn = true;
    {
        // Check all dependended system parameters is ready
        std::lock_guard<std::mutex> lock(dependedSystemParamsVecMutex_);
        for (auto &sysParam : dependedSystemParamsVec_) {
            if (!sysParam.isSatisfied) {
                isAllDependencyOn = false;
                return;
            }
        }
    }
    {
        // Check all dependended system ability is on
        std::lock_guard<std::mutex> lock(dependedSystemAbilityMapMutex_);
        for (auto [_, isSystemAbilityOn] : dependedSystemAbilityMap_) {
            if (!isSystemAbilityOn) {
                isAllDependencyOn = false;
                return;
            }
        }
    }
// glass do not wait for the data share. can not modify the value of air plane due to no view of glass
#ifndef BT_MCU_PROXY_ENABLE
    isAllDependencyOn = isDataShareReady_.load() ? isAllDependencyOn : false;
#endif
    if (isAllDependencyOn) {
        HILOGI("CheckAllDependencySatisfied success");
        dependencyCallback_();
    }
}

void SwitchDependency::OnDataShareReadyEvent(void)
{
    HILOGI("DataShare is ready");
    isDataShareReady_ = true;
    CheckAllDependencySatisfied();
}

void SwitchDependency::OnAddSystemAbility(int32_t systemAbilityId)
{
    // Must wait common event sa started
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID && !isDataShareReady_.load()) {
        RegisterDataShareReadyCommonEvent();
        RegisterBootCompleteCommonEvent();
    }
    if (systemAbilityId == DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID) {
        isDataShareReady_ = BluetoothDataShareHelperUtils::CheckDataShareIsReady();
        HILOGI("datashare is ready: %{public}d", isDataShareReady_.load());
    }

    {
        // Update systemAbility information
        std::lock_guard<std::mutex> lock(dependedSystemAbilityMapMutex_);
        auto it = dependedSystemAbilityMap_.find(systemAbilityId);
        if (it == dependedSystemAbilityMap_.end()) {
            HILOGE("systemAbilityId(%{public}d) is not the listener id", systemAbilityId);
            return;
        }
        it->second = true;
    }
    CheckAllDependencySatisfied();
}

void SwitchDependency::OnSystemParameterChanged(std::string key, std::string value)
{
    {
        std::lock_guard<std::mutex> lock(dependedSystemParamsVecMutex_);
        auto it = std::find_if(dependedSystemParamsVec_.begin(), dependedSystemParamsVec_.end(),
            [key](DependedSystemParam &sysParam) { return sysParam.key == key; }
        );
        if (it != dependedSystemParamsVec_.end()) {
            if (value == it->expectValue) {
                HILOGI("%{public}s is ready", key.c_str());
                it->isSatisfied = true;
            }
        }
    }
    CheckAllDependencySatisfied();
}

std::string SwitchDependency::ToLogString(void)
{
    std::string log = "";
    {
        std::lock_guard<std::mutex> lock(dependedSystemAbilityMapMutex_);
        for (auto [said, isSystemAbilityOn] : dependedSystemAbilityMap_) {
            log += "(said: " + std::to_string(said) + ", is ready: " + std::to_string(isSystemAbilityOn) + ") ";
        }
    }
    {
        std::lock_guard<std::mutex> lock(dependedSystemParamsVecMutex_);
        for (auto &sysParam : dependedSystemParamsVec_) {
            log += "(sysparam: " + sysParam.key + ", is ready: " +
                std::to_string(sysParam.isSatisfied) + ") ";
        }
    }
    log += "(datashare is ready: " + std::to_string(isDataShareReady_.load()) + ")";
    return log;
}

SystemAbilityStatusListener::SystemAbilityStatusListener(std::weak_ptr<SwitchDependency> ptr)
    : ptr_(ptr)
{}

void SystemAbilityStatusListener::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbilityId(%{public}d) is added", systemAbilityId);
    DoInAdapterManagerThread([systemAbilityId, ptr = ptr_]() {
        auto switchDependencyPtr = ptr.lock();
        if (switchDependencyPtr == nullptr) {
            HILOGE("switchDependencyPtr is nullptr");
            return;
        }
        switchDependencyPtr->OnAddSystemAbility(systemAbilityId);
    });
}

void SystemAbilityStatusListener::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbilityId(%{public}d) is removed", systemAbilityId);
}

}  // namespace bluetooth
}  // namespace OHOS