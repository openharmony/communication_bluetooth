/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef HEARING_AID_STATEMACHINE_H
#define HEARING_AID_STATEMACHINE_H
#include <list>
#include <memory>
#include <string>

#include "interface_adapter_classic.h"
#include "interface_adapter_manager.h"
#include "hearing_aid_defines.h"
#include "hearing_aid_message.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"
#include "bt_hearing_aid.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Class for hearing aid state machine.This class is not thraed safe.
 */
class HearingAidStateMachine : public utility::StateMachine {
public:
    /**
     * @brief Construct a new HearingAidStateMachine object.
     *
     * @param address Device address.
     */
    explicit HearingAidStateMachine(const std::string &address);

    /**
     * @brief Destroy the HearingAidStateMachine object.
     */
    ~HearingAidStateMachine() override = default;

    /**
     * @brief Initialise the state machine.
     */
    void Init();

    /**
     * @brief Get the State Int object.
     *
     * @return Returns the state number.
     */
    int GetStateInt() const;

    int GetDeviceType();

    /**
     * @brief Check if current statemachine is removing.
     *
     * @return Returns <b>true</b> if the statemachine is removing; returns <b>false</b> if not.
     */
    bool IsRemoving() const;

    /**
     * @brief Mark statemachine removing.
     *
     * @param isRemoving removing mark.
     */
    void SetRemoving(bool isRemoving);

    void AddDeferredMessage(const HearingAidMessage &msg);
    void ProcessDeferredMessage();
    std::string GetDeviceAddress();
    void NotifyStateTransitions();
    void StartConnectionTimer() const;
    void StopConnectionTimer() const;
    void ConnectionTimeout() const;

    inline static const std::string DISCONNECTED = "Disconnected";
    inline static const std::string CONNECTING = "Connecting";
    inline static const std::string DISCONNECTING = "Disconnecting";
    inline static const std::string CONNECTED = "Connected";
    static const int CONNECTION_TIMEOUT_MS = 30000; // 30s

private:
    const std::string address_;
    std::unique_ptr<utility::Timer> connTimer_ {nullptr};
    int preState_ {0};
    std::list<HearingAidMessage> deferMsgs_ {};
    BT_DISALLOW_COPY_AND_ASSIGN(HearingAidStateMachine);
    bool isRemoving_ {false};
};

class HearingAidState : public utility::StateMachine::State {
public:
    HearingAidState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent, const char *title)
        : State(name, statemachine, parent), stateInt_(stateInt),
        stateMachine_(reinterpret_cast<HearingAidStateMachine &>(statemachine)), title_(title)
    {}

    HearingAidState(const std::string &name, utility::StateMachine &statemachine, int stateInt, const char *title)
        : State(name, statemachine),
        stateInt_(stateInt), stateMachine_(reinterpret_cast<HearingAidStateMachine &>(statemachine)), title_(title)
    {}

    ~HearingAidState() override
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_ {HEARING_AID_STATE_DISCONNECTED};
    HearingAidStateMachine &stateMachine_;
    const char* title_;
    ::bluetooth::hearing_aid::HearingAidInterface* GetInterface();
};

class HearingAidDisconnectedState : public HearingAidState {
public:
    HearingAidDisconnectedState(const std::string &name, utility::StateMachine &statemachine)
        : HearingAidState(name, statemachine, HEARING_AID_STATE_DISCONNECTED,
        "[HEARING_AID_STATE_MACHINE][Disconnected]")
    {}
    ~HearingAidDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    void ProcessStackEvent(std::string address, int stackState);

private:
    bool isReentry_ {false};
};

class HearingAidConnectingState : public HearingAidState {
public:
    HearingAidConnectingState(const std::string &name, utility::StateMachine &statemachine)
        : HearingAidState(name, statemachine, HEARING_AID_STATE_CONNECTING, "[HEARING_AID_STATE_MACHINE][Connecting]")
    {}
    ~HearingAidConnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    void ProcessStackEvent(std::string address, int stackState);

private:
};

class HearingAidDisconnectingState : public HearingAidState {
public:
    HearingAidDisconnectingState(const std::string &name, utility::StateMachine &statemachine)
        : HearingAidState(name, statemachine, HEARING_AID_STATE_DISCONNECTING,
        "[HEARING_AID_STATE_MACHINE][Disconnecting]")
    {}
    ~HearingAidDisconnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    void ProcessStackEvent(std::string address, int stackState);

private:
};

class HearingAidConnectedState : public HearingAidState {
public:
    HearingAidConnectedState(const std::string &name, utility::StateMachine &statemachine)
        : HearingAidState(name, statemachine, HEARING_AID_STATE_CONNECTED, "[HEARING_AID_STATE_MACHINE][Connected]")
    {}
    ~HearingAidConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
    void ProcessStackEvent(std::string address, int stackState);

private:
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // HEARING_AID_STATEMACHINE_H
