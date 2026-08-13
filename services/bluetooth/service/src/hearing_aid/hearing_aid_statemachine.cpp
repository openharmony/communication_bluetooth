/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_hearing_aid_statemachine"
#endif

#include "hearing_aid_statemachine.h"
#include "common_util.h"
#include "hearing_aid_service_impl.h"
#include "service_util.h"

namespace OHOS {
namespace bluetooth {
HearingAidStateMachine::HearingAidStateMachine(const std::string &address)
    : address_(address)
{
    HILOGI("device %{public}s statemachince construction", GetEncryptAddr(address).c_str());
}

void HearingAidStateMachine::Init()
{
    connTimer_ = std::make_unique<utility::Timer>([this]() { this->ConnectionTimeout(); });

    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<HearingAidDisconnectedState>(DISCONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<HearingAidConnectingState>(CONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> disconnectingState =
        std::make_unique<HearingAidDisconnectingState>(DISCONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<HearingAidConnectedState>(CONNECTED, *this);

    Move(disconnectedState);
    Move(connectingState);
    Move(disconnectingState);
    Move(connectedState);

    InitState(DISCONNECTED);
}

int HearingAidStateMachine::GetStateInt() const
{
    const HearingAidState* state = static_cast<const HearingAidState*>(GetState());
    if (state == nullptr) {
        return HEARING_AID_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

std::string HearingAidStateMachine::GetDeviceAddress()
{
    return address_;
}

bool HearingAidStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void HearingAidStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

void HearingAidStateMachine::ConnectionTimeout() const
{
    HearingAidMessage event(HEARING_AID_CONNECT_TIMEOUT_EVT, address_);
    HearingAidServiceImpl::GetServiceImpl()->PostEventToService(event);
}

void HearingAidStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(CONNECTION_TIMEOUT_MS);
    HILOGI("Start connection timer!");
}

void HearingAidStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    HILOGI("Stop connection timer!");
}

void HearingAidDisconnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();

    if (isReentry_) {
        stateMachine_.SetRemoving(true);
        HearingAidMessage event(HEARING_AID_REMOVE_STATE_MACHINE_EVT, stateMachine_.GetDeviceAddress());
        HearingAidServiceImpl::GetServiceImpl()->PostEventToService(event);
        stateMachine_.NotifyStateTransitions();
    }
}

void HearingAidDisconnectedState::Exit()
{
    isReentry_ = true;
}

bool HearingAidDisconnectedState::Dispatch(const utility::Message &msg)
{
    HearingAidMessage &event = reinterpret_cast<HearingAidMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("%{public}s:[%{public}s]", title_, HearingAidServiceImpl::GetHearingAidEventName(event.what_));
    BLUEDROID::RawAddress rawAddr;
    RawAddress addr;
    if (event.msgAddr.length() != 0) {
        addr = RawAddress(event.msgAddr);
        rawAddr = ServiceUtil::AddrToBluedroid(addr);
    }
    ::bluetooth::hearing_aid::HearingAidInterface* bluetoothHearingAidInterface = GetInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothHearingAidInterface != nullptr, false, "%{public}s:interface is null", title_);
    switch (event.what_) {
        case HEARING_AID_CONNECT_EVT:
            HILOGI("%{public}s:device connect address[%{public}s]", title_,
                GetEncryptAddr(stateMachine_.GetDeviceAddress()).c_str());
            bluetoothHearingAidInterface->Connect(rawAddr);
            Transition(HearingAidStateMachine::CONNECTING);
            break;
        case HEARING_AID_STACK_STATE_EVT:
            ProcessStackEvent(addr.GetAddress(), event.state);
            break;
        case HEARING_AID_DISCONNECT_EVT:
        case HEARING_AID_CONNECT_TIMEOUT_EVT:
        default:
            break;
    }
    return true;
}

void HearingAidDisconnectedState::ProcessStackEvent(std::string address, int stackState)
{
    HILOGI("%{public}s:process stack state %{public}d address[%{public}s]",
        title_, stackState, GetEncryptAddr(stateMachine_.GetDeviceAddress()).c_str());
    ::bluetooth::hearing_aid::HearingAidInterface* bluetoothHearingAidInterface = GetInterface();
    CHECK_AND_RETURN_LOG(bluetoothHearingAidInterface != nullptr, "%{public}s:interface is null", title_);
    switch (stackState) {
        case BTHEARING_AID_STATE_CONNECTING:
            if (HearingAidServiceImpl::GetServiceImpl()->GetServiceAllowConnect(RawAddress(address))) {
                HILOGI("%{public}s:incoming connection accepted %{public}s", title_, GetEncryptAddr(address).c_str());
                Transition(HearingAidStateMachine::CONNECTING);
            } else {
                bluetoothHearingAidInterface->Disconnect(ServiceUtil::AddrToBluedroid(RawAddress(address)));
            }
            break;
        case BTHEARING_AID_STATE_CONNECTED:
            if (HearingAidServiceImpl::GetServiceImpl()->GetServiceAllowConnect(RawAddress(address))) {
                HILOGI("%{public}s:connected from stack %{public}s", title_, GetEncryptAddr(address).c_str());
                Transition(HearingAidStateMachine::CONNECTED);
            } else {
                bluetoothHearingAidInterface->Disconnect(ServiceUtil::AddrToBluedroid(RawAddress(address)));
            }
            break;
        default:
            break;
    }
}

void HearingAidConnectingState::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}
void HearingAidConnectingState::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool HearingAidConnectingState::Dispatch(const utility::Message &msg)
{
    HearingAidMessage &event = reinterpret_cast<HearingAidMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("%{public}s:[%{public}s]", title_, HearingAidServiceImpl::GetHearingAidEventName(event.what_));
    BLUEDROID::RawAddress rawAddr;
    RawAddress addr;
    if (event.msgAddr.length() != 0) {
        addr = RawAddress(event.msgAddr);
        rawAddr = ServiceUtil::AddrToBluedroid(addr);
    }
    ::bluetooth::hearing_aid::HearingAidInterface* bluetoothHearingAidInterface = GetInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothHearingAidInterface != nullptr, false, "%{public}s:interface is null", title_);
    switch (event.what_) {
        case HEARING_AID_DISCONNECT_EVT:
            HILOGI("%{public}s:canel connect address[%{public}s]", title_,
                GetEncryptAddr(stateMachine_.GetDeviceAddress()).c_str());
            bluetoothHearingAidInterface->Disconnect(rawAddr);
            Transition(HearingAidStateMachine::DISCONNECTED);
            break;
        case HEARING_AID_CONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HEARING_AID_CONNECT_TIMEOUT_EVT:
            HILOGI("%{public}s:timeout address[%{public}s]", title_,
                GetEncryptAddr(stateMachine_.GetDeviceAddress()).c_str());
            bluetoothHearingAidInterface->Disconnect(rawAddr);
            // if another side is connected, add to accept list
            if (HearingAidServiceImpl::GetServiceImpl()->IsConnectedPeerDevices(
                addr, HearingAidServiceImpl::GetServiceImpl()->GetServiceData())) {
                bluetoothHearingAidInterface->AddToAcceptlist(rawAddr);
            }
            Transition(HearingAidStateMachine::DISCONNECTING);
            break;
        case HEARING_AID_STACK_STATE_EVT:
            ProcessStackEvent(addr.GetAddress(), event.state);
            break;
        default:
            break;
    }
    return true;
}

void HearingAidConnectingState::ProcessStackEvent(std::string address, int stackState)
{
    HILOGI("%{public}s:process stack state %{public}d address[%{public}s]",
        title_, stackState, GetEncryptAddr(stateMachine_.GetDeviceAddress()).c_str());
    switch (stackState) {
        case BTHEARING_AID_STATE_DISCONNECTED:
            HILOGW("%{public}s:stack disconnected address[%{public}s]", title_,
                GetEncryptAddr(stateMachine_.GetDeviceAddress()).c_str());
            Transition(HearingAidStateMachine::DISCONNECTED);
            break;
        case BTHEARING_AID_STATE_DISCONNECTING:
            HILOGW("%{public}s:stack disconnected address[%{public}s]", title_,
                GetEncryptAddr(stateMachine_.GetDeviceAddress()).c_str());
            Transition(HearingAidStateMachine::DISCONNECTING);
            break;
        case BTHEARING_AID_STATE_CONNECTED:
            Transition(HearingAidStateMachine::CONNECTED);
            break;
        case BTHEARING_AID_STATE_CONNECTING:
        default:
            break;
    }
}

void HearingAidDisconnectingState::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}

void HearingAidDisconnectingState::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool HearingAidDisconnectingState::Dispatch(const utility::Message &msg)
{
    HearingAidMessage &event = reinterpret_cast<HearingAidMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("%{public}s:[%{public}s]", title_, HearingAidServiceImpl::GetHearingAidEventName(event.what_));
    BLUEDROID::RawAddress rawAddr;
    RawAddress addr;
    ::bluetooth::hearing_aid::HearingAidInterface* bluetoothHearingAidInterface = GetInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothHearingAidInterface != nullptr, false, "%{public}s:interface is null", title_);
    if (event.msgAddr.length() != 0) {
        addr = RawAddress(event.msgAddr);
        rawAddr = ServiceUtil::AddrToBluedroid(addr);
    }
    switch (event.what_) {
        case HEARING_AID_DISCONNECT_EVT:
        case HEARING_AID_CONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HEARING_AID_CONNECT_TIMEOUT_EVT:
            HILOGW("%{public}s:disconnecting timeout", title_);
            bluetoothHearingAidInterface->Disconnect(rawAddr);
            Transition(HearingAidStateMachine::DISCONNECTED);
            break;
        case HEARING_AID_STACK_STATE_EVT:
            ProcessStackEvent(addr.GetAddress(), event.state);
            break;
        default:
            break;
    }
    return true;
}

