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
#include <set>
#include <string>
#include "base_def.h"
#include "bt_def.h"
#include "btcommon/state_machine.h"

namespace OHOS {
namespace bluetooth {

/**
 * @brief Four-state bluetooth switch state machine, built on the common
 * utility::StateMachine mechanism (states dispatch switch messages and
 * drive Entry/Exit, all transitions flow through messages).
 *
 * States (persisted in "persist.bluetooth.switch_enable"):
 *   STATE_ON             "1"  both stacks on, all functions available
 *   STATE_OFF            "0"  both stacks off
 *   STATE_HALF           "2"  restricted half-open: BLE on, BR hidden
 *   STATE_BLE_OWNER_ONLY "3"  both stacks on underneath; BLE visible/usable
 *                             only by the registered owner pids; BR refused
 *                             for everyone. Owner pids live in memory only
 *                             (cleared on reboot / on leaving this state).
 *
 * Legal transitions:
 *   OFF -> ON / HALF / BLE_OWNER_ONLY;  ON -> HALF (watch legacy) / OFF;
 *   HALF -> ON / OFF;  BLE_OWNER_ONLY -> ON / OFF / HALF.
 */
class BluetoothSwitchStateMachine {
public:
    // switch state machine message kinds
    enum SwitchStateMessage {
        MSG_SWITCH_ENABLE_ON = 1,     // -> STATE_ON
        MSG_SWITCH_ENABLE_HALF,       // -> STATE_HALF
        MSG_SWITCH_ENTER_OWNER_ONLY,  // -> STATE_BLE_OWNER_ONLY (arg1_ = owner pid)
        MSG_SWITCH_DISABLE,           // -> STATE_OFF, memory only (unload flow persists "0")
        MSG_SWITCH_INIT,              // restore, arg1_ = BluetoothSwitchState
    };

    // state names registered in the utility state machine
    static const std::string SWITCH_STATE_OFF_NAME;
    static const std::string SWITCH_STATE_ON_NAME;
    static const std::string SWITCH_STATE_HALF_NAME;
    static const std::string SWITCH_STATE_OWNER_ONLY_NAME;

    static BluetoothSwitchStateMachine &GetInstance();

    /**
     * @brief Restore the switch state from the persisted property, called on reboot.
     * Owner pids are NOT restored (a pid is meaningless after reboot); a restored
     * BLE_OWNER_ONLY state therefore denies BLE for everyone until an app
     * re-enters the mode (fail-safe).
     */
    void InitFromProperty();

    BluetoothSwitchState GetSwitchState() const;
    bool IsBluetoothRestricted() const;
    bool IsBleOwnerOnlyMode() const;

    /**
     * @brief Request a transition by message; legality is checked by the
     * current state's dispatch, leaving BLE_OWNER_ONLY clears the owner set.
     * STATE_BLE_OWNER_ONLY is not a valid target here - use TryEnterBleOwnerOnlyMode.
     */
    int32_t TransitionTo(BluetoothSwitchState target);

    /**
     * @brief Atomically enter BLE_OWNER_ONLY mode for the caller pid.
     * From OFF: establish the mode with the first owner; while already in the
     * mode: append the pid (multiple owners supported, capped at MAX_OWNERS);
     * from any other state: rejected.
     */
    int32_t TryEnterBleOwnerOnlyMode(int32_t pid);

    /**
     * @brief Move to STATE_OFF in memory only (disable path; the persisted
     * property is written to "0" by the stack unload flow). Clears owners.
     */
    void SyncState(BluetoothSwitchState state);

    /**
     * @brief Resolve which V2 switch event a BR STATE_TURN_ON callback should
     * report, snapshotted under the lock: STATE_ON normally, STATE_HALF in the
     * restricted half mode, STATE_BLE_OWNER_ONLY in the owner-only mode.
     */
    BluetoothSwitchState ResolveBrOnEvent() const;

    /**
     * @brief Whether the caller pid belongs to the owner set.
     * Only meaningful in BLE_OWNER_ONLY mode; other modes always allow.
     */
    bool IsOwnerAccessible(int32_t pid) const;

    /**
     * @brief Whether BR functions (discovery/pairing/connection) are allowed.
     * Only forbidden in BLE_OWNER_ONLY mode.
     */
    bool IsBrAllowed() const;

    std::set<int32_t> GetOwnerPids() const;

    static const char *SwitchStateToPropertyValue(BluetoothSwitchState state);
    static int PropertyValueToSwitchState(const std::string &value);

    static constexpr int32_t MAX_OWNERS = 8;
    static constexpr int32_t INVALID_OWNER_PID = -1;

private:
    BluetoothSwitchStateMachine();

    // entry hook invoked by each state's Entry(); clears the owner set when
    // leaving BLE_OWNER_ONLY and persists the property when requested
    void OnStateEntered(BluetoothSwitchState state, bool persist);

    // message helpers, called with stateMutex_ held
    int32_t DispatchMessage(const utility::Message &msg);
    std::string StateName(BluetoothSwitchState state) const;
    BluetoothSwitchState CurrentStateLocked() const;

    mutable std::mutex stateMutex_ {};
    utility::StateMachine machine_;
    bool persistOnEntry_ {true};
    int32_t pendingOwnerPid_ {INVALID_OWNER_PID};
    std::set<int32_t> ownerPids_ {};

    friend class SwitchOffState;
    friend class SwitchOnState;
    friend class SwitchHalfState;
    friend class SwitchOwnerOnlyState;
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_SWITCH_STATE_MACHINE_H
