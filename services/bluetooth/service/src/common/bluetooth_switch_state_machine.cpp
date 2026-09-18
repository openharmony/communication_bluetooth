/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_switch_state_machine"
#endif

#include "bluetooth_switch_state_machine.h"

#include <cstdlib>
#include "bluetooth_errorcode.h"
#include "log.h"
#include "parameter.h"

namespace OHOS {
namespace bluetooth {
namespace {
const int32_t PROPERTY_VALUE_MAX_LEN = 128;
const char *BLUETOOTH_SWITCH_STATE_PROPERTY = "persist.bluetooth.switch_enable";
const char *SWITCH_STATE_VALUE_OFF = "0";
const char *SWITCH_STATE_VALUE_ON = "1";
const char *SWITCH_STATE_VALUE_HALF = "2";
const char *SWITCH_STATE_VALUE_BLE_OWNER_ONLY = "3";
}  // namespace

const std::string BluetoothSwitchStateMachine::SWITCH_STATE_OFF_NAME = "SwitchOff";
const std::string BluetoothSwitchStateMachine::SWITCH_STATE_ON_NAME = "SwitchOn";
const std::string BluetoothSwitchStateMachine::SWITCH_STATE_HALF_NAME = "SwitchHalf";
const std::string BluetoothSwitchStateMachine::SWITCH_STATE_OWNER_ONLY_NAME = "SwitchOwnerOnly";

// State classes live in OHOS::bluetooth (not an anonymous namespace) so they
// match the friend declarations in the header and may drive the controller.
// common base: dispatch maps accepted messages to state transitions, entry
// hooks persist the property and manage the owner set via the controller
class SwitchStateBase : public utility::StateMachine::State {
public:
    SwitchStateBase(const std::string &name, utility::StateMachine &machine,
        BluetoothSwitchStateMachine &owner, BluetoothSwitchState state)
        : State(name, machine), owner_(owner), state_(state) {}
    ~SwitchStateBase() override = default;

