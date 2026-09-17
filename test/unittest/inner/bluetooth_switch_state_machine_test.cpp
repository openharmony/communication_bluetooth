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
        SetParameter("persist.bluetooth.half_app_registered_owner", "");
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
        BluetoothSwitchState::STATE_HALF_APP_REGISTERED), "3");

    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("0"),
        BluetoothSwitchState::STATE_OFF);
    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("1"),
        BluetoothSwitchState::STATE_ON);
    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("2"),
        BluetoothSwitchState::STATE_HALF);
    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("3"),
        BluetoothSwitchState::STATE_HALF_APP_REGISTERED);
    EXPECT_EQ(BluetoothSwitchStateMachine::PropertyValueToSwitchState("invalid"),
        BluetoothSwitchState::STATE_OFF);
}

/**
 * @tc.name: ValidTransitions
 * @tc.desc: legal four-state transitions succeed and persist
 */
HWTEST_F(BluetoothSwitchStateMachineTest, ValidTransitions, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    using S = BluetoothSwitchState;

    // OFF -> ON -> OFF
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    EXPECT_TRUE(sm.GetSwitchState() == S::STATE_ON);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    // OFF -> HALF -> ON -> OFF
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsBluetoothRestricted());
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    // OFF -> HALF -> OFF
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    // OFF -> HALF_APP_REGISTERED -> ON / HALF / OFF
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF_APP_REGISTERED), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsHalfAppRegisteredMode());
    EXPECT_FALSE(sm.IsBrAllowed());
    EXPECT_EQ(sm.TransitionTo(S::STATE_ON), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsBrAllowed());
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF_APP_REGISTERED), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsBluetoothRestricted());
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);

    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF_APP_REGISTERED), BT_NO_ERROR);
    EXPECT_EQ(sm.TransitionTo(S::STATE_OFF), BT_NO_ERROR);
}

/**
 * @tc.name: InvalidTransitionRejected
 * @tc.desc: transition to the same state is a no-op success, table-checked transitions only
 */
HWTEST_F(BluetoothSwitchStateMachineTest, InvalidTransitionRejected, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    using S = BluetoothSwitchState;

    // ON -> HALF and ON -> HALF_APP_REGISTERED are forbidden
    sm.SyncState(S::STATE_ON);
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_ERR_INVALID_STATE);
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF_APP_REGISTERED), BT_ERR_INVALID_STATE);
    EXPECT_TRUE(sm.GetSwitchState() == S::STATE_ON);

    // HALF -> HALF_APP_REGISTERED is forbidden
    sm.SyncState(S::STATE_HALF);
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF_APP_REGISTERED), BT_ERR_INVALID_STATE);
    EXPECT_TRUE(sm.GetSwitchState() == S::STATE_HALF);

    // same-state transition is accepted as no-op
    EXPECT_EQ(sm.TransitionTo(S::STATE_HALF), BT_NO_ERROR);
    EXPECT_TRUE(sm.GetSwitchState() == S::STATE_HALF);

    sm.SyncState(S::STATE_OFF);
}

/**
 * @tc.name: EnterHalfAppRegisteredModeOwnerCheck
 * @tc.desc: entering half-app-registered records the owner app and gates BLE access
 */
HWTEST_F(BluetoothSwitchStateMachineTest, EnterHalfAppRegisteredModeOwnerCheck, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    EXPECT_EQ(sm.EnterHalfAppRegisteredMode("com.example.owner"), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsHalfAppRegisteredMode());
    EXPECT_EQ(sm.GetOwnerName(), "com.example.owner");

    // owner allowed, other apps and empty name refused
    EXPECT_TRUE(sm.IsOwnerAccessible("com.example.owner"));
    EXPECT_FALSE(sm.IsOwnerAccessible("com.example.other"));
    EXPECT_FALSE(sm.IsOwnerAccessible(""));

    // idempotent re-enter refreshes the owner
    EXPECT_EQ(sm.EnterHalfAppRegisteredMode("com.example.owner2"), BT_NO_ERROR);
    EXPECT_EQ(sm.GetOwnerName(), "com.example.owner2");

    // leaving half-app-registered reopens access for everyone
    EXPECT_EQ(sm.TransitionTo(BluetoothSwitchState::STATE_ON), BT_NO_ERROR);
    EXPECT_TRUE(sm.IsOwnerAccessible("com.example.other"));
    EXPECT_EQ(sm.GetOwnerName(), "");
}

/**
 * @tc.name: RestoreFromProperty
 * @tc.desc: reboot restore reads persisted state and owner
 */
HWTEST_F(BluetoothSwitchStateMachineTest, RestoreFromProperty, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    EXPECT_EQ(sm.EnterHalfAppRegisteredMode("com.example.owner"), BT_NO_ERROR);

    // simulate reboot: reset memory then restore from property
    sm.SyncState(BluetoothSwitchState::STATE_OFF);
    EXPECT_FALSE(sm.IsHalfAppRegisteredMode());
    sm.InitFromProperty();
    EXPECT_TRUE(sm.IsHalfAppRegisteredMode());
    EXPECT_EQ(sm.GetOwnerName(), "com.example.owner");
    EXPECT_FALSE(sm.IsBrAllowed());
}

/**
 * @tc.name: SyncStateDoesNotPersist
 * @tc.desc: SyncState only changes memory, property restore overrides it
 */
HWTEST_F(BluetoothSwitchStateMachineTest, SyncStateDoesNotPersist, TestSize.Level1)
{
    auto &sm = BluetoothSwitchStateMachine::GetInstance();
    EXPECT_EQ(sm.EnterHalfAppRegisteredMode("com.example.owner"), BT_NO_ERROR);
    sm.SyncState(BluetoothSwitchState::STATE_ON);
    EXPECT_TRUE(sm.GetSwitchState() == BluetoothSwitchState::STATE_ON);
    // property still holds "3", restore brings half-app-registered back
    sm.InitFromProperty();
    EXPECT_TRUE(sm.IsHalfAppRegisteredMode());
}
}  // namespace bluetooth
}  // namespace OHOS
