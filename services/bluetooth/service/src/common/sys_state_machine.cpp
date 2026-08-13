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
#define LOG_TAG "bt_service_sys_state"
#endif

#include "sys_state_machine.h"
#include "adapter_manager.h"
#include "log.h"
#include "parameter.h"

namespace OHOS {
namespace bluetooth {
const char *g_bluetoothWatchStrModeProperty = "persist.bluetooth.watch_str_enable";

void SysStateMachine::Init(AdapterManager &am)
{
    std::unique_ptr<utility::StateMachine::State> stopped = std::make_unique<SysStoppedState>(*this, am);
    Move(stopped);
    std::unique_ptr<utility::StateMachine::State> started = std::make_unique<SysStartedState>(*this, am);
    Move(started);
    std::unique_ptr<utility::StateMachine::State> stopping = std::make_unique<SysStoppingState>(*this, am);
    Move(stopping);
    std::unique_ptr<utility::StateMachine::State> resetting = std::make_unique<SysResettingState>(*this, am);
    Move(resetting);
    std::unique_ptr<utility::StateMachine::State> factoryResetting =
        std::make_unique<SysFactoryResettingState>(*this, am);
    Move(factoryResetting);
    InitState(SYS_STATE_STOPPED);
}

void SysStoppingBaseState::StoppingBaseProcess(int stateBREDR, int stateBLE)
{
    if ((stateBREDR == BTStateID::STATE_TURN_OFF) && (stateBLE == BTStateID::STATE_TURN_OFF)) {
        HILOGI("all off");
        am_.AdapterStop();
    } else if ((stateBREDR == BTStateID::STATE_TURNING_ON) || (stateBLE == BTStateID::STATE_TURNING_ON) ||
               (stateBREDR == BTStateID::STATE_TURNING_OFF) || (stateBLE == BTStateID::STATE_TURNING_OFF)) {
        // wait turn on/turn off
        HILOGI("at least one is *ing");
    } else {
        int lastBtState = GetIntParameter(g_bluetoothWatchStrModeProperty, 0);
        if (lastBtState > 0) {
            HILOGI("don't do anything");
            return;
        } else {
            am_.Disable(BTTransport::ADAPTER_BREDR);
        }
    }
}

void SysStoppedState::Entry()
{
    HILOGI("Entry");
}

bool SysStoppedState::Dispatch(const utility::Message &msg)
{
    HILOGI("Dispatch %{public}d", msg.what_);
    switch (msg.what_) {
        case SysStateMachine::MSG_SYS_START_CMP:
            Transition(SYS_STATE_STARTED);
            return true;
        default:
            return false;
    }
}

void SysStartedState::Entry()
{
    HILOGI("Entry");
    am_.OnSysStateChange(SYS_STATE_STARTED);
}

bool SysStartedState::Dispatch(const utility::Message &msg)
{
    HILOGD("Dispatch %{public}d", msg.what_);
    switch (msg.what_) {
        case SysStateMachine::MSG_SYS_RESET_REQ:
            Transition(SYS_STATE_RESETTING);
            break;
        case SysStateMachine::MSG_SYS_FACTORY_RESET_REQ:
            Transition(SYS_STATE_FRESETTING);
            break;
        case SysStateMachine::MSG_SYS_STOP_REQ:
            Transition(SYS_STATE_STOPPING);
            break;
        default:
            return false;
    }
    return true;
}

void SysStoppingState::Entry()
{
    HILOGI("Entry");

    int stateBREDR = am_.GetState(BTTransport::ADAPTER_BREDR);
    int stateBLE = am_.GetState(BTTransport::ADAPTER_BLE);

    am_.OnSysStateChange(SYS_STATE_STOPPING);
    StoppingBaseProcess(stateBREDR, stateBLE);
}

void SysStoppingState::Exit()
{
    am_.OnSysStateExit(SYS_STATE_STOPPING);
}

bool SysStoppingState::Dispatch(const utility::Message &msg)
{
    HILOGI("Dispatch %{public}d", msg.what_);
    int stateBREDR = BTStateID::STATE_TURN_OFF;
    int stateBLE = BTStateID::STATE_TURN_OFF;
    switch (msg.what_) {
        case SysStateMachine::MSG_SYS_STOP_CMP:
            am_.OnSysStateChange(SYS_STATE_STOPPED);
            Transition(SYS_STATE_STOPPED);
            return true;
        case SysStateMachine::MSG_SYS_ADAPTER_STATE_CHANGE_REQ:
            stateBREDR = (static_cast<unsigned int>(msg.arg1_) & 0xF0) >> CLASSIC_ENABLE_STATE_BIT;
            stateBLE = static_cast<unsigned int>(msg.arg1_) & 0x0F;
            StoppingBaseProcess(stateBREDR, stateBLE);
            return true;
        default:
            return false;
    }
}

void SysResettingState::Entry()
{
    HILOGI("Entry");

    int stateBREDR = am_.GetState(BTTransport::ADAPTER_BREDR);
    int stateBLE = am_.GetState(BTTransport::ADAPTER_BLE);

    am_.OnSysStateChange(SYS_STATE_RESETTING);
    StoppingBaseProcess(stateBREDR, stateBLE);
}

bool SysResettingState::Dispatch(const utility::Message &msg)
{
    HILOGI("Dispatch %{public}d %{public}d", msg.what_, msg.arg1_);
    int stateBREDR = BTStateID::STATE_TURN_OFF;
    int stateBLE = BTStateID::STATE_TURN_OFF;
    switch (msg.what_) {
        case SysStateMachine::MSG_SYS_ADAPTER_STATE_CHANGE_REQ:
            stateBREDR = (static_cast<unsigned int>(msg.arg1_) & 0xF0) >> CLASSIC_ENABLE_STATE_BIT;
            stateBLE = static_cast<unsigned int>(msg.arg1_) & 0x0F;
            StoppingBaseProcess(stateBREDR, stateBLE);
            break;
        case SysStateMachine::MSG_SYS_STOP_CMP:
            am_.Start();
            Transition(SYS_STATE_STARTED);
            break;
        case SysStateMachine::MSG_SYS_STOP_REQ:
            Transition(SYS_STATE_STOPPING);
            break;
        default:
            return false;
    }
    return true;
}

// SysResettingState
void SysFactoryResettingState::Entry()
{
    HILOGI("Entry");

    int stateBREDR = am_.GetState(BTTransport::ADAPTER_BREDR);
    int stateBLE = am_.GetState(BTTransport::ADAPTER_BLE);

    am_.OnSysStateChange(SYS_STATE_FRESETTING);
    StoppingBaseProcess(stateBREDR, stateBLE);
}

void SysFactoryResettingState::Exit()
{
    am_.OnSysStateExit(SYS_STATE_FRESETTING);
}

bool SysFactoryResettingState::Dispatch(const utility::Message &msg)
{
    HILOGI("Dispatch %{public}d", msg.what_);
    int stateBREDR = BTStateID::STATE_TURN_OFF;
    int stateBLE = BTStateID::STATE_TURN_OFF;
    switch (msg.what_) {
        case SysStateMachine::MSG_SYS_ADAPTER_STATE_CHANGE_REQ:
            stateBREDR = (static_cast<unsigned int>(msg.arg1_) & 0xF0) >> CLASSIC_ENABLE_STATE_BIT;
            stateBLE = static_cast<unsigned int>(msg.arg1_) & 0x0F;
            StoppingBaseProcess(stateBREDR, stateBLE);
            break;
        case SysStateMachine::MSG_SYS_STOP_CMP:
            am_.ClearAllStorage();
            break;
        case SysStateMachine::MSG_SYS_STOP_REQ:
            Transition(SYS_STATE_STOPPING);
            break;
        case SysStateMachine::MSG_SYS_CLEAR_ALL_STORAGE_CMP:
            am_.Start();
            Transition(SYS_STATE_STARTED);
            break;
        default:
            return false;
    }
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS