/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_pan_statemachine"
#endif

#include "pan_statemachine.h"
#include "common_util.h"
#include "pan_service.h"
#include "pan_service_impl.h"

namespace OHOS {
namespace Bluetooth {
PanStateMachine::PanStateMachine(const std::string &address)
    : address_(address)
{}

void PanStateMachine::Init(PanServiceImpl *serviceImpl)
{
    connTimer_ = std::make_unique<utility::Timer>([this]() { this->ConnectionTimeout(); });
    panServiceImpl_ = serviceImpl;

    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<PanDisconnectedState>(DISCONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<PanConnectingState>(CONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> disconnectingState =
        std::make_unique<PanDisconnectingState>(DISCONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<PanConnectedState>(CONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> gettingIpState =
        std::make_unique<PanNetworkEstablished>(NETWORKESTABLISHED, *this);

    reinterpret_cast<PanState*>(disconnectedState.get())->panServiceImpl_ = serviceImpl;
    reinterpret_cast<PanState*>(connectingState.get())->panServiceImpl_ = serviceImpl;
    reinterpret_cast<PanState*>(disconnectingState.get())->panServiceImpl_ = serviceImpl;
    reinterpret_cast<PanState*>(connectedState.get())->panServiceImpl_ = serviceImpl;
    reinterpret_cast<PanState*>(gettingIpState.get())->panServiceImpl_ = serviceImpl;

    Move(disconnectedState);
    Move(connectingState);
    Move(disconnectingState);
    Move(connectedState);
    Move(gettingIpState);

    InitState(DISCONNECTED);
}

std::string PanStateMachine::GetDeviceAdress()
{
    return address_;
}

void PanStateMachine::ConnectionTimeout() const
{
    PanMessage event(PAN_CONNECT_TIMEOUT_EVT);
    event.dev_ = address_;
    CHECK_AND_RETURN_LOG(panServiceImpl_, "panServiceImpl_ is nullptr");
    panServiceImpl_->PostEventFunc_(event);
}

void PanStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(CONNECTION_TIMEOUT_MS);
    HILOGI("Start connection timer!");
}

void PanStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    HILOGI("Stop connection timer!");
}

void PanDisconnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    if (isReentry_) {
        CHECK_AND_RETURN_LOG(panServiceImpl_, "panServiceImpl_ is nullptr");
        PanMessage event(PAN_REMOVE_STATE_MACHINE_EVT);
        event.dev_ = stateMachine_.GetDeviceAdress();
        panServiceImpl_->PostEventFunc_(event);
        stateMachine_.NotifyStateTransitions();
    }
}

void PanDisconnectedState::Exit()
{
    isReentry_ = true;
}

bool PanDisconnectedState::Dispatch(const utility::Message &msg)
{
    PanMessage &event = reinterpret_cast<PanMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Disconnected][%{public}s]", PanStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case PAN_CONNECT_EVT:
            stateMachine_.ProcessConnectReqEvent(event);
            break;
        case PAN_CONNECTED_IND_EVT:
            Transition(PanStateMachine::CONNECTED);
            stateMachine_.ProcessOpenEvent(event);
            break;
        case PAN_CONNECTING_IND_EVT:
            Transition(PanStateMachine::CONNECTING);
            break;
        case PAN_CONNECT_TIMEOUT_EVT:
        default:
            break;
    }
    return true;
}


void PanConnectingState::Entry()
{
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}
void PanConnectingState::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool PanConnectingState::Dispatch(const utility::Message &msg)
{
    PanMessage &event = reinterpret_cast<PanMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Connecting][%{public}s]", PanStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case PAN_DISCONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case PAN_CONNECTED_IND_EVT:
            Transition(PanStateMachine::CONNECTED);
            stateMachine_.ProcessOpenEvent(event);
            break;
        case PAN_DISCONNECTING_IND_EVT:
            Transition(PanStateMachine::DISCONNECTING);
            break;
        case PAN_DISCONNECTED_IND_EVT:
            stateMachine_.GetPanNetworkManager().UnregisterNetSupplier();
            Transition(PanStateMachine::DISCONNECTED);
            break;
        case PAN_CONNECT_TIMEOUT_EVT: {
            stateMachine_.ProcessCloseReqEvent(event);
            Transition(PanStateMachine::DISCONNECTED);
            break;
        }
        default:
            break;
    }
    return true;
}

void PanDisconnectingState::Entry()
{
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}

void PanDisconnectingState::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool PanDisconnectingState::Dispatch(const utility::Message &msg)
{
    PanMessage &event = reinterpret_cast<PanMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Disconnecting][%{public}s]", PanStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case PAN_DISCONNECTED_IND_EVT:
            stateMachine_.GetPanNetworkManager().UnregisterNetSupplier();
            Transition(PanStateMachine::DISCONNECTED);
            break;
        case PAN_CONNECT_TIMEOUT_EVT: {
            stateMachine_.ProcessCloseReqEvent(event);
            Transition(PanStateMachine::DISCONNECTED);
            break;
        }
        default:
            break;
    }
    return true;
}

void PanConnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
    int remoteRole = stateMachine_.GetPanRole();
    if (remoteRole == REMOTE_PANU_ROLE) {
        return;
    } else if ( remoteRole == REMOTE_NAP_ROLE) {
        SetupNapNetwork();
    } else {
        HILOGE("invalid remote role");
    }
}

void PanConnectedState::SetupNapNetwork()
{
    std::string ifname = panServiceImpl_->GetIfname();
    stateMachine_.GetPanNetworkManager().SetIfname(ifname);
    stateMachine_.GetPanNetworkManager().Clear();
    PanNetworkManager::SetDhcpResultCallback(
        [this](int evt, int ipType, const DhcpResult &result) {
            stateMachine_.GetPanNetworkManager().SaveDhcpResult(ipType, result);
            PanMessage event(evt);
            event.dev_ = stateMachine_.GetDeviceAdress();
            event.ifname_ = panServiceImpl_->GetIfname();
            stateMachine_.ProcessMessage(event);
        }
    );
    stateMachine_.GetPanNetworkManager().RegisterNetSupplier();
    stateMachine_.GetPanNetworkManager().UpdateNetSupplierInfo(true);
    stateMachine_.GetPanNetworkManager().RegisterDhcpClientCallback(ifname);
    int ret = stateMachine_.GetPanNetworkManager().StartDhcpClient(ifname);
    if (ret != DHCP_SUCCESS) {
        HILOGE("[Connected] StartDhcpClient failed: %{public}d", ret);
        stateMachine_.GetPanNetworkManager().UnregisterNetSupplier();
        PanMessage event;
        event.dev_ = stateMachine_.GetDeviceAdress();
        stateMachine_.ProcessCloseReqEvent(event);
    }
}

void PanConnectedState::Exit()
{
}

bool PanConnectedState::Dispatch(const utility::Message &msg)
{
    PanMessage &event = reinterpret_cast<PanMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[Connected][%{public}s]", PanStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case PAN_DISCONNECT_EVT:
            stateMachine_.ProcessCloseReqEvent(event);
            break;
        case PAN_DISCONNECTED_IND_EVT:
            stateMachine_.GetPanNetworkManager().UnregisterNetSupplier();
            Transition(PanStateMachine::DISCONNECTED);
            break;
        case PAN_DISCONNECTING_IND_EVT:
            Transition(PanStateMachine::DISCONNECTING);
            break;
        case PAN_GET_IP_SUCCESS_EVT:
            if (stateMachine_.GetPanNetworkManager().DealDhcpResult()) {
                Transition(PanStateMachine::NETWORKESTABLISHED);
            } else {
                stateMachine_.ProcessCloseReqEvent(event);
            }
            break;
        case PAN_GET_IP_FAIL_EVT:
            stateMachine_.GetPanNetworkManager().DealDhcpFailed();
            stateMachine_.ProcessCloseReqEvent(event);
            break;
        default:
            break;
    }
    return true;
}

