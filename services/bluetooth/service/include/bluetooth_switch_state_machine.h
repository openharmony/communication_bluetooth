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

#ifndef BLUETOOTH_SWITCH_STATE_MACHINE_H
#define BLUETOOTH_SWITCH_STATE_MACHINE_H

#include <mutex>
#include <string>
#include "base_def.h"
#include "bt_def.h"

namespace OHOS {
namespace bluetooth {

/**
 * @brief Four-state bluetooth switch state machine.
 *
 * States (persisted in "persist.bluetooth.switch_enable"):
 *   STATE_ON       "1"  both BLE and BR stacks on, all functions available
 *   STATE_OFF      "0"  both stacks off
 *   STATE_HALF     "2"  restricted half-open: BLE on, BR stack hidden
 *   STATE_BLE_ONLY "3"  both stacks on underneath, but only BLE is allowed
 *                       and only the owner app (the app which enabled
 *                       bluetooth) can use BLE; BR functions are refused.
 */
class BluetoothSwitchStateMachine {
public:
    static BluetoothSwitchStateMachine &GetInstance();

    /**
     * @brief Restore the switch state from the persisted property, called on reboot.
     */
    void InitFromProperty();

    BluetoothSwitchState GetSwitchState() const;
    bool IsBluetoothRestricted() const;
    bool IsBleOnlyMode() const;

    /**
     * @brief Transition to the target state with legality check and persistence.
     */
    int32_t TransitionTo(BluetoothSwitchState target);

    /**
     * @brief Update the in-memory state without persistence or legality check.
     * Only for callers which persist/derive the state elsewhere (e.g. disable path).
     */
    void SyncState(BluetoothSwitchState state);

    /**
     * @brief Enter STATE_BLE_ONLY, recording the app which enabled bluetooth as owner.
     */
    int32_t EnterBleOnlyMode(const std::string &ownerName);

    /**
     * @brief Whether the caller matches the owner app recorded on entering
     * STATE_BLE_ONLY. Callers should combine this with their own
     * system-hap/native check for the full accessibility decision.
     * Other states always allow.
     */
    bool IsOwnerAccessible(const std::string &callingName) const;

    /**
     * @brief Whether BR functions (discovery/pairing/connection) are allowed.
     * Only forbidden in STATE_BLE_ONLY.
     */
    bool IsBrAllowed() const;

    std::string GetOwnerName() const;

    // map between switch state and persisted property value
    static const char *SwitchStateToPropertyValue(BluetoothSwitchState state);
    static int PropertyValueToSwitchState(const std::string &value);

private:
    BluetoothSwitchStateMachine() = default;
    ~BluetoothSwitchStateMachine() = default;
    BT_DISALLOW_COPY_AND_ASSIGN(BluetoothSwitchStateMachine);

    bool IsTransitionValid(BluetoothSwitchState from, BluetoothSwitchState to) const;
    void PersistState(BluetoothSwitchState state) const;

    mutable std::mutex stateMutex_ {};
    BluetoothSwitchState switchState_ = BluetoothSwitchState::STATE_OFF;
    std::string ownerName_ = "";
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_SWITCH_STATE_MACHINE_H
