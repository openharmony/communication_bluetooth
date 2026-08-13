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

#ifndef ADAPTER_STATE_MACHINE_H
#define ADAPTER_STATE_MACHINE_H

#include "bluetooth.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"

#include "interface_adapter.h"

namespace OHOS {
namespace bluetooth {
// adapter state machine each state name
const std::string TURNING_ON_STATE = "TurningOn";
const std::string TURN_ON_STATE = "TurnOn";
const std::string TURNING_OFF_STATE = "TurningOff";
const std::string TURN_OFF_STATE = "TurnOff";

/**
 * @brief Represents adapter state machine.
 *
 * @since 6
 */
class AdapterStateMachine : public utility::StateMachine {
public:
    // define adapter state machine message kinds
    enum AdapterStateMessage {
        MSG_USER_ENABLE_REQ,
        MSG_USER_DISABLE_REQ,
        MSG_STACK_ENABLE_CMP,
        MSG_STACK_DISABLE_CMP,
        MSG_ADAPTER_ENABLE_CMP,
        MSG_ADAPTER_DISABLE_CMP,
        MSG_PROFILE_ENABLE_CMP,
        MSG_PROFILE_DISABLE_CMP,
        MSG_ADAPTER_ENABLE_TIME_OUT,
        MSG_PROFILE_ENABLE_TIME_OUT,
        MSG_ADAPTER_DISABLE_TIME_OUT,
        MSG_PROFILE_DISABLE_TIME_OUT,
    };

    /**
     * @brief A constructor used to create an <b>AdapterStateMachine</b> instance.
     *
     * @since 6
     */
    AdapterStateMachine(void) = default;

    /**
     * @brief A destructor used to delete the <b>AdapterStateMachine</b> instance.
     *
     * @since 6
     */
    ~AdapterStateMachine(void) override = default;

    /**
     * @brief A constructor used to create an <b>AdapterStateMachine</b> instance.
     *
     * @param adapter Adapter pointer(classic adapter or ble adapter).
     * @since 6
     */
    void Init(utility::Context *context);
    void SetCallingName(const std::string &name) { callingName_ = name; }

    std::string callingName_ = "";
};



/**
 * @brief Represents basic adapter state.
 *
 * @since 6
 */
class AdapterState : public utility::StateMachine::State {
public:
    /**
     * @brief A constructor used to create an <b>AdapterState</b> instance.
     *
     * @param name Name of adapter state.
     * @param stateMachine The state machine which this state belong to.
     * @param adapter Adapter pointer(classic adapter or ble adapter).
     * @since 6
     */
    AdapterState(const std::string &name, AdapterStateMachine &stateMachine, utility::Context *context)
        : State(name, stateMachine), adapterContext(context), adapterStateMachine_(stateMachine){};
    /**
     * @brief A destructor used to delete the <b>AdapterState</b> instance.
     *
     * @since 6
     */
    ~AdapterState() override = default;

protected:
    utility::Context *adapterContext;
    AdapterStateMachine &adapterStateMachine_;
};

class AdapterTurningOnState : public AdapterState {
public:
    /**
     * @brief A constructor used to create an <b>AdapterTurningOnState</b> instance.
     *
     * @param stateMachine The state machine which this state belong to.
     * @param adapter Adapter pointer(classic adapter or ble adapter).
     * @since 6
     */
    AdapterTurningOnState(AdapterStateMachine &stateMachine, utility::Context *context);

    /**
     * @brief A destructor used to delete the <b>AdapterTurningOnState</b> instance.
     *
     * @since 6
     */
    ~AdapterTurningOnState() override = default;
    /**
     * @brief Entry adapter turning on state.
     *
     * @since 6
     */
    void Entry() override;

    /**
     * @brief Exit adapter turning on state.
     *
     * @since 6
     */
    void Exit() override {};

