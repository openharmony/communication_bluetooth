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
const char *BLUETOOTH_HALF_APP_REGISTERED_OWNER_PROPERTY = "persist.bluetooth.half_app_registered_owner";
const char *SWITCH_STATE_VALUE_OFF = "0";
const char *SWITCH_STATE_VALUE_ON = "1";
const char *SWITCH_STATE_VALUE_HALF = "2";
const char *SWITCH_STATE_VALUE_HALF_APP_REGISTERED = "3";
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
        case BluetoothSwitchState::STATE_HALF_APP_REGISTERED:
            return SWITCH_STATE_VALUE_HALF_APP_REGISTERED;
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
    if (value == SWITCH_STATE_VALUE_HALF_APP_REGISTERED) {
        return BluetoothSwitchState::STATE_HALF_APP_REGISTERED;
    }
    return BluetoothSwitchState::STATE_OFF;
}

void BluetoothSwitchStateMachine::InitFromProperty()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    char value[PROPERTY_VALUE_MAX_LEN] = {0};
    GetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, "0", value, PROPERTY_VALUE_MAX_LEN - 1);
    switchState_ = static_cast<BluetoothSwitchState>(std::atoi(value));
    if (switchState_ == BluetoothSwitchState::STATE_HALF_APP_REGISTERED) {
        char owner[PROPERTY_VALUE_MAX_LEN] = {0};
        GetParameter(BLUETOOTH_HALF_APP_REGISTERED_OWNER_PROPERTY, "", owner, PROPERTY_VALUE_MAX_LEN - 1);
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

bool BluetoothSwitchStateMachine::IsHalfAppRegisteredMode() const
{
    return GetSwitchState() == BluetoothSwitchState::STATE_HALF_APP_REGISTERED;
}

bool BluetoothSwitchStateMachine::IsTransitionValid(BluetoothSwitchState from, BluetoothSwitchState to) const
{
    // Linear model: enable flows OFF -> ON/HALF/HALF_APP_REGISTERED; the two restricted
    // states can only upgrade to ON or shut down to OFF, and HALF_APP_REGISTERED (both
    // stacks already up) may additionally degrade to HALF. Degradation from ON and
    // HALF -> HALF_APP_REGISTERED are forbidden.
    static const std::pair<BluetoothSwitchState, BluetoothSwitchState> validTransitions[] = {
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_HALF},
        {BluetoothSwitchState::STATE_OFF, BluetoothSwitchState::STATE_HALF_APP_REGISTERED},
        {BluetoothSwitchState::STATE_ON, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_HALF, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_HALF, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_HALF_APP_REGISTERED, BluetoothSwitchState::STATE_ON},
        {BluetoothSwitchState::STATE_HALF_APP_REGISTERED, BluetoothSwitchState::STATE_OFF},
        {BluetoothSwitchState::STATE_HALF_APP_REGISTERED, BluetoothSwitchState::STATE_HALF},
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
    if (state != BluetoothSwitchState::STATE_HALF_APP_REGISTERED) {
        SetParameter(BLUETOOTH_HALF_APP_REGISTERED_OWNER_PROPERTY, "");
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
    if (target != BluetoothSwitchState::STATE_HALF_APP_REGISTERED) {
        ownerName_ = "";
    }
    PersistState(target);
    return BT_NO_ERROR;
}

int32_t BluetoothSwitchStateMachine::EnterHalfAppRegisteredMode(const std::string &ownerName)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (switchState_ == BluetoothSwitchState::STATE_HALF_APP_REGISTERED) {
        // idempotent: re-entering only refreshes the owner
        ownerName_ = ownerName;
        SetParameter(BLUETOOTH_HALF_APP_REGISTERED_OWNER_PROPERTY, ownerName.c_str());
        return BT_NO_ERROR;
    }
    if (!IsTransitionValid(switchState_, BluetoothSwitchState::STATE_HALF_APP_REGISTERED)) {
        HILOGE("invalid switch state transition %{public}d -> HALF_APP_REGISTERED, owner(%{public}s)",
            switchState_, ownerName.c_str());
        return BT_ERR_INVALID_STATE;
    }
    HILOGI("switch state transition %{public}d -> HALF_APP_REGISTERED, owner(%{public}s)", switchState_, ownerName.c_str());
    switchState_ = BluetoothSwitchState::STATE_HALF_APP_REGISTERED;
    ownerName_ = ownerName;
    SetParameter(BLUETOOTH_SWITCH_STATE_PROPERTY, SWITCH_STATE_VALUE_HALF_APP_REGISTERED);
    SetParameter(BLUETOOTH_HALF_APP_REGISTERED_OWNER_PROPERTY, ownerName.c_str());
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
    if (state != BluetoothSwitchState::STATE_HALF_APP_REGISTERED) {
        ownerName_ = "";
    }
}

bool BluetoothSwitchStateMachine::IsOwnerAccessible(const std::string &callingName) const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (switchState_ != BluetoothSwitchState::STATE_HALF_APP_REGISTERED) {
        return true;
    }
    return !callingName.empty() && callingName == ownerName_;
}

bool BluetoothSwitchStateMachine::IsBrAllowed() const
{
    return !IsHalfAppRegisteredMode();
}

std::string BluetoothSwitchStateMachine::GetOwnerName() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return ownerName_;
}

}  // namespace bluetooth
}  // namespace OHOS
