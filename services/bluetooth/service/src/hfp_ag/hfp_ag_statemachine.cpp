/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_hfp_ag"
#endif

#include "hfp_ag_statemachine.h"

#include "remote_device_properties.h"
#include "common_util.h"
#include "hfp_ag_defines.h"
#include "hfp_ag_message.h"
#include "hfp_ag_service.h"
#include "permission_manager.h"
#include "power_manager.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "hitrace_meter.h"
#include "common/bluetooth_hw_interface.h"
#include "refuse_play_helper.h"
#include "bt_func_hook.h"

namespace OHOS {
namespace bluetooth {
HfpAgStateMachine::HfpAgStateMachine(const std::string &address) : address_(address), eventProcessor_(address)
{
}

void HfpAgStateMachine::Init()
{
    connTimer_ = std::make_unique<utility::Timer>([this]() { this->ConnectionTimeout(); });

    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<HfpAgDisconnected>(DISCONNECTED, *this, eventProcessor_);
    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<HfpAgConnecting>(CONNECTING, *this, eventProcessor_);
    std::unique_ptr<utility::StateMachine::State> disconnectingState =
        std::make_unique<HfpAgDisconnecting>(DISCONNECTING, *this, eventProcessor_);
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<HfpAgConnected>(CONNECTED, *this, eventProcessor_);
    std::unique_ptr<utility::StateMachine::State> audioConnecting =
        std::make_unique<HfpAgAudioConnecting>(AUDIO_CONNECTING, *this, eventProcessor_, *connectedState);
    std::unique_ptr<utility::StateMachine::State> audioConnected =
        std::make_unique<HfpAgAudioConnected>(AUDIO_CONNECTED, *this, eventProcessor_, *connectedState);
    std::unique_ptr<utility::StateMachine::State> audioDisconnecting =
        std::make_unique<HfpAgAudioDisconnecting>(AUDIO_DISCONNECTING, *this, eventProcessor_, *connectedState);

    Move(disconnectedState);
    Move(connectingState);
    Move(disconnectingState);
    Move(audioConnecting);
    Move(audioConnected);
    Move(audioDisconnecting);
    Move(connectedState);   // Add parenet state at last
    InitState(DISCONNECTED);
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(GetDeviceAddr(),
        static_cast<int>(HFP_AG_STATE_DISCONNECTED));
}

int HfpAgStateMachine::GetPreState() const
{
    return preState_;
}

inline std::string HfpAgStateMachine::GetDeviceAddr() const
{
    return address_;
}

int HfpAgStateMachine::GetStateInt() const
{
    const HfpAgState* state = static_cast<const HfpAgState*>(GetState());
    if (state == nullptr) {
        return HFP_AG_STATE_DISCONNECTED;
    }
    return state->GetStateInt();
}

void HfpAgStateMachine::AddDeferredMessage(const HfpAgMessage &msg)
{
    deferMsgs_.push_back(msg);
}

void HfpAgStateMachine::ProcessDeferredMessage()
{
    auto size = deferMsgs_.size();
    while (size > 0 && !deferMsgs_.empty()) {
        HfpAgMessage event = deferMsgs_.front();
        deferMsgs_.pop_front();
        auto hfpAgService = HfpAgService::GetService();
        if (hfpAgService == nullptr) {
            return;
        }
        hfpAgService->PostEvent(event);
        size--;
    }
}

void HfpAgStateMachine::NotifyStateTransitions()
{
    int toState = GetStateInt();
    if (toState == HFP_AG_STATE_CONNECTED && preState_ == HFP_AG_STATE_CONNECTING) {
        HfpAgSystemInterface::GetInstance().QueryPhoneState();
    }

    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        RawAddress device(address_);
        if ((preState_ != toState) && (toState <= HFP_AG_STATE_CONNECTED)) {
            service->NotifySlcStateChanged(device, toState);
        }

        if ((preState_ != toState) && (preState_ >= HFP_AG_AUDIO_STATE_DISCONNECTED) &&
            (toState >= HFP_AG_AUDIO_STATE_DISCONNECTED)) {
            ProcessAudioStateChanged(address_, preState_, toState);
        }

        if (((toState == HFP_AG_STATE_CONNECTED) && (preState_ < toState)) ||
            ((toState == HFP_AG_STATE_DISCONNECTED) && (preState_ > toState))) {
            service->SlcStateChanged(address_, toState);
        }

        if (toState == HFP_AG_STATE_CONNECTED) {
            service->NotifyProfileState(device);
            service->NotifyWearDetectionActionAfterConnected(device);
        }

        if (preState_ != HFP_AG_STATE_DISCONNECTED && toState == HFP_AG_STATE_DISCONNECTED) {
            service->SendBsirToLastRemainingDevice();
        }
    }

    preState_ = toState;
}

void HfpAgStateMachine::ProcessAudioStateChanged(const std::string &address, int fromState, int toState)
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "service is null");
    RawAddress device(address);
    BtChrUeManager::GetInstance()->WriteConnecionStateChangeUe(CHR_UE_BRCAST_SCO_STATE_CHANGE, device, 0,
        fromState, toState);
    service->ScoStateChanged(address, toState);
    if (fromState == HFP_AG_AUDIO_STATE_CONNECTING && toState == HFP_AG_AUDIO_STATE_CONNECTED) {
        service->NotifyAudioStateChanged(device, toState, HFP_AG_SCO_LOCAL_USER_SET_UP);
    } else if (fromState == HFP_AG_STATE_CONNECTED && toState == HFP_AG_AUDIO_STATE_CONNECTED) {
        service->NotifyAudioStateChanged(device, toState, HFP_AG_SCO_REMOTE_USER_SET_UP);
        service->NotifyHfpStackChanged(UpdateOutputStackAction::ACTION_USER_OPERATION, RawAddress(address));
    } else {
        service->NotifyAudioStateChanged(device, toState);
    }
}

