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

#include "hid_host_statemachine.h"
#include "hid_host_service.h"

namespace bluetooth {
HidHostStateMachine::HidHostStateMachine(const std::string &address)
    : address_(address), l2capConnection_(address), uhid_(address), sdpClient_(address)
{}

void HidHostStateMachine::Init()
{
    connTimer_ = std::make_unique<utility::Timer>(
        std::bind(&bluetooth::HidHostStateMachine::ConnectionTimeout, this));

    disconnTimer_ = std::make_unique<utility::Timer>(
        std::bind(&bluetooth::HidHostStateMachine::DisonnectionTimeout, this));

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

uint16_t HidHostStateMachine::GetDeviceControlLcid()
{
    return l2capConnection_.GetControlLcid();
}

uint16_t HidHostStateMachine::GetDeviceInterruptLcid()
{
    return l2capConnection_.GetInterruptLcid();
}

void HidHostStateMachine::ProcessL2capConnectionEvent(
    const HidHostMessage &event)
{
    l2capConnection_.ProcessEvent(event);
}

std::string HidHostStateMachine::GetDeviceAdress()
{
    return address_;
}

void HidHostDisconnectedState::Entry()
{
    stateMachine_.ProcessDeferredMessage();

    if (isReentry_) {
        stateMachine_.SetRemoving(true);
        HidHostService *service = HidHostService::GetService();
        if (service != nullptr) {
            service->RemoveStateMachine(stateMachine_.GetDeviceAdress());
        } else {
            LOG_ERROR("[HIDH Machine]%{public}s():HidHostService is nullptr!", __FUNCTION__);
        }
        stateMachine_.NotifyStateTransitions();
    }
}

void HidHostDisconnectedState::Exit()
{
    isReentry_ = true;
}

bool HidHostDisconnectedState::Dispatch(const utility::Message &msg)
{
    HidHostMessage &event = (HidHostMessage &)msg;
    LOG_DEBUG("[HIDH Machine]%{public}s():[Disconnected][%{public}s]",
        __FUNCTION__, HidHostStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_API_OPEN_EVT:
            stateMachine_.ProcessStartSdp(event);
            Transition(HidHostStateMachine::CONNECTING);
            break;
        case HID_HOST_API_CLOSE_EVT:
            break;
        case HID_HOST_INT_OPEN_EVT:
            stateMachine_.ProcessOpenDevice(event);
            Transition(HidHostStateMachine::CONNECTING);
            break;
        case HID_HOST_INT_CLOSE_EVT:
            stateMachine_.ProcessCloseDevice(event);
            break;
        case HID_HOST_INT_DATA_EVT:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_INT_CTRL_DATA:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_INT_HANDSK_EVT:
            break;
        case HID_HOST_SDP_CMPL_EVT:
            break;
        case HID_HOST_API_WRITE_DEV_EVT:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_API_GET_DSCP_EVT:
            break;
        case HID_HOST_OPEN_CMPL_EVT:
            stateMachine_.ProcessOpenComplete(event);
            Transition(HidHostStateMachine::CONNECTED);
            break;
        case HID_HOST_L2CAP_CONNECT_REQ_EVT:
            Transition(HidHostStateMachine::CONNECTING);
            break;
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
    HidHostMessage &event = (HidHostMessage &)msg;
    LOG_DEBUG("[HIDH Machine]%{public}s():[Connecting][%{public}s]", __FUNCTION__,
        HidHostStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_API_OPEN_EVT:
            break;
        case HID_HOST_API_CLOSE_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HID_HOST_INT_OPEN_EVT:
            stateMachine_.ProcessOpenDevice(event);
            break;
        case HID_HOST_INT_CLOSE_EVT:
            Transition(HidHostStateMachine::DISCONNECTED);
            break;
        case HID_HOST_INT_DATA_EVT:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_INT_CTRL_DATA:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_INT_HANDSK_EVT:
            break;
        case HID_HOST_SDP_CMPL_EVT:
            stateMachine_.ProcessSdpComplete(event);
            break;
        case HID_HOST_API_WRITE_DEV_EVT:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_API_GET_DSCP_EVT:
            break;
        case HID_HOST_OPEN_CMPL_EVT:
            stateMachine_.ProcessOpenComplete(event);
            Transition(HidHostStateMachine::CONNECTED);
            break;
        case HID_HOST_CONNECTION_TIMEOUT_EVT:
            Transition(HidHostStateMachine::DISCONNECTED);
            break;
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
    HidHostMessage &event = (HidHostMessage &)msg;
    LOG_DEBUG("[HIDH Machine]%{public}s():[Disconnecting][%{public}s]", __FUNCTION__,
        HidHostStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_API_OPEN_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HID_HOST_API_CLOSE_EVT:
            break;
        case HID_HOST_INT_OPEN_EVT:
            stateMachine_.ProcessOpenDevice(event);
            break;
        case HID_HOST_INT_CLOSE_EVT:
            stateMachine_.ProcessCloseDevice(event);
            Transition(HidHostStateMachine::DISCONNECTED);
            break;
        case HID_HOST_INT_DATA_EVT:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_INT_CTRL_DATA:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_INT_HANDSK_EVT:
            break;
        case HID_HOST_SDP_CMPL_EVT:
            break;
        case HID_HOST_API_WRITE_DEV_EVT:
            PacketFree(event.l2capInfo.pkt);
            break;
        case HID_HOST_API_GET_DSCP_EVT:
            break;
        case HID_HOST_OPEN_CMPL_EVT:
            stateMachine_.ProcessOpenComplete(event);
            Transition(HidHostStateMachine::CONNECTED);
            break;
        case HID_HOST_DISCONNECTION_TIMEOUT_EVT:
            Transition(HidHostStateMachine::CONNECTED);
            break;
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
    HidHostMessage &event = (HidHostMessage &)msg;
    LOG_DEBUG("[HIDH Machine]%{public}s():[Connected][%{public}s]", __FUNCTION__,
        HidHostStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HID_HOST_API_OPEN_EVT:
            break;
        case HID_HOST_API_CLOSE_EVT:
            stateMachine_.ProcessCloseDeviceReq(event);
            Transition(HidHostStateMachine::DISCONNECTING);
            break;
        case HID_HOST_INT_OPEN_EVT:
            break;
        case HID_HOST_INT_CLOSE_EVT:
            stateMachine_.ProcessCloseDevice(event);
            Transition(HidHostStateMachine::DISCONNECTED);
            break;
        case HID_HOST_INT_DATA_EVT:
            stateMachine_.ProcessReciveData(event);
            break;
        case HID_HOST_INT_CTRL_DATA:
            stateMachine_.ProcessReciveControlData(event);
            break;
        case HID_HOST_INT_HANDSK_EVT:
            stateMachine_.ProcessReciveHandshake(event);
            break;
        case HID_HOST_SDP_CMPL_EVT:
            break;
        case HID_HOST_API_WRITE_DEV_EVT:
            stateMachine_.ProcessWriteData(event);
            break;
        case HID_HOST_OPEN_CMPL_EVT:
            break;
        default:
            break;
    }
    return true;
}

void HidHostStateMachine::ProcessStartSdp(const HidHostMessage &msg)
{
    if (sdpClient_.CheckIsSdpDone()) {
        HidHostMessage event(HID_HOST_SDP_CMPL_EVT, HID_HOST_SDP_SUCCESS);
        event.dev_ = address_;
        HidHostService::GetService()->PostEvent(event);
    } else {
        sdpClient_.DoDiscovery(address_);
    }
}

void HidHostStateMachine::ProcessOpenDevice(const HidHostMessage &msg)
{
    if (sdpClient_.CheckIsSdpDone()) {
        HidHostMessage event(HID_HOST_OPEN_CMPL_EVT);
        event.dev_ = address_;
        HidHostService::GetService()->PostEvent(event);
    } else {
        sdpClient_.DoDiscovery(address_);
    }
}

void HidHostStateMachine::ProcessCloseDeviceReq(const HidHostMessage &msg)
{
    l2capConnection_.Disconnect();
}

void HidHostStateMachine::ProcessCloseDevice(const HidHostMessage &msg)
{
    uhid_.Close();
    uhid_.Destroy();
}

void HidHostStateMachine::ProcessReciveData(const HidHostMessage &msg)
{
    uint16_t dataLength = PacketSize(msg.l2capInfo.pkt);
    uint8_t *data = nullptr;
    if (dataLength > 0) {
        data = (uint8_t *)malloc(dataLength);
    }
    if (data != nullptr) {
        (void)memset_s(data, dataLength, 0, dataLength);
        PacketRead(msg.l2capInfo.pkt, data, 0, dataLength);
        uhid_.SendData(data, dataLength);
        free(data);
    }

    PacketFree(msg.l2capInfo.pkt);
}

void HidHostStateMachine::ProcessReciveControlData(const HidHostMessage &msg)
{
    uint16_t dataLength = PacketSize(msg.l2capInfo.pkt);
    uint8_t *data = nullptr;
    if (dataLength > 0) {
        data = (uint8_t *)malloc(dataLength);
    }
    if (data != nullptr) {
        (void)memset_s(data, dataLength, 0, dataLength);
        PacketRead(msg.l2capInfo.pkt, data, 0, dataLength);
        uhid_.SendControlData(data, dataLength);
        free(data);
    } else {
        LOG_INFO("[HIDH Machine]%{public}s():data is null", __FUNCTION__);
        uhid_.SendControlData(nullptr, 0);
    }
    PacketFree(msg.l2capInfo.pkt);
}

void HidHostStateMachine::ProcessReciveHandshake(const HidHostMessage &msg)
{
    uhid_.SendHandshake(msg.arg1_);
}

void HidHostStateMachine::ProcessWriteData(const HidHostMessage &msg)
{
    l2capConnection_.SendData(msg.sendData.type, msg.sendData.param,
        msg.sendData.data, msg.sendData.reportId, msg.l2capInfo.pkt);
    PacketFree(msg.l2capInfo.pkt);
}

void HidHostStateMachine::ProcessSdpComplete(const HidHostMessage &msg)
{
    LOG_INFO("[HIDH Machine]%{public}s():result=%{public}d", __FUNCTION__, msg.arg1_);
    if ((msg.arg1_ == HID_HOST_SDP_SUCCESS) && sdpClient_.CheckIsSdpDone()) {
        l2capConnection_.Connect();
    } else if ((msg.arg1_ == HID_HOST_SDP_SUCCESS) && !sdpClient_.CheckIsSdpDone()) {
        sdpClient_.DoDiscovery(address_);
    } else {
        l2capConnection_.Disconnect();
    }
}

void HidHostStateMachine::ProcessOpenComplete(const HidHostMessage &msg)
{
    uhid_.Open();
    PnpInformation pnpInf = sdpClient_.GetRemoteSdpPnpInfo();
    HidInformation hidInf = sdpClient_.GetRemoteSdpHidInfo();

    char cachedName[] = { "Bluetooth HID" };
    LOG_INFO("[HIDH Machine]%{public}s():cachedName[%{public}s],vendorId[%{public}d],productId[%{public}d]",
        __FUNCTION__, cachedName, pnpInf.vendorId, pnpInf.productId);
    LOG_INFO("[HIDH Machine]%{public}s():version[%{public}d],ctryCode[%{public}d],descLength[%{public}d]",
        __FUNCTION__, pnpInf.version, hidInf.ctryCode, hidInf.descLength);
    uhid_.SendHidInfo(cachedName, pnpInf, hidInf);
}

int HidHostStateMachine::GetDeviceStateInt() const
{
    return static_cast<const HidHostState*>(GetState())->GetStateInt();
}

void HidHostStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(CONNECTION_TIMEOUT_MS);
    LOG_INFO("[HIDH Machine]%{public}s():Start connection timer!", __FUNCTION__);
}

void HidHostStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    LOG_INFO("[HIDH Machine]%{public}s():Stop connection timer!", __FUNCTION__);
}

void HidHostStateMachine::ConnectionTimeout() const
{
    HidHostMessage event(HID_HOST_CONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    HidHostService::GetService()->PostEvent(event);
}

void HidHostStateMachine::StartDisconnectionTimer() const
{
    disconnTimer_->Start(DISCONNECTION_TIMEOUT_MS);
    LOG_INFO("[HIDH Machine]%{public}s():Start disconnection timer!", __FUNCTION__);
}

void HidHostStateMachine::StopDisconnectionTimer() const
{
    disconnTimer_->Stop();
    LOG_INFO("[HIDH Machine]%{public}s():Stop disconnection timer!", __FUNCTION__);
}

void HidHostStateMachine::DisonnectionTimeout() const
{
    HidHostMessage event(HID_HOST_DISCONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    HidHostService::GetService()->PostEvent(event);
}

std::string HidHostStateMachine::GetEventName(int what)
{
    switch (what) {
        case HID_HOST_SERVICE_STARTUP_EVT:
            return "HID_HOST_SERVICE_STARTUP_EVT";
        case HID_HOST_SERVICE_SHUTDOWN_EVT:
            return "HID_HOST_SERVICE_SHUTDOWN_EVT";
        case HID_HOST_API_OPEN_EVT:
            return "HID_HOST_API_OPEN_EVT";
        case HID_HOST_API_CLOSE_EVT:
            return "HID_HOST_API_CLOSE_EVT";
        case HID_HOST_INT_OPEN_EVT:
            return "HID_HOST_INT_OPEN_EVT";
        case HID_HOST_INT_CLOSE_EVT:
            return "HID_HOST_INT_CLOSE_EVT";
        case HID_HOST_INT_DATA_EVT:
            return "HID_HOST_INT_DATA_EVT";
        case HID_HOST_INT_CTRL_DATA:
            return "HID_HOST_INT_CTRL_DATA";
        case HID_HOST_INT_HANDSK_EVT:
            return "HID_HOST_INT_HANDSK_EVT";
        case HID_HOST_SDP_CMPL_EVT:
            return "HID_HOST_SDP_CMPL_EVT";
        case HID_HOST_API_WRITE_DEV_EVT:
            return "HID_HOST_API_WRITE_DEV_EVT";
        case HID_HOST_API_GET_DSCP_EVT:
            return "HID_HOST_API_GET_DSCP_EVT";
        case HID_HOST_OPEN_CMPL_EVT:
            return "HID_HOST_OPEN_CMPL_EVT";
        case HID_HOST_L2CAP_CONNECT_REQ_EVT:
            return "HID_HOST_L2CAP_CONNECT_REQ_EVT";
        case HID_HOST_CONNECTION_TIMEOUT_EVT:
            return "HID_HOST_CONNECTION_TIMEOUT_EVT";
        case HID_HOST_DISCONNECTION_TIMEOUT_EVT:
            return "HID_HOST_DISCONNECTION_TIMEOUT_EVT";
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
    while (size-- > 0 && !deferMsgs_.empty()) {
        HidHostMessage event = deferMsgs_.front();
        deferMsgs_.pop_front();
        HidHostService::GetService()->PostEvent(event);
    }
}
}  // namespace bluetooth
