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
#define LOG_TAG "bt_service_system_ability"
#endif

#include "system_ability_processer.h"

#include "bluetooth_datashare_utils.h"
#include "bluetooth_errorcode.h"
#include "iservice_registry.h"
#include "log.h"
#include "thread_util.h"
#include "bt_chr_dft_exception.h"
#ifdef BLUETOOTH_HFP_AG_ENABLE
#include "hfp_ag_system_interface.h"
#endif
#ifdef BLUETOOTH_HFP_HF_ENABLE
#include "hfp_hf_service.h"
#endif
#include "a2dp_service.h"

namespace OHOS {
namespace bluetooth {
SystemAbilityProcesser::SystemAbilityProcesser()
{}

SystemAbilityProcesser *SystemAbilityProcesser::GetInstance()
{
    static SystemAbilityProcesser systemAbilityProcesser;
    return &systemAbilityProcesser;
}

void SystemAbilityProcesser::Initialize()
{
    GetInstance()->Start();
}

void SystemAbilityProcesser::Uninitialize()
{
    GetInstance()->Stop();
}

void SystemAbilityProcesser::Start()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("failed to get samgrProxy");
        return;
    }
    statusChangeListener_ = new SystemAbilityStatusChange();
    int32_t ret = samgrProxy->SubscribeSystemAbility(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID, statusChangeListener_);
    if (ret != ERR_OK) {
        HILOGE("subscribe systemAbilityId: call manager service failed!");
        statusChangeListener_ = nullptr;
        return;
    }
    ret = samgrProxy->SubscribeSystemAbility(TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID, statusChangeListener_);
    if (ret != ERR_OK) {
        HILOGE("subscribe systemAbilityId: core service failed!");
        statusChangeListener_ = nullptr;
        return;
    }
    ret = samgrProxy->SubscribeSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, statusChangeListener_);
    if (ret != ERR_OK) {
        HILOGE("subscribe systemAbilityId: audio server failed!");
        statusChangeListener_ = nullptr;
        return;
    }
}

void SystemAbilityProcesser::Stop()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("failed to get samgrProxy");
        return;
    }
    if (statusChangeListener_ == nullptr) {
        HILOGE("statusChangeListener_ is nullptr!");
        return;
    }

    samgrProxy->UnSubscribeSystemAbility(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID, statusChangeListener_);
    samgrProxy->UnSubscribeSystemAbility(TELEPHONY_CORE_SERVICE_SYS_ABILITY_ID, statusChangeListener_);
    samgrProxy->UnSubscribeSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, statusChangeListener_);
}

void SystemAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbilityId(%{public}d) is added", systemAbilityId);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    HfpAgSystemInterface::GetInstance().OnAddSystemAbility(systemAbilityId);
#endif
#ifdef BLUETOOTH_HFP_HF_ENABLE
    switch (systemAbilityId) {
        case AUDIO_DISTRIBUTED_SERVICE_ID:
            DoInHfThread([]() {
                HfpHfService *hfService = HfpHfService::GetService();
                CHECK_AND_RETURN_LOG(hfService != nullptr, "get hf service failed!");
                hfService->SetAudioParameters();
            });
            break;
        default:
            HILOGE("unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
#endif
    return;
}

void SystemAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbilityId(%{public}d) is removed", systemAbilityId);
#ifdef BLUETOOTH_HFP_AG_ENABLE
    HfpAgSystemInterface::GetInstance().OnRemoveSystemAbility(systemAbilityId);
#endif
    switch (systemAbilityId) {
        case AUDIO_DISTRIBUTED_SERVICE_ID:
            DoInA2dpThread([deviceId]() {
                A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
                CHECK_AND_RETURN_LOG(a2dpService != nullptr, "get a2dp service failed!");
                a2dpService->ForceStopOffloadPlaying(a2dpService->GetActiveSinkDevice());
                a2dpService->UpdateActiveDevice(ServiceUtil::AddrFromBluedroid(BLUEDROID::RawAddress::kEmpty));
            });
            break;
        default:
            HILOGE("unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
    return;
}
}  // namespace bluetooth
}