void HfpAgStateMachine::NotifyChildStateToParentState(int fromState, int toState, int reason)
{
    HILOGI("[HFP_STATE_MACHINE]FromState[%{public}d], toState[%{public}d], reason[%{public}d]",
        fromState, toState, reason);
    HfpAgService *service = HfpAgService::GetService();
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(address_, toState);
    if (service != nullptr) {
        service->ScoStateChanged(address_, toState);
        if ((fromState != toState) && (fromState >= HFP_AG_AUDIO_STATE_DISCONNECTED) &&
            (toState >= HFP_AG_AUDIO_STATE_DISCONNECTED)) {
            RawAddress device(address_);
            BtChrUeManager::GetInstance()->WriteConnecionStateChangeUe(CHR_UE_BRCAST_SCO_STATE_CHANGE, device, reason,
                preState_, toState);
            service->NotifyAudioStateChanged(device, toState, reason);
        }

        if (((toState == HFP_AG_STATE_CONNECTED) && (fromState < toState)) ||
            ((toState == HFP_AG_STATE_DISCONNECTED) && (fromState > toState))) {
            service->SlcStateChanged(address_, toState);
        }
    }
    preState_ = toState;
    ProcessDeferredMessage();
}

void HfpAgStateMachine::ProcessAudioDisconnected(int reason)
{
    IPowerManager::GetInstance()->StatusUpdate(RequestStatus::SCO_OFF, PROFILE_NAME_HFP_AG, RawAddress(address_));
    NotifyChildStateToParentState(HFP_AG_AUDIO_STATE_CONNECTED, HFP_AG_AUDIO_STATE_DISCONNECTED, reason);
    RefusePlayHelper::GetInstance()->SetLastHangUpTime(GetDeviceAddr());
}

void HfpAgStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(CONNECTION_TIMEOUT_MS);
    HILOGI("[HFP_STATE_MACHINE]Start connection timer!");
}

void HfpAgStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    HILOGI("[HFP_STATE_MACHINE]Stop connection timer!");
}

bool HfpAgStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void HfpAgStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

void HfpAgStateMachine::ConnectionTimeout() const
{
    HITRACE_METER(BT_TRACE_TAG);
    HfpAgMessage event(HFP_AG_CONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    hfpAgService->PostEvent(event);
}

void HfpAgDisconnected::Entry()
{
    stateMachine_.ProcessDeferredMessage();

    if (isReentry_) {
        stateMachine_.SetRemoving(true);
        eventProcessor_.RemoveTask();
        BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
            stateMachine_.GetDeviceAddr(), static_cast<int>(HFP_AG_STATE_DISCONNECTED));
        stateMachine_.NotifyStateTransitions();
        HfpAgService *service = HfpAgService::GetService();
        if (service != nullptr) {
            service->ProcessRemoveStateMachine(stateMachine_.GetDeviceAddr());
        } else {
            HILOGE("[HFP_STATE_MACHINE]HfpAgService is nullptr!");
        }
    }
}

void HfpAgDisconnected::Exit()
{
    isReentry_ = true;
}

bool HfpAgDisconnected::Dispatch(const utility::Message &msg)
{
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
        HILOGI("[HFP_STATE_MACHINE][Disconnected]Address=%{public}s, EventName=%{public}s",
            GET_ENCRYPT_STR_ADDR(event.dev_), HfpAgStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_AG_CONNECT_EVT:
            if (!stateMachine_.ProcessConnectEvent()) {
                break;
            }
            ProcessTransition(HfpAgStateMachine::CONNECTING);
            break;
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            switch (event.arg1_) {
                case HFP_AG_STATE_CONNECTING:
                    HILOGI("HFP_STATE_MACHINE]SLC connecting");
                    BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, event.dev_, "HFPINITIATOR", CHR_INITIATOR_PEER);
                    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_F_HFP_CONN, event.dev_, UE_HFP_PEER_CONN, "");
                    ProcessTransition(HfpAgStateMachine::CONNECTING);
                    break;
                case HFP_AG_STATE_CONNECTED:
                    BtChrBtExcpEvent(event.dev_, BTOPT_HFP, CHR_SUB_ERRCODE_CASE13);
                    break;
                default:
                    HILOGE("HFP_STATE_MACHINE]Incorrect state=%{public}d", event.arg1_);
                    break;
            }
            break;
        case HFP_AG_RETRY_CONNECT_EVT: {
            RawAddress device(event.dev_);
            std::shared_ptr<BluetoothDevice> bluetoothDevice =
                RemoteDeviceProperties::GetInstance()->FindRemoteDevice(device);
            CHECK_AND_RETURN_LOG_RET(bluetoothDevice != nullptr, false, "device is null");
            bool isNeedToReConnect = stateMachine_.GetPreState() == HFP_AG_STATE_DISCONNECTING &&
                stateMachine_.GetStateInt() == HFP_AG_STATE_DISCONNECTED && stateMachine_.GetTryReconnectHfp() &&
                bluetoothDevice->IsAclConnected() && bluetoothDevice->IsPaired();
            if (isNeedToReConnect) {
                HfpAgMessage reconnectEvent(HFP_AG_CONNECT_EVT);
                reconnectEvent.dev_ = event.dev_;
                HILOGI("reconnect hfp, device is %{public}s", GET_ENCRYPT_ADDR(device));
                CHECK_AND_RETURN_LOG_RET(HfpAgService::GetService() != nullptr, false, "hfp service is null");
                HfpAgService::GetService()->ProcessConnectEvent(reconnectEvent);
            }
            stateMachine_.SetTryReconnectHfp(false);
            break;
        }
        default:
            break;
    }
    return true;
}

void HfpAgConnecting::Entry()
{
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        stateMachine_.GetDeviceAddr(), static_cast<int>(HFP_AG_STATE_CONNECTING));
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}

void HfpAgConnecting::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool HfpAgConnecting::ProcessConnectionStateChangedEvent(int connectState)
{
    HILOGI("[HFP_STATE_MACHINE]ConnectState=[%{public}d]", connectState);
    switch (connectState) {
        case HFP_AG_STATE_DISCONNECTED:
            HILOGI("HFP_STATE_MACHINE]Disconnected");
            ProcessTransition(HfpAgStateMachine::DISCONNECTED);
            break;
        case HFP_AG_STATE_CONNECTED:
            HILOGI("HFP_STATE_MACHINE]SLC connected");
            ProcessTransition(HfpAgStateMachine::CONNECTED);
            break;
        case HFP_AG_STATE_CONNECTING:
        case HFP_AG_STATE_DISCONNECTING:
            break;
        default:
            HILOGE("HFP_STATE_MACHINE]Incorrect state=%{public}d", connectState);
            break;
    }
    return true;
}

