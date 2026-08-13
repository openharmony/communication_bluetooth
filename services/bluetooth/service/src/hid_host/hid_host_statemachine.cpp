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
#define LOG_TAG "bt_service_hid_host"
#endif

#include "hid_host_statemachine.h"

#include "bt_hh.h"
#include "hid_host_service.h"
#include "common_util.h"
#include "remote_device_properties.h"
#include "service_util.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {

HidHostStateMachine::HidHostStateMachine(const std::string &address)
    : address_(address)
{
}

void HidHostStateMachine::Init()
{
    connTimer_ = std::make_unique<utility::Timer>(std::bind([this] {this->ConnectionTimeout();}));
    disconnTimer_ = std::make_unique<utility::Timer>(std::bind([this] {this->DisconnectionTimeout();}));

    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<HidHostDisconnectedState>(DISCONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<HidHostConnectingState>(CONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> disconnectingState =
        std::make_unique<HidHostDisconnectingState>(DISCONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<HidHostConnectedState>(CONNECTED, *this);

    Move(disconnectedState);
    Move(connectingState);
    Move(disconnectingState);
    Move(connectedState);

    InitState(DISCONNECTED);
}

bool HidHostStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void HidHostStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

std::string HidHostStateMachine::GetDeviceAddress()
{
    return address_;
}

void HidHostDisconnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();

    if (isReentry_) {
        stateMachine_.SetRemoving(true);
        stateMachine_.NotifyStateTransitions();
        HidHostService *service = HidHostService::GetService();
        if (service != nullptr) {
            service->RemoveStateMachine(stateMachine_.GetDeviceAddress());
        } else {
            HILOGE("[HID_MACHINE]HidHostService is nullptr!");
        }
    }
}

void HidHostDisconnectedState::Exit()
{
    isReentry_ = true;
}

bool HidHostDisconnectedState::Dispatch(const utility::Message &msg)
{
    HidHostMessage &event = static_cast<HidHostMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HID_MACHINE][Disconnected]EventName=%{public}s", HidHostStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_CONNECT_EVT: {
            if (!stateMachine_.ProcessConnectEvent()) {
                break;
            }
            Transition(HidHostStateMachine::CONNECTING);
            break;
        }
        case HID_HOST_CONNECTION_STATE_CHANGED_EVT: {
            uint8_t state = event.sendData_.param;
            if (state == HID_HOST_STATE_CONNECTING) {
                Transition(HidHostStateMachine::CONNECTING);
            }
            if (state == HID_HOST_STATE_CONNECTED) {
                Transition(HidHostStateMachine::CONNECTED);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void HidHostConnectingState::Entry()
{
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}
void HidHostConnectingState::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool HidHostConnectingState::Dispatch(const utility::Message &msg)
{
    HidHostMessage &event = static_cast<HidHostMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HID_MACHINE][Connecting]EventName=%{public}s", HidHostStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_CONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HID_HOST_DISCONNECT_EVT:
            stateMachine_.ProcessDisConnectEvent();
            Transition(HidHostStateMachine::DISCONNECTED);
            break;
        case HID_HOST_CONNECTION_STATE_CHANGED_EVT: {
            uint8_t state = event.sendData_.param;
            if (state == HID_HOST_STATE_CONNECTED) {
                Transition(HidHostStateMachine::CONNECTED);
            }
            if (state == HID_HOST_STATE_DISCONNECTED) {
                Transition(HidHostStateMachine::DISCONNECTED);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void HidHostDisconnectingState::Entry()
{
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartDisconnectionTimer();
}

void HidHostDisconnectingState::Exit()
{
    stateMachine_.StopDisconnectionTimer();
}

bool HidHostDisconnectingState::Dispatch(const utility::Message &msg)
{
    HidHostMessage &event = static_cast<HidHostMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HID_MACHINE]Disconnecting]EventName=%{public}s", HidHostStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_CONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HID_HOST_DISCONNECT_EVT:
            Transition(HidHostStateMachine::DISCONNECTED);
            break;
        case HID_HOST_CONNECTION_STATE_CHANGED_EVT: {
            uint8_t state = event.sendData_.param;
            HILOGI("[HID_MACHINE][Disconnecting]ConnectState=%{public}d]", state);
            if (state == HID_HOST_STATE_DISCONNECTED) {
                Transition(HidHostStateMachine::DISCONNECTED);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void HidHostConnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
}

void HidHostConnectedState::Exit()
{
}

bool HidHostConnectedState::Dispatch(const utility::Message &msg)
{
    HidHostMessage &event = static_cast<HidHostMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HID_MACHINE][Connected]EventName=%{public}s", HidHostStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_CONNECT_EVT:
            HILOGE("[HID_MACHINE][Connected] device has been connected!");
            break;
        case HID_HOST_DISCONNECT_EVT: {
            if (!stateMachine_.ProcessDisConnectEvent()) {
                Transition(HidHostStateMachine::DISCONNECTED);
                break;
            }
            Transition(HidHostStateMachine::DISCONNECTING);
            break;
        }
        case HID_HOST_CONNECTION_STATE_CHANGED_EVT: {
            uint8_t state = event.sendData_.param;
            HILOGI("[HID_MACHINE][Connected]ConnectState=%{public}d]", state);
            if (state == HID_HOST_STATE_CONNECTED) {
                Transition(HidHostStateMachine::CONNECTED);
                break;
            }
            if (state == HID_HOST_STATE_DISCONNECTED) {
                Transition(HidHostStateMachine::DISCONNECTED);
            }
            break;
        }
        case HID_HOST_VC_UNPLUG_EVT:
            stateMachine_.ProcessVCUnplugEvent();
            break;
        case HID_HOST_SET_REPORT_EVT:
            stateMachine_.ProcessSetReportEvent(event);
            break;
        case HID_HOST_GET_REPORT_EVT:
            stateMachine_.ProcessGetReportEvent(event);
            break;
        default:
            break;
    }
    return true;
}

bool HidHostStateMachine::ProcessConnectEvent()
{
    RawAddress device(address_);
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    HidHostService *hidHostService = HidHostService::GetService();
    if (hidHostService == nullptr) {
        HILOGE("hidHostService is null");
        return false;
    }
    bthh_interface_t* bluetoothHidInterface = hidHostService->getBluetoothHidInterface();
    if (bluetoothHidInterface == nullptr) {
        return false;
    }
    HILOGI("[HID_MACHINE]Hid device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHidInterface->connect(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HID_MACHINE]Failed HID channel connection, status: %{public}d", status);
        return false;
    }
    return true;
}

bool HidHostStateMachine::ProcessDisConnectEvent()
{
    HidHostService *hidHostService = HidHostService::GetService();
    if (hidHostService == nullptr) {
        HILOGE("hidHostService is null");
        return false;
    }
    bthh_interface_t* bluetoothHidInterface = hidHostService->getBluetoothHidInterface();
    if (bluetoothHidInterface == nullptr) {
        return false;
    }
    RawAddress device(address_);
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    HILOGI("[HID_MACHINE]Hid device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHidInterface->disconnect(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HID_MACHINE]Failed disconnect hid channel, status: %{public}d", status);
        return false;
    }
    return true;
}

void HidHostStateMachine::ProcessVCUnplugEvent()
{
    RawAddress device(address_);
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    HidHostService *hidHostService = HidHostService::GetService();
    if (hidHostService == nullptr) {
        HILOGE("hidHostService is null");
        return;
    }
    bthh_interface_t* bluetoothHidInterface = hidHostService->getBluetoothHidInterface();
    if (bluetoothHidInterface == nullptr) {
        return;
    }
    HILOGI("[HID_MACHINE]Hid device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHidInterface->virtual_unplug(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HID_MACHINE]Failed virtual unplug, status: %{public}d", status);
        return;
    }
}

void HidHostStateMachine::ProcessSetReportEvent(const HidHostMessage &msg)
{
    RawAddress device(address_);
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    HidHostService *hidHostService = HidHostService::GetService();
    if (hidHostService == nullptr) {
        HILOGE("hidHostService is null");
        return;
    }
    bthh_interface_t* bluetoothHidInterface = hidHostService->getBluetoothHidInterface();
    if (bluetoothHidInterface == nullptr) {
        return;
    }
    HILOGI("[HID_MACHINE]hid device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHidInterface->set_report(
        &rawAddr, static_cast<bthh_report_type_t>(msg.sendData_.type), reinterpret_cast<char*>(msg.data_.get()));
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HID_MACHINE]Failed set report, status: %{public}d", status);
        return;
    }
}

void HidHostStateMachine::ProcessGetReportEvent(const HidHostMessage &msg)
{
    RawAddress device(address_);
    BLUEDROID::RawAddress rawAddr = ServiceUtil::AddrToBluedroid(device);
    HidHostService *hidHostService = HidHostService::GetService();
    if (hidHostService == nullptr) {
        HILOGE("hidHostService is null");
        return;
    }
    bthh_interface_t* bluetoothHidInterface = hidHostService->getBluetoothHidInterface();
    if (bluetoothHidInterface == nullptr) {
        return;
    }
    HILOGI("[HID_MACHINE]Hid device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHidInterface->get_report(
        &rawAddr, static_cast<bthh_report_type_t>(msg.sendData_.type), msg.sendData_.reportId, msg.sendData_.dataSize);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HID_MACHINE]Failed get report, status: %{public}d", status);
        return;
    }
}

int HidHostStateMachine::GetDeviceStateInt() const
{
    const HidHostState* state = static_cast<const HidHostState*>(GetState());
    if (state == nullptr) {
        return HID_HOST_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

void HidHostStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(CONNECTION_TIMEOUT_MS);
    HILOGI("[HID_MACHINE]Start connection timer!");
}

void HidHostStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    HILOGI("[HID_MACHINE]Stop connection timer!");
}

void HidHostStateMachine::ConnectionTimeout() const
{
    HITRACE_METER(BT_TRACE_TAG);
    HidHostMessage event(HID_HOST_CONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    HidHostService *hidHostService = HidHostService::GetService();
    if (hidHostService == nullptr) {
        HILOGE("hidHostService is null");
        return;
    }
    hidHostService->PostEvent(event);
}

void HidHostStateMachine::StartDisconnectionTimer() const
{
    disconnTimer_->Start(DISCONNECTION_TIMEOUT_MS);
    HILOGI("[HID_MACHINE]Start disconnection timer!");
}

void HidHostStateMachine::StopDisconnectionTimer() const
{
    disconnTimer_->Stop();
    HILOGI("[HID_MACHINE]Stop disconnection timer!");
}

void HidHostStateMachine::DisconnectionTimeout() const
{
    HITRACE_METER(BT_TRACE_TAG);
    HidHostMessage event(HID_HOST_DISCONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    HidHostService *hidHostService = HidHostService::GetService();
    if (hidHostService == nullptr) {
        HILOGE("hidHostService is null");
        return;
    }
    hidHostService->PostEvent(event);
}

std::string HidHostStateMachine::GetEventName(int what)
{
    switch (what) {
        case HID_HOST_CONNECTION_TIMEOUT_EVT:
            return "HID_HOST_CONNECTION_TIMEOUT_EVT";
        case HID_HOST_DISCONNECTION_TIMEOUT_EVT:
            return "HID_HOST_DISCONNECTION_TIMEOUT_EVT";
        case HID_HOST_CONNECT_EVT:
            return "HID_HOST_CONNECT_EVT";
        case HID_HOST_DISCONNECT_EVT:
            return "HID_HOST_DISCONNECT_EVT";
        case HID_HOST_CONNECTION_STATE_CHANGED_EVT:
            return "HID_HOST_CONNECTION_STATE_CHANGED_EVT";
        case HID_HOST_VC_UNPLUG_EVT:
            return "HID_HOST_VC_UNPLUG_EVT";
        case HID_HOST_SEND_DATA_EVT:
            return "HID_HOST_SEND_DATA_EVT";
        case HID_HOST_SET_REPORT_EVT:
            return "HID_HOST_SET_REPORT_EVT";
        case HID_HOST_GET_REPORT_EVT:
            return "HID_HOST_GET_REPORT_EVT";
        case HID_HOST_INT_HANDSK_EVT:
            return "HID_HOST_INT_HANDSK_EVT";
        case HID_HOST_REMOVE_STATE_MACHINE_EVT:
            return "HID_HOST_REMOVE_STATE_MACHINE_EVT";
        default:
            return "Unknown";
    }
}

void HidHostStateMachine::NotifyStateTransitions()
{
    HidHostService *service = HidHostService::GetService();
    int toState = GetDeviceStateInt();
    if (service != nullptr) {
        RawAddress device(address_);
        if ((preState_ != toState) && (preState_ <= HID_HOST_STATE_CONNECTED)
            && (toState <= HID_HOST_STATE_CONNECTED)) {
            service->NotifyStateChanged(device, toState);
        }
    }

    preState_ = toState;
}

void HidHostStateMachine::AddDeferredMessage(const HidHostMessage &msg)
{
    deferMsgs_.push_back(msg);
}

void HidHostStateMachine::ProcessDeferredMessage()
{
    auto size = deferMsgs_.size();
    while (size > 0 && !deferMsgs_.empty()) {
        HidHostMessage event = deferMsgs_.front();
        deferMsgs_.pop_front();
        HidHostService *hidHostService = HidHostService::GetService();
        if (hidHostService == nullptr) {
            HILOGE("hidHostService is null");
            return;
        }
        hidHostService->PostEvent(event);
        size--;
    }
}
}  // namespace bluetooth
}
