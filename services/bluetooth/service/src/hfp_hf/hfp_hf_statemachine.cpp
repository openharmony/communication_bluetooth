/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_hfp_hf"
#endif

#include "hfp_hf_statemachine.h"
#include "hfp_hf_service.h"
#include "power_manager.h"
#include "common_util.h"
#include "adapter_manager.h"
#include "connect_strategy_manager.h"
#include "profile_config.h"
#include "bluetooth_audio_framework_adapter.h"
#include "bt_chr_base.h"
#ifdef BLUETOOTH_WATCH_ENABLE
#include "watch_service.h"
#endif

namespace OHOS {
namespace bluetooth {
HfpHfStateMachine::HfpHfStateMachine(const std::string &address)
    : address_(address)
{}

void HfpHfStateMachine::Init()
{
    calls_ = std::make_unique<HfpHfCallManager>(address_);
    connTimer_ = std::make_unique<utility::Timer>(std::bind(&bluetooth::HfpHfStateMachine::ConnectionTimeout, this));

    std::unique_ptr<utility::StateMachine::State> disconnectedState =
        std::make_unique<HfpHfDisconnected>(DISCONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> connectingState =
        std::make_unique<HfpHfConnecting>(CONNECTING, *this);
    std::unique_ptr<utility::StateMachine::State> connectedState =
        std::make_unique<HfpHfConnected>(CONNECTED, *this);
    std::unique_ptr<utility::StateMachine::State> audioConnected =
        std::make_unique<HfpHfAudioConnected>(AUDIO_CONNECTED, *this, *connectedState);

    Move(disconnectedState);
    Move(connectingState);
    Move(audioConnected);
    Move(connectedState);

    InitState(DISCONNECTED);
    GetBluetoothHfInterface();
}

void HfpHfDisconnected::Entry()
{
    HILOGE("[HFP_STATE_MACHINE] Disconnected Entry");
    stateMachine_.ProcessDeferredMessage();

    if (isReentry_) {
        stateMachine_.SetRemoving(true);
        stateMachine_.NotifyStateTransitions();
        HfpHfService *service = HfpHfService::GetService();
        if (service != nullptr) {
            service->ProcessRemoveStateMachine(stateMachine_.GetDeviceAddr());
        } else {
            HILOGE("[HFP_STATE_MACHINE]HfpHfService is nullptr!");
        }
    }
}

void HfpHfDisconnected::Exit()
{
    isReentry_ = true;
}

bool HfpHfDisconnected::Dispatch(const utility::Message &msg)
{
    const HfpHfMessage &event = static_cast<const HfpHfMessage &>(msg);
    HILOGI("[HFP_STATE_MACHINE][Disconnected][%{public}s]", HfpHfStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_HF_CONNECT_EVT:
            if (!stateMachine_.ProcessConnectEvent()) {
                break;
            }
            Transition(CONNECTING);
            break;
        case HFP_HF_CONNECTED_EVT:
            if (stateMachine_.IsAllowAgConnected(event.dev_)) {
                HILOGI("[HFP_STATE_MACHINE] Incoming AG accepted");
                BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, event.dev_, "HFPINITIATOR", CHR_INITIATOR_PEER);
                BtChrEventWriteTime(CHR_BT_WATCH_CONNECT, event.dev_, "HFPCONNECTIONTIME");
                Transition(CONNECTING);
            } else {
                HILOGI("[HFP_STATE_MACHINE] Incoming AG rejected");
                stateMachine_.ProcessDisconnectEvent();
            }
            break;
        default:
            break;
    }
    return true;
}

void HfpHfConnecting::Entry()
{
    HILOGI("[HFP_STATE_MACHINE] Connecting Entry");
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
    stateMachine_.StartConnectionTimer();
}
void HfpHfConnecting::Exit()
{
    stateMachine_.StopConnectionTimer();
}

bool HfpHfConnecting::Dispatch(const utility::Message &msg)
{
    const HfpHfMessage &event = static_cast<const HfpHfMessage &>(msg);
    HILOGI("HFP_STATE_MACHINE][Connecting][%{public}s]", HfpHfStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_HF_CONNECTED_EVT:
            break;
        case HFP_HF_DISCONNECTED_EVT:
            Transition(DISCONNECTED);
            break;
        case HFP_HF_CONNECT_EVT:
        case HFP_HF_DISCONNECT_EVT:
        case HFP_HF_AUDIO_CONNECTED_MSBC_EVT:
        case HFP_HF_AUDIO_CONNECTED_EVT:
        case HFP_HF_AUDIO_DISCONNECTED_EVT:
        case HFP_HF_BATTERY_LEVEL_CHANGED_EVT:
        case HFP_HF_INTERACTIVE_EVT:
            stateMachine_.AddDeferredMessage(event);
            break;
        case HFP_HF_SLC_ESTABLISHED_EVT:
            stateMachine_.ProcessSlcEstablished(event);
            Transition(CONNECTED);
            break;
        case HFP_HF_CONNECTION_TIMEOUT_EVT:
            Transition(DISCONNECTED);
            break;
        default:
            break;
    }
    return true;
}

void HfpHfConnected::Entry()
{
    HILOGE("[HFP_STATE_MACHINE] Connected Entry");
    stateMachine_.SetAudioWbs(false);
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
#ifdef BLUETOOTH_WATCH_ENABLE
    stateMachine_.WatchDialCall();
#endif
}

bool HfpHfConnected::Dispatch(const utility::Message &msg)
{
    const HfpHfMessage &event = static_cast<const HfpHfMessage &>(msg);
    HILOGI("[HFP_STATE_MACHINE][Connected][%{public}s]", HfpHfStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_HF_CONNECT_AUDIO_EVT:
        case HFP_HF_RETRY_CONNECT_AUDIO_EVT:
            stateMachine_.ProcessConnectScoEvent();
            break;
        case HFP_HF_AUDIO_CONNECTED_MSBC_EVT:
            stateMachine_.SetAudioWbs(true);
            [[fallthrough]];
        case HFP_HF_AUDIO_CONNECTED_EVT:
            IPowerManager::GetInstance()->StatusUpdate(
                RequestStatus::SCO_ON, PROFILE_NAME_HFP_HF, RawAddress(event.dev_));
#ifdef BLUETOOTH_WATCH_ENABLE
            if (!WatchService::GetInstance()->IsAllowHfScoConnect(event.dev_)) {
                stateMachine_.ProcessDisconnectScoEvent();
                break;
            }
#endif
            Transition(AUDIO_CONNECTED);
            break;
        case HFP_HF_DISCONNECT_EVT:
            stateMachine_.ProcessDisconnectEvent();
            break;
        case HFP_HF_DISCONNECTED_EVT:
            IPowerManager::GetInstance()->StatusUpdate(
                RequestStatus::CONNECT_OFF, PROFILE_NAME_HFP_HF, RawAddress(event.dev_));
            Transition(DISCONNECTED);
            break;
        case HFP_HF_INTERACTIVE_EVT:
            stateMachine_.ProcessInteractiveEvent(event);
            break;
        case HFP_HF_SEND_DTMF_EVT:
            stateMachine_.PrecessSendDtmf(event.arg1_);
            break;
        case HFP_HF_SET_VOLUME_EVT:
            stateMachine_.ProcessSetHfVolume(event.arg1_, event.arg3_);
            break;
        case HFP_HF_OPEN_VOICE_RECOGNITION_EVT:
            stateMachine_.ProcessOpenVoiceRecognition();
            break;
        case HFP_HF_CLOSE_VOICE_RECOGNITION_EVT:
            stateMachine_.ProcessCloseVoiceRecognition();
            break;
        default:
            DispatchOthers(msg);
            break;
    }
    return true;
}

void HfpHfConnected::DispatchOthers(const utility::Message &msg)
{
    const HfpHfMessage &event = static_cast<const HfpHfMessage &>(msg);
    HILOGI("[HFP_STATE_MACHINE][Connected][%{public}s]", HfpHfStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_HF_ACCEPT_CALL_EVT:
            stateMachine_.ProcessAcceptCall(event.arg1_);
            break;
        case HFP_HF_HOLD_CALL_EVT:
            stateMachine_.ProcessHoldCall();
            break;
        case HFP_HF_REJECT_CALL_EVT:
            stateMachine_.ProcessRejectCall();
            break;
        case HFP_HF_SEND_KEY_PRESSED:
            stateMachine_.PrecessSendKeyPressed();
            break;
        case HFP_HF_FINISH_CALL_EVT:
            stateMachine_.ProcessFinishActiveCall(event);
            break;
        case HFP_HF_DIAL_CALL_EVT:
            stateMachine_.ProcessDialCall(event);
            break;
        case HFP_HF_BATTERY_LEVEL_CHANGED_EVT:
            stateMachine_.ProcessBatteryLevelUpdate(event.arg1_);
            break;
        case HFP_HF_QUERY_CURRENT_CALLS:
            stateMachine_.ProcessCallIndicatorEvent(event);
            break;
        default:
            break;
    }
}

void HfpHfAudioConnected::Entry()
{
    HILOGE("[HFP_STATE_MACHINE] AudioConnected Entry");
    stateMachine_.ProcessDeferredMessage();
    stateMachine_.NotifyStateTransitions();
}

bool HfpHfAudioConnected::Dispatch(const utility::Message &msg)
{
    const HfpHfMessage &event = static_cast<const HfpHfMessage &>(msg);
    HILOGI("[HFP_STATE_MACHINE][AudioConnected][%{public}s]",
        HfpHfStateMachine::GetEventName(event.what_).c_str());
    switch (event.what_) {
        case HFP_HF_DISCONNECTED_EVT:
            stateMachine_.ProcessAudioDisconnected();
            Transition(DISCONNECTED);
            break;
        case HFP_HF_HOLD_CALL_EVT:
            stateMachine_.ProcessHoldCall();
            break;
        case HFP_HF_DISCONNECT_EVT:
            stateMachine_.AddDeferredMessage(event);
            [[fallthrough]];
        case HFP_HF_DISCONNECT_AUDIO_EVT:
            if (!stateMachine_.ProcessDisconnectScoEvent()) {
                HILOGE("[HFP_STATE_MACHINE]Disconnect audio fail");
            }
            break;
        case HFP_HF_AUDIO_DISCONNECTED_EVT:
            stateMachine_.ProcessAudioDisconnected();
            Transition(CONNECTED);
            break;
        default:
            return false;
    }
    return true;
}

inline std::string HfpHfStateMachine::GetDeviceAddr() const
{
    return address_;
}

void HfpHfStateMachine::GetBluetoothHfInterface()
{
    HfpHfService* service = HfpHfService::GetService();
    if (service != nullptr) {
        bluetoothHfInterface = service->GetBluetoothHfInterface();
        return;
    }
    HILOGE("[HFP_STATE_MACHINE]service is null");
}

bool HfpHfStateMachine::ProcessConnectEvent()
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG_RET(bluetoothHfInterface != nullptr, false,
        "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    BtStackStatus status = bluetoothHfInterface->connect(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed connect, status: %{public}d", status);
        return false;
    }
    BtChrEventWriteInt(CHR_BT_WATCH_CONNECT, address_, "HFPINITIATOR", CHR_INITIATOR_LOCAL);
    BtChrEventWriteTime(CHR_BT_WATCH_CONNECT, address_, "HFPCONNECTIONTIME");
    return true;
}

