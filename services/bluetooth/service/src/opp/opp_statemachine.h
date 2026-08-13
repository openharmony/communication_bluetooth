/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef OPP_STATEMACHINE_H
#define OPP_STATEMACHINE_H

#include <list>
#include <memory>
#include <string>
#include "btcommon/timer_manager.h"
#include "bluetooth.h"
#include "interface_adapter_manager.h"
#include "interface_adapter_classic.h"
#include "btcommon/state_machine.h"
#include "service_util.h"
#include "opp_message.h"
#include "opp_obex_client.h"
#include "opp_obex_server.h"
#include "../obex/obex_server_transport.h"
#include "../obex/obex_server_session.h"

namespace OHOS {
namespace bluetooth {
class OppServiceImpl;
/**
 * @brief Class for opp state machine.
 */
class OppStateMachine : public utility::StateMachine {
public:
    explicit OppStateMachine(const std::string &address, std::shared_ptr<ObexSocketDevice> socketDevice,
        OppServiceImpl *oppServiceImpl);

    ~OppStateMachine();

    void Init();
    bool IsRemoving() const;

    /**
     * @brief Get the State Int object.
     *
     * @return Returns the state number.
     */
    int GetDeviceStateInt() const;

    void SetSocketService(std::shared_ptr<ObexSocketDevice> socketDevice);
    void RemoveStateMachine(const std::string &address);
    void SetRemoving(bool isRemoving);

    void DisconnectionTimeout() const;
    void StartConnectionTimer() const;

    std::string GetDeviceAdress();
    static std::string GetEventName(int what);
    void ConnectionTimeout() const;
    void StopConnectionTimer() const;
    void StartDisconnectionTimer() const;
    void StopDisconnectionTimer() const;
    void NotifyStateTransitions();

    bool ProcessSdpCompleteEvent(const OppMessage &msg);
    bool ProcessConnectReqEvent(const OppMessage &msg);
    bool ProcessConnectedEvent(const OppMessage &msg);
    void ProcessDisconnectReqEvent(const OppMessage &msg);
    void ProcessDisconnectedEvent(const OppMessage &msg);
private:
    std::string address_;
    std::shared_ptr<ObexSocketDevice> socketDevice_;
    std::shared_ptr<ObexClientConfig> obexConfig_ = nullptr;
    bool isRemoving_ = false;
    int preState_ = OPP_STATE_DISCONNECTED;
    std::unique_ptr<utility::Timer> connTimer_ = nullptr;
    std::unique_ptr<utility::Timer> disconnTimer_ = nullptr;
    const int connectionTimeoutMs = 60000;
    const int disconnectionTimeoutMs = 60000;

    OppServiceImpl *oppServiceImpl_ = nullptr;
    std::shared_ptr<ObexServerTransport> obexTransport_ = nullptr;
    std::shared_ptr<OppObexServer> oppObexServer_ = nullptr;
    std::shared_ptr<ObexServerSession> obexServerSession_ = nullptr;

    BT_DISALLOW_COPY_AND_ASSIGN(OppStateMachine);
};

class OppState : public utility::StateMachine::State {
public:
    OppState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, statemachine, parent),
        stateInt_(stateInt),
        stateMachine_(static_cast<OppStateMachine &>(statemachine))
    {}

    OppState(const std::string &name, utility::StateMachine &statemachine, int stateInt)
        : State(name, statemachine),
        stateInt_(stateInt),
        stateMachine_(static_cast<OppStateMachine &>(statemachine))
    {}

    virtual ~OppState()
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_ {OPP_STATE_DISCONNECTED};
    OppStateMachine &stateMachine_;
};

class OppDisconnectedState : public OppState {
public:
    OppDisconnectedState(const std::string &name, utility::StateMachine &statemachine)
        : OppState(name, statemachine, OPP_STATE_DISCONNECTED)
    {}
    ~OppDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_ {false};
};

class OppConnectingState : public OppState {
public:
    OppConnectingState(const std::string &name, utility::StateMachine &statemachine)
        : OppState(name, statemachine, OPP_STATE_CONNECTING)
    {}
    ~OppConnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class OppDisconnectingState : public OppState {
public:
    OppDisconnectingState(const std::string &name, utility::StateMachine &statemachine)
        : OppState(name, statemachine, OPP_STATE_DISCONNECTING)
    {}
    ~OppDisconnectingState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class OppConnectedState : public OppState {
public:
    OppConnectedState(const std::string &name, utility::StateMachine &statemachine)
        : OppState(name, statemachine, OPP_STATE_CONNECTED)
    {}
    ~OppConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};
}  // namespace bluetooth
}
#endif  // OPP_STATEMACHINE_H