bool HfpAgConnecting::Dispatch(const utility::Message &msg)
{
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HFP_STATE_MACHINE][Connecting]Address=%{public}s, EventName=%{public}s",
        GET_ENCRYPT_STR_ADDR(event.dev_), HfpAgStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_AG_CONNECT_EVT:
        case HFP_AG_CONNECT_AUDIO_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            ProcessConnectionStateChangedEvent(event.arg1_);
            break;
        case HFP_AG_CONTROL_OTHER_MODULES_EVT:
            eventProcessor_.ExecuteEventProcess(event);
            break;
        case HFP_AG_DISCONNECT_EVT:
            stateMachine_.ProcessDisConnectEvent();
            ProcessTransition(HfpAgStateMachine::DISCONNECTED);
            break;
        case HFP_AG_CONNECTION_TIMEOUT_EVT:
            ProcessTransition(HfpAgStateMachine::DISCONNECTED);
            break;
        case HFP_AG_SEND_CCLC_RESPONSE:
            ProcessResponseClcc(event);
            break;
        case HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT:
            ProcessClccTimeoutRsp();
            break;
        default:
            break;
    }
    return true;
}

void HfpAgDisconnecting::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        stateMachine_.GetDeviceAddr(), static_cast<int>(HFP_AG_STATE_DISCONNECTING));
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}

void HfpAgDisconnecting::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool HfpAgDisconnecting::Dispatch(const utility::Message &msg)
{
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HFP_STATE_MACHINE][Disconnecting]Address=%{public}s, EventName=%{public}s",
        GET_ENCRYPT_STR_ADDR(event.dev_), HfpAgStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_AG_CONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            switch (event.arg1_) {
                case HFP_AG_STATE_DISCONNECTED:
                    HILOGI("HFP_STATE_MACHINE]Disconnected");
                    IPowerManager::GetInstance()->StatusUpdate(
                        RequestStatus::CONNECT_OFF, PROFILE_NAME_HFP_AG, RawAddress(event.dev_));
                    ProcessTransition(HfpAgStateMachine::DISCONNECTED);
                    break;
                case HFP_AG_STATE_CONNECTED:
                    HILOGI("HFP_STATE_MACHINE]SLC connected");
                    ProcessTransition(HfpAgStateMachine::CONNECTED);
                    break;
                default:
                    HILOGE("HFP_STATE_MACHINE]Incorrect state=%{public}d", event.arg1_);
                    break;
            }
            break;
        case HFP_AG_CONNECTION_TIMEOUT_EVT:
            ProcessTransition(HfpAgStateMachine::DISCONNECTED);
            BtChrBtExcpEvent(event.dev_, BTOPT_HFP, CHR_SUB_ERRCODE_CASE1);
            break;
        default:
            break;
    }
    return true;
}

void HfpAgConnected::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    HfpAgMessage event(HFP_AG_CONTROL_OTHER_MODULES_EVT);
    event.type_ = HFP_AG_MSG_TYPE_QUERY_AG_INDICATOR;
    eventProcessor_.ExecuteEventProcess(event);
    eventProcessor_.UpdateDeviceNameAndTimestamp();

    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        stateMachine_.GetDeviceAddr(), static_cast<int>(HFP_AG_STATE_CONNECTED));
    stateMachine_.NotifyStateTransitions();
}

void HfpAgConnected::ProcessOpenRecognitionResultEvent(int result)
{
    eventProcessor_.ProcessAtResponseCodeEvent(result == HFP_AG_RESULT_OK ? HFP_AG_RESULT_OK : HFP_AG_RESULT_ERROR, 0);
}

void HfpAgConnected::ProcessConnectionStateChanged(const HfpAgMessage &event)
{
    switch (event.arg1_) {
        case HFP_AG_STATE_DISCONNECTING:
            HILOGI("HFP_STATE_MACHINE]HfpAgConnected, Disconnecting");
            ProcessTransition(HfpAgStateMachine::DISCONNECTING);
            break;
        case HFP_AG_STATE_DISCONNECTED:
            HILOGI("HFP_STATE_MACHINE]HfpAgConnected,Disconnected");
            IPowerManager::GetInstance()->StatusUpdate(
                RequestStatus::CONNECT_OFF, PROFILE_NAME_HFP_AG, RawAddress(event.dev_));
            ProcessTransition(HfpAgStateMachine::DISCONNECTED);
            break;
        default:
            HILOGE("HFP_STATE_MACHINE]HfpAgConnected, Incorrect state=%{public}d", event.arg1_);
            break;
        }
}

void HfpAgConnected::ProcessAudioConnectionStateChanged(const HfpAgMessage &event)
{
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    switch (event.arg1_) {
        case HFP_AG_AUDIO_STATE_CONNECTED:
            HILOGI("HFP_STATE_MACHINE]HfpAgConnected, AUDIO_CONNECTED");
            IPowerManager::GetInstance()->StatusUpdate(
                RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG, RawAddress(event.dev_));
            if (!hfpAgService->IsScoAcceptableExceptADevice(event.dev_)) {
                stateMachine_.ProcessDisconnectAudioEvent();
                break;
            }
            ProcessTransition(HfpAgStateMachine::AUDIO_CONNECTED);
            break;
        case HFP_AG_AUDIO_STATE_CONNECTING:
            HILOGI("HFP_STATE_MACHINE]HfpAgConnected, AUDIO_CONNECTING");
            IPowerManager::GetInstance()->StatusUpdate(
                RequestStatus::CONNECT_OFF, PROFILE_NAME_HFP_AG, RawAddress(event.dev_));
            ProcessTransition(HfpAgStateMachine::AUDIO_CONNECTING);
            break;
        case HFP_AG_AUDIO_STATE_DISCONNECTED:
        case HFP_AG_AUDIO_STATE_DISCONNECTING:
        default:
            HILOGE("HFP_STATE_MACHINE]HfpAgConnected, Incorrect state=%{public}d", event.arg1_);
            break;
    }
}

