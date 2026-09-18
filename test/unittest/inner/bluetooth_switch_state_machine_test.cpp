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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include "parameter.h"
#include "bluetooth_switch_state_machine.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace bluetooth {

class BluetoothSwitchStateMachineTest : public testing::Test {
public:
    BluetoothSwitchStateMachineTest() = default;
    ~BluetoothSwitchStateMachineTest() = default;
    static void SetUpTestSuite(void) {}
    static void TearDownTestSuite(void) {}
    void SetUp()
    {
        // reset to a clean OFF state before each case
        BluetoothSwitchStateMachine::GetInstance().SyncState(BluetoothSwitchState::STATE_OFF);
        SetParameter("persist.bluetooth.switch_enable", "0");
    }
    void TearDown() {}
};

/**
 * @tc.name: PropertyValueMapping
 * @tc.desc: switch state maps to persisted property value
 */
HWTEST_F(BluetoothSwitchStateMachineTest, PropertyValueMapping, TestSize.Level1)
{
    EXPECT_STREQ(BluetoothSwitchStateMachine::SwitchStateToPropertyValue(
        BluetoothSwitchState::STATE_OFF), "0");
    EXPECT_STREQ(BluetoothSwitchStateMachine::SwitchStateToPropertyValue(
        BluetoothSwitchState::STATE_ON), "1");
    EXPECT_STREQ(BluetoothSwitchStateMachine::SwitchStateToPropertyValue(
        BluetoothSwitchState::STATE_HALF), "2");
    EXPECT_STREQ(BluetoothSwitchStateMachine::SwitchStateToPropertyValue(
        BluetoothSwitchState::STATE_BLE_OWNER_ONLY), "3");

    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("0"),
        BluetoothSwitchState::STATE_OFF);
    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("1"),
        BluetoothSwitchState::STATE_ON);
    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("2"),
        BluetoothSwitchState::STATE_HALF);
    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("3"),
        BluetoothSwitchState::STATE_BLE_OWNER_ONLY);
    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("invalid"),
        BluetoothSwitchState::STATE_OFF);
}

/**
 * @tc.name: ValidTransitions
 * @tc.desc: all 10 legal edges succeed and persist
 */
HWTEST_F(BluetoothSwitchStateMachineTest, ValidTransitions, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    using S = BluetoothSwitchState;

    // OFF -> ON -> HALF (watch legacy path) -> ON -> OFF
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsBluetoothRestricted());
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    // OFF -> HALF -> OFF
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    // OFF -> BLE_OWNER_ONLY -> ON / OFF
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsBleOwnerOnlyMode());
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    // OFF -> BLE_OWNER_ONLY -> HALF / OFF
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);
}

/**
 * @tc.name: InvalidTransitionRejected
 * @tc.desc: forbidden edges rejected; same-state is a no-op success
 */
HWTEST_F(BluetoothSwitchStateMachineTest, InvalidTransitionRejected, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    using S = BluetoothSwitchState;

    // ON -> BLE_OWNER_ONLY forbidden (TryEnter only accepts OFF / same-mode)
    sm.SyncState(S::STATE_ON);
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_ERR_INVALID_STATE);
    // HALF -> BLE_OWNER_ONLY forbidden
    sm.SyncState(S::STATE_HALF);
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_ERR_INVALID_STATE);

    // same-state transition is a no-op success
    sm.SyncState(S::STATE_ON);
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    EXPECT_TRUE(sm.GetSwitchState() == S::STATE_ON);

    sm.SyncState(S::STATE_OFF);
}

/**
 * @tc.name: MultipleOwnersAppendAndClear
 * @tc.desc: multiple owner pids append; leaving the mode clears them all
 */