    void Entry() override
    {
        owner_.OnStateEntered(state_, owner_.persistOnEntry_);
    }
    void Exit() override {}

protected:
    BluetoothSwitchStateMachine &owner_;
    BluetoothSwitchState state_;
};

class SwitchOffState final : public SwitchStateBase {
public:
    using SwitchStateBase::SwitchStateBase;

private:
    bool Dispatch(const utility::Message &msg) override
    {
        switch (msg.what_) {
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENABLE_ON:
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_ON_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENABLE_HALF:
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_HALF_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENTER_OWNER_ONLY:
                owner_.pendingOwnerPid_ = msg.arg1_;
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_OWNER_ONLY_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_INIT:
                if (static_cast<BluetoothSwitchState>(msg.arg1_) == BluetoothSwitchState::STATE_OFF) {
                    return true;
                }
                owner_.pendingOwnerPid_ = BluetoothSwitchStateMachine::INVALID_OWNER_PID;
                switch (static_cast<BluetoothSwitchState>(msg.arg1_)) {
                    case BluetoothSwitchState::STATE_ON:
                        Transition(BluetoothSwitchStateMachine::SWITCH_STATE_ON_NAME);
                        return true;
                    case BluetoothSwitchState::STATE_HALF:
                        Transition(BluetoothSwitchStateMachine::SWITCH_STATE_HALF_NAME);
                        return true;
                    case BluetoothSwitchState::STATE_BLE_OWNER_ONLY:
                        Transition(BluetoothSwitchStateMachine::SWITCH_STATE_OWNER_ONLY_NAME);
                        return true;
                    default:
                        return false;
                }
            default:
                return false;
        }
    }
};

class SwitchOnState final : public SwitchStateBase {
public:
    using SwitchStateBase::SwitchStateBase;

private:
    bool Dispatch(const utility::Message &msg) override
    {
        switch (msg.what_) {
            // watch legacy path: degrade to half without a stack restart
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENABLE_HALF:
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_HALF_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_DISABLE:
                owner_.persistOnEntry_ = false;
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_OFF_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENABLE_ON:
                return true;  // already on, no-op
            default:
                return false;
        }
    }
};

class SwitchHalfState final : public SwitchStateBase {
public:
    using SwitchStateBase::SwitchStateBase;

private:
    bool Dispatch(const utility::Message &msg) override
    {
        switch (msg.what_) {
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENABLE_ON:
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_ON_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_DISABLE:
                owner_.persistOnEntry_ = false;
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_OFF_NAME);
                return true;
            default:
                return false;
        }
    }
};

class SwitchOwnerOnlyState final : public SwitchStateBase {
public:
    using SwitchStateBase::SwitchStateBase;

private:
    bool Dispatch(const utility::Message &msg) override
    {
        switch (msg.what_) {
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENABLE_ON:
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_ON_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENABLE_HALF:
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_HALF_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_DISABLE:
                owner_.persistOnEntry_ = false;
                Transition(BluetoothSwitchStateMachine::SWITCH_STATE_OFF_NAME);
                return true;
            case BluetoothSwitchStateMachine::MSG_SWITCH_ENTER_OWNER_ONLY:
                // multiple owners supported: append the pid in place
                if (static_cast<int32_t>(owner_.ownerPids_.size()) >=
                    BluetoothSwitchStateMachine::MAX_OWNERS) {
                    HILOGE("owner set full, reject pid %{public}d", msg.arg1_);
                    return false;
                }
                owner_.ownerPids_.insert(msg.arg1_);
                HILOGI("append owner pid %{public}d, size=%{public}zu", msg.arg1_, owner_.ownerPids_.size());
                return true;
            default:
                return false;
        }
    }
};

BluetoothSwitchStateMachine &BluetoothSwitchStateMachine::GetInstance()
{
    static BluetoothSwitchStateMachine instance;
    return instance;
}

BluetoothSwitchStateMachine::BluetoothSwitchStateMachine()
{
    std::unique_ptr<utility::StateMachine::State> offState =
        std::make_unique<SwitchOffState>(SWITCH_STATE_OFF_NAME, machine_, *this, BluetoothSwitchState::STATE_OFF);
    std::unique_ptr<utility::StateMachine::State> onState =
        std::make_unique<SwitchOnState>(SWITCH_STATE_ON_NAME, machine_, *this, BluetoothSwitchState::STATE_ON);
    std::unique_ptr<utility::StateMachine::State> halfState =
        std::make_unique<SwitchHalfState>(SWITCH_STATE_HALF_NAME, machine_, *this, BluetoothSwitchState::STATE_HALF);
    std::unique_ptr<utility::StateMachine::State> ownerOnlyState = std::make_unique<SwitchOwnerOnlyState>(
        SWITCH_STATE_OWNER_ONLY_NAME, machine_, *this, BluetoothSwitchState::STATE_BLE_OWNER_ONLY);
    machine_.Move(offState);
    machine_.Move(onState);
    machine_.Move(halfState);
    machine_.Move(ownerOnlyState);
    // InitState triggers the initial state's Entry(): keep it from touching
    // the persisted property, InitFromProperty() restores the real state
    persistOnEntry_ = false;
    machine_.InitState(SWITCH_STATE_OFF_NAME);
    persistOnEntry_ = true;
}

const char *BluetoothSwitchStateMachine::SwitchStateToPropertyValue(BluetoothSwitchState state)
{
    switch (state) {
        case BluetoothSwitchState::STATE_ON:
            return SWITCH_STATE_VALUE_ON;
        case BluetoothSwitchState::STATE_HALF:
            return SWITCH_STATE_VALUE_HALF;
        case BluetoothSwitchState::STATE_BLE_OWNER_ONLY:
            return SWITCH_STATE_VALUE_BLE_OWNER_ONLY;
        case BluetoothSwitchState::STATE_OFF:
        default:
            return SWITCH_STATE_VALUE_OFF;
    }
}

int BluetoothSwitchStateMachine::PropertyValueToSwitchState(const std::string &value)
{
    if (value == SWITCH_STATE_VALUE_ON) {
        return BluetoothSwitchState::STATE_ON;
    }
    if (value == SWITCH_STATE_VALUE_HALF) {
        return BluetoothSwitchState::STATE_HALF;
    }
    if (value == SWITCH_STATE_VALUE_BLE_OWNER_ONLY) {
        return BluetoothSwitchState::STATE_BLE_OWNER_ONLY;
    }
    return BluetoothSwitchState::STATE_OFF;
}

void BluetoothSwitchStateMachine::OnStateEntered(BluetoothSwitchState state, bool persist)
{
    if (state == BluetoothSwitchState::STATE_BLE_OWNER_ONLY) {
        ownerPids_.clear();
        if (pendingOwnerPid_ != INVALID_OWNER_PID) {
            ownerPids_.insert(pendingOwnerPid_);
        }
        pendingOwnerPid_ = INVALID_OWNER_PID;
        persistOnEntry_ = true;
        if (persist) {
            SetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, SWITCH_STATE_VALUE_BLE_OWNER_ONLY);
        }
        return;
    }
    // leaving the owner-only mode drops every owner identity
    ownerPids_.clear();
    pendingOwnerPid_ = INVALID_OWNER_PID;
    persistOnEntry_ = true;
    if (persist) {
        SetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, SwitchStateToPropertyValue(state));
    }
}

