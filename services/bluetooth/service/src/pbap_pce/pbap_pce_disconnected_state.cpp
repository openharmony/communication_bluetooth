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
#define LOG_TAG "bt_pbap_pce_disconnected_state"
#endif
#include "pbap_pce_disconnected_state.h"
#include "pbap_pce_def.h"
#include "pbap_pce_state_machine.h"
#include "power_manager.h"

namespace OHOS {
namespace bluetooth {
PceDisconnectedState::PceDisconnectedState(
    const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer)
    : PceBaseState(name, sm, std::move(observer))
{}

void PceDisconnectedState::Entry()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);

    PBAP_PCE_LOG_INFO("%{public}s, observer->OnServiceConnectionStateChanged", __PRETTY_FUNCTION__);
    auto &device = sm_.GetDevice();
    IPowerManager::GetInstance()->StatusUpdate(RequestStatus::CONNECT_OFF, PROFILE_NAME_PBAP_PCE, device);
    if (observer_) {
        observer_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::DISCONNECTED), 0);
    }
    // 清理 obex client 引用，确保重连时 CreatePceObexClient 新建对象。
    // ffrt 下载线程持有的 shared_ptr 仍保持 obexClient 存活，直到其 lambda 退出。
    sm_.ResetObexClient();
    // sm_.RemoveBTMLogging();
    sm_.TransitTargetState();
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PceDisconnectedState::Exit()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

bool PceDisconnectedState::Dispatch(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    switch (msg.what_) {
        case PCE_REQ_SET_TARGET_STATE:
            sm_.SetTargetState(msg.arg1_);
            break;
        case PCE_REQ_TRANSIT_TARGET_STATE:
            sm_.TransitTargetState();
            break;
        default:
            sm_.TryReleasePbapMsg(msg);
            break;
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS
