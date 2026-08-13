/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_service_bip_statemachine"
#endif

#include "log.h"
#include "avrcp_tg_bip_statemachine.h"
#include "avrcp_tg_bip_message.h"
#include "avrcp_cover_art_storage.h"
#include "avrcp_bip_service.h"

namespace OHOS {
namespace bluetooth {
namespace {
    inline static const std::string CONNECTED = "Connected";
    inline static const std::string DISCONNECTED = "Disconnected";
}

BipStateMachine::BipStateMachine(std::shared_ptr<ObexSocketDevice> socketDevice,
    const std::weak_ptr<BipService> bipService)
    : socketDevice_(socketDevice), bipService_(bipService)
{
    HILOGI("BipStateMachine create");
}

BipStateMachine::~BipStateMachine()
{
    HILOGI("BipStateMachine release");
    if (obexServerSession_ != nullptr) {
        obexServerSession_->Stop();
        obexServerSession_ = nullptr;
    }

    obexTransport_ = nullptr;
    bipObexServer_ = nullptr;
    socketDevice_ = nullptr;
}

void BipStateMachine::Init()
{
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<BipConnectedState>(CONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<BipDisconnectedState>(DISCONNECTED, *this);

    Move(connectedState);
    Move(disconnectedState);

    InitState(DISCONNECTED);
}

bool BipStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void BipStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

std::string BipStateMachine::GetDeviceAddress()
{
    if (socketDevice_ == nullptr) {
        HILOGI("socketDevice_ is null");
        return "";
    }
    return socketDevice_->mDeviceAddress;
}

void BipConnectedState::Entry()
{
    HILOGI("Entry ConnectedState");
    stateMachine_.ProcessConnectEvent();
}

void BipConnectedState::Exit()
{
    HILOGI("Exit ConnectedState");
}

bool BipConnectedState::Dispatch(const utility::Message &msg)
{
    BipMessage &event = static_cast<BipMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Connected] EventName=%{public}s", BipStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case BIP_DISCONNECT_EVT: {
            stateMachine_.ProcessDisConnectEvent();
            Transition(DISCONNECTED);
            break;
        }
        case BIP_CONNECT_EVT:
            HILOGI("Already connected, ignore connect event");
            break;
        default:
            break;
    }
    return true;
}

void BipDisconnectedState::Entry()
{
    HILOGI("Entry DisconnectedState");
    if (isReentry_) {
        stateMachine_.SetRemoving(true);
        stateMachine_.RemoveStateMachine(stateMachine_.GetDeviceAddress());
    }
}

void BipDisconnectedState::Exit()
{
    HILOGI("Exit DisconnectedState");
    isReentry_ = true;
}

bool BipDisconnectedState::Dispatch(const utility::Message &msg)
{
    BipMessage &event = static_cast<BipMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Disconnected]EventName=%{public}s", BipStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case BIP_CONNECT_EVT: {
            Transition(CONNECTED);
            break;
        }
        case BIP_DISCONNECT_EVT:
            HILOGI("Already disconnected, ignore disconnect event");
            break;
        default:
            break;
    }
    return true;
}

void BipStateMachine::ProcessConnectEvent()
{
    obexTransport_ = std::make_shared<ObexServerTransport>(socketDevice_);
    bipObexServer_ = std::make_shared<AvrcpBipObexServer>(socketDevice_->mDeviceAddress, bipService_);
    obexServerSession_ = std::make_shared<ObexServerSession>(obexTransport_, bipObexServer_);
    if (obexServerSession_) {
        obexServerSession_->Start();
    }
}
void BipStateMachine::ProcessDisConnectEvent()
{
    if (obexServerSession_ != nullptr) {
        obexServerSession_->Stop();
    }
    obexServerSession_ = nullptr;
    obexTransport_ = nullptr;
    bipObexServer_ = nullptr;
}

void BipStateMachine::RemoveStateMachine(const std::string &address)
{
    auto service = bipService_.lock();
    if (service) {
        service->RemoveStateMachine(address);
    }
}

int BipStateMachine::GetDeviceStateInt() const
{
    const BipState* state = static_cast<const BipState*>(GetState());
    if (state == nullptr) {
        return BIP_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

std::string BipStateMachine::GetEventName(int what)
{
    switch (what) {
        case BIP_CONNECT_EVT:
            return "BIP_CONNECT_EVT";
        case BIP_DISCONNECT_EVT:
            return "BIP_DISCONNECT_EVT";
        case BIP_REMOVE_STATE_MACHINE_EVT:
            return "BIP_REMOVE_STATE_MACHINE_EVT";
        default:
            return "UNKNOWN";
    }
}

}  // namespace bluetooth
}  // namespace OHOS