int32_t BluetoothSwitchStateMachine::DispatchMessage(const utility::Message &msg)
{
    if (machine_.ProcessMessage(msg)) {
        return BT_NO_ERROR;
    }
    HILOGE("switch state %{public}d rejects message %{public}d", CurrentStateLocked(), msg.what_);
    return BT_ERR_INVALID_STATE;
}

BluetoothSwitchState BluetoothSwitchStateMachine::CurrentStateLocked() const
{
    const auto *state = machine_.GetState();
    if (state == nullptr) {
        return BluetoothSwitchState::STATE_OFF;
    }
    if (state->Name() == SWITCH_STATE_ON_NAME) {
        return BluetoothSwitchState::STATE_ON;
    }
    if (state->Name() == SWITCH_STATE_HALF_NAME) {
        return BluetoothSwitchState::STATE_HALF;
    }
    if (state->Name() == SWITCH_STATE_OWNER_ONLY_NAME) {
        return BluetoothSwitchState::STATE_BLE_OWNER_ONLY;
    }
    return BluetoothSwitchState::STATE_OFF;
}

void BluetoothSwitchStateMachine::InitFromProperty()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    char value[PROPERTY_VALUE_MAX_LEN] = {0};
    GetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, "0", value, PROPERTY_VALUE_MAX_LEN - 1);
    // pids do not survive a reboot: a restored BLE_OWNER_ONLY denies BLE for
    // everyone until some app re-enters the mode (fail-safe)
    ownerPids_.clear();
    pendingOwnerPid_ = INVALID_OWNER_PID;
    utility::Message msg(MSG_SWITCH_INIT, PropertyValueToSwitchState(value));
    (void)DispatchMessage(msg);
    HILOGI("restore switch state %{public}d", CurrentStateLocked());
}

BluetoothSwitchState BluetoothSwitchStateMachine::GetSwitchState() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return CurrentStateLocked();
}

bool BluetoothSwitchStateMachine::IsBluetoothRestricted() const
{
    return GetSwitchState() == BluetoothSwitchState::STATE_HALF;
}

bool BluetoothSwitchStateMachine::IsBleOwnerOnlyMode() const
{
    return GetSwitchState() == BluetoothSwitchState::STATE_BLE_OWNER_ONLY;
}

int32_t BluetoothSwitchStateMachine::TransitionTo(BluetoothSwitchState target)
{
    if (target == BluetoothSwitchState::STATE_BLE_OWNER_ONLY) {
        // owner-only mode must be entered with an owner pid, use TryEnterBleOwnerOnlyMode
        HILOGE("TransitionTo(BLE_OWNER_ONLY) is not supported");
        return BT_ERR_INVALID_PARAM;
    }
    std::lock_guard<std::mutex> lock(stateMutex_);
    persistOnEntry_ = true;
    utility::Message msg(target == BluetoothSwitchState::STATE_ON ? MSG_SWITCH_ENABLE_ON
                                                                  : MSG_SWITCH_ENABLE_HALF);
    return DispatchMessage(msg);
}

int32_t BluetoothSwitchStateMachine::TryEnterBleOwnerOnlyMode(int32_t pid)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    pendingOwnerPid_ = pid;
    utility::Message msg(MSG_SWITCH_ENTER_OWNER_ONLY, pid);
    return DispatchMessage(msg);
}

void BluetoothSwitchStateMachine::SyncState(BluetoothSwitchState state)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (CurrentStateLocked() == state) {
        return;
    }
    if (state == BluetoothSwitchState::STATE_OFF) {
        // disable path: memory only, the unload flow persists "0"
        persistOnEntry_ = false;
        utility::Message msg(MSG_SWITCH_DISABLE);
        (void)DispatchMessage(msg);
        return;
    }
    // other sync targets fall back to a persisted transition
    // (dispatch directly: the mutex is already held)
    persistOnEntry_ = true;
    utility::Message msg(state == BluetoothSwitchState::STATE_ON ? MSG_SWITCH_ENABLE_ON
                                                                  : MSG_SWITCH_ENABLE_HALF);
    (void)DispatchMessage(msg);
}

BluetoothSwitchState BluetoothSwitchStateMachine::ResolveBrOnEvent() const
{
    return GetSwitchState();
}

bool BluetoothSwitchStateMachine::IsOwnerAccessible(int32_t pid) const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (CurrentStateLocked() != BluetoothSwitchState::STATE_BLE_OWNER_ONLY) {
        return true;
    }
    return ownerPids_.find(pid) != ownerPids_.end();
}

bool BluetoothSwitchStateMachine::IsBrAllowed() const
{
    return !IsBleOwnerOnlyMode();
}

std::set<int32_t> BluetoothSwitchStateMachine::GetOwnerPids() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return ownerPids_;
}

}  // namespace bluetooth
}  // namespace OHOS