    /**
     * @brief Adapter turning on state's dispatch.
     *
     * @param msg Message context which is used in dispath.
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 6
     */
    bool Dispatch(const utility::Message &msg) override;

private:
    // adapter turning off state timer
    std::unique_ptr<utility::Timer> adapterTimer_ = nullptr;
    // profile turning off state timer
    std::unique_ptr<utility::Timer> profileTimer_ = nullptr;
    bool DispatchOtherMsg(const utility::Message msg);
};

class AdapterTurnOnState : public AdapterState {
public:
    /**
     * @brief A constructor used to create an <b>AdapterTurnOnState</b> instance.
     *
     * @param stateMachine The state machine which this state belong to.
     * @param adapter Adapter pointer(classic adapter or ble adapter).
     * @since 6
     */
    AdapterTurnOnState(AdapterStateMachine &stateMachine, utility::Context *context)
        : AdapterState(TURN_ON_STATE, stateMachine, context){};

    /**
     * @brief A destructor used to delete the <b>AdapterTurnOnState</b> instance.
     *
     * @since 6
     */
    ~AdapterTurnOnState() override = default;
    /**
     * @brief Entry adapter turn on state.
     *
     * @since 6
     */
    void Entry() override;

    /**
     * @brief Exit adapter turn on state.
     *
     * @since 6
     */
    void Exit() override {};

    /**
     * @brief Adapter turn on state's dispatch.
     *
     * @param msg Message context which is used in dispath.
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 6
     */
    bool Dispatch(const utility::Message &msg) override;
};

class AdapterTurningOffState : public AdapterState {
public:
    /**
     * @brief A constructor used to create an <b>AdapterTurningOffState</b> instance.
     *
     * @param stateMachine The state machine which this state belong to.
     * @param adapter Adapter pointer(classic adapter or ble adapter).
     * @since 6
     */
    AdapterTurningOffState(AdapterStateMachine &stateMachine, utility::Context *context);

    /**
     * @brief A destructor used to delete the <b>AdapterTurningOffState</b> instance.
     *
     * @since 6
     */
    ~AdapterTurningOffState() override = default;

    /**
     * @brief Entry adapter turning off state.
     *
     * @since 6
     */
    void Entry() override;

    /**
     * @brief Exit adapter turning off state.
     *
     * @since 6
     */
    void Exit() override {};

    /**
     * @brief Adapter turning off state's dispatch.
     *
     * @param msg Message context which is used in dispath.
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 6
     */
    bool Dispatch(const utility::Message &msg) override;

private:
    // adapter turning off state timer
    std::unique_ptr<utility::Timer> adapterTimer_ = nullptr;
    // profile turning off state timer
    std::unique_ptr<utility::Timer> profileTimer_ = nullptr;
    void HandleDisableProfileCompleteEvent(bool isClassicAdapter);
    bool HandleDisableAdapterCompleteEvent(bool isClassicAdapter);
    void HandleDisableStackCompleteEvent(bool isClassicAdapter);
    void HandleDisableProfileTimeOutEvent(bool isClassicAdapter);
    bool HandleDisableAdapterTimeOutEvent(bool isClassicAdapter);
};

class AdapterTurnOffState : public AdapterState {
public:
    /**
     * @brief A constructor used to create an <b>AdapterTurnOffState</b> instance.
     *
     * @param stateMachine The state machine which this state belong to.
     * @param adapter Adapter pointer(classic adapter or ble adapter).
     * @since 6
     */
    AdapterTurnOffState(AdapterStateMachine &stateMachine, utility::Context *context)
        : AdapterState(TURN_OFF_STATE, stateMachine, context){};

    /**
     * @brief A destructor used to delete the <b>AdapterTurnOffState</b> instance.
     *
     * @since 6
     */
    ~AdapterTurnOffState() override = default;

    /**
     * @brief Entry adapter turn off state.
     *
     * @since 6
     */
    void Entry() override;

    /**
     * @brief Exit adapter turn off state.
     *
     * @since 6
     */
    void Exit() override {};

    /**
     * @brief Adapter turn off state's dispatch.
     *
     * @param msg Message context which is used in dispath.
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 6
     */
    bool Dispatch(const utility::Message &msg) override;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // ADAPTER_STATE_MACHINE_H