bool HfpHfStateMachine::ProcessDisconnectEvent()
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG_RET(bluetoothHfInterface != nullptr, false,
        "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    BtStackStatus status = bluetoothHfInterface->disconnect(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed disconnect, status: %{public}d", status);
        return false;
    }
    return true;
}

bool HfpHfStateMachine::ProcessConnectScoEvent()
{
#ifdef BLUETOOTH_WATCH_ENABLE
    auto watchService = WatchService::GetInstance();
    if (watchService != nullptr && !watchService->IsHfOnCall(address_)) {
        HILOGE("[HFP_STATE_MACHINE]Not allow to connect audio when watch is not in call");
        return false;
    }
#endif

    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG_RET(bluetoothHfInterface != nullptr, false,
        "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    BtStackStatus status = bluetoothHfInterface->connectAudio(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed connect sco, status: %{public}d", status);
        return false;
    }
    return true;
}

bool HfpHfStateMachine::ProcessDisconnectScoEvent()
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG_RET(bluetoothHfInterface != nullptr, false,
        "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    HILOGI("[HFP_STATE_MACHINE]Hfp device address[%{public}s]", GetEncryptAddr(address_).c_str());
    BtStackStatus status = bluetoothHfInterface->disconnectAudio(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed disonnect sco, status: %{public}d", status);
        return false;
    }
    SetAudioConnectionState(false);
    return true;
}

void HfpHfStateMachine::ProcessSetHfVolume(int volume, int type)
{
    if (type == HFP_HF_VOLUME_TYPE_MIC) {
        HILOGE("[HFP_STATE_MACHINE]Called only for Mute/Un-mute - Mic volume change is not allowed.");
        return;
    }
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    BtStackStatus status = bluetoothHfInterface->volumeControl(&rawAddr, (BthfClientVolumeType)type, volume);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed ProcessSetHfVolume");
    } else {
        queuedActions.push(HFP_HF_SET_VOLUME_EVT);
        SetAudioExtraParameters("hfp_volume", std::to_string(volume));
    }
}