int PanStateMachine::GetDeviceStateInt() const
{
    const PanState* state = static_cast<const PanState*>(GetState());
    if (state == nullptr) {
        return PAN_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

std::string PanStateMachine::GetEventName(int what)
{
    switch (what) {
        case PAN_DISCONNECTED_IND_EVT:
            return "PAN_DISCONNECTED_IND_EVT";
        case PAN_DISCONNECTING_IND_EVT:
            return "PAN_DISCONNECTING_IND_EVT";
        case PAN_CONNECTING_IND_EVT:
            return "PAN_CONNECTING_IND_EVT";
        case PAN_CONNECTED_IND_EVT:
            return "PAN_CONNECTED_IND_EVT";
        case PAN_DISCONNECT_EVT:
            return "PAN_DISCONNECT_EVT";
        case PAN_CONNECT_EVT:
            return "PAN_CONNECT_EVT";
        case PAN_CONNECT_TIMEOUT_EVT:
            return "PAN_CONNECT_TIMEOUT_EVT";
        case PAN_REMOVE_STATE_MACHINE_EVT:
            return "PAN_REMOVE_STATE_MACHINE_EVT";
        case PAN_GET_IP_SUCCESS_EVT:
            return "PAN_GET_IP_SUCCESS_EVT";
        case PAN_GET_IP_FAIL_EVT:
            return "PAN_GET_IP_FAIL_EVT";
        default:
            return "Unknown";
    }
}

void PanStateMachine::NotifyStateTransitions()
{
    CHECK_AND_RETURN_LOG(panServiceImpl_, "panServiceImpl_ is nullptr");
    int toState = GetDeviceStateInt();
    if (panServiceImpl_ != nullptr && panServiceImpl_->NotifyStateChangedFunc_ != nullptr) {
        bluetooth::RawAddress device(address_);
        if (preState_ != toState) {
            panServiceImpl_->NotifyStateChangedFunc_(device, toState, role_);
        }
    }

    preState_ = toState;
}

void PanStateMachine::AddDeferredMessage(const PanMessage &msg)
{
    deferMsgs_.push_back(msg);
}

void PanStateMachine::ProcessDeferredMessage()
{
    CHECK_AND_RETURN_LOG(panServiceImpl_, "panServiceImpl_ is nullptr");
    auto size = deferMsgs_.size();
    while (size > 0 && !deferMsgs_.empty()) {
        PanMessage event = deferMsgs_.front();
        deferMsgs_.pop_front();
        panServiceImpl_->PostEventFunc_(event);
        size--;
    }
}

void PanStateMachine::ProcessOpenEvent(const PanMessage &msg)
{
    CHECK_AND_RETURN_LOG(panServiceImpl_, "panServiceImpl_ is nullptr");
    if (role_ == REMOTE_PANU_ROLE && panServiceImpl_->IsTetheringOnFunc_ && !panServiceImpl_->IsTetheringOnFunc_()) {
        HILOGI("tethering is off, disconnect now!");
        PanMessage event(PAN_DISCONNECT_EVT);
        event.dev_ = address_;
        panServiceImpl_->ProcessDisconnect(event);
        return;
    }
    if (panServiceImpl_->BringUpNetwork() != PAN_SUCCESS) {
        HILOGE("BringUpNetwork failed");
    }
}

void PanStateMachine::SetPanRole(const PanMessage &msg)
{
    role_ = msg.remoteRole;
}

int  PanStateMachine::GetPanRole()
{
    return role_;
}

PanNetworkManager &PanStateMachine::GetPanNetworkManager()
{
    return panNetworkManager_;
}

void PanStateMachine::ProcessConnectReqEvent(const PanMessage &msg)
{
    CHECK_AND_RETURN_LOG(panServiceImpl_, "panServiceImpl_ is nullptr");
    BtpanInterface* panInterface = panServiceImpl_->GetPanInterface();
    if (panInterface == nullptr) {
        HILOGE("panInterface is nullptr.");
        return;
    }
    OHOS::bluetooth::RawAddress rawAddr(address_);
    HILOGI("Connect address[%{public}s]", bluetooth::GetEncryptAddr(address_).c_str());
    BtStackStatus status = panInterface->connect(&rawAddr, LOCAL_PANU_ROLE, REMOTE_NAP_ROLE);
    if (status != BT_STATUS_SUCCESS && status != BT_STATUS_BUSY) {
        HILOGE("Failed connect pan channel, status: %{public}d", status);
        return;
    }
}

void PanStateMachine::ProcessCloseReqEvent(const PanMessage &msg)
{
    CHECK_AND_RETURN_LOG(panServiceImpl_, "panServiceImpl_ is nullptr");
    BtpanInterface* panInterface = panServiceImpl_->GetPanInterface();
    if (panInterface == nullptr) {
        HILOGE("panInterface is nullptr.");
        return;
    }
    OHOS::bluetooth::RawAddress rawAddr(address_);
    HILOGI("Disconnect address[%{public}s]", bluetooth::GetEncryptAddr(address_).c_str());
    BtStackStatus status = panInterface->disconnect(&rawAddr);
    if (status != BT_STATUS_SUCCESS && status != BT_STATUS_BUSY) {
        HILOGE("Failed disconnect pan channel, status: %{public}d", status);
        return;
    }
}

void PanNetworkEstablished::Entry()
{
    HILOGI("[NetworkEstablished] Entry");
    stateMachine_.NotifyStateTransitions();
}

void PanNetworkEstablished::Exit()
{
    HILOGI("[NetworkEstablished] Exit");
    stateMachine_.GetPanNetworkManager().UnregisterNetSupplier();
}

bool PanNetworkEstablished::Dispatch(const utility::Message &msg)
{
    PanMessage &event = reinterpret_cast<PanMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[NetworkEstablished][%{public}s]", PanStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case PAN_DISCONNECT_EVT:
            stateMachine_.ProcessCloseReqEvent(event);
            break;
        case PAN_DISCONNECTED_IND_EVT:
            stateMachine_.GetPanNetworkManager().StopDhcpClient(
                panServiceImpl_->GetIfname());
            stateMachine_.GetPanNetworkManager().Clear();
            Transition(PanStateMachine::DISCONNECTED);
            break;
        default:
            break;
    }
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS
