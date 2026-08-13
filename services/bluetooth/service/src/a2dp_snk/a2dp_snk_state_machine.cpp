/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_a2dp_snk_machine"
#endif

#include "log.h"
#include "a2dp_snk_state_machine.h"
#include "a2dp_snk_service.h"

namespace OHOS {
namespace bluetooth {

A2dpSnkStateMachine::A2dpSnkStateMachine(const std::string &address, A2dpSnkService* service) : address_(address), service_(service)
{
    HILOGI("A2dpSnkStateMachine Create");
}

void A2dpSnkStateMachine::Init()
{
    connTimer_ = std::make_unique<utility::Timer>(std::bind(&bluetooth::A2dpSnkStateMachine::ConnectionTimeout, this));
    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<A2dpSnkDisconnectedState>(DISCONNECTED, *this);

    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<A2dpSnkConnectingState>(CONNECTING, *this);

    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<A2dpSnkConnectedState>(CONNECTED, *this);

    std::unique_ptr<utility::StateMachine::State> disconnectingState =
        std::make_unique<A2dpSnkDisconnectingState>(DISCONNECTING, *this);

    Move(disconnectedState);
    Move(connectingState);
    Move(connectedState);
    Move(disconnectingState);
    InitState(DISCONNECTED);
}

int A2dpSnkStateMachine::GetDeviceState() const
{
    const A2dpSnkState *state = static_cast<const A2dpSnkState *>(GetState());
    if (!state) {
        return A2DP_SNK_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

std::string A2dpSnkStateMachine::GetEventName(int what)
{
    switch (what) {
        case A2DP_SNK_CONN_CONNECT_EVT:
            return "A2DP_SNK_CONN_CONNECT_EVT";
        case A2DP_SNK_CONN_TIMEOUT_EVT:
            return "A2DP_SNK_CONN_TIMEOUT_EVT";
        case A2DP_SNK_CONN_DISCONNECT_EVT:
            return "A2DP_SNK_CONN_DISCONNECT_EVT";
        case A2DP_SNK_CONN_STACK_CONNECTING_EVT:
            return "A2DP_SNK_CONN_STACK_CONNECTING_EVT";
        case A2DP_SNK_CONN_STACK_CONNECTED_EVT:
            return "A2DP_SNK_CONN_STACK_CONNECTED_EVT";
        case A2DP_SNK_CONN_STACK_DISCONNECTING_EVT:
            return "A2DP_SNK_CONN_STACK_DISCONNECTING_EVT";
        case A2DP_SNK_CONN_STACK_DISCONNECTED_EVT:
            return "A2DP_SNK_CONN_STACK_DISCONNECTED_EVT";
        case A2DP_SNK_CONN_STACK_AUDIO_CONFIG_EVT:
            return "A2DP_SNK_CONN_STACK_AUDIO_CONFIG_EVT";
        case A2DP_SNK_STREAM_AUDIO_STARTED_EVT:
            return "A2DP_SNK_STREAM_AUDIO_STARTED_EVT";
        case A2DP_SNK_STREAM_AUDIO_STOPPED_EVT:
            return "A2DP_SNK_STREAM_AUDIO_STOPPED_EVT";
        case A2DP_SNK_STREAM_USER_PLAY_EVT:
            return "A2DP_SNK_STREAM_USER_PLAY_EVT";
        case A2DP_SNK_STREAM_USER_PAUSE_EVT:
            return "A2DP_SNK_STREAM_USER_PAUSE_EVT";
        case A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT:
            return "A2DP_SNK_STREAM_DEVICE_DISCONNECTED_EVT";
        case A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT:
            return "A2DP_SNK_STREAM_PLAYBACK_DISALLOWED_EVT";
        case A2DP_SNK_STREAM_A2DP_CONNECTED_EVT:
            return "A2DP_SNK_STREAM_A2DP_CONNECTED_EVT";
        case A2DP_SNK_STREAM_FOCUS_CHANGE_EVT:
            return "A2DP_SNK_STREAM_FOCUS_CHANGE_EVT";
        case A2DP_SNK_STREAM_DELAYED_PAUSE_EVT:
            return "A2DP_SNK_STREAM_DELAYED_PAUSE_EVT";
        default:
            return "Unkown";
    }
}

void A2dpSnkStateMachine::NotifyStateChanged()
{
    int state = GetDeviceState();
    if (previousState_ == state) {
        return;
    }
    HILOGI("Connection state %{public}s: %{public}d -> %{public}d",
        address_.c_str(), previousState_, state);
    if (service_) {
        service_->NotifyConnStateChanged(RawAddress(address_), previousState_, state, 0);
    } else {
        HILOGE("A2dpSnkService is nullptr!");
    }
    previousState_ = state;
}

bool A2dpSnkStateMachine::AllowIncomingConnection(const std::string &address)
{
    if (service_ == nullptr) {
        HILOGE("A2dpSnkService is nullptr, addr=%{public}s", GetEncryptAddr(address).c_str());
        return false;
    }
    if (!service_->AcceptIncomingConnection(RawAddress(address))) {
        HILOGW("Not allow incoming connection: %{public}s", GetEncryptAddr(address).c_str());
        service_->NativeDisconnect(RawAddress(address));
        return false;
    }
    return true;
}

bool A2dpSnkStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void A2dpSnkStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

void A2dpSnkStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(CONNECTION_TIMEOUT_MS);
    HILOGI("Start connection timer!");
}

void A2dpSnkStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    HILOGI("Stop connection timer!");
}

void A2dpSnkStateMachine::ConnectionTimeout() const
{
    HILOGI("connection timeout!");
    A2dpSnkMessage event(A2DP_SNK_CONN_TIMEOUT_EVT);
    event.addr_ = address_;
    if (service_) {
        service_->PostEvent(event);
    }
}

void A2dpSnkStateMachine::AddDeferredMessage(const A2dpSnkMessage &msg)
{
    deferMsgs_.push_back(msg);
}

void A2dpSnkStateMachine::ProcessDeferredMessage()
{
    while (!deferMsgs_.empty()) {
        A2dpSnkMessage event = deferMsgs_.front();
        deferMsgs_.pop_front();
        if (service_) {
            service_->PostEvent(event);
        }
    }
}

bool A2dpSnkStateMachine::HasDeferredMessage(int what) const
{
    for (const auto &msg : deferMsgs_) {
        if (msg.what_ == what) {
            return true;
        }
    }
    return false;
}

void A2dpSnkStateMachine::RemoveDeferredMessages(int what)
{
    auto it = deferMsgs_.begin();
    while (it != deferMsgs_.end()) {
        if (it->what_ == what) {
            it = deferMsgs_.erase(it);
        } else {
            ++it;
        }
    }
}

void A2dpSnkStateMachine::DeferUserCommand(const A2dpSnkMessage &msg)
{
    int opposite = (msg.what_ == A2DP_SNK_CONN_CONNECT_EVT)
                       ? A2DP_SNK_CONN_DISCONNECT_EVT
                       : A2DP_SNK_CONN_CONNECT_EVT;
    RemoveDeferredMessages(opposite);
    if (!HasDeferredMessage(msg.what_)) {
        AddDeferredMessage(msg);
    }
}

// --------------------------------------------------已断开
void A2dpSnkDisconnectedState::Entry()
{
    HILOGI("[Disconnected] entry");

    // Disconnected 状态忽略 DISCONNECT 消息
    stateMachine_.RemoveDeferredMessages(A2DP_SNK_CONN_DISCONNECT_EVT);

    if (isReentry_) {
        // 回退进入 Disconnected（非初始化）：
        //   有 deferred CONNECT → 保留状态机，重投 deferred 消息等 CONNECT 处理
        //   无 deferred CONNECT → 删除状态机
        bool hasPendingConnect = stateMachine_.HasDeferredMessage(A2DP_SNK_CONN_CONNECT_EVT);
        if (hasPendingConnect) {
            HILOGI("[Disconnected] has deferred CONNECT, keep state machine");
            stateMachine_.NotifyStateChanged();
            stateMachine_.ProcessDeferredMessage();
        } else {
            stateMachine_.SetRemoving(true);
            stateMachine_.NotifyStateChanged();
            auto *service = stateMachine_.GetService();
            if (service) {
                service->ProcessRemoveStateMachine(stateMachine_.GetDeviceName());
            } else {
                HILOGE("[Disconnected] A2dpSnkService is nullptr!");
            }
        }
    }
}

bool A2dpSnkDisconnectedState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Disconnected] EventName=%{public}s", A2dpSnkStateMachine::GetEventName(event.what_).c_str());
    auto *svc = stateMachine_.GetService();
    switch (event.what_) {
        case A2DP_SNK_CONN_CONNECT_EVT:
            // 本地发起连接：native connect 是异步投递到 btif 线程，成功后转 Connecting 等待栈回调。
            // 入队失败（极端场景）停留 Disconnected，避免 20s 超时白等。
            if (svc && svc->NativeConnect(RawAddress(event.addr_))) {
                Transition(A2dpSnkStateMachine::CONNECTING);
            } else {
                HILOGE("[Disconnected] NativeConnect failed, stay Disconnected");
            }
            break;
        case A2DP_SNK_CONN_STACK_CONNECTING_EVT:
            if (stateMachine_.AllowIncomingConnection(event.addr_)) {
                Transition(A2dpSnkStateMachine::CONNECTING);
            }
            break;
        case A2DP_SNK_CONN_STACK_CONNECTED_EVT:
            if (stateMachine_.AllowIncomingConnection(event.addr_)) {
                Transition(A2dpSnkStateMachine::CONNECTED);
            }
            break;
        default:
            break;
    }
    return true;
}

void A2dpSnkDisconnectedState::Exit()
{
    HILOGI("[Disconnected] Exit");
    isReentry_ = true;
}

// --------------------------------------------------连接中
void A2dpSnkConnectingState::Entry()
{
    HILOGI("[Connecting] entry");
    stateMachine_.NotifyStateChanged();
    stateMachine_.StartConnectionTimer();
}

bool A2dpSnkConnectingState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Connecting] EventName=%{public}s", A2dpSnkStateMachine::GetEventName(event.what_).c_str());
    auto *svc = stateMachine_.GetService();
    switch (event.what_) {
        case A2DP_SNK_CONN_TIMEOUT_EVT:
            HILOGW("[Connecting] connect timeout!");
            if (svc) svc->NativeDisconnect(RawAddress(event.addr_));
            Transition(A2dpSnkStateMachine::DISCONNECTED);
            break;
        case A2DP_SNK_CONN_DISCONNECT_EVT:
        case A2DP_SNK_CONN_CONNECT_EVT:
            stateMachine_.DeferUserCommand(event);
            break;
        case A2DP_SNK_CONN_STACK_CONNECTED_EVT:
            if (stateMachine_.AllowIncomingConnection(event.addr_)) {
                Transition(A2dpSnkStateMachine::CONNECTED);
            } else {
                Transition(A2dpSnkStateMachine::DISCONNECTED);
            }
            break;
        case A2DP_SNK_CONN_STACK_DISCONNECTED_EVT:
            Transition(A2dpSnkStateMachine::DISCONNECTED);
            break;

        default:
            break;
    }
    return true;
}

