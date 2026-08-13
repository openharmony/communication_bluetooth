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
#define LOG_TAG "bt_service_opp_statemachine"
#endif

#include "common_util.h"
#include "opp_statemachine.h"
#include "opp_service_impl.h"
#include "remote_device_properties.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {
namespace {
    inline static const std::string DISCONNECTED = "Disconnected";
    inline static const std::string CONNECTING = "Connecting";
    inline static const std::string DISCONNECTING = "Disconnecting";
    inline static const std::string CONNECTED = "Connected";
}

OppStateMachine::OppStateMachine(const std::string &address, std::shared_ptr<ObexSocketDevice> socketDevice,
    OppServiceImpl *oppServiceImpl)
    : address_(address), socketDevice_(socketDevice), oppServiceImpl_(oppServiceImpl)
{}

OppStateMachine::~OppStateMachine()
{
    HILOGI("~OppStateMachine");
    if (obexServerSession_ != nullptr) {
        obexServerSession_->Stop();
    }
    obexServerSession_ = nullptr;
    obexTransport_ = nullptr;
    oppObexServer_ = nullptr;
}

void OppStateMachine::Init()
{
    connTimer_ = std::make_unique<utility::Timer>([this] {this->ConnectionTimeout();});
    disconnTimer_ = std::make_unique<utility::Timer>([this] {this->DisconnectionTimeout();});

    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<OppDisconnectedState>(DISCONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<OppConnectingState>(CONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> disconnectingState =
        std::make_unique<OppDisconnectingState>(DISCONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<OppConnectedState>(CONNECTED, *this);

    Move(disconnectedState);
    Move(connectingState);
    Move(disconnectingState);
    Move(connectedState);

    InitState(DISCONNECTED);
}

bool OppStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void OppStateMachine::ConnectionTimeout() const
{
    OppMessage event(OPP_CONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    oppServiceImpl_->PostEvent(event);
}

void OppStateMachine::DisconnectionTimeout() const
{
    OppMessage event(OPP_DISCONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    oppServiceImpl_->PostEvent(event);
}

void OppStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

bool OppStateMachine::ProcessSdpCompleteEvent(const OppMessage &msg)
{
    obexConfig_ = std::make_shared<ObexClientConfig>();
    obexConfig_->addr = msg.dev_;
    obexConfig_->l2capLocalPsm = msg.oppRecord_.l2capPsm;
    obexConfig_->l2capPsm = msg.oppRecord_.l2capPsm;
    obexConfig_->rfCommChannel = msg.oppRecord_.channel;
    obexConfig_->serviceUUID = msg.oppRecord_.uuid;
    oppServiceImpl_->ConnectObex(obexConfig_);
    return true;
}


void OppStateMachine::SetSocketService(std::shared_ptr<ObexSocketDevice> socketDevice)
{
    socketDevice_ = socketDevice;
}

int OppStateMachine::GetDeviceStateInt() const
{
    return static_cast<const OppState*>(GetState())->GetStateInt();
}

std::string OppStateMachine::GetDeviceAdress()
{
    return address_;
}

std::string OppStateMachine::GetEventName(int what)
{
    switch (what) {
        case OPP_CONNECT_REQ_EVT:
            return "OPP_CONNECT_REQ_EVT";
        case OPP_DISCONNECT_REQ_EVT:
            return "OPP_DISCONNECT_REQ_EVT";
        case OPP_CONNECTED_EVT:
            return "OPP_CONNECTED_EVT";
        case OPP_DISCONNECTED_EVT:
            return "OPP_DISCONNECTED_EVT";
        case OPP_SDP_CMPL_EVT:
            return "OPP_SDP_CMPL_EVT";
        case OPP_CONNECTION_TIMEOUT_EVT:
            return "OPP_CONNECTION_TIMEOUT_EVT";
        case OPP_DISCONNECTION_TIMEOUT_EVT:
            return "OPP_DISCONNECTION_TIMEOUT_EVT";
        default:
            return "Unknown";
    }
}

bool OppStateMachine::ProcessConnectedEvent(const OppMessage &msg)
{
    oppServiceImpl_->OnObexConnected(msg.dev_);
    return true;
}

void OppStateMachine::ProcessDisconnectReqEvent(const OppMessage &msg)
{
    oppServiceImpl_->DisconnectObex(address_);
}

void OppStateMachine::ProcessDisconnectedEvent(const OppMessage &msg)
{
}

bool OppStateMachine::ProcessConnectReqEvent(const OppMessage &msg)
{
    obexTransport_ = std::make_shared<ObexServerTransport>(socketDevice_);
    oppObexServer_ = std::make_shared<OppObexServer>(socketDevice_->mDeviceAddress, oppServiceImpl_);
    obexServerSession_ = std::make_shared<ObexServerSession>(obexTransport_, oppObexServer_);
    oppObexServer_->SetObexServerSession(obexServerSession_);
    obexServerSession_->Start();
    return true;
}

void OppStateMachine::RemoveStateMachine(const std::string &address)
{
    CHECK_AND_RETURN_LOG(oppServiceImpl_, "oppServiceImpl_ is null");
    oppServiceImpl_->RemoveStateMachine(address);
}

void OppDisconnectedState::Entry()
{
    HILOGI("OppDisconnectedState::Entry isReentry_ is %{public}d", isReentry_);
    if (!isReentry_) {
        return;
    }
    stateMachine_.SetRemoving(true);
    stateMachine_.RemoveStateMachine(stateMachine_.GetDeviceAdress());
    stateMachine_.NotifyStateTransitions();
}

bool OppDisconnectedState::Dispatch(const utility::Message &msg)
{
    OppMessage &event = static_cast<OppMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[OppDisconnectedState][%{public}s]", OppStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case OPP_CONNECT_REQ_EVT:
            stateMachine_.ProcessConnectReqEvent(event);
            Transition(CONNECTING);
            break;
        case OPP_SDP_CMPL_EVT:
            stateMachine_.ProcessSdpCompleteEvent(event);
            break;
        case OPP_CONNECTED_EVT:
            stateMachine_.ProcessConnectedEvent(event);
            Transition(CONNECTED);
            break;
        default:
            break;
    }
    return true;
}

void OppDisconnectedState::Exit()
{
    isReentry_ = true;
}

void OppConnectingState::Entry()
{
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}

bool OppConnectingState::Dispatch(const utility::Message &msg)
{
    OppMessage &event = static_cast<OppMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Connecting]EventName=%{public}s", OppStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case OPP_CONNECT_REQ_EVT:
            stateMachine_.ProcessConnectReqEvent(event);
            Transition(CONNECTED);
            break;
        case OPP_DISCONNECT_REQ_EVT:
            stateMachine_.ProcessDisconnectReqEvent(event);
            Transition(DISCONNECTING);
            break;
        case OPP_CONNECTED_EVT:
            stateMachine_.ProcessConnectedEvent(event);
            Transition(CONNECTED);
            break;
        case OPP_DISCONNECTED_EVT:
            stateMachine_.ProcessDisconnectedEvent(event);
            Transition(DISCONNECTED);
            break;
        case OPP_CONNECTION_TIMEOUT_EVT:
            stateMachine_.ProcessDisconnectedEvent(event);
            Transition(DISCONNECTED);
            break;
        default:
            break;
    }
    return true;
}

void OppConnectingState::Exit()
{
    stateMachine_.StopConnectionTimer();
}

void OppConnectedState::Entry()
{
    HILOGI("Entry ConnectedState");
    stateMachine_.NotifyStateTransitions();
}

bool OppConnectedState::Dispatch(const utility::Message &msg)
{
    OppMessage &event = static_cast<OppMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Connected]EventName=%{public}s", OppStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case OPP_CONNECT_REQ_EVT:
            stateMachine_.ProcessConnectReqEvent(event);
            Transition(CONNECTING);
            break;
        case OPP_DISCONNECT_REQ_EVT:
            stateMachine_.ProcessDisconnectReqEvent(event);
            Transition(DISCONNECTING);
            break;
        case OPP_DISCONNECTED_EVT:
            stateMachine_.ProcessDisconnectedEvent(event);
            Transition(DISCONNECTED);
            break;
        default:
            break;
    }
    return true;
}

void OppConnectedState::Exit()
{
}


void OppDisconnectingState::Entry()
{
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartDisconnectionTimer();
}

bool OppDisconnectingState::Dispatch(const utility::Message &msg)
{
    OppMessage &event = static_cast<OppMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Disconnecting]EventName=%{public}s", OppStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case OPP_CONNECTED_EVT:
            stateMachine_.ProcessConnectedEvent(event);
            Transition(CONNECTED);
            break;
        case OPP_DISCONNECTED_EVT:
            stateMachine_.ProcessDisconnectedEvent(event);
            Transition(DISCONNECTED);
            break;
        case OPP_DISCONNECTION_TIMEOUT_EVT:
            stateMachine_.ProcessDisconnectedEvent(event);
            Transition(DISCONNECTED);
            break;
        default:
            break;
    }
    return true;
}

void OppDisconnectingState::Exit()
{
    stateMachine_.StopDisconnectionTimer();
}

void OppStateMachine::NotifyStateTransitions()
{
    int toState = GetDeviceStateInt();
    if (oppServiceImpl_ != nullptr) {
        RawAddress device(address_);
        if ((preState_ != toState) &&
            (preState_ <= OPP_STATE_CONNECTED) &&
            (toState <= OPP_STATE_CONNECTED)) {
            oppServiceImpl_->NotifyStateChanged(device, toState);
        }
    }
    preState_ = toState;
}

void OppStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(connectionTimeoutMs);
    HILOGI("[OppStateMachine]:Start connection timer!");
}

void OppStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    HILOGI("[OPP Machine]:Stop connection timer!");
}

void OppStateMachine::StartDisconnectionTimer() const
{
    disconnTimer_->Start(disconnectionTimeoutMs);
    HILOGI("[OPP Machine]:Start disconnection timer!");
}

void OppStateMachine::StopDisconnectionTimer() const
{
    disconnTimer_->Stop();
    HILOGI("[OPP Machine]:Stop disconnection timer!");
}
}  // namespace bluetooth
}