void HfpHfStateMachine::ProcessFinishActiveCall(const HfpHfMessage &event)
{
    int action = HFP_HF_CALL_DEFAULT_ACTION;
    if ((calls_->HasCallState(BTHF_CLIENT_CALL_STATE_ACTIVE)) ||
        (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_DIALING)) ||
        (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_ALERTING))) {
        HILOGI("[HFP_STATE_MACHINE]Finish active or dialing or alerting call");
        action = BTHF_CLIENT_CALL_ACTION_CHUP;
    } else if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_HELD)) {
        HILOGI("[HFP_STATE_MACHINE]Finish hold call");
        action = BTHF_CLIENT_CALL_ACTION_CHLD_0;
    } else {
        HILOGI("[HFP_STATE_MACHINE]no call to finish");
        return;
    }

    HandleCallAction((BthfClientCallAction)action, HFP_HF_FINISH_CALL_EVT);
}

void HfpHfStateMachine::ProcessDialCall(const HfpHfMessage &event)
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    std::string number = event.calls_.GetNumber();
    BtStackStatus status = bluetoothHfInterface->dial(&rawAddr, number.c_str());
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed DisConnect");
    } else {
        queuedActions.push(HFP_HF_DIAL_CALL_EVT);
        HandsFreeUnitCalls call(
            address_, HFP_HF_OUTGOING_CALL_ID, HFP_CALL_STATE_DIALING, number, false, true, false);
        calls_->UpdateCall(call);
    }
}

void HfpHfStateMachine::ProcessOpenVoiceRecognition()
{
    if (GetVoiceRecognitionState() == HFP_HF_VR_STATE_CLOSED) {
        RawAddress device(address_);
        STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
        CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

        BtStackStatus status = bluetoothHfInterface->startVoiceRecognition(&rawAddr);
        if (status != BT_STATUS_SUCCESS) {
            HILOGE("[HFP_STATE_MACHINE]can't open the voice recognition!");
        } else {
            queuedActions.push(HFP_HF_OPEN_VOICE_RECOGNITION_EVT);
        }
    }
}

void HfpHfStateMachine::ProcessCloseVoiceRecognition()
{
    if (GetVoiceRecognitionState() == HFP_HF_VR_STATE_OPENED) {
        RawAddress device(address_);
        STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
        CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

        BtStackStatus status = bluetoothHfInterface->stopVoiceRecognition(&rawAddr);
        if (status != BT_STATUS_SUCCESS) {
            HILOGE("[HFP_STATE_MACHINE]can't close the voice recognition!");
        } else {
            queuedActions.push(HFP_HF_CLOSE_VOICE_RECOGNITION_EVT);
        }
    }
}

void HfpHfStateMachine::ProcessBatteryLevelUpdate(int batteryLevel)
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    BtStackStatus status =
        bluetoothHfInterface->sendAtCmd(&rawAddr, HANDSFREECLIENT_AT_CMD_BIEV, 2, batteryLevel, NULL);
    if (status !=BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]ProcessBatteryLevelUpdate Fail!");
    } else {
        queuedActions.push(HFP_HF_BATTERY_LEVEL_CHANGED_EVT);
    }
}

void HfpHfStateMachine::ProcessRejectCall()
{
    int action = HFP_HF_CALL_DEFAULT_ACTION;
    if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_INCOMING)) {
        HILOGI("[HFP_STATE_MACHINE]Reject incoming call");
        action = BTHF_CLIENT_CALL_ACTION_CHUP;
    } else if ((calls_->HasCallState(BTHF_CLIENT_CALL_STATE_HELD)) ||
               (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_WAITING))) {
        HILOGI("[HFP_STATE_MACHINE]Reject held or waiting call");

        action = BTHF_CLIENT_CALL_ACTION_CHLD_0;
    } else if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_HELD_BY_RESP_HOLD)) {
        action = BTHF_CLIENT_CALL_ACTION_BTRH_2;
    } else {
        HILOGI("[HFP_STATE_MACHINE]No call to reject");
        return;
    }

    HandleCallAction((BthfClientCallAction)action, HFP_HF_REJECT_CALL_EVT);
}

void HfpHfStateMachine::PrecessSendDtmf(int flag)
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    BtStackStatus status = bluetoothHfInterface->sendDtmf(&rawAddr, flag);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Send dtmf failed");
    } else {
        queuedActions.push(HFP_HF_SEND_DTMF_EVT);
    }
}

void HfpHfStateMachine::PrecessSendKeyPressed()
{
    const char *ckpd = "+CKPD=200\r";
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    BtStackStatus status =
        bluetoothHfInterface->sendAtCmd(&rawAddr, HANDSFREECLIENT_AT_CMD_VENDOR_SPECIFIC_CMD, 0, 0, ckpd);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Send ckpd failed");
    } else {
        queuedActions.push(HANDSFREECLIENT_AT_CMD_VENDOR_SPECIFIC_CMD);
    }
}

void HfpHfStateMachine::ProcessHoldCall()
{
    int action = HFP_HF_CALL_DEFAULT_ACTION;
    if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_INCOMING)) {
        action = BTHF_CLIENT_CALL_ACTION_BTRH_0;
    } else {
        if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_ACTIVE)) {
            action = BTHF_CLIENT_CALL_ACTION_CHLD_2;
        }
    }

    HandleCallAction((BthfClientCallAction)action, HFP_HF_HOLD_CALL_EVT);
}

