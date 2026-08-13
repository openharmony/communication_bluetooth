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
#define LOG_TAG "bt_pbap_pce_connected_state"
#endif

#include "pbap_pce_connected_state.h"
#include <codecvt>
#include <locale>
#include "pbap_pce_service_impl.h"
#include "power_manager.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
PceConnectedState::PceConnectedState(
    const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer)
    : PceBaseState(name, sm, std::move(observer))
{}

void PceConnectedState::Entry()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    PBAP_PCE_LOG_INFO("%{public}s, observer->OnConnectionStateChanged", __PRETTY_FUNCTION__);
    auto &device = sm_.GetDevice();
    IPowerManager::GetInstance()->StatusUpdate(RequestStatus::CONNECT_ON, PROFILE_NAME_PBAP_PCE, device);
    if (observer_) {
        observer_->OnConnectionStateChanged(device, static_cast<int>(BTConnectState::CONNECTED), 0);
    }
    auto obexClient = sm_.GetObexClientPtr();
    if (obexClient != nullptr) {
        PBAP_PCE_LOG_INFO("%{public}s triggering auto-download", __PRETTY_FUNCTION__);
        DoInPbapSyncThread([obexClient]() {
            obexClient->DownloadContactsAndCallLogs();
        });
    } else {
        PBAP_PCE_LOG_ERROR("%{public}s obexClient is null, skip auto-download", __PRETTY_FUNCTION__);
    }
    sm_.TransitTargetState();
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PceConnectedState::Exit()
{
    PBAP_PCE_LOG_INFO("%{public}s start", __PRETTY_FUNCTION__);
    PBAP_PCE_LOG_INFO("%{public}s end", __PRETTY_FUNCTION__);
}

void PceConnectedState::ReqPullPhoneBook(const utility::Message &msg) const
{
    // 🔴 接收方负责释放内存
    std::unique_ptr<PbapPceAppParams> appParams(
        static_cast<PbapPceAppParams*>(msg.arg2_));
    
    if (!appParams) {
        PBAP_PCE_LOG_ERROR("appParams is null");
        return;
    }
    
    // 直接使用 appParams
    sm_.PullPhoneBook(*appParams);
}

bool PceConnectedState::DispatchActionCompleted(const utility::Message &msg) const
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    bool ret = true;

    switch (msg.what_) {
        case PCE_PULLPHONEBOOKSIZE_COMPLETED:
            sm_.HandlePhoneBookSizeActionCompleted(msg);
            break;
        case PCE_PULLPHONEBOOK_COMPLETED:
            sm_.HandlePullPhoneBookActionCompleted(msg);
            break;
        case PCE_ABORTDOWNLOADING_COMPLETED:
            sm_.HandleAbortDownloadingActionCompleted(msg);
            break;
        default:
            ret = false;
            sm_.TryReleasePbapMsg(msg);
            PBAP_PCE_LOG_ERROR("%{public}s msg.what_=%{public}d can't be handled.", __PRETTY_FUNCTION__, msg.what_);
            break;
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    return ret;
}

bool PceConnectedState::Dispatch(const utility::Message &msg)
{
    PBAP_PCE_LOG_INFO("%{public}s start, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    bool ret = true;

    switch (msg.what_) {
        case PCE_REQ_DISCONNECTED:
            Transition(PCE_DISCONNECTING_STATE);
            sm_.ForceCloseObexClient();
            break;
        case PCE_TRANSPORT_FAILED:
            PBAP_PCE_LOG_WARN("%{public}s PCE_TRANSPORT_FAILED, force close and transition", __PRETTY_FUNCTION__);
            Transition(PCE_DISCONNECTING_STATE);
            sm_.ForceCloseObexClient();
            break;
        case PCE_REQ_SET_TARGET_STATE:
            sm_.SetTargetState(msg.arg1_);
            break;
        case PCE_REQ_TRANSIT_TARGET_STATE:
            sm_.TransitTargetState();
            break;
        case PCE_OBEX_DISCONNECTED:
            Transition(PCE_DISCONNECTING_STATE);
            Transition(PCE_DISCONNECTED_STATE);
            break;
        case PCE_REQ_PULLPHONEBOOK:
            ReqPullPhoneBook(msg);
            break;
        case PCE_REQ_ABORTDOWNLOADING:
            sm_.AbortDownloading();
            break;
        case PCE_DOWNLOAD_COMPLETE:
            PBAP_PCE_LOG_INFO("%{public}s PCE_DOWNLOAD_COMPLETE received", __PRETTY_FUNCTION__);
            if (observer_) {
                observer_->OnActionCompleted(sm_.GetDevice(), 0, PBAP_ACTION_PULLPHONEBOOK);
            }
            sm_.TransitTargetState();
            break;
        default:
            ret = DispatchActionCompleted(msg);
            break;
    }
    PBAP_PCE_LOG_INFO("%{public}s end, msg.what_=[%{public}d]", __PRETTY_FUNCTION__, msg.what_);
    return ret;
}
}  // namespace bluetooth
}  // namespace OHOS
