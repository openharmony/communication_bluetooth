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
const char *BLUETOOTH_BLE_ONLY_OWNER_PROPERTY = "persist.bluetooth.ble_only_owner";
const char *SWITCH_STATE_VALUE_OFF = "0";
const char *SWITCH_STATE_VALUE_ON = "1";
const char *SWITCH_STATE_VALUE_HALF = "2";
const char *SWITCH_STATE_VALUE_BLE_ONLY = "3";
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
        case BluetoothSwitchState::STATE_BLE_ONLY:
            return SWITCH_STATE_VALUE_BLE_ONLY;
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
    if (value == SWITCH_STATE_VALUE_BLE_ONLY) {
        return BluetoothSwitchState::STATE_BLE_ONLY;
    }
    return BluetoothSwitchState::STATE_OFF;
}

void BluetoothSwitchStateMachine::InitFromProperty()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    char value[PROPERTY_VALUE_MAX_LEN] = {0};
    GetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, "0", value, PROPERTY_VALUE_MAX_LEN - 1);
    switchState_ = static_cast<BluetoothSwitchState>(std::atoi(value));
    if (switchState_ == BluetoothSwitchState::STATE_BLE_ONLY) {
        char owner[PROPERTY_VALUE_MAX_LEN] = {0};
        GetParameter(BLUETOOTH_BLE_ONLY_OWNER_PROPERTY, "", owner, PROPERTY_VALUE_MAX_LEN - 1);
        ownerName_ = owner;
    } else {
        ownerName_ = "";
    }
    HILOGI("restore switch state %{public}d, owner empty=%{public}d", switchState_, ownerName_.empty());
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

bool BluetoothSwitchStateMachine::IsBleOnlyMode() const
{
    return GetSwitchState() == BluetoothSwitchState::STATE_BLE_ONLY;
}

bool BluetoothSwitchStateMachine::IsTransitionValid(BluetoothSwitchState from, BluetoothSwitchState to) const
{
    // Any state can go to OFF (disable) or ON (full enable, includes half/full -> on);
    // ON can fall back to HALF or BLE_ONLY without restarting the stacks;
    // HALF and BLE_ONLY can switch between each other directly.
    static const std::pair<BluetoothSwitchState, BluetoothSwitchState> validTransitions[] = {
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_HALF},
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_BLE_ONLY},
        {BluetoothSwitchState::STATE_ON, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_ON, BluetoothSwitchState::STATE_HALF},
        {BluetoothSwitchState::STATE_ON, BluetoothSwitchState::STATE_BLE_ONLY},
        {BluetoothSwitchState::STATE_HALF, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_HALF, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_HALF, BluetoothSwitchState::STATE_BLE_ONLY},
        {BluetoothSwitchState::STATE_BLE_ONLY, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_BLE_ONLY, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_BLE_ONLY, BluetoothSwitchState::STATE_HALF},
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
    if (state != BluetoothSwitchState::STATE_BLE_ONLY) {
        SetParameter(BLUETOOTH_BLE_ONLY_OWNER_PROPERTY, "");
    }
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
    if (target != BluetoothSwitchState::STATE_BLE_ONLY) {
        ownerName_ = "";
    }
    PersistState(target);
    return BT_NO_ERROR;
}

int32_t BluetoothSwitchStateMachine::EnterBleOnlyMode(const std::string &ownerName)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!IsTransitionValid(switchState_, BluetoothSwitchState::STATE_BLE_ONLY)) {
        HILOGE("invalid switch state transition %{public}d -> BLE_ONLY, owner(%{public}s)",
            switchState_, ownerName.c_str());
        return BT_ERR_INVALID_STATE;
    }
    HILOGI("switch state transition %{public}d -> BLE_ONLY, owner(%{public}s)", switchState_, ownerName.c_str());
    switchState_ = BluetoothSwitchState::STATE_BLE_ONLY;
    ownerName_ = ownerName;
    SetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, SWITCH_STATE_VALUE_BLE_ONLY);
    SetParameter(BLUETOOTH_BLE_ONLY_OWNER_PROPERTY, ownerName.c_str());
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
    if (state != BluetoothSwitchState::STATE_BLE_ONLY) {
        ownerName_ = "";
    }
}

bool BluetoothSwitchStateMachine::IsOwnerAccessible(const std::string &callingName) const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (switchState_ != BluetoothSwitchState::STATE_BLE_ONLY) {
        return true;
    }
    return !callingName.empty() && callingName == ownerName_;
}

bool BluetoothSwitchStateMachine::IsBrAllowed() const
{
    return !IsBleOnlyMode();
}

std::string BluetoothSwitchStateMachine::GetOwnerName() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return ownerName_;
}

}  // namespace bluetooth
}  // namespace OHOS