void HearingAidDisconnectingState::ProcessStackEvent(std::string address, int stackState)
{
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(RawAddress(address));
    ::bluetooth::hearing_aid::HearingAidInterface* bluetoothHearingAidInterface = GetInterface();
    CHECK_AND_RETURN_LOG(bluetoothHearingAidInterface != nullptr, "%{public}s:interface is null", title_);
    switch (stackState) {
        case BTHEARING_AID_STATE_DISCONNECTED:
            Transition(HearingAidStateMachine::DISCONNECTED);
            break;
        case BTHEARING_AID_STATE_CONNECTING:
            if (HearingAidServiceImpl::GetServiceImpl()->GetServiceAllowConnect(RawAddress(address))) {
                HILOGW("%{public}s:disconnecting stopped, due to device try to reconnect",  title_);
                Transition(HearingAidStateMachine::CONNECTING);
            } else {
                HILOGW("%{public}s: incoming connecting rejected", title_);
                bluetoothHearingAidInterface->Disconnect(rawAddr);
            }
            break;
        case BTHEARING_AID_STATE_CONNECTED:
            if (HearingAidServiceImpl::GetServiceImpl()->GetServiceAllowConnect(RawAddress(address))) {
                HILOGW("%{public}s:disconnecting stopped, due to device connected",  title_);
                Transition(HearingAidStateMachine::CONNECTED);
            } else {
                HILOGW("%{public}s: incoming connected rejected", title_);
                bluetoothHearingAidInterface->Disconnect(rawAddr);
            }
            break;
        default:
            break;
    }
}

void HearingAidConnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
}

void HearingAidConnectedState::Exit()
{
}

bool HearingAidConnectedState::Dispatch(const utility::Message &msg)
{
    HearingAidMessage &event = reinterpret_cast<HearingAidMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("%{public}s:[%{public}s]", title_, HearingAidServiceImpl::GetHearingAidEventName(event.what_));
    BLUEDROID::RawAddress rawAddr;
    RawAddress addr;
    if (event.msgAddr.length() != 0) {
        addr = RawAddress(event.msgAddr);
        rawAddr = ServiceUtil::AddrToBluedroid(addr);
    }
    ::bluetooth::hearing_aid::HearingAidInterface* bluetoothHearingAidInterface = GetInterface();
    CHECK_AND_RETURN_LOG_RET(bluetoothHearingAidInterface != nullptr, false, "%{public}s:interface is null", title_);
    switch (event.what_) {
        case HEARING_AID_DISCONNECT_EVT:
            bluetoothHearingAidInterface->Disconnect(rawAddr);
            Transition(HearingAidStateMachine::DISCONNECTING);
            break;
        case HEARING_AID_STACK_STATE_EVT:
            ProcessStackEvent(addr.GetAddress(), event.state);
            break;
        case HEARING_AID_SET_VOLUME_BY_ADDR:
            bluetoothHearingAidInterface->SetVolumeByAddr(rawAddr, event.volume);
            break;
        case HEARING_AID_DISABLE_EVT:
            bluetoothHearingAidInterface->Disconnect(rawAddr);
            Transition(HearingAidStateMachine::DISCONNECTED);
            break;
        default:
            break;
    }
    return true;
}

void HearingAidConnectedState::ProcessStackEvent(std::string address, int stackState)
{
    switch (stackState) {
        case BTHEARING_AID_STATE_DISCONNECTED:
            Transition(HearingAidStateMachine::DISCONNECTED);
            break;
        case BTHEARING_AID_STATE_DISCONNECTING:
            Transition(HearingAidStateMachine::DISCONNECTING);
            break;
        case BTHEARING_AID_STATE_CONNECTING:
        case BTHEARING_AID_STATE_CONNECTED:
        default:
            break;
    }
}

void HearingAidStateMachine::NotifyStateTransitions()
{
    int toState = GetStateInt();
    RawAddress device(address_);
    if ((preState_ != toState) && (preState_ <= HEARING_AID_STATE_DISCONNECTING) &&
        (toState <= HEARING_AID_STATE_DISCONNECTING)) {
        HearingAidServiceImpl::GetServiceImpl()->NotifyStateChanged(
            device, preState_, toState, 0, HearingAidServiceImpl::GetServiceImpl()->GetServiceData());
    }

    preState_ = toState;
}

void HearingAidStateMachine::AddDeferredMessage(const HearingAidMessage &msg)
{
    deferMsgs_.push_back(msg);
}

void HearingAidStateMachine::ProcessDeferredMessage()
{
    auto size = deferMsgs_.size();
    while (size > 0 && !deferMsgs_.empty()) {
        HearingAidMessage event(deferMsgs_.front());
        deferMsgs_.pop_front();
        HearingAidServiceImpl::GetServiceImpl()->PostEventToService(event);
        size--;
    }
}

::bluetooth::hearing_aid::HearingAidInterface* HearingAidState::GetInterface()
{
    return HearingAidServiceImpl::GetServiceImpl()->GetServiceData().bluetoothHearingAidInterface_;
}

}  // namespace bluetooth
}  // namespace OHOS
