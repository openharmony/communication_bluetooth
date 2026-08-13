/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_pbap_pse_statemachine"
#endif

#include "common_util.h"
#include "pbap_pse_service.h"
#include "pbap_pse_statemachine.h"
#include "service_util.h"
#include "pbap_pse_service_impl.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
namespace {
    inline static const std::string WAITFORAUTH = "WaitForAuth";
    inline static const std::string DISCONNECTED = "Disconnected";
    inline static const std::string CONNECTED = "Connected";
}

PbapPseStateMachine::PbapPseStateMachine(std::shared_ptr<ObexSocketDevice> socketDevice,
    PbapPseServiceImpl *pbapPseServiceImpl)
    : socketDevice_(socketDevice), pbapPseServiceImpl_(pbapPseServiceImpl)
{
    HILOGI("PbapPseStateMachine create");
}
PbapPseStateMachine::~PbapPseStateMachine()
{
    if (obexServerSession_ != nullptr) {
        obexServerSession_->Stop();
        obexServerSession_ = nullptr;
    }

    obexTransport_ = nullptr;
    pbapPseObexServer_ = nullptr;
    socketDevice_ = nullptr;
}

void PbapPseStateMachine::Init()
{
    requestPermissionTimer_ = std::make_shared<utility::Timer>([this]() { this->RequestPermissionTimeout(); });

    std::unique_ptr<utility::StateMachine::State> waitForAuthState =
        std::make_unique<PbapPseWaitForAuthState>(WAITFORAUTH, *this);
    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<PbapPseDisconnectedState>(DISCONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<PbapPseConnectedState>(CONNECTED, *this);

    Move(waitForAuthState);
    Move(disconnectedState);
    Move(connectedState);

    InitState(DISCONNECTED);
}

bool PbapPseStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void PbapPseStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

std::string PbapPseStateMachine::GetDeviceAddress()
{
    if (socketDevice_ == nullptr) {
        HILOGI("socketDevice_ is null");
        return "";
    }
    return socketDevice_->mDeviceAddress;
}

void PbapPseWaitForAuthState::Entry()
{
    HILOGI("Entry WaitForAuthState");
    std::string address = stateMachine_.GetDeviceAddress();
    stateMachine_.RequestPhonebookPermission(address);
}

void PbapPseWaitForAuthState::Exit()
{
    HILOGI("Exit WaitForAuthState");
    stateMachine_.StopRequestPermissionTimer();
}

bool PbapPseWaitForAuthState::Dispatch(const utility::Message &msg)
{
    PbapPseMessage &event = static_cast<PbapPseMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[WaitForAuth]EventName=%{public}s", PbapPseStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case PBAP_PSE_VERIFY_RESULT_EVT: {
            if (event.isAccess_) {
                Transition(CONNECTED);
            } else {
                stateMachine_.RejectConnection();
                auto taskFunc = [this]() {
                    Transition(DISCONNECTED);
                };
                ThreadUtil::GetInstance().PostTask(
                    THREAD_ID_PBAP, taskFunc, DISCONNECT_DELAY_TIME_MS, "DISCONNECT_PBAP_TASK");
            }
            break;
        }
        case PBAP_PSE_DISCONNECT_EVT: {
            ThreadUtil::GetInstance().RemoveTask(THREAD_ID_PBAP, "DISCONNECT_PBAP_TASK");
            Transition(DISCONNECTED);
            break;
        }
        default:
            break;
    }
    return true;
}

void PbapPseDisconnectedState::Entry()
{
    HILOGI("Entry DisconnectedState");
    stateMachine_.ProcessDeferredMessage();

    if (isReentry_) {
        stateMachine_.SetRemoving(true);
        stateMachine_.RemoveStateMachine(stateMachine_.GetDeviceAddress());
        stateMachine_.NotifyStateTransitions();
    }
}

void PbapPseDisconnectedState::Exit()
{
    HILOGI("Exit DisconnectedState");
    isReentry_ = true;
}

bool PbapPseDisconnectedState::Dispatch(const utility::Message &msg)
{
    PbapPseMessage &event = static_cast<PbapPseMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Disconnected]EventName=%{public}s", PbapPseStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case PBAP_PSE_CONNECT_EVT: {
            Transition(WAITFORAUTH);
            break;
        }
        default:
            break;
    }
    return true;
}

void PbapPseConnectedState::Entry()
{
    HILOGI("Entry ConnectedState");
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
    stateMachine_.ProcessConnectEvent();
}

void PbapPseConnectedState::Exit()
{
    HILOGI("Exit ConnectedState");
}

bool PbapPseConnectedState::Dispatch(const utility::Message &msg)
{
    PbapPseMessage &event = static_cast<PbapPseMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Connected]EventName=%{public}s", PbapPseStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case PBAP_PSE_CONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case PBAP_PSE_DISCONNECT_EVT: {
            stateMachine_.ProcessDisConnectEvent();
            Transition(DISCONNECTED);
            break;
        }
        default:
            break;
    }
    return true;
}

bool PbapPseStateMachine::ProcessConnectEvent()
{
    obexTransport_ = std::make_shared<ObexServerTransport>(socketDevice_);
    pbapPseObexServer_ = std::make_shared<PbapPseObexServer>(socketDevice_->mDeviceAddress, pbapPseServiceImpl_);
    obexServerSession_ = std::make_shared<ObexServerSession>(obexTransport_, pbapPseObexServer_);
    obexServerSession_->Start();
    return true;
}

bool PbapPseStateMachine::ProcessDisConnectEvent()
{
    if (obexServerSession_ != nullptr) {
        obexServerSession_->Stop();
    }
    obexServerSession_ = nullptr;
    obexTransport_ = nullptr;
    pbapPseObexServer_ = nullptr;
    obexRejectServer_ = nullptr;
    return true;
}

void PbapPseStateMachine::RequestPhonebookPermission(std::string &address)
{
    CHECK_AND_RETURN_LOG(pbapPseServiceImpl_, "pbapPseServiceImpl_ is null");
    SetRequestPermissionFlag(true);
    PbapPseMessage event(PBAP_PSE_REQUEST_PERMISSION_EVT);
    event.dev_ = address;
    event.timer_ = requestPermissionTimer_;
    pbapPseServiceImpl_->PostEvent(event);
}

void PbapPseStateMachine::RemoveStateMachine(const std::string &address)
{
    CHECK_AND_RETURN_LOG(pbapPseServiceImpl_, "pbapPseServiceImpl_ is null");
    pbapPseServiceImpl_->RemoveStateMachine(address);
}

int PbapPseStateMachine::GetDeviceStateInt() const
{
    const PbapPseState* state = static_cast<const PbapPseState*>(GetState());
    if (state == nullptr) {
        return PBAP_PSE_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

void PbapPseStateMachine::StopRequestPermissionTimer() const
{
    HILOGI("Stop RequestPermission timer!");
    requestPermissionTimer_->Stop();
}

void PbapPseStateMachine::RequestPermissionTimeout()
{
    HITRACE_METER(BT_TRACE_TAG);
    CHECK_AND_RETURN_LOG(pbapPseServiceImpl_, "pbapPseServiceImpl_ is null");
    SetRequestPermissionFlag(false);
    PbapPseMessage event(PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT);
    event.dev_ = GetDeviceAddress();
    pbapPseServiceImpl_->PostEvent(event);
}

bool PbapPseStateMachine::GetRequestPermissionFlag() const
{
    return isRequestPermission_;
}

void PbapPseStateMachine::SetRequestPermissionFlag(bool flag)
{
    isRequestPermission_ = flag;
}

std::string PbapPseStateMachine::GetEventName(int what)
{
    switch (what) {
        case PBAP_PSE_CONNECT_EVT:
            return "PBAP_PSE_CONNECT_EVT";
        case PBAP_PSE_DISCONNECT_EVT:
            return "PBAP_PSE_DISCONNECT_EVT";
        case PBAP_PSE_INVALID_EVT:
            return "PBAP_PSE_INVALID_EVT";
        case PBAP_PSE_REQUEST_PERMISSION_EVT:
            return "PBAP_PSE_REQUEST_PERMISSION_EVT";
        case PBAP_PSE_VERIFY_RESULT_EVT:
            return "PBAP_PSE_VERIFY_RESULT_EVT";
        case PBAP_PSE_REMOVE_STATE_MACHINE_EVT:
            return "PBAP_PSE_REMOVE_STATE_MACHINE_EVT";
        case PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT:
            return "PBAP_PSE_REQ_PREMISSION_TIMEOUT_EVT";
        default:
            return "Unknown";
    }
}

void PbapPseStateMachine::NotifyStateTransitions()
{
    int toState = GetDeviceStateInt();
    RawAddress device(GetDeviceAddress());
    if (preState_ == toState) {
        HILOGI("NotifyStateTransitions preState_ equal toState, preState_ = %{public}d", preState_);
        return;
    }
    if (toState == PBAP_PSE_STATE_DISCONNECTED || toState == PBAP_PSE_STATE_CONNECTED) {
        if (pbapPseServiceImpl_ != nullptr) {
            PbapPseMessage event(PBAP_PSE_STATE_CHANGED);
            event.dev_ = GetDeviceAddress();
            event.state_ = toState;
            pbapPseServiceImpl_->PostEvent(event);
        }
    }
    preState_ = toState;
}

void PbapPseStateMachine::AddDeferredMessage(const PbapPseMessage &msg)
{
    deferMsgs_.push_back(msg);
}

void PbapPseStateMachine::ProcessDeferredMessage()
{
    CHECK_AND_RETURN_LOG(pbapPseServiceImpl_, "pbapPseServiceImpl_ is null");
    auto size = deferMsgs_.size();
    while (size > 0 && !deferMsgs_.empty()) {
        PbapPseMessage event = deferMsgs_.front();
        deferMsgs_.pop_front();
        pbapPseServiceImpl_->PostEvent(event);
        size--;
    }
}

void PbapPseStateMachine::RejectConnection()
{
    obexTransport_ = std::make_shared<ObexServerTransport>(socketDevice_);
    obexRejectServer_ = std::make_shared<ObexRejectServer>();
    obexServerSession_ = std::make_shared<ObexServerSession>(obexTransport_, obexRejectServer_);
    obexServerSession_->Start();
}
}  // namespace bluetooth
}
