/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_pbap_pce_disconnecting_state"
#endif
#include "pbap_pce_disconnecting_state.h"
#include "power_manager.h"

namespace OHOS {
namespace bluetooth {
PceDisconnectingState::PceDisconnectingState(
    const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer)
    : PceBaseState(name, sm, std::move(observer))
{}

void PceDisconnectingState::Entry()
{
    PBAP_PCE_LOG_INFO("%{public}s start, Observer()->OnServiceConnectionStateChanged()->DISCONNECTING",
        __PRETTY_FUNCTION__);
    auto &device = sm_.GetDevice();
    if (observer_) {
        observer_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::DISCONNECTING), 0);
    }

    const int DISCONNECT_TIMEOUT_MS = 3000;
    disconnectTimer_ = std::make_shared<utility::Timer>([this]() { OnDisconnectTimeout(); });
    disconnectTimer_->Start(DISCONNECT_TIMEOUT_MS);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PceDisconnectingState::Exit()
{
    if (disconnectTimer_ != nullptr) {
        disconnectTimer_->Stop();
        disconnectTimer_ = nullptr;
    }
}

void PceDisconnectingState::OnDisconnectTimeout()
{
    PBAP_PCE_LOG_WARN("Disconnect timeout, force closing socket and transition to DISCONNECTED");
    sm_.ForceCloseObexClient();
    Transition(PCE_DISCONNECTED_STATE);
}

bool PceDisconnectingState::Dispatch(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    uint8_t ret = true;
    switch (msg.what_) {
        case PCE_OBEX_DISCONNECTED:
            Transition(PCE_DISCONNECTED_STATE);
            break;
        case PCE_DOWNLOAD_COMPLETE:
            PBAP_PCE_LOG_INFO("%{public}s PCE_DOWNLOAD_COMPLETE ignored in DISCONNECTING", __PRETTY_FUNCTION__);
            break;
        case PCE_DISCONNECT_TIMEOUT:
            OnDisconnectTimeout();
            break;
        case PCE_REQ_SET_TARGET_STATE:
            sm_.SetTargetState(msg.arg1_);
            break;
        case PCE_REQ_TRANSIT_TARGET_STATE:
            sm_.TransitTargetState();
            break;
        default:
            sm_.TryReleasePbapMsg(msg);
            return false;
    }

    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    return ret;
}
}  // namespace bluetooth
}  // namespace OHOS
