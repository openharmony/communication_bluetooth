/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_param_update"
#endif

#include "param_update_event_manager.h"
 
#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_support.h>
#include <memory>
#include <unistd.h>
#include "common_event_subscriber.h"
#include "log.h"
#include "bt_def.h"
#include "thread_util.h"
#include "adapter_manager.h"
#include "hw_bt_hwif.h"

namespace OHOS {
namespace bluetooth {

const std::string EVENT_INFO_TYPE = "type";
const std::string EVENT_INFO_SUBTYPE = "subtype";
const std::string CONFIG_UPDATED_ACTION = "usual.event.DUE_SA_CFG_UPDATED";
const std::string CONFIG_TYPE = "BLUETOOTH";
const int32_t PARAM_UPDATE_RETRY_MAX_COUNT = 3;
const int32_t PARAM_UPDATE_RETRY_INTERVAL_IN_MILLIS = 60 * 1000;

ParamUpdateEventManager::ParamUpdateEventManager() {}
 
ParamUpdateEventManager::~ParamUpdateEventManager() {}

void ParamUpdateEventManager::SubscriberEvent()
{
    std::lock_guard<std::mutex> lock(paramUpdateLock_);
    if (subscriber_) {
        HILOGI("Common Event is already subscribered.");
        return;
    }
 
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CONFIG_UPDATED_ACTION);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPermission("ohos.permission.RECEIVE_UPDATE_MESSAGE");
    subscriber_ = std::make_shared<ParamCommonEventSubscriber>(subscribeInfo, *this);
    // Require permission ohos.permission.RECEIVE_UPDATE_MESSAGE
    bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
    if (!subscribeResult) {
        HILOGE("SubscriberEvent failed.");
        subscriber_ = nullptr;
        return;
    }
}
 
void ParamUpdateEventManager::UnSubscriberEvent()
{
    std::lock_guard<std::mutex> lock(paramUpdateLock_);
    if (subscriber_) {
        bool subscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        HILOGI("subscribeResult = %{public}d", subscribeResult);
        subscriber_ = nullptr;
    }
}
 
void ParamUpdateEventManager::OnReceiveEvent(const AAFwk::Want &want)
{
    std::string action = want.GetAction();
    std::string type = want.GetStringParam(EVENT_INFO_TYPE);
    std::string subtype = want.GetStringParam(EVENT_INFO_SUBTYPE);
    HILOGI("Recive param update event: %{public}s, %{public}s, %{public}s", action.c_str(), type.c_str(),
        subtype.c_str());
    if (action != CONFIG_UPDATED_ACTION || type != CONFIG_TYPE) {
        HILOGI("Ignore other event.");
        return;
    }
    HandleParamUpdate();
}

void ParamUpdateEventManager::HandleParamUpdate()
{
    HILOGI("HandleParamUpdate start.");
    if (AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR) != BTStateID::STATE_TURN_ON &&
        AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE) != BTStateID::STATE_TURN_ON &&
        AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BREDR) != BTStateID::STATE_TURNING_ON &&
        AdapterManager::GetInstance()->GetState(BTTransport::ADAPTER_BLE) != BTStateID::STATE_TURNING_ON) {
        /*
        * For STATE_OFF/STATE_TURNING_OFF, as the BtPara data would be dropped anyway,
        * we do not need to do the update.
        */
        HILOGW("BT current state is not enabled!");
        return;
    }

    /*
    * CfgUpdate event may arrive BEFORE, DURING and AFTER BluetoothSysStateStarted.
    * For BEFORE and DURING case, we always delay checking
    * and updating CfgUpdate data to after BluetoothSysStateStarted,
    */
    if (!AdapterManager::GetInstance()->IsBluetoothSysStateStarted()) {
        HILOGW("Receive CotaUpdate event while bt sys state is not started!");
        return;
    }

    std::lock_guard<std::mutex> lock(paramUpdateLock_);
    if (isParamUpdateHandling_) {
        HILOGW("Now param update is Handling!");
        return;
    }
    isParamUpdateHandling_ = true;
    std::shared_ptr<ParamUpdateEventManager> eventManager = shared_from_this();
    DoInAdapterManagerThread([eventManager] {eventManager->CheckIdleStateToTryParamUpdate();});
}

void ParamUpdateEventManager::DoBtParamUpdate()
{
    auto btInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (btInterface == nullptr || btInterface->getProfileInterface == nullptr) {
        HILOGE("Failed to get btInterface");
        return;
    }
    const BthwifInterface *bthwInterface =
        reinterpret_cast<const BthwifInterface*>(btInterface->getProfileInterface(BT_VENDER_INTERFACE_ID));
    if (bthwInterface == nullptr || bthwInterface->hwDynamicConfig == nullptr) {
        HILOGE("Failed to get bthwInterface");
        return;
    }
    (void)bthwInterface->hwDynamicConfig();
}

void ParamUpdateEventManager::CheckIdleStateToTryParamUpdate()
{
    std::lock_guard<std::mutex> lock(paramUpdateLock_);
    isParamUpdateHandling_ = true;
    if (AdapterManager::GetInstance()->GetAdapterConnectState() != BTConnectState::CONNECTING) {
        HILOGI("Do Param Update in idle state.");
        DoBtParamUpdate();
        isParamUpdateHandling_ = false;
        paramUpdateRetryCount_ = 0;
        return;
    }

    if (paramUpdateRetryCount_ > PARAM_UPDATE_RETRY_MAX_COUNT) {
        isParamUpdateHandling_ = false;
        paramUpdateRetryCount_ = 0;
        HILOGE("Reach Param Update max retry count.");
        return;
    }
    paramUpdateRetryCount_++;
    HILOGI("Retry Param Update 1 minute later, retry count=%{public}d.", paramUpdateRetryCount_);
    int32_t delayMs = PARAM_UPDATE_RETRY_INTERVAL_IN_MILLIS;
    std::shared_ptr<ParamUpdateEventManager> eventManager = shared_from_this();
    ThreadUtil::GetInstance().PostTask(
        THREAD_ID_ADAPTER_MANAGER, [eventManager] {eventManager->CheckIdleStateToTryParamUpdate();},
        delayMs, "ParamUpdateRetry");
}
}  // namespace bluetooth
}  // namespace OHOS
