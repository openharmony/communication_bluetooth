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

BluetoothSwitchStateMachine &BluetoothSwitchStateMachine::GetInstance()
{
    static BluetoothSwitchStateMachine instance;
    return instance;
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

void BluetoothSwitchStateMachine::InitFromProperty()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    char value[PROPERTY_VALUE_MAX_LEN] = {0};
    GetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, "0", value, PROPERTY_VALUE_MAX_LEN - 1);
    switchState_ = static_cast<BluetoothSwitchState>(std::atoi(value));
    // pids do not survive a reboot: a restored BLE_OWNER_ONLY denies BLE for
    // everyone until some app re-enters the mode (fail-safe)
    ownerPids_.clear();
    HILOGI("restore switch state %{public}d, owners cleared", switchState_);
}

BluetoothSwitchState BluetoothSwitchStateMachine::GetSwitchState() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return switchState_;
}

bool BluetoothSwitchStateMachine::IsBluetoothRestricted() const
{
    return GetSwitchState() == BluetoothSwitchState::STATE_HALF;
}

bool BluetoothSwitchStateMachine::IsBleOwnerOnlyMode() const
{
    return GetSwitchState() == BluetoothSwitchState::STATE_BLE_OWNER_ONLY;
}

bool BluetoothSwitchStateMachine::IsTransitionValid(BluetoothSwitchState from, BluetoothSwitchState to) const
{
    // OFF fans out to the three enable targets; ON additionally degrades to HALF
    // (watch legacy path); the restricted states can only upgrade to ON, shut
    // down to OFF, or (owner-only, stacks already up) degrade to HALF.
    static const std::pair<BluetoothSwitchState, BluetoothSwitchState> validTransitions[] = {
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_HALF},
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_BLE_OWNER_ONLY},
        {BluetoothSwitchState::STATE_ON, BluetoothSwitchState::STATE_HALF},
        {BluetoothSwitchState::STATE_ON, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_HALF, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_HALF, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_BLE_OWNER_ONLY, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_BLE_OWNER_ONLY, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_BLE_OWNER_ONLY, BluetoothSwitchState::STATE_HALF},
    };
    for (const auto &transition : validTransitions) {
        if (transition.first == from && transition.second == to) {
            return true;
        }
    }
    return false;
}

void BluetoothSwitchStateMachine::PersistState(BluetoothSwitchState state) const
{
    SetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, SwitchStateToPropertyValue(state));
}

int32_t BluetoothSwitchStateMachine::TransitionTo(BluetoothSwitchState target)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (switchState_ == target) {
        return BT_NO_ERROR;
    }
    if (!IsTransitionValid(switchState_, target)) {
        HILOGE("invalid switch state transition %{public}d -> %{public}d", switchState_, target);
        return BT_ERR_INVALID_STATE;
    }
    HILOGI("switch state transition %{public}d -> %{public}d", switchState_, target);
    switchState_ = target;
    if (switchState_ != BluetoothSwitchState::STATE_BLE_OWNER_ONLY) {
        ownerPids_.clear();
    }
    PersistState(target);
    return BT_NO_ERROR;
}

int32_t BluetoothSwitchStateMachine::TryEnterBleOwnerOnlyMode(int32_t pid)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (switchState_ == BluetoothSwitchState::STATE_BLE_OWNER_ONLY) {
        // multiple owners supported: append the pid; no hijack risk because
        // every appended pid gets the same access
        if (static_cast<int32_t>(ownerPids_.size()) >= MAX_OWNERS) {
            HILOGE("owner set full (%{public}d), reject pid %{public}d",
                static_cast<int>(ownerPids_.size()), pid);
            return BT_ERR_INVALID_STATE;
        }
        ownerPids_.insert(pid);
        HILOGI("append owner pid %{public}d, size=%{public}zu", pid, ownerPids_.size());
        return BT_NO_ERROR;
    }
    if (switchState_ != BluetoothSwitchState::STATE_OFF) {
        HILOGE("state %{public}d cannot enter BLE_OWNER_ONLY, pid %{public}d", switchState_, pid);
        return BT_ERR_INVALID_STATE;
    }
    HILOGI("switch state transition OFF -> BLE_OWNER_ONLY, first owner pid %{public}d", pid);
    switchState_ = BluetoothSwitchState::STATE_BLE_OWNER_ONLY;
    ownerPids_.clear();
    ownerPids_.insert(pid);
    PersistState(switchState_);
    return BT_NO_ERROR;
}

void BluetoothSwitchStateMachine::SyncState(BluetoothSwitchState state)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (switchState_ == state) {
        return;
    }
    HILOGI("sync switch state %{public}d -> %{public}d", switchState_, state);
    switchState_ = state;
    ownerPids_.clear();
}

BluetoothSwitchState BluetoothSwitchStateMachine::ResolveBrOnEvent() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (switchState_ == BluetoothSwitchState::STATE_HALF) {
        return BluetoothSwitchState::STATE_HALF;
    }
    if (switchState_ == BluetoothSwitchState::STATE_BLE_OWNER_ONLY) {
        return BluetoothSwitchState::STATE_BLE_OWNER_ONLY;
    }
    return BluetoothSwitchState::STATE_ON;
}

bool BluetoothSwitchStateMachine::IsOwnerAccessible(int32_t pid) const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (switchState_ != BluetoothSwitchState::STATE_BLE_OWNER_ONLY) {
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