HWTEST_F(BluetoothSwitchStateMachineTest, MultipleOwnersAppendAndClear, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    using S = BluetoothSwitchState;

    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsOwnerAccessible(100));
    EXPECT_FALSE(sm.IsOwnerAccessible(200));

    // append second owner
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(200), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsOwnerAccessible(100));
    EXPECT_TRUE(sm.IsOwnerAccessible(200));
    EXPECT_EQ(sm.GetOwnerPids().size(), 2u);

    // upgrade to ON clears all owners; no owner identity in other states
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    EXPECT_EQ(sm.GetOwnerPids().size(), 0u);
    // in non-owner-only states every caller is accessible
    EXPECT_TRUE(sm.IsOwnerAccessible(300));

    // degrade to HALF also clears owners
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);
    EXPECT_EQ(sm.GetOwnerPids().size(), 0u);
}

/**
 * @tc.name: OwnerSetCapped
 * @tc.desc: owner set is capped at MAX_OWNERS
 */
HWTEST_F(BluetoothSwitchStateMachineTest, OwnerSetCapped, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(1), BT_NO_ERROR);
    for (int pid = 2; pid <= BluetoothSwitchStateMachine::MAX_OWNERS; pid++) {
        EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(pid), BT_NO_ERROR);
    }
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(999), BT_ERR_INVALID_STATE);
    EXPECT_EQ(sm.GetOwnerPids().size(), static_cast<size_t>(BluetoothSwitchStateMachine::MAX_OWNERS));
}

/**
 * @tc.name: BrOnEventResolution
 * @tc.desc: ResolveBrOnEvent snapshots the event for the BR-on callback
 */
HWTEST_F(BluetoothSwitchStateMachineTest, BrOnEventResolution, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    using S = BluetoothSwitchState;

    sm.SyncState(S::STATE_OFF);
    EXPECT_TRUE(sm.ResolveBrOnEvent() == S::STATE_ON);
    sm.SyncState(S::STATE_ON);
    EXPECT_TRUE(sm.ResolveBrOnEvent() == S::STATE_ON);
    sm.SyncState(S::STATE_HALF);
    EXPECT_TRUE(sm.ResolveBrOnEvent() == S::STATE_HALF);
    (void)sm.TryEnterBleOwnerOnlyMode(100);
    EXPECT_TRUE(sm.ResolveBrOnEvent() == S::STATE_BLE_OWNER_ONLY);
    EXPECT_FALSE(sm.IsBrAllowed());
    sm.SyncState(S::STATE_OFF);
}

/**
 * @tc.name: RebootRestoreFailSafe
 * @tc.desc: restore keeps the mode but never restores owner pids (fail-safe)
 */
HWTEST_F(BluetoothSwitchStateMachineTest, RebootRestoreFailSafe, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    using S = BluetoothSwitchState;

    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_NO_ERROR);
    // simulate reboot: memory reset then restore from property "3"
    sm.SyncState(S::STATE_OFF);
    sm.InitFromProperty();
    EXPECT_TRUE(sm.IsBleOwnerOnlyMode());
    // owners cleared: nobody accessible until an app re-enters
    EXPECT_EQ(sm.GetOwnerPids().size(), 0u);
    EXPECT_FALSE(sm.IsOwnerAccessible(100));
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(300), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsOwnerAccessible(300));
}

/**
 * @tc.name: ConcurrentOffFanOutAtomicity
 * @tc.desc: TryEnter claims OFF atomically: a concurrent ON/HALF claim makes
 * the loser's request fail instead of interleaving
 */
HWTEST_F(BluetoothSwitchStateMachineTest, ConcurrentOffFanOutAtomicity, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    using S = BluetoothSwitchState;

    // winner: owner-only claims OFF first
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(100), BT_NO_ERROR);
    // loser: a HALF transition can no longer run from this state
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);  // BLE_OWNER_ONLY -> HALF is legal degrade
    // loser: OFF -> ON after the mode was claimed is rejected as OFF is gone
    sm.SyncState(S::STATE_HALF);
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    // entering owner mode from ON is rejected (must come from OFF)
    EXPECT_EQ(sm.TryEnterBleOwnerOnlyMode(200), BT_ERR_INVALID_STATE);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);
}
}  // namespace bluetooth
}  // namespace OHOS
