/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef HID_HOST_STATEMACHINE_H
#define HID_HOST_STATEMACHINE_H
#include <list>
#include <memory>
#include <string>

#include "interface_adapter_manager.h"
#include "interface_adapter_classic.h"
#include "btcommon/state_machine.h"
#include "hid_host_defines.h"
#include "hid_host_message.h"

#include "btcommon/timer_manager.h"
#include "service_util.h"
#include "bluetooth.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Class for hid host state machine.
 */
class HidHostStateMachine : public utility::StateMachine {
public:
    /**
     * @brief Construct a new HidHostStateMachine object.
     *
     * @param address Device address.
     */
    explicit HidHostStateMachine(const std::string &address);

    /**
     * @brief Destroy the HidHostStateMachine object.
     */
    ~HidHostStateMachine() override = default;

    /**
     * @brief Initialise the state machine.
     */
    void Init();

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

    /**
     * @brief Get the State Int object.
     *
     * @return Returns the state number.
     */
    int GetDeviceStateInt() const;

    void ConnectionTimeout() const;
    void DisconnectionTimeout() const;
    void StartConnectionTimer() const;
    void StopConnectionTimer() const;
    void StartDisconnectionTimer() const;
    void StopDisconnectionTimer() const;
    void AddDeferredMessage(const HidHostMessage &msg);
    void ProcessDeferredMessage();
    std::string GetDeviceAddress();
    void NotifyStateTransitions();
    static std::string GetEventName(int what);

    bool ProcessConnectEvent();
    bool ProcessDisConnectEvent();
    void ProcessVCUnplugEvent();
    void ProcessSetReportEvent(const HidHostMessage &msg);
    void ProcessGetReportEvent(const HidHostMessage &msg);

    inline static const std::string DISCONNECTED = "Disconnected";
    inline static const std::string CONNECTING = "Connecting";
    inline static const std::string DISCONNECTING = "Disconnecting";
    inline static const std::string CONNECTED = "Connected";

private:

    std::string address_;
    bool isRemoving_ {false};
    int preState_ {0};
    std::list<HidHostMessage> deferMsgs_ {};
    std::unique_ptr<utility::Timer> connTimer_ {nullptr};
    std::unique_ptr<utility::Timer> disconnTimer_ {nullptr};
    inline static const int CONNECTION_TIMEOUT_MS {60000};  // 60s
    inline static const int DISCONNECTION_TIMEOUT_MS {60000};

    BT_DISALLOW_COPY_AND_ASSIGN(HidHostStateMachine);
};

class HidHostState : public utility::StateMachine::State {
public:
    HidHostState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, statemachine, parent), stateInt_(stateInt),
        stateMachine_(static_cast<HidHostStateMachine &>(statemachine))
    {}

    HidHostState(const std::string &name, utility::StateMachine &statemachine, int stateInt)
        : State(name, statemachine), stateInt_(stateInt),
        stateMachine_(static_cast<HidHostStateMachine &>(statemachine))
    {}

    ~HidHostState() override
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_ {HID_HOST_STATE_DISCONNECTED};
    HidHostStateMachine &stateMachine_;
};

class HidHostDisconnectedState : public HidHostState {
public:
    HidHostDisconnectedState(const std::string &name, utility::StateMachine &statemachine)
        : HidHostState(name, statemachine, HID_HOST_STATE_DISCONNECTED)
    {}
    ~HidHostDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_ {false};

    bool DispatchBle(const utility::Message &msg);
};

class HidHostConnectingState : public HidHostState {
public:
    HidHostConnectingState(const std::string &name, utility::StateMachine &statemachine)
        : HidHostState(name, statemachine, HID_HOST_STATE_CONNECTING)
    {}
    ~HidHostConnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool DispatchBle(const utility::Message &msg);
};

class HidHostDisconnectingState : public HidHostState {
public:
    HidHostDisconnectingState(const std::string &name, utility::StateMachine &statemachine)
        : HidHostState(name, statemachine, HID_HOST_STATE_DISCONNECTING)
    {}
    ~HidHostDisconnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool DispatchBle(const utility::Message &msg);
};

class HidHostConnectedState : public HidHostState {
public:
    HidHostConnectedState(const std::string &name, utility::StateMachine &statemachine)
        : HidHostState(name, statemachine, HID_HOST_STATE_CONNECTED)
    {}
    ~HidHostConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool DispatchBle(const utility::Message &msg);
};
}  // namespace bluetooth
}
#endif  // HID_HOST_STATEMACHINE_H