void HfpHfStateMachine::ProcessAcceptCall(int flag)
{
    int action = HFP_HF_CALL_DEFAULT_ACTION;
    if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_INCOMING)) {
        if (flag != HFP_HF_ACCEPT_CALL_ACTION_NONE) {
            return;
        }
#ifdef BLUETOOTH_WATCH_ENABLE
        auto watchService = WatchService::GetInstance();
        if (watchService != nullptr) {
            watchService->SetAllowHfScoConnect(address_, true);
        }
#endif
        action = BTHF_CLIENT_CALL_ACTION_ATA;
        HILOGI("[HFP_STATE_MACHINE]Accept incoming call");
    } else if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_WAITING)) {
        if ((calls_->HasCallState(BTHF_CLIENT_CALL_STATE_ACTIVE) == false) &&
            (flag != HFP_HF_ACCEPT_CALL_ACTION_NONE)) {
            return;
        }

        if ((flag == HFP_HF_ACCEPT_CALL_ACTION_NONE) || (flag == HFP_HF_ACCEPT_CALL_ACTION_HOLD)) {
            action = HFP_HF_CHLD_ACTION_2;
        } else if (flag == HFP_HF_ACCEPT_CALL_ACTION_FINISH) {
            action = HFP_HF_CHLD_ACTION_1;
        } else {
            HILOGE("[HFP_STATE_MACHINE]Invalid flag[%{public}d]", flag);
            return;
        }
    } else if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_HELD)) {
        action = HFP_HF_CHLD_ACTION_2;
        if (flag == HFP_HF_ACCEPT_CALL_ACTION_FINISH) {
            action = HFP_HF_CHLD_ACTION_1;
        } else if ((flag == HFP_HF_ACCEPT_CALL_ACTION_NONE) || (flag == HFP_HF_ACCEPT_CALL_ACTION_HOLD)) {
            action = HFP_HF_CHLD_ACTION_2;
        } else if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_ACTIVE)) {
            action = HFP_HF_CHLD_ACTION_3;
        }

        HILOGI("[HFP_STATE_MACHINE]Accept held call");
    } else if (calls_->HasCallState(BTHF_CLIENT_CALL_STATE_HELD_BY_RESP_HOLD)) {
        action = BTHF_CLIENT_CALL_ACTION_BTRH_1;
    } else {
        HILOGI("[HFP_STATE_MACHINE]No call to accept");
        return;
    }

    HandleCallAction((BthfClientCallAction)action, HFP_HF_ACCEPT_CALL_EVT);

    if (flag == HFP_HF_ACCEPT_CALL_ACTION_HOLD) {
        SetAudioConnectionState(true);
    }
}

void HfpHfStateMachine::HandleCallAction(int action, int event)
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    BtStackStatus status = bluetoothHfInterface->handleCallAction(&rawAddr, (BthfClientCallAction)action, 0);
    if (status == BT_STATUS_SUCCESS) {
        queuedActions.push(event);
        HILOGE("[HFP_STATE_MACHINE]Send action success[%{public}d]", action);
    }
}

bool HfpHfStateMachine::HandleVoipCall(const HfpHfMessage &event)
{
    std::string number = event.call_.number;
    /* The viop call number is the local number plus +. */
    if (!number.empty() && number[0] == '+') {
        number.erase(number.begin());
    }
    /* If the call is a voip call, return. */
    if (number == "00000000000" || number == "10000000" || (!number.empty() && (number == subscriberNumber_))) {
        HILOGE("[HFP_STATE_MACHINE]voip number");
        return true;
    }
#ifdef BLUETOOTH_WATCH_ENABLE
    auto watchService = WatchService::GetInstance();
    if (watchService != nullptr) {
        watchService->SetIsHfOnCall(address_, true);
    }
#endif
    return false;
}

void HfpHfStateMachine::HandleCurrentCall(const HfpHfMessage &event)
{
#ifdef BLUETOOTH_WATCH_ENABLE
    if (HandleVoipCall(event)) {
        return;
    }
#endif
    ProcessCurrentCallEvent(event);
}

void HfpHfStateMachine::ProcessInteractiveEvent(const HfpHfMessage &event)
{
    HILOGI("[HFP_STATE_MACHINE][%{public}s]", GetInteractiveEventName(event.type_).c_str());
    switch (event.type_) {
        case HFP_HF_TYPE_CALL_STATE:
        case HFP_HF_TYPE_CALL_SETUP_STATE:
        case HFP_HF_TYPE_CALL_HELD_STATE:
        case HFP_HF_TYPE_HOLD_RESULT:
        case HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION:
        case HFP_HF_TYPE_CALL_WAITING:
            ProcessCallIndicatorEvent(event);
            break;
        case HFP_HF_TYPE_CURRENT_CALLS:
            HandleCurrentCall(event);
            break;
        case HFP_HF_TYPE_SET_VOLUME:
            ProcessSetVolumeEvent(event);
            break;
        case HFP_HF_TYPE_SUBSCRIBER_NUMBER:
            ProcessSubscriberNumberEvent(event);
            break;
        case HFP_HF_TYPE_INBAND_RING:
            ProcessInbandRingEvent(event);
            break;
        case HFP_HF_TYPE_NETWORK_STATE:
            ProcessNetworkStateEvent(event);
            break;
        case HFP_HF_TYPE_NETWORK_ROAM:
            ProcessNetworkRoamEvent(event);
            break;
        case HFP_HF_TYPE_NETWORK_SIGNAL:
            ProcessNetworkSignalEvent(event);
            break;
        case HFP_HF_TYPE_BATTERY_LEVEL:
            ProcessBatteryLevelEvent(event);
            break;
        case HFP_HF_TYPE_CURRENT_OPERATOR:
            ProcessOperatorEvent(event);
            break;
        case HFP_HF_TYPE_VOICE_RECOGNITION_CHANGED:
            NotifyVoiceRecognitionStatusChanged(event.arg1_);
            break;
        case HFP_HF_TYPE_AT_CMD_RESULT:
            ProcessAtCommandResult(event);
            break;
        default:
            break;
    }
}

void HfpHfStateMachine::ProcessAtCommandResult(const HfpHfMessage &event)
{
    HILOGI("[HFP_STATE_MACHINE]action = %{public}d", queuedActions.front());
    switch (queuedActions.front()) {
        case HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE:
            calls_->UpdateCallDone();
            isOnClcc_ = false;
            break;
        case HFP_HF_OPEN_VOICE_RECOGNITION_EVT:
            ProcessOpenVoiceRecognitionResult(event);
            break;
        case HFP_HF_CLOSE_VOICE_RECOGNITION_EVT:
            ProcessCloseVoiceRecognitionResult(event);
            break;
        case HFP_HF_DIAL_CALL_EVT:
            if (event.arg1_ == BTHF_CLIENT_CMD_COMPLETE_OK) {
#ifdef BLUETOOTH_WATCH_ENABLE
                auto watchService = WatchService::GetInstance();
                if (watchService != nullptr) {
                    watchService->SetDialingByWatch(address_);
                    watchService->SetAllowHfScoConnect(address_, true);
                }
#endif
            } else {
                calls_->UpdateCallDone();
            }
            break;
        case HFP_HF_FINISH_CALL_EVT:
        case HFP_HF_REJECT_CALL_EVT:
            if (event.arg1_ != BTHF_CLIENT_CMD_COMPLETE_OK) {
                calls_->UpdateCallDone();
            }
            break;
        default:
            break;
    }
    if (queuedActions.size() > 0) {
        queuedActions.pop();
    }
}