void A2dpSnkConnectingState::Exit()
{
    HILOGI("[Connecting] Exit");
    stateMachine_.StopConnectionTimer();
}

// ---------------------------------------------------已连接
void A2dpSnkConnectedState::Entry()
{
    HILOGI("[Connected] entry");

    stateMachine_.NotifyStateChanged();
    stateMachine_.ProcessDeferredMessage();
}

bool A2dpSnkConnectedState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Connected] EventName=%{public}s", A2dpSnkStateMachine::GetEventName(event.what_).c_str());
    auto *svc = stateMachine_.GetService();

    switch (event.what_) {
        case A2DP_SNK_CONN_DISCONNECT_EVT:
            if (svc) {
                svc->NativeDisconnect(RawAddress(event.addr_));
            }
            Transition(A2dpSnkStateMachine::DISCONNECTING);
            break;
        case A2DP_SNK_CONN_STACK_DISCONNECTING_EVT:
            Transition(A2dpSnkStateMachine::DISCONNECTING);
            break;
        case A2DP_SNK_CONN_STACK_DISCONNECTED_EVT:
            Transition(A2dpSnkStateMachine::DISCONNECTED);
            break;
        case A2DP_SNK_CONN_STACK_AUDIO_CONFIG_EVT:
            HILOGI("[Connected] update audio config sampleRate=%{public}u channelCount=%{public}u",
                event.sampleRate_, event.channelCount_);
            {
                A2dpSnkCodecInfo info{};
                info.sampleRate = event.sampleRate_;
                info.channelMode = event.channelCount_;
                stateMachine_.SetAudioConfig(info);
            }
            break;
        default:
            break;
    }
    return true;
}

void A2dpSnkConnectedState::Exit()
{
    HILOGI("[Connected] Exit");
}

// ---------------------------------------------------断开中
void A2dpSnkDisconnectingState::Entry()
{
    HILOGI("[Disconnecting] entry");
    stateMachine_.NotifyStateChanged();
    stateMachine_.StartConnectionTimer();
}

bool A2dpSnkDisconnectingState::Dispatch(const utility::Message &msg)
{
    A2dpSnkMessage &event = static_cast<A2dpSnkMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Disconnecting] EventName=%{public}s", A2dpSnkStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case A2DP_SNK_CONN_CONNECT_EVT:
        case A2DP_SNK_CONN_DISCONNECT_EVT:
            stateMachine_.DeferUserCommand(event);
            break;
        case A2DP_SNK_CONN_STACK_CONNECTING_EVT:
            if (stateMachine_.AllowIncomingConnection(event.addr_)) {
                Transition(A2dpSnkStateMachine::CONNECTING);
            }
            break;
        case A2DP_SNK_CONN_STACK_CONNECTED_EVT:
            if (stateMachine_.AllowIncomingConnection(event.addr_)) {
                Transition(A2dpSnkStateMachine::CONNECTED);
            }
            break;
        case A2DP_SNK_CONN_STACK_DISCONNECTED_EVT:
            Transition(A2dpSnkStateMachine::DISCONNECTED);
            break;
        default:
            break;
    }
    return true;
}

void A2dpSnkDisconnectingState::Exit()
{
    HILOGI("[Disconnecting] Exit");
    stateMachine_.StopConnectionTimer();
}

}  // namespace bluetooth
}  // namespace OHOS