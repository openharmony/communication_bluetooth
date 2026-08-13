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

#ifndef PAN_STATEMACHINE_H
#define PAN_STATEMACHINE_H
#include <list>
#include <memory>
#include <string>

#include "interface_adapter_classic.h"
#include "interface_adapter_manager.h"
#include "pan_defines.h"
#include "pan_message.h"
#include "pan_network_manager.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief Class for pan state machine.
 */

class PanServiceImpl;
class PanStateMachine : public utility::StateMachine {
public:
    /**
     * @brief Construct a new PanStateMachine object.
     *
     * @param address Device address.
     */
    explicit PanStateMachine(const std::string &address);

    /**
     * @brief Destroy the PanStateMachine object.
     */
    ~PanStateMachine() override = default;

    /**
     * @brief Initialise the state machine.
     */
    void Init(PanServiceImpl *serviceImpl);

    /**
     * @brief Get the State Int object.
     *
     * @return Returns the state number.
     */
    int GetDeviceStateInt() const;

    int GetDeviceType();

    void AddDeferredMessage(const PanMessage &msg);
    void ProcessDeferredMessage();
    std::string GetDeviceAdress();
    void NotifyStateTransitions();
    static std::string GetEventName(int what);
    void SetPanRole(const PanMessage &msg);
    int GetPanRole();
    PanNetworkManager &GetPanNetworkManager();

    void ProcessConnectReqEvent(const PanMessage &msg);
    void ProcessCloseReqEvent(const PanMessage &msg);
    void ProcessOpenEvent(const PanMessage &msg);
    void ProcessCloseEvent(const PanMessage &msg);
    void StartConnectionTimer() const;
    void StopConnectionTimer() const;
    void ConnectionTimeout() const;

    inline static const std::string DISCONNECTED = "Disconnected";
    inline static const std::string CONNECTING = "Connecting";
    inline static const std::string DISCONNECTING = "Disconnecting";
    inline static const std::string CONNECTED = "Connected";
    inline static const std::string NETWORKESTABLISHED = "NetworkEstablished";
    static const int CONNECTION_TIMEOUT_MS = 30000;

private:
    std::string address_;
    int preState_ {0};
    int role_ {INVALID_ROLE};
    std::list<PanMessage> deferMsgs_ {};
    PanServiceImpl *panServiceImpl_ {nullptr};
    std::unique_ptr<utility::Timer> connTimer_ {nullptr};
    PanNetworkManager panNetworkManager_;
    BT_DISALLOW_COPY_AND_ASSIGN(PanStateMachine);
};

class PanState : public utility::StateMachine::State {
public:
    PanState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, statemachine, parent), stateInt_(stateInt),
        stateMachine_(reinterpret_cast<PanStateMachine &>(statemachine))
    {}

    PanState(const std::string &name, utility::StateMachine &statemachine, int stateInt)
        : State(name, statemachine),
        stateInt_(stateInt), stateMachine_(reinterpret_cast<PanStateMachine &>(statemachine))
    {}

    ~PanState() override
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }
    PanServiceImpl *panServiceImpl_ {nullptr};
protected:
    int stateInt_ {PAN_STATE_DISCONNECTED};
    PanStateMachine &stateMachine_;
};

class PanDisconnectedState : public PanState {
public:
    PanDisconnectedState(const std::string &name, utility::StateMachine &statemachine)
        : PanState(name, statemachine, PAN_STATE_DISCONNECTED)
    {}
    ~PanDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_ {false};
};

class PanConnectingState : public PanState {
public:
    PanConnectingState(const std::string &name, utility::StateMachine &statemachine)
        : PanState(name, statemachine, PAN_STATE_CONNECTING)
    {}
    ~PanConnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
};

class PanDisconnectingState : public PanState {
public:
    PanDisconnectingState(const std::string &name, utility::StateMachine &statemachine)
        : PanState(name, statemachine, PAN_STATE_DISCONNECTING)
    {}
    ~PanDisconnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
};

class PanConnectedState : public PanState {
public:
    PanConnectedState(const std::string &name, utility::StateMachine &statemachine)
        : PanState(name, statemachine, PAN_STATE_CONNECTED)
    {}
    ~PanConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    void SetupNapNetwork();
};

class PanNetworkEstablished : public PanState {
public:
    PanNetworkEstablished(const std::string &name, utility::StateMachine &statemachine)
        : PanState(name, statemachine, PAN_STATE_NETWORK_ESTABLISHED)
    {}
    ~PanNetworkEstablished() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // Pan_STATEMACHINE_H