void HfpAgConnected::DispatchOtherMsg(const HfpAgMessage &event)
{
    switch (event.what_) {
        case HFP_AG_OPEN_VOICE_RECOGNITION_EVT:
            stateMachine_.ProcessOpenRecognitionEvent(event.arg1_);
            break;
        case HFP_AG_VOICE_RECOGNITION_RESULT_EVT:
            ProcessOpenRecognitionResultEvent(event.arg1_);
            break;
        case HFP_AG_CLOSE_VOICE_RECOGNITION_EVT:
            stateMachine_.ProcessCloseRecognitionEvent();
            break;
        case HFP_AG_VOICE_RECOGNITION_TIME_OUT_EVT:
            ProcessOpenRecognitionResultEvent(HFP_AG_RESULT_ERROR);
            break;
        case HFP_AG_PROCESS_CKPD_EVT:
            stateMachine_.ProcessKeyPressed(RawAddress(event.dev_), callState_);
            break;
        case HFP_AG_SEND_CCLC_RESPONSE:
            ProcessResponseClcc(event);
            break;
        case HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT:
            ProcessClccTimeoutRsp();
            break;
        case HFP_AG_CALL_STATE_CHANGE:
            callState_ = event.state_.callState;
            eventProcessor_.ProcessPhoneStateChange(event.state_);
            break;
        case HFP_AG_NOTIFY_DEVICE_STATE_CHANGED:
            eventProcessor_.NotifyDeviceStatusChangedEvent();
            break;
        case HFP_AG_DIALING_OUT_RESULT:
            eventProcessor_.ProfcessDialingOutResultEvent(event.arg1_);
            break;
        case HFP_AG_SET_INBAND_RING_TONE_EVT:
            eventProcessor_.SendBSIRValueEvent(event.arg1_);
            break;
        case HFP_AG_SET_VOLUME_EVT:
            eventProcessor_.ProcessScoVolumeChangedWithoutSendAt(event.arg1_, event.arg3_);
            break;
        default:
            break;
    }
}

bool HfpAgConnected::Dispatch(const utility::Message &msg)
{
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HFP_STATE_MACHINE][Connected]Address=%{public}s, EventName=%{public}s",
        GET_ENCRYPT_STR_ADDR(event.dev_), HfpAgStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_AG_CONNECT_EVT:
            break;
        case HFP_AG_DISCONNECT_EVT:
            if (!stateMachine_.ProcessDisConnectEvent()) {
                ProcessTransition(HfpAgStateMachine::DISCONNECTED);
                break;
            }
            ProcessTransition(HfpAgStateMachine::DISCONNECTING);
            break;
        case HFP_AG_CONNECT_AUDIO_EVT:
            if (!stateMachine_.ProcessConnectAudioEvent()) {
                break;
            }
            ProcessTransition(HfpAgStateMachine::AUDIO_CONNECTING);
            break;
        case HFP_AG_DISCONNECT_AUDIO_EVT:
            break;
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            ProcessConnectionStateChanged(event);
            break;
        case HFP_AG_AUDIO_STATE_CHANGED_EVT:
            ProcessAudioConnectionStateChanged(event);
            break;
        case HFP_AG_CONTROL_OTHER_MODULES_EVT:
            eventProcessor_.ExecuteEventProcess(event);
            break;
        case HFP_AG_PHONEBOOK_AUTH_RESULT:
            eventProcessor_.ProcessPhonebookAuthResultEvent(event);
            break;
        default:
            DispatchOtherMsg(event);
            break;
    }
    return true;
}

void HfpAgState::ProcessResponseClcc(const HfpAgMessage &event) const
{
    if (eventProcessor_.ResponseClccTimeout_ == nullptr) {
        HILOGD("Ignore clcc response, query device is not currentDevice(%{public}s)", GET_ENCRYPT_STR_ADDR(event.dev_));
        return;
    }

    if (event.call_.index == 0) {
        eventProcessor_.ResponseClccTimeout_->Stop();
        eventProcessor_.ResponseClccTimeout_ = nullptr;
    }
    eventProcessor_.ProcessClccResponseCmd(event.call_);
}

void HfpAgState::ProcessClccTimeoutRsp() const
{
    if (eventProcessor_.ResponseClccTimeout_ == nullptr) {
        HILOGD("Ignore clcc timeout response, query device is not currentDevice");
        return;
    }

    eventProcessor_.ResponseClccTimeout_->Stop();
    eventProcessor_.ResponseClccTimeout_ = nullptr;

    HfpAgCallList callList;
    callList.index = 0;
    callList.dir = 0;
    callList.state = 0;
    callList.mode = 0;
    callList.mpty = 0;
    callList.number = "";
    callList.type = 0;
    eventProcessor_.ProcessClccResponseCmd(callList);
}

void HfpAgAudioConnecting::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        stateMachine_.GetDeviceAddr(), static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTING));
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}

void HfpAgAudioConnecting::Exit()
{
    stateMachine_.StopConnectionTimer();
}

void HfpAgAudioConnecting::ProcessConnectionStateChanged(const HfpAgMessage &event)
{
    switch (event.arg1_) {
        case HFP_AG_STATE_DISCONNECTING:
            HILOGI("HFP_STATE_MACHINE]HfpAgAudioConnecting,Disconnecting");
            ProcessTransition(HfpAgStateMachine::DISCONNECTING);
            break;
        case HFP_AG_STATE_DISCONNECTED:
            HILOGI("HFP_STATE_MACHINE]HfpAgAudioConnecting,Disconnected");
            stateMachine_.AddDeferredMessage(event);
            stateMachine_.NotifyChildStateToParentState(HFP_AG_AUDIO_STATE_CONNECTING,
                HFP_AG_AUDIO_STATE_DISCONNECTED);
            ProcessTransition(HfpAgStateMachine::CONNECTED);
            break;
        default:
            HILOGE("HFP_STATE_MACHINE]HfpAgAudioConnecting,Incorrect state=%{public}d", event.arg1_);
            break;
        }
}