#ifdef BLUETOOTH_WATCH_ENABLE
void HfpHfStateMachine::WatchDialCall()
{
    auto watchService = WatchService::GetInstance();
    if (watchService != nullptr && watchService->IsNeedDialCall()) {
        std::string number = watchService->GetDialedCallNumber();
        if (!number.empty()) {
            HILOGI("[HFP_STATE_MACHINE] watch start dial call");
            HfpHfMessage event(HFP_HF_DIAL_CALL_EVT);
            HandsFreeUnitCalls callTmp(
                GetDeviceAddr(), HFP_HF_OUTGOING_CALL_ID, HFP_CALL_STATE_DIALING, number, false, true, false);
            event.calls_ = callTmp;
            ProcessDialCall(event);
        }
        watchService->DialCallFinished();
    }
}
#endif

void HfpHfStateMachine::NotifyStateTransitions()
{
    HfpHfService *service = HfpHfService::GetService();
    int toState = GetDeviceState();
    if (service != nullptr) {
        RawAddress device(address_);
        if ((preState_ != toState) && (preState_ <= HFP_HF_STATE_CONNECTED) && (toState <= HFP_HF_STATE_CONNECTED)) {
            service->NotifyStateChanged(device, toState);
        }

        if ((preState_ != toState) && (preState_ >= HFP_HF_AUDIO_STATE_DISCONNECTED) &&
            (toState >= HFP_HF_AUDIO_STATE_DISCONNECTED)) {
            service->NotifyScoStateChanged(device, toState);
        }

        // Bluetooth is disconnected during a active call
        if ((preState_ == HFP_HF_AUDIO_STATE_CONNECTED) && (toState <= HFP_HF_STATE_DISCONNECTED)) {
            service->NotifyStateChanged(device, toState);
            service->NotifyScoStateChanged(device, HFP_HF_AUDIO_STATE_DISCONNECTED);
        }
    }

    if (toState == HFP_HF_AUDIO_STATE_CONNECTED || toState == HFP_HF_AUDIO_STATE_DISCONNECTED) {
        SetAudioParameter(toState);
    }
    preState_ = toState;
}

void HfpHfStateMachine::NotifyChildStateToParentState(int fromState, int toState)
{
    HfpHfService *service = HfpHfService::GetService();
    if (service != nullptr) {
        if ((fromState != toState) && (fromState >= HFP_HF_AUDIO_STATE_DISCONNECTED) &&
            (toState >= HFP_HF_AUDIO_STATE_DISCONNECTED)) {
            RawAddress device(address_);
            service->NotifyScoStateChanged(device, toState);
        }
    }

    if (toState == HFP_HF_AUDIO_STATE_DISCONNECTED) {
        SetAudioConnectionState(false);
    }

    preState_ = toState;
    ProcessDeferredMessage();
}

void HfpHfStateMachine::ProcessAudioDisconnected()
{
    IPowerManager::GetInstance()->StatusUpdate(RequestStatus::SCO_OFF, PROFILE_NAME_HFP_HF, RawAddress(address_));
#ifdef BLUETOOTH_WATCH_ENABLE
    auto watchService = WatchService::GetInstance();
    if (watchService != nullptr) {
        watchService->StopIphoneScoDisconnectTimer(address_);
    }
#endif
    NotifyChildStateToParentState(HFP_HF_AUDIO_STATE_CONNECTED, HFP_HF_AUDIO_STATE_DISCONNECTED);
}

void HfpHfStateMachine::SetAudioConnectedParameters()
{
    isScoConnected_ = false;  // reset flag of sco connected
    SetAudioParameter(HFP_HF_AUDIO_STATE_CONNECTED);
}

void HfpHfStateMachine::SetAudioParameter(int audioStatus)
{
    if (audioStatus == HFP_HF_AUDIO_STATE_CONNECTED) {
        HILOGI("[HFP_STATE_MACHINE]isAudioWbs=%{public}d", isAudioWbs_);
        if (isAudioWbs_) {
            SetAudioExtraParameters("hfp_set_sampling_rate", "16000");
        } else {
            SetAudioExtraParameters("hfp_set_sampling_rate", "8000");
        }

        int spkVolume = BluetoothAudioFrameworkAdapter::GetStreamVolume(STREAM_VOICE_CALL);
        SetAudioExtraParameters("hfp_volume", std::to_string(spkVolume));
        SetAudioConnectionState(true);
    }

    if (audioStatus == HFP_HF_AUDIO_STATE_DISCONNECTED) {
        SetAudioConnectionState(false);
        if (voiceRecognitionStatus_ == HFP_HF_VR_STATE_OPENED) {
            NotifyVoiceRecognitionStatusChanged(HFP_HF_VR_STATE_CLOSED);
        }
    }
}

int HfpHfStateMachine::GetVoiceRecognitionState() const
{
    return voiceRecognitionStatus_;
}

bool HfpHfStateMachine::IsInbandRing() const
{
    return isInBandRing_;
}

int HfpHfStateMachine::GetVolume() const
{
    return volume_;
}

int HfpHfStateMachine::GetDeviceState() const
{
    return static_cast<const HfpHfState*>(GetState())->GetStateInt();
}

std::vector<HandsFreeUnitCalls> HfpHfStateMachine::GetCurrentCallList()
{
    return calls_->GetCurrentCalls();
}

void HfpHfStateMachine::AddDeferredMessage(const HfpHfMessage &msg)
{
    deferMsgs_.push_back(msg);
    HILOGI("[HFP_STATE_MACHINE]AddDeferredMessage msg=%{public}d", msg.what_);
}

void HfpHfStateMachine::ProcessDeferredMessage()
{
    auto size = deferMsgs_.size();
    HILOGI("[HFP_STATE_MACHINE]ProcessDeferredMessage msg size=%{public}d", size);
    while (size > 0 && !deferMsgs_.empty()) {
        HfpHfMessage event = deferMsgs_.front();
        deferMsgs_.pop_front();
        HfpHfService* service = HfpHfService::GetService();
        CHECK_AND_RETURN_LOG(service != nullptr, "[HFP_STATE_MACHINE]service is null");
        service->PostEvent(event);
        size--;
        HILOGI("[HFP_STATE_MACHINE]ProcessDeferredMessage msg=%{public}d", event.what_);
    }
}

void HfpHfStateMachine::StartConnectionTimer() const
{
    connTimer_->Start(HF_CONNECTION_TIMEOUT_MS);
    HILOGI("[HFP_STATE_MACHINE]Start connection timer!");
}

void HfpHfStateMachine::StopConnectionTimer() const
{
    connTimer_->Stop();
    HILOGI("[HFP_STATE_MACHINE]Stop connection timer!");
}

void HfpHfStateMachine::ConnectionTimeout() const
{
    HfpHfMessage event(HFP_HF_CONNECTION_TIMEOUT_EVT);
    event.dev_ = address_;
    HfpHfService* service = HfpHfService::GetService();
    CHECK_AND_RETURN_LOG(service != nullptr, "[HFP_STATE_MACHINE]service is null");
    service->PostEvent(event);
}

bool HfpHfStateMachine::IsRemoving() const
{
    return isRemoving_;
}

void HfpHfStateMachine::SetRemoving(bool isRemoving)
{
    isRemoving_ = isRemoving;
}

void HfpHfStateMachine::ProcessNetworkStateEvent(const HfpHfMessage &event)
{
    HfpHfService *service = HfpHfService::GetService();
    RawAddress rawAddress(address_);
    if (networkState_ != event.arg1_) {
        networkState_ = event.arg1_;
        if (service != nullptr) {
            service->NotifyRegistrationStatusChanged(rawAddress, networkState_);
        } else {
            HILOGE("[HFP_STATE_MACHINE]service is null.");
        }
    }

    if (networkState_ == HFP_HF_NETWORK_STATE_AVAILABLE) {
        STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(rawAddress);
        CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

        BtStackStatus status = bluetoothHfInterface->queryCurrentOperatorName(&rawAddr);
        if (status != BT_STATUS_SUCCESS) {
            HILOGE("[HFP_STATE_MACHINE]can't query the operator name!");
        } else {
            queuedActions.push(HFP_HF_TYPE_NETWORK_STATE);
        }
    }
}
void HfpHfStateMachine::ProcessNetworkRoamEvent(const HfpHfMessage &event)
{
    HfpHfService *service = HfpHfService::GetService();
    RawAddress rawAddress(address_);
    if (networkTRoam_ != event.arg1_) {
        networkTRoam_ = event.arg1_;
        if (service != nullptr) {
            service->NotifyRoamingStatusChanged(rawAddress, networkTRoam_);
        } else {
            HILOGE("[HFP_STATE_MACHINE]service is null.");
        }
    }
}
void HfpHfStateMachine::ProcessNetworkSignalEvent(const HfpHfMessage &event)
{
    HfpHfService *service = HfpHfService::GetService();
    RawAddress rawAddress(address_);
    if (networkSignal_ != event.arg1_) {
        networkSignal_ = event.arg1_;
        if (service != nullptr) {
            service->NotifySignalStrengthChanged(rawAddress, networkSignal_);
        } else {
            HILOGE("[HFP_STATE_MACHINE]service is null.");
        }
    }
}
void HfpHfStateMachine::ProcessBatteryLevelEvent(const HfpHfMessage &event)
{
    HfpHfService *service = HfpHfService::GetService();
    RawAddress rawAddress(address_);
    if (batteryLevel_ != event.arg1_) {
        batteryLevel_ = event.arg1_;
        if (service != nullptr) {
            service->NotifyBatteryLevelChanged(rawAddress, batteryLevel_);
        } else {
            HILOGE("[HFP_STATE_MACHINE]service is null.");
        }
    }
}

void HfpHfStateMachine::ProcessOperatorEvent(const HfpHfMessage &event)
{
    HfpHfService *service = HfpHfService::GetService();
    RawAddress rawAddress(address_);
    if (operatorName_ != event.str_) {
        operatorName_ = event.str_;
        if (service != nullptr) {
            service->NotifyOperatorSelectionChanged(rawAddress, operatorName_);
        } else {
            HILOGE("[HFP_STATE_MACHINE]service is null.");
        }
    }
}

void HfpHfStateMachine::ProcessCallIndicatorEvent(const HfpHfMessage &event)
{
#ifdef BLUETOOTH_WATCH_ENABLE
    if (isOnClcc_) {
        return;
    }
    auto watchService = WatchService::GetInstance();
    if (watchService != nullptr) {
        watchService->SetIsHfOnCall(address_, true);
    }
#endif

    RawAddress device(address_);
    HILOGI("[HFP_STATE_MACHINE]Send CLCC!");
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    BtStackStatus status = bluetoothHfInterface->queryCurrentCalls(&rawAddr);
    if (status != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]can't query the current calls!");
    } else {
        isOnClcc_ = true;
        queuedActions.push(HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE);
    }
}

void HfpHfStateMachine::ProcessSlcEstablished(const HfpHfMessage &event)
{
    RawAddress device(address_);
    STACK::RawAddress rawAddr = ServiceUtil::AddrToStack(device);
    CHECK_AND_RETURN_LOG(bluetoothHfInterface != nullptr, "[HFP_STATE_MACHINE]BluetoothHfInterface is null");

    peerFeatures_ = event.arg1_;
    chldFeatures_ = event.arg3_;
    // We do not support devices which do not support enhanced call status (ECS).
    if ((static_cast<uint32_t>(peerFeatures_) & HFP_HF_AG_FEATURES_ENHANCED_CALL_STATUS) == 0) {
        bluetoothHfInterface->disconnect(&rawAddr);
        return;
    }
#ifdef BLUETOOTH_WATCH_ENABLE
    auto watchService = WatchService::GetInstance();
    if (watchService != nullptr && !(watchService->IsDialingByWatch(address_))) {
        ProcessCallIndicatorEvent(event);
    }
#endif

    if (HANDSFREECLIENT_NREC_SUPPORTED && ((static_cast<uint32_t>(peerFeatures_) &
        HFP_HF_AG_FEATURES_ECNR) == HFP_HF_AG_FEATURES_ECNR)) {
        BtStackStatus status = bluetoothHfInterface->sendAtCmd(&rawAddr, HANDSFREECLIENT_AT_CMD_NREC, 1, 0, NULL);
        if (status != BT_STATUS_SUCCESS) {
            HILOGE("[HFP_STATE_MACHINE]Failed to send NREC!");
        } else {
            queuedActions.push(HANDSFREECLIENT_AT_CMD_NREC);
        }
    }
    int volume = BluetoothAudioFrameworkAdapter::GetStreamVolume(STREAM_VOICE_CALL);
    ProcessSetHfVolume(volume, HFP_HF_VOLUME_TYPE_SPK);

    BtStackStatus subscriberStatus = bluetoothHfInterface->retrieveSubscriberInfo(&rawAddr);
    if (subscriberStatus != BT_STATUS_SUCCESS) {
        HILOGE("[HFP_STATE_MACHINE]Failed to retrieve subscriber info!");
    } else {
        queuedActions.push(HFP_HF_SUBSCRIBER_INFO);
    }

#ifdef BLUETOOTH_WATCH_ENABLE
    if (watchService != nullptr && watchService->GetBluetoothWatchInterface() != nullptr) {
        watchService->GetBluetoothWatchInterface()->sendTbsf(&rawAddr);
    }
#endif
}

void HfpHfStateMachine::ProcessCurrentCallEvent(const HfpHfMessage &event) const
{
    HandsFreeUnitCalls call(address_,
        event.call_.index,
        event.call_.status,
        event.call_.number,
        (event.call_.mprty == HFP_HF_MPTY_TYPE_MULTI),
        (event.call_.dir == HFP_HF_DIRECTION_TYPE_OUTGOING),
        isInBandRing_);
    calls_->UpdateCall(call);
}

void HfpHfStateMachine::ProcessSetVolumeEvent(const HfpHfMessage &event)
{
    if (event.arg1_ == HFP_HF_VOLUME_TYPE_SPK) {
        BluetoothAudioFrameworkAdapter::SetStreamVolume(STREAM_VOICE_CALL, event.arg3_, 1);
        volume_ = event.arg3_;
    } else if (event.arg1_ == HFP_HF_VOLUME_TYPE_MIC) {
        BluetoothAudioFrameworkAdapter::SetAudioMicrophoneMute(event.arg3_ == 0);
    } else {
        HILOGE("[HFP_STATE_MACHINE]the error volume type!");
    }
}

void HfpHfStateMachine::ProcessSubscriberNumberEvent(const HfpHfMessage &event)
{
    HfpHfService *service = HfpHfService::GetService();
    RawAddress rawAddress(address_);
    if (subscriberNumber_ != event.str_) {
        subscriberNumber_ = event.str_;
        if (service != nullptr) {
            service->NotifySubscriberNumberChanged(rawAddress, subscriberNumber_);
        } else {
            HILOGE("[HFP_STATE_MACHINE]service is null.");
        }
    }
}
void HfpHfStateMachine::ProcessInbandRingEvent(const HfpHfMessage &event)
{
    HfpHfService *service = HfpHfService::GetService();
    RawAddress rawAddress(address_);
    bool ring = (event.arg1_ == 0) ? false : true;
    if (isInBandRing_ != ring) {
        isInBandRing_ = ring;
        if (service != nullptr) {
            service->NotifyInBandRingTone(rawAddress, event.arg1_);
        } else {
            HILOGE("[HFP_STATE_MACHINE]service is null.");
        }
    }
}

void HfpHfStateMachine::NotifyVoiceRecognitionStatusChanged(int status)
{
    if (status == voiceRecognitionStatus_) {
        return;
    }
    voiceRecognitionStatus_ = status;
    HfpHfService *service = HfpHfService::GetService();
    if (service != nullptr) {
        RawAddress device(address_);
        service->NotifyVoiceRecognitionStatusChanged(device, voiceRecognitionStatus_);
    }
}

void HfpHfStateMachine::SetAudioConnectionState(bool isScoConnected)
{
    if (isScoConnected) {
        if (!isScoConnected_) {
            SetAudioExtraParameters("hfp_enable", "true");
        }
    } else {
        SetAudioExtraParameters("hfp_enable", "false");
    }
    isScoConnected_ = isScoConnected;
    calls_->SetScoConnected(isScoConnected_);
}

void HfpHfStateMachine::SetAudioWbs(bool isAudioWbs)
{
    isAudioWbs_ = isAudioWbs;
}

void HfpHfStateMachine::ProcessOpenVoiceRecognitionResult(const HfpHfMessage &event)
{
    int status = voiceRecognitionStatus_;
    if (event.arg1_ == HFP_HF_AT_RESULT_OK) {
        status = HFP_HF_VR_STATE_OPENED;
    }
    NotifyVoiceRecognitionStatusChanged(status);
}

void HfpHfStateMachine::ProcessCloseVoiceRecognitionResult(const HfpHfMessage &event)
{
    int status = voiceRecognitionStatus_;
    if (event.arg1_ == HFP_HF_AT_RESULT_OK) {
        status = HFP_HF_VR_STATE_CLOSED;
    }
    NotifyVoiceRecognitionStatusChanged(status);
}

bool HfpHfStateMachine::IsAllowAgConnected(const std::string &address)
{
    RawAddress device(address);
    int strategy = ConnectStrategyManager::GetInstance()->QueryConnectStrategy(device,
        PROPERTY_HFP_CLIENT_CONNECTION_POLICY);
    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    if (classicAdapter == nullptr) {
        HILOGI("[HFP_STATE_MACHINE]classicAdapter is nullptr");
        return false;
    }
#ifdef BLUETOOTH_WATCH_ENABLE
    if ((classicAdapter->GetPairState(device) != PAIR_NONE) &&
        strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN))
#else
    if (classicAdapter->GetPairState(device) == PAIR_PAIRED &&
        strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN))
#endif
    {
        HILOGI("[HFP_STATE_MACHINE]Device is ok to connect");
        return true;
    }
    HILOGI("[HFP_STATE_MACHINE]Connect strategy=%{public}d, pair state=%{public}d",
        strategy, classicAdapter->GetPairState(device));
    return false;
}

void HfpHfStateMachine::SetAudioExtraParameters(const std::string &key, const std::string &value)
{
    std::vector<std::pair<std::string, std::string>> vec = {
        std::pair<std::string, std::string>(key, value)
    };
    HILOGI("[HFP_STATE_MACHINE]key = %{public}s, value = %{public}s", key.c_str(), value.c_str());
    BluetoothAudioFrameworkAdapter::HfpSetExtraAudioParameters(AUDIO_PARAM_KEY, vec);
}

std::string HfpHfStateMachine::GetEventName(int what)
{
    switch (what) {
        case HFP_HF_SERVICE_STARTUP_EVT:
            return "HFP_HF_SERVICE_STARTUP_EVT";
        case HFP_HF_SERVICE_SHUTDOWN_EVT:
            return "HFP_HF_SERVICE_SHUTDOWN_EVT";
        case HFP_HF_REMOVE_STATE_MACHINE_EVT:
            return "HFP_HF_REMOVE_STATE_MACHINE_EVT";
        case HFP_HF_INTERACTIVE_EVT:
            return "HFP_HF_INTERACTIVE_EVT";
        case HFP_HF_SEND_DTMF_EVT:
            return "HFP_HF_SEND_DTMF_EVT";
        case HFP_HF_ACCEPT_CALL_EVT:
            return "HFP_HF_ACCEPT_CALL_EVT";
        case HFP_HF_HOLD_CALL_EVT:
            return "HFP_HF_HOLD_CALL_EVT";
        case HFP_HF_REJECT_CALL_EVT:
            return "HFP_HF_REJECT_CALL_EVT";
        case HFP_HF_HANDLE_INCOMING_CALL_EVT:
            return "HFP_HF_HANDLE_INCOMING_CALL_EVT";
        case HFP_HF_HANDLE_MULTI_CALL_EVT:
            return "HFP_HF_HANDLE_MULTI_CALL_EVT";
        case HFP_HF_DIAL_LAST_NUMBER:
            return "HFP_HF_DIAL_LAST_NUMBER";
        case HFP_HF_DIAL_MEMORY:
            return "HFP_HF_DIAL_MEMORY";
        case HFP_HF_FINISH_CALL_EVT:
            return "HFP_HF_FINISH_CALL_EVT";
        case HFP_HF_DIAL_CALL_EVT:
            return "HFP_HF_DIAL_CALL_EVT";
        case HFP_HF_SEND_AT_COMMAND_EVT:
            return "HFP_HF_SEND_AT_COMMAND_EVT";
        case HFP_HF_OPEN_VOICE_RECOGNITION_EVT:
            return "HFP_HF_OPEN_VOICE_RECOGNITION_EVT";
        case HFP_HF_CLOSE_VOICE_RECOGNITION_EVT:
            return "HFP_HF_CLOSE_VOICE_RECOGNITION_EVT";
        case HFP_HF_SET_VOLUME_EVT:
            return "HFP_HF_SET_VOLUME_EVT";
        case HFP_HF_BATTERY_LEVEL_CHANGED_EVT:
            return "HFP_HF_BATTERY_LEVEL_CHANGED_EVT";
        case HFP_HF_ENHANCED_DRIVER_SAFETY_CHANGED_EVT:
            return "HFP_HF_ENHANCED_DRIVER_SAFETY_CHANGED_EVT";
        case HFP_HF_SDP_DISCOVERY_RESULT_SUCCESS:
            return "HFP_HF_SDP_DISCOVERY_RESULT_SUCCESS";
        case HFP_HF_SDP_DISCOVERY_RESULT_FAIL:
            return "HFP_HF_SDP_DISCOVERY_RESULT_FAIL";
        default:
            return GetConnectEventName(what);
    }
}

std::string HfpHfStateMachine::GetConnectEventName(int what)
{
    switch (what) {
        case HFP_HF_CONNECT_EVT:
            return "HFP_HF_CONNECT_EVT";
        case HFP_HF_DISCONNECT_EVT:
            return "HFP_HF_DISCONNECT_EVT";
        case HFP_HF_CONNECT_AUDIO_EVT:
            return "HFP_HF_CONNECT_AUDIO_EVT";
        case HFP_HF_DISCONNECT_AUDIO_EVT:
            return "HFP_HF_DISCONNECT_AUDIO_EVT";
        case HFP_HF_RETRY_CONNECT_AUDIO_EVT:
            return "HFP_HF_RETRY_CONNECT_AUDIO_EVT";
        case HFP_HF_CONNECTION_TIMEOUT_EVT:
            return "HFP_HF_CONNECTION_TIMEOUT_EVT";
        case HFP_HF_DISCONNECT_TIMEOUT_EVT:
            return "HFP_HF_DISCONNECT_TIMEOUT_EVT";
        case HFP_HF_CONNECT_AUDIO_TIMEOUT_EVT:
            return "HFP_HF_CONNECT_AUDIO_TIMEOUT_EVT";
        case HFP_HF_DISCONNECT_AUDIO_TIMEOUT_EVT:
            return "HFP_HF_DISCONNECT_AUDIO_TIMEOUT_EVT";
        case HFP_HF_CONNECTED_EVT:
            return "HFP_HF_CONNECTED_EVT";
        case HFP_HF_DISCONNECTED_EVT:
            return "HFP_HF_DISCONNECTED_EVT";
        case HFP_HF_SLC_ESTABLISHED_EVT:
            return "HFP_HF_SLC_ESTABLISHED_EVT";
        case HFP_HF_AUDIO_CONNECTING_EVT:
            return "HFP_HF_AUDIO_CONNECTING_EVT";
        case HFP_HF_AUDIO_CONNECTED_EVT:
            return "HFP_HF_AUDIO_CONNECTED_EVT";
        case HFP_HF_AUDIO_CONNECTED_MSBC_EVT:
            return "HFP_HF_AUDIO_CONNECTED_MSBC_EVT";
        case HFP_HF_AUDIO_DISCONNECTED_EVT:
            return "HFP_HF_AUDIO_DISCONNECTED_EVT";
        case HFP_HF_SEND_KEY_PRESSED:
            return "HFP_HF_SEND_KEY_PRESSED";
        default:
            return "Unknown";
    }
}

std::string HfpHfStateMachine::GetInteractiveEventName(int type) const
{
    switch (type) {
        case HFP_HF_TYPE_NETWORK_STATE:
            return "HFP_HF_TYPE_NETWORK_STATE";
        case HFP_HF_TYPE_NETWORK_ROAM:
            return "HFP_HF_TYPE_NETWORK_ROAM";
        case HFP_HF_TYPE_NETWORK_SIGNAL:
            return "HFP_HF_TYPE_NETWORK_SIGNAL";
        case HFP_HF_TYPE_BATTERY_LEVEL:
            return "HFP_HF_TYPE_BATTERY_LEVEL";
        case HFP_HF_TYPE_CURRENT_OPERATOR:
            return "HFP_HF_TYPE_CURRENT_OPERATOR";
        case HFP_HF_TYPE_CALL_STATE:
            return "HFP_HF_TYPE_CALL_STATE";
        case HFP_HF_TYPE_CALL_SETUP_STATE:
            return "HFP_HF_TYPE_CALL_SETUP_STATE";
        case HFP_HF_TYPE_CALL_HELD_STATE:
            return "HFP_HF_TYPE_CALL_HELD_STATE";
        case HFP_HF_TYPE_HOLD_RESULT:
            return "HFP_HF_TYPE_HOLD_RESULT";
        case HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION:
            return "HFP_HF_TYPE_CALLING_LINE_IDENTIFICATION";
        case HFP_HF_TYPE_CALL_WAITING:
            return "HFP_HF_TYPE_CALL_WAITING";
        case HFP_HF_TYPE_CURRENT_CALLS:
            return "HFP_HF_TYPE_CURRENT_CALLS";
        case HFP_HF_TYPE_SET_VOLUME:
            return "HFP_HF_TYPE_SET_VOLUME";
        case HFP_HF_TYPE_AT_CMD_RESULT:
            return "HFP_HF_TYPE_AT_CMD_RESULT";
        case HFP_HF_TYPE_SUBSCRIBER_NUMBER:
            return "HFP_HF_TYPE_SUBSCRIBER_NUMBER";
        case HFP_HF_TYPE_INBAND_RING:
            return "HFP_HF_TYPE_INBAND_RING";
        case HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE:
            return "HFP_HF_TYPE_QUERY_CURRENT_CALLS_DONE";
        default:
            return "Unknown";
    }
}
}  // namespace bluetooth
}  // namespace OHOS
