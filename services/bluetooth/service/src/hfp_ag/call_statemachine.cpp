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
#define LOG_TAG "bt_service_hfp_ag"
#endif

#include "call_statemachine.h"

#include "bluetooth_log.h"
#include "common/bluetooth_hw_interface.h"
#include "hw_interop.h"
#include "thread_util.h"
#include "remote_device_properties.h"

namespace OHOS {
namespace bluetooth {
using utility::StateMachine;
using std::make_unique;

CallStateMachine::CallStateMachine(const std::string &address) : eventProcessor_(address), address_(address)
{
    std::unique_ptr<StateMachine::State> dialing = make_unique<DialingState>(*this);
    StateMachine::Move(dialing);
    std::unique_ptr<StateMachine::State> alerting = make_unique<AlertingState>(*this);
    StateMachine::Move(alerting);
    std::unique_ptr<StateMachine::State> active = make_unique<ActiveState>(*this);
    StateMachine::Move(active);
    std::unique_ptr<StateMachine::State> end = make_unique<EndState>(*this);
    StateMachine::Move(end);

    StateMachine::InitState(CALL_END_STATE);
}

std::string CallStateMachine::GetDeviceAddr() const
{
    return address_;
}

void CallStateMachine::ReInit(void)
{
    RemoveTask();
    StateMachine::InitState(CALL_END_STATE);
    phoneState_ = {0, 0, 0, "", 0, ""};
}

const char* CallStateMachine::GetEventName(int event)
{
    switch (event) {
        case HFP_AG_CALL_STATE_CHANGE:
            return "HFP_AG_CALL_STATE_CHANGE";
        case CallStateMachine::CALL_ALERT_EVENT:
            return "CALL_ALERT_EVENT";
        case CallStateMachine::CALL_ACTIVE_EVENT:
            return "CALL_ACTIVE_EVENT";
        case CallStateMachine::CALL_END_EVENT:
            return "CALL_END_EVENT";
        default: break;
    }
    return "Unknown";
}

void DialingState::Entry()
{
    sm_.ProcessPhoneStateChange();
    HILOGI("call dialing");
}

void DialingState::Exit()
{
    HILOGI("call dialing");
}

bool DialingState::Dispatch(const utility::Message &msg)
{
    HILOGI("receive msg: %{public}s in dialing state", CallStateMachine::GetEventName(msg.what_));
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HfpAgPhoneState phoneState = event.state_;
    switch (msg.what_) {
        case HFP_AG_CALL_STATE_CHANGE:
            sm_.DealCallStateChangeWhileDialing(phoneState);
            break;
        case CallStateMachine::CALL_ALERT_EVENT:
            Transition(ALERTING_STATE);
            break;
        default:
            HILOGE("UnSupport Event");
            break;
    }

    return true;
}

void AlertingState::Entry()
{
    sm_.ProcessPhoneStateChange();
    HILOGI("call alerting");
}

void AlertingState::Exit()
{
    HILOGI("call alerting");
}

bool AlertingState::Dispatch(const utility::Message &msg)
{
    HILOGI("receive msg: %{public}s in alerting state", CallStateMachine::GetEventName(msg.what_));
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HfpAgPhoneState phoneState = event.state_;
    switch (msg.what_) {
        case HFP_AG_CALL_STATE_CHANGE:
            sm_.DealCallStateChangeWhileAlerting(phoneState);
            break;
        case CallStateMachine::CALL_ACTIVE_EVENT:
            Transition(CALL_ACTIVE_STATE);
            break;
        case CallStateMachine::CALL_END_EVENT:
            Transition(CALL_END_STATE);
            break;
        default:
            HILOGE("UnSupport Event");
            break;
    }

    return true;
}

void ActiveState::Entry()
{
    sm_.ProcessPhoneStateChange();
    HILOGI("call active");
}

void ActiveState::Exit()
{
    HILOGI("call active");
}

bool ActiveState::Dispatch(const utility::Message &msg)
{
    HILOGI("receive msg: %{public}s in call active state",
        CallStateMachine::GetEventName(msg.what_));
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HfpAgPhoneState phoneState = event.state_;
    switch (msg.what_) {
        case HFP_AG_CALL_STATE_CHANGE: {
            if (phoneState.callState == HFP_AG_CALL_STATE_IDLE && phoneState.activeNum == 0) {
                sm_.SetPhoneState(phoneState);
                Transition(CALL_END_STATE);
                HILOGI("call moved to end state from active state");
            } else {
                HILOGE("error call state");
            }
            break;
        }
        default:
            HILOGE("UnSupport Event");
            break;
    }

    return true;
}

void EndState::Entry()
{
    if (sm_.GetCallState() == HFP_AG_CALL_STATE_ACTIVE) {
        return;
    }
    sm_.ProcessPhoneStateChange();
    HILOGI("call end");
}

void EndState::Exit()
{
    HILOGI("call end");
}

bool EndState::Dispatch(const utility::Message &msg)
{
    HILOGI("receive msg: %{public}s in call end state",
        CallStateMachine::GetEventName(msg.what_));
    HfpAgMessage &event = static_cast<HfpAgMessage &>(const_cast<utility::Message &>(msg));
    HfpAgPhoneState phoneState = event.state_;
    switch (msg.what_) {
        case HFP_AG_CALL_STATE_CHANGE: {
            if (phoneState.callState == HFP_AG_CALL_STATE_DIALING) {
                sm_.SetPhoneState(phoneState);
                Transition(DIALING_STATE);
                HILOGI("call moved to dialing state from end state");
            } else {
                HILOGE("error call state");
            }
            break;
        }
        default:
            HILOGE("UnSupport Event");
            break;
    }

    return true;
}

void CallStateMachine::DealVoipCallStateChanged(const HfpAgMessage &event)
{
    ProcessMessage(event);
}

int CallStateMachine::GetActiveNum(void)
{
    return phoneState_.activeNum;
}

int CallStateMachine::GetCallState(void)
{
    return phoneState_.callState;
}

void CallStateMachine::SetPhoneState(const HfpAgPhoneState &phoneState)
{
    phoneState_ = phoneState;
}

void CallStateMachine::ProcessPhoneStateChange(void)
{
    eventProcessor_.ProcessPhoneStateChange(phoneState_);
}

void CallStateMachine::TransitionToTargetState(int stateEvent)
{
    HfpAgMessage event(stateEvent);
    ProcessMessage(event);
}

void CallStateMachine::DealCallStateChangeWhileDialing(HfpAgPhoneState &phoneState)
{
    bool disableDelayed = BluetoothHwInterface::GetInstance()->
        InteropMatch(INTEROP_DISABLE_DELAY_SEND_CALL_STATE, RawAddress(address_));
    std::shared_ptr<BluetoothDevice> remoteDevice =
        RemoteDeviceProperties::GetInstance()->FindRemoteDevice(RawAddress(address_));
    if (!disableDelayed && remoteDevice != nullptr && remoteDevice->isBandDevice()) {
        HILOGI("remoteDevice is band device, sends CIEV without delay");
        disableDelayed = true;
    }
    int alertDelayedTime = disableDelayed ? NO_DELAYED : VIRTUAL_CALL_ALERTING_DELAY_TIME;
    int activeDelayedTime = disableDelayed ? NO_DELAYED : VIRTUAL_CALL_ACTIVE_STATE_DELAY_TIME;
    HILOGI("call State: %{public}d, device: %{public}s, alertDelayedTime:%{public}d, activeDelayedTime:%{public}d",
        phoneState.callState, GET_ENCRYPT_STR_ADDR(address_), alertDelayedTime, activeDelayedTime);

    if (phoneState.callState == HFP_AG_CALL_STATE_ALERTING) {
        ThreadUtil::GetInstance().PostTask(THREAD_ID_HFP,
            [this, phoneState]() { this->ProcessAlertingCallState(phoneState); },
            alertDelayedTime,
            "TASK_PROCESS_DEALYED_ALERTING_STATE");
    } else if (phoneState.callState == HFP_AG_CALL_STATE_IDLE && phoneState.activeNum == 1) {
        ThreadUtil::GetInstance().PostTask(THREAD_ID_HFP,
            [this, phoneState]() { this->ProcessIdleCallState(phoneState); },
            activeDelayedTime,
            "TASK_PROCESS_DEALYED_ACTIVE_STATE");
    } else if (phoneState.callState == HFP_AG_CALL_STATE_IDLE && phoneState.activeNum == 0) {
        RemoveTask();
        // send 2,3 although the call is ended to make sure that we are sending 2,3 always
        phoneState.callState = HFP_AG_CALL_STATE_ALERTING;
        SetPhoneState(phoneState);
        TransitionToTargetState(CALL_ALERT_EVENT);
        HILOGI("call moved to alerting state from dialing state before call end");

        phoneState.callState = HFP_AG_CALL_STATE_IDLE;
        SetPhoneState(phoneState);
        TransitionToTargetState(CALL_END_EVENT);
        HILOGI("call moved to end state from dialing state");
    } else {
        HILOGE("error call state");
    }
}

void CallStateMachine::DealCallStateChangeWhileAlerting(HfpAgPhoneState &phoneState)
{
    int delayedTime = BluetoothHwInterface::GetInstance()->
        InteropMatch(INTEROP_DISABLE_DELAY_SEND_CALL_STATE, RawAddress(address_)) ?
        NO_DELAYED : VIRTUAL_CALL_IDLE_DELAY_TIME;
    HILOGI("call State: %{public}d, device: %{public}s, delayedTime:%{public}d",
        phoneState.callState, GET_ENCRYPT_STR_ADDR(address_), delayedTime);

    if (phoneState.callState == HFP_AG_CALL_STATE_IDLE && phoneState.activeNum == 1) {
        ThreadUtil::GetInstance().PostTask(THREAD_ID_HFP,
            [this, phoneState]() { this->ProcessIdleCallState(phoneState); },
            delayedTime,
            "TASK_PROCESS_DEALYED_ACTIVE_STATE");
    } else if (phoneState.callState == HFP_AG_CALL_STATE_IDLE && phoneState.activeNum == 0) {
        ThreadUtil::GetInstance().RemoveTask(THREAD_ID_HFP, "TASK_PROCESS_DEALYED_ACTIVE_STATE");
        SetPhoneState(phoneState);
        TransitionToTargetState(CALL_END_EVENT);
        HILOGI("call moved to end state from alerting state");
    } else {
        HILOGE("error call state");
    }
}

void CallStateMachine::ProcessAlertingCallState(const HfpAgPhoneState &phoneState)
{
    SetPhoneState(phoneState);
    TransitionToTargetState(CALL_ALERT_EVENT);
    HILOGI("call moved to alerting state from dialing state");
}


void CallStateMachine::ProcessIdleCallState(const HfpAgPhoneState &phoneState)
{
    SetPhoneState(phoneState);
    TransitionToTargetState(CALL_ACTIVE_EVENT);
    HILOGI("call moved to active state from dialing/alerting state");
}

void CallStateMachine::RemoveTask(void)
{
    ThreadUtil::GetInstance().RemoveTask(THREAD_ID_HFP, "TASK_PROCESS_DEALYED_ALERTING_STATE");
    ThreadUtil::GetInstance().RemoveTask(THREAD_ID_HFP, "TASK_PROCESS_DEALYED_ACTIVE_STATE");
}
}  // namespace bluetooth
}  // namespace OHOS