void HfpAgAudioConnecting::ProcessAudioConnectionStateChanged(const HfpAgMessage &event)
{
    HILOGI("[HFP_STATE_MACHINE][AudioConnecting]AudioStatus=%{public}d", event.arg1_);
    switch (event.arg1_) {
        case HFP_AG_AUDIO_STATE_CONNECTED:
            IPowerManager::GetInstance()->StatusUpdate(RequestStatus::SCO_ON, PROFILE_NAME_HFP_AG,
                RawAddress(event.dev_));
            ProcessTransition(HfpAgStateMachine::AUDIO_CONNECTED);
            break;
        case HFP_AG_AUDIO_STATE_DISCONNECTED:
            stateMachine_.NotifyChildStateToParentState(HFP_AG_AUDIO_STATE_CONNECTING,
                HFP_AG_AUDIO_STATE_DISCONNECTED, HfpAgScoStateChangeReason::HFP_AG_SCO_REMOTE_USER_TERMINATED);
            ProcessTransition(HfpAgStateMachine::CONNECTED);
            break;
        case HFP_AG_AUDIO_STATE_CONNECTING:
        case HFP_AG_AUDIO_STATE_DISCONNECTING:
            break;
        default:
            HILOGE("HFP_STATE_MACHINE]HfpAgAudioConnecting, incorrect state=%{public}d", event.arg1_);
            break;
    }
}

bool HfpAgAudioConnecting::Dispatch(const utility::Message &msg)
{
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
        HILOGI("[HFP_STATE_MACHINE][AudioConnecting]Address=%{public}s, EventName=%{public}s",
            GET_ENCRYPT_STR_ADDR(event.dev_), HfpAgStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_AG_CONNECT_EVT:
        case HFP_AG_DISCONNECT_EVT:
        case HFP_AG_CONNECT_AUDIO_EVT:
        case HFP_AG_DISCONNECT_AUDIO_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HFP_AG_CONTROL_OTHER_MODULES_EVT:
            eventProcessor_.ExecuteEventProcess(event);
            break;
        case HFP_AG_AUDIO_CONNECT_FAILED_EVT:
        case HFP_AG_CONNECTION_TIMEOUT_EVT:
            stateMachine_.NotifyChildStateToParentState(HFP_AG_AUDIO_STATE_CONNECTING, HFP_AG_AUDIO_STATE_DISCONNECTED);
            ProcessTransition(HfpAgStateMachine::CONNECTED);
            BtChrCallExcpEvent(event.dev_, ERRCODE_SCO_DISC_SLOW, CHR_SUB_ERRCODE_CASE1, -1);
            break;
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            ProcessConnectionStateChanged(event);
            break;
        case HFP_AG_AUDIO_STATE_CHANGED_EVT:
            ProcessAudioConnectionStateChanged(event);
            break;
        case HFP_AG_CALL_STATE_CHANGE:
            callState_ = event.state_.callState;
            eventProcessor_.ProcessPhoneStateChange(event.state_);
            break;
        default:
            return false;
    }
    return true;
}

void HfpAgAudioDisconnecting::Entry()
{
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        stateMachine_.GetDeviceAddr(), static_cast<int>(HFP_AG_AUDIO_STATE_DISCONNECTING));
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}

void HfpAgAudioDisconnecting::Exit()
{
    stateMachine_.StopConnectionTimer();
}

void HfpAgAudioDisconnecting::ProcessConnectionStateChanged(const HfpAgMessage &event)
{
    switch (event.arg1_) {
        case HFP_AG_STATE_DISCONNECTED:
            stateMachine_.NotifyChildStateToParentState(
                HFP_AG_AUDIO_STATE_DISCONNECTING, HFP_AG_AUDIO_STATE_DISCONNECTED);
            ProcessTransition(HfpAgStateMachine::DISCONNECTED);
            break;
        case HFP_AG_STATE_DISCONNECTING:
            ProcessTransition(HfpAgStateMachine::DISCONNECTING);
            break;
        default:
            HILOGE("HFP_STATE_MACHINE]HfpAgAudioDisconnecting, Incorrect state=%{public}d", event.arg1_);
            break;
    }
}

void HfpAgAudioDisconnecting::ProcessAudioConnectionStateChanged(const HfpAgMessage &event)
{
    switch (event.arg1_) {
        case HFP_AG_AUDIO_STATE_DISCONNECTED:
            stateMachine_.ProcessAudioDisconnected(HfpAgScoStateChangeReason::HFP_AG_SCO_LOCAL_USER_TERMINATED);
            ProcessTransition(HfpAgStateMachine::CONNECTED);
            break;
        case HFP_AG_AUDIO_STATE_CONNECTING:
        case HFP_AG_AUDIO_STATE_DISCONNECTING:
            break;
        case HFP_AG_AUDIO_STATE_CONNECTED:
            ProcessTransition(HfpAgStateMachine::AUDIO_CONNECTED);
            break;
        default:
            HILOGE("HFP_STATE_MACHINE]HfpAgAudioDisconnecting, Incorrect state=%{public}d", event.arg1_);
            break;
    }
}

bool HfpAgAudioDisconnecting::Dispatch(const utility::Message &msg)
{
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HFP_STATE_MACHINE][AudioDisconnecting]Address=%{public}s, EventName=%{public}s",
        GET_ENCRYPT_STR_ADDR(event.dev_), HfpAgStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_AG_DISCONNECT_EVT:
        case HFP_AG_CONNECT_AUDIO_EVT:
        case HFP_AG_AUDIO_CONNECT_REQUEST_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HFP_AG_DISCONNECT_AUDIO_EVT:
        case HFP_AG_AUDIO_CONNECT_FAILED_EVT:
            break;
        case HFP_AG_AUDIO_DISCONNECT_FAILED_EVT:
        case HFP_AG_CONNECTION_TIMEOUT_EVT:
            stateMachine_.NotifyChildStateToParentState(HFP_AG_AUDIO_STATE_DISCONNECTING, HFP_AG_AUDIO_STATE_CONNECTED);
            ProcessTransition(HfpAgStateMachine::AUDIO_CONNECTED);
            BtChrCallExcpEvent(event.dev_, ERRCODE_SCO_DISC_SLOW, CHR_SUB_ERRCODE_CASE2, -1);
            break;
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            ProcessConnectionStateChanged(event);
            break;
        case HFP_AG_AUDIO_STATE_CHANGED_EVT:
            ProcessAudioConnectionStateChanged(event);
            break;
        default:
            return false;
    }
    return true;
}

void HfpAgAudioConnected::Entry()
{
    stateMachine_.ProcessDeferredMessage();
    BluetoothStateManager::GetInstance()->SetRemoteDeviceHfpAgState(
        stateMachine_.GetDeviceAddr(), static_cast<int>(HFP_AG_AUDIO_STATE_CONNECTED));
    stateMachine_.NotifyStateTransitions();
    eventProcessor_.RecoverScoVolume();
    eventProcessor_.SetAudioParameters(HFP_AG_SERVER_NREC_NAME);
    eventProcessor_.SetAudioParameters(HFP_AG_SERVER_WBS_NAME);
    RefusePlayHelper::GetInstance()->JudgeScoPause(stateMachine_.GetDeviceAddr());
}

void HfpAgAudioConnected::ProcessConnectionStateChanged(const HfpAgMessage &event)
{
    switch (event.arg1_) {
        case HFP_AG_STATE_DISCONNECTED:
            stateMachine_.AddDeferredMessage(event);
            stateMachine_.NotifyChildStateToParentState(HFP_AG_AUDIO_STATE_CONNECTED, HFP_AG_AUDIO_STATE_DISCONNECTED);
            ProcessTransition(HfpAgStateMachine::DISCONNECTED);
            break;
        default:
            HILOGE("HFP_STATE_MACHINE] Incorrect state=%{public}d", event.arg1_);
            break;
    }
}

void HfpAgAudioConnected::ProcessAudioConnectionStateChanged(const HfpAgMessage &event)
{
    switch (event.arg1_) {
        case HFP_AG_AUDIO_STATE_DISCONNECTED:
            stateMachine_.ProcessAudioDisconnected(HfpAgScoStateChangeReason::HFP_AG_SCO_REMOTE_USER_TERMINATED);
            ProcessTransition(HfpAgStateMachine::CONNECTED);
            break;
        case HFP_AG_AUDIO_STATE_DISCONNECTING:
            ProcessTransition(HfpAgStateMachine::AUDIO_DISCONNECTING);
            break;
        default:
            HILOGE("HFP_STATE_MACHINE] Incorrect state=%{public}d", event.arg1_);
            break;
    }
}

bool HfpAgAudioConnected::Dispatch(const utility::Message &msg)
{
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HILOGI("[HFP_STATE_MACHINE][AudioConnected]Address=%{public}s, EventName=%{public}s",
        GET_ENCRYPT_STR_ADDR(event.dev_), HfpAgStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_AG_CONNECT_EVT:
        case HFP_AG_CONNECT_AUDIO_EVT:
            break;
        case HFP_AG_DISCONNECT_EVT:
            if (!stateMachine_.ProcessDisConnectEvent()) {
                ProcessTransition(HfpAgStateMachine::DISCONNECTED);
                break;
            }
            ProcessTransition(HfpAgStateMachine::AUDIO_DISCONNECTING);
            break;
        case HFP_AG_DISCONNECT_AUDIO_EVT:
            if (!stateMachine_.ProcessDisconnectAudioEvent()) {
                ProcessTransition(HfpAgStateMachine::CONNECTED);
                break;
            }
            ProcessTransition(HfpAgStateMachine::AUDIO_DISCONNECTING);
            break;
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            ProcessConnectionStateChanged(event);
            break;
        case HFP_AG_AUDIO_STATE_CHANGED_EVT:
            ProcessAudioConnectionStateChanged(event);
            break;
        case HFP_AG_SET_VOLUME_EVT:
            eventProcessor_.ProcessScoVolumeChangedEvent(event.arg1_, event.arg3_);
            break;
        case HFP_AG_CALL_STATE_CHANGE:
            callState_ = event.state_.callState;
            eventProcessor_.ProcessPhoneStateChange(event.state_);
            break;
        default:
            return false;
    }
    return true;
}

bool HfpAgStateMachine::ProcessConnectEvent()
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return false;
    }
    ::bluetooth::headset::Interface* bluetoothHfpInterface = hfpAgService->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_STATE_MACHINE]BluetoothHfpInterface is null");
        return false;
    }
    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHfpInterface->Connect(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed Connect, status: %{public}d", status);
        return false;
    }
    return true;
}

bool HfpAgStateMachine::ProcessDisConnectEvent()
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return false;
    }
    ::bluetooth::headset::Interface* bluetoothHfpInterface = hfpAgService->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_STATE_MACHINE]BluetoothHfpInterface is null");
        return false;
    }
    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHfpInterface->Disconnect(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed DisConnect, status: %{public}d", status);
        return false;
    }
    return true;
}

void HfpAgStateMachine::ProcessOpenRecognitionEvent(int requestSource)
{
    std::string callingName = Bluetooth::PermissionManager::GetCallingName();
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    ::bluetooth::headset::Interface* bluetoothHfpInterface = hfpAgService->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_STATE_MACHINE]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHfpInterface->StartVoiceRecognition(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Fail open recognition, status: %{public}d", status);
        return;
    }

    if (hfpAgService->IsAudioConnected(address_)) {
        HILOGW("[HFP_STATE_MACHINE]Audio is connected, not need connect audio.");
        int sceneCode = (requestSource == HFP_START_VOICE_REG_BY_AG) ?
            UE_LOCAL_START_VR_FAIL_SCO_CONNECTED : UE_REMOTE_START_VR_FAIL_SCO_CONNECTED;
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_START_VOICE_REG, RawAddress(address_),
            sceneCode, callingName);
        return;
    }

    HfpAgMessage msg(HFP_AG_CONNECT_AUDIO_EVT);
    msg.dev_ = address_;
    hfpAgService->PostEvent(msg);
}

void HfpAgStateMachine::ProcessCloseRecognitionEvent()
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    ::bluetooth::headset::Interface* bluetoothHfpInterface = hfpAgService->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_STATE_MACHINE]BluetoothHfpInterface is null");
        return;
    }
    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHfpInterface->StopVoiceRecognition(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Fail CloseRecognition, status: %{public}d", status);
        return;
    }
}

