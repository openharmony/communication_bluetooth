/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "power_on_reconnect"
#endif

#include "power_on_reconnect.h"
#include "adapter_manager.h"
#include "bluetooth_hw_interface.h"
#include "connect_strategy_manager.h"
#include "bluetooth_connection_manager.h"
#include "hid_host/hid_host_service.h"
#include "thread_util.h"

constexpr int32_t AUTO_CONNECT_DELAY_MS = 500;

namespace OHOS {
namespace bluetooth {
void PowerOnReconnectSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    auto want = data.GetWant();
    std::string action = want.GetAction();
    HILOGI("PowerOnReconnectSubscriber OnReceiveEvent:%{public}s", action.c_str());
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_ENTER_FORCE_SLEEP) {
        PowerOnReconnect::GetInstance()->DisconnectPeripheralsInStr();
        return;
    }
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON) {
        PowerOnReconnect::GetInstance()->StartAutoConnect();
    }
}

PowerOnReconnect *PowerOnReconnect::GetInstance()
{
    static PowerOnReconnect instance;
    return &instance;
}

void PowerOnReconnect::Initialize()
{
    SubscribeTvCommonEvent();
    hidHostCallback_ = std::make_unique<HidHostCallback>(this);
    if (hidHostCallback_ != nullptr) {
        HidHostService::GetService()->RegisterObserver(*hidHostCallback_);
    }
}

void PowerOnReconnect::Uninitialize()
{
    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
    if (hidHostCallback_ != nullptr) {
        HidHostService::GetService()->DeregisterObserver(*hidHostCallback_);
        hidHostCallback_ = nullptr;
    }
}

void PowerOnReconnect::SubscribeTvCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_ENTER_FORCE_SLEEP);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    if (subscriber_ == nullptr) {
        subscriber_ = std::make_shared<PowerOnReconnectSubscriber>(subscriberInfo);
    }
    bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
    if (subscribeResult) {
        HILOGI("register SubscribeCommonEvent");
    }
}

void PowerOnReconnect::DisconnectPeripheralsInStr()
{
    isForceSleep_.store(true);
    auto connectionManager = BluetoothConnectionManager::GetInstance();
    connectionManager->RemoveAllAclLinks();

    size_t connectedNum = HidHostService::GetService()->GetConnectDevices().size();
    if (connectedNum == 0) {
        HidHostService::GetService()->ClearUpStackHidProfile();
    }
}

void PowerOnReconnect::StartAutoConnect()
{
    if (!isForceSleep_.load()) {
        return;
    }
    HILOGI("awake: start auto connect.");
    isForceSleep_.store(false);
    HidHostService::GetService()->ReStartStackHidProfile();
    auto taskFunc = []() { ConnectStrategyManager::GetInstance()->AutoConnect(); };
    ThreadUtil::GetInstance().PostTask(THREAD_ID_VENDOR, taskFunc, AUTO_CONNECT_DELAY_MS, "PowerOnAutoConnect");
}

void PowerOnReconnect::ProcessHidConnectionStateChanged(const RawAddress &device, int state, int cause)
{
    HILOGI("state = %{public}d", state);
    HidHostService* hidHostService = HidHostService::GetService();
    if ((hidHostService != nullptr) && isForceSleep_.load()
        && (state == static_cast<int>(BTConnectState::DISCONNECTED))
        && (hidHostService->GetConnectDevices().size() == 0)) {
        hidHostService->ClearUpStackHidProfile();
    }
}
}  // namespace bluetooth
}  // namespace OHOS