bool HfpAgStateMachine::ProcessConnectAudioEvent()
{
    HfpAgService *service = HfpAgService::GetService();
    CHECK_AND_RETURN_LOG_RET(service != nullptr, false, "service is null");
    RawAddress device(address_);
    if (service->IsNoNeedConnectSco()) {
        HILOGE("no need connect sco in ringing,, notify SCO disconnected");
        service->NotifyAudioStateChanged(device, HFP_AG_AUDIO_STATE_DISCONNECTED,
            HFP_AG_SCO_REMOTE_USER_TERMINATED);
        service->SetResumeDevice(address_);
        return false;
    }

    eventProcessor_.ProcessA2dpSuspendStateEvent(true);
    service->StopA2dpOffload();
    BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_SET_A2DP_SUSPEND, RawAddress(""), UE_COMMON_SCENE_CASE1,
        UE_COMMON_SCENE_CASE1);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    ::bluetooth::headset::Interface* bluetoothHfpInterface = service->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_STATE_MACHINE]BluetoothHfpInterface is null");
        return false;
    }
    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHfpInterface->ConnectAudio(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        bt_status_t vrStatus = bluetoothHfpInterface->StopVoiceRecognition(&rawAddr);
        if (vrStatus != BT_STATUS_SUCCESS) {
            HILOGE("[HFP_STATE_MACHINE]Fail CloseRecognition, status: %{public}d", vrStatus);
        }
        HILOGE("[HFP_STATE_MACHINE]Failed ConnectAudio, status: %{public}d", status);
        eventProcessor_.ProcessA2dpSuspendStateEvent(false);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_SET_A2DP_SUSPEND, RawAddress(""), UE_COMMON_SCENE_CASE2,
            UE_COMMON_SCENE_CASE1);
        return false;
    }
    return true;
}

bool HfpAgStateMachine::ProcessDisconnectAudioEvent()
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return false;
    }
    ::bluetooth::headset::Interface* bluetoothHfpInterface = hfpAgService->getBluetoothHfpInterface();
    if (bluetoothHfpInterface == nullptr) {
        HILOGE("[HFP_STATE_MACHINE]BluetoothHfpInterface is null");
        return false;
    }
    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    bt_status_t status = bluetoothHfpInterface->DisconnectAudio(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed DisconnectAudio, status: %{public}d", status);
        return false;
    }
    return true;
}

void HfpAgStateMachine::ProcessKeyPressed(const RawAddress &device, const int &callState)
{
    HfpAgSystemInterface& mSystemInterface = HfpAgSystemInterface::GetInstance();
    auto hfpAgService = HfpAgService::GetService();
    if (hfpAgService == nullptr) {
        return;
    }
    if (mSystemInterface.IsRinging()) {
        mSystemInterface.AnswerCall(device.GetAddress());
    } else if (mSystemInterface.IsInCall()) {
        if (GetStateInt() == HfpAgAudioState::HFP_AG_AUDIO_STATE_DISCONNECTED) {
            if (!hfpAgService->SetActiveDevice(device)) {
                HILOGI("[HFP_STATE_MACHINE][failed to set active device to][%{public}s]",
                    GetEncryptAddr(device.GetAddress()).c_str());
            }
        } else {
            mSystemInterface.HangupCall(device.GetAddress());
        }
    } else if (GetStateInt() != HfpAgAudioState::HFP_AG_AUDIO_STATE_DISCONNECTED) {
        ProcessDisconnectAudioEvent();
    } else {
        if (callState == HFP_AG_CALL_STATE_DIALING) {
            HILOGI("[HFP_STATE_MACHINE]Already dialling!");
            return;
        }
        std::string dialNumber = mSystemInterface.GetLastDialNumber();
        if (dialNumber.empty()) {
            HILOGI("[HFP_STATE_MACHINE]Last dial number null!");
            return;
        }
        mSystemInterface.DialOutCall(device.GetAddress(), dialNumber);
    }
}

/**
* When the child state transitions to the parent state, the entry method of the parent state will not be executed.
* It is necessary to ensure that CleanHfpScoOccupied can be executed when the state transitions to
* Connected or Disconnected.
*/
void HfpAgState::ProcessTransition(const std::string &name)
{
    std::string preState = stateMachine_.GetStateName(stateMachine_.GetPreState());
    HILOGI("[HFP_STATE_MACHINE]Device[%{public}s] transition [%{public}s]->[%{public}s]",
        GET_ENCRYPT_STR_ADDR(stateMachine_.GetDeviceAddr()), preState.c_str(), name.c_str());
    if (name == HfpAgStateMachine::DISCONNECTED || name == HfpAgStateMachine::CONNECTED) {
        BluetoothHwInterface::GetInstance()->CleanHfpScoOccupied(RawAddress(stateMachine_.GetDeviceAddr()));
    }
    if (name == HfpAgStateMachine::CONNECTING) {
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, stateMachine_.GetDeviceAddr(), "HFPSTATUS",
            ::bluetooth::headset::BTHF_CONNECTION_STATE_CONNECTING);
    }
    if (preState == HfpAgStateMachine::CONNECTING && name == HfpAgStateMachine::CONNECTED) {
        BtChrEventWriteTime(CHR_USER_DISCONNECT, stateMachine_.GetDeviceAddr(), "HFPCONNECTEDTIME");
        BtChrDftEventWriteInt(CHR_USER_DISCONNECT, stateMachine_.GetDeviceAddr(), "HFPSTATUS",
            ::bluetooth::headset::BTHF_CONNECTION_STATE_CONNECTED);
    }
    Transition(name);
}

std::string HfpAgStateMachine::GetStateName(int state) const
{
    switch (state) {
        case HFP_AG_STATE_DISCONNECTED:
            return "Disconnected";
        case HFP_AG_STATE_CONNECTING:
            return "Connecting";
        case HFP_AG_STATE_DISCONNECTING:
            return "Disconnecting";
        case HFP_AG_STATE_CONNECTED:
            return "Connected(AudioDisconnected)";
        case HFP_AG_AUDIO_STATE_CONNECTING:
            return "AudioConnecting";
        case HFP_AG_AUDIO_STATE_DISCONNECTING:
            return "AudioDisconnecting";
        case HFP_AG_AUDIO_STATE_CONNECTED:
            return "AudioConnected";
        default:
            return "Unknown";
    }
}

std::string HfpAgStateMachine::GetConnectEventName(int what)
{
    switch (what) {
        case HFP_AG_CONNECT_EVT:
            return "HFP_AG_CONNECT_EVT";
        case HFP_AG_DISCONNECT_EVT:
            return "HFP_AG_DISCONNECT_EVT";
        case HFP_AG_CONNECTION_STATE_CHANGED_EVT:
            return "HFP_AG_CONNECTION_STATE_CHANGED_EVT";
        case HFP_AG_CONNECT_AUDIO_EVT:
            return "HFP_AG_CONNECT_AUDIO_EVT";
        case HFP_AG_DISCONNECT_AUDIO_EVT:
            return "HFP_AG_DISCONNECT_AUDIO_EVT";
        case HFP_AG_CONNECTED_EVT:
            return "HFP_AG_CONNECTED_EVT";
        case HFP_AG_DISCONNECTED_EVT:
            return "HFP_AG_DISCONNECTED_EVT";
        case HFP_AG_CONNECT_FAILED_EVT:
            return "HFP_AG_CONNECT_FAILED_EVT";
        case HFP_AG_DISCONNECT_FAILED_EVT:
            return "HFP_AG_DISCONNECT_FAILED_EVT";
        case HFP_AG_AUDIO_CONNECTING_EVT:
            return "HFP_AG_AUDIO_CONNECTING_EVT";
        case HFP_AG_AUDIO_DISCONNECTING_EVT:
            return "HFP_AG_AUDIO_DISCONNECTING_EVT";
        case HFP_AG_AUDIO_CONNECTED_EVT:
            return "HFP_AG_AUDIO_CONNECTED_EVT";
        case HFP_AG_AUDIO_DISCONNECTED_EVT:
            return "HFP_AG_AUDIO_DISCONNECTED_EVT";
        case HFP_AG_AUDIO_CONNECT_FAILED_EVT:
            return "HFP_AG_AUDIO_CONNECT_FAILED_EVT";
        case HFP_AG_AUDIO_DISCONNECT_FAILED_EVT:
            return "HFP_AG_AUDIO_DISCONNECT_FAILED_EVT";
        case HFP_AG_AUDIO_STATE_CHANGED_EVT:
            return "HFP_AG_AUDIO_STATE_CHANGED_EVT";
        case HFP_AG_CONNECTION_TIMEOUT_EVT:
            return "HFP_AG_CONNECTION_TIMEOUT_EVT";
        default:
            return "Unknown";
    }
}

std::string HfpAgStateMachine::GetEventName(int what)
{
    switch (what) {
        case HFP_AG_INVALID_EVT:
            return "HFP_AG_INVALID_EVT";
        case HFP_AG_SERVICE_STARTUP_EVT:
            return "HFP_AG_SERVICE_STARTUP_EVT";
        case HFP_AG_SERVICE_SHUTDOWN_EVT:
            return "HFP_AG_SERVICE_SHUTDOWN_EVT";
        case HFP_AG_PROCESS_CKPD_EVT:
            return "HFP_AG_PROCESS_CKPD_EVT";
        case HFP_AG_OPEN_VOICE_RECOGNITION_EVT:
            return "HFP_AG_OPEN_VOICE_RECOGNITION_EVT";
        case HFP_AG_CLOSE_VOICE_RECOGNITION_EVT:
            return "HFP_AG_CLOSE_VOICE_RECOGNITION_EVT";
        case HFP_AG_VOICE_RECOGNITION_RESULT_EVT:
            return "HFP_AG_VOICE_RECOGNITION_RESULT_EVT";
        case HFP_AG_SET_VOLUME_EVT:
            return "HFP_AG_SET_VOLUME_EVT";
        case HFP_AG_SET_INBAND_RING_TONE_EVT:
            return "HFP_AG_SET_INBAND_RING_TONE_EVT";
        case HFP_AG_SEND_SUBSCRIBER_NUMBER_EVT:
            return "HFP_AG_SEND_SUBSCRIBER_NUMBER_EVT";
        case HFP_AG_SEND_NETWORK_OPERATOR_EVT:
            return "HFP_AG_SEND_NETWORK_OPERATOR_EVT";
        case HFP_AG_CONTROL_OTHER_MODULES_EVT:
            return "HFP_AG_CONTROL_OTHER_MODULES_EVT";
        case HFP_AG_DIALING_OUT_RESULT:
            return "HFP_AG_DIALING_OUT_RESULT";
        case HFP_AG_CALL_STATE_CHANGE:
            return "HFP_AG_CALL_STATE_CHANGE";
        case HFP_AG_SEND_CCLC_RESPONSE:
            return "HFP_AG_SEND_CCLC_RESPONSE";
        case HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT:
            return "HFP_AG_RESPONSE_CLCC_TIME_OUT_EVT";
        case HFP_AG_DIAL_TIME_OUT_EVT:
            return "HFP_AG_DIAL_TIME_OUT_EVT";
        case HFP_AG_PHONEBOOK_AUTH_RESULT:
            return "HFP_AG_PHONEBOOK_AUTH_RESULT";
        case HFP_AG_NOTIFY_DEVICE_STATE_CHANGED:
            return "HFP_AG_NOTIFY_DEVICE_STATE_CHANGED";
        default:
            return GetConnectEventName(what);
    }
}

void HfpAgStateMachine::SetTryReconnectHfp(bool tryToReconnect)
{
    tryToReconnect_ = tryToReconnect;
}

bool HfpAgStateMachine::GetTryReconnectHfp() const
{
    return tryToReconnect_;
}

void HfpAgStateMachine::SetAudioParameters()
{
    eventProcessor_.SetAudioParameters(HFP_AG_SERVER_NREC_NAME);
    eventProcessor_.SetAudioParameters(HFP_AG_SERVER_WBS_NAME);
}

void HfpAgStateMachine::SendAtCommandByEventProcessor(AppCategory topApptype)
{
    eventProcessor_.SendAtCmdOnApptypeChange(topApptype);
}

}  // namespace bluetooth
}
