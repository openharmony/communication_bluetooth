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

#ifndef PBAP_PSE_STATEMACHINE_H
#define PBAP_PSE_STATEMACHINE_H

#include <list>
#include <memory>
#include <string>
#include "btcommon/timer_manager.h"
#include "bluetooth.h"
#include "interface_adapter_manager.h"
#include "interface_adapter_classic.h"
#include "btcommon/state_machine.h"
#include "service_util.h"
#include "pbap_pse_message.h"
#include "pbap_pse_obex_server.h"
#include "../obex/obex_server_transport.h"
#include "../obex/obex_server_session.h"
#include "obex_reject_server.h"

namespace OHOS {
namespace bluetooth {
class PbapPseServiceImpl;
/**
 * @brief Class for pbap pse state machine.
 */
class PbapPseStateMachine : public utility::StateMachine {
public:
    explicit PbapPseStateMachine(std::shared_ptr<ObexSocketDevice> socketDevice,
        PbapPseServiceImpl *pbapPseServiceImpl);

    ~PbapPseStateMachine();

    void Init();
    bool IsRemoving() const;
    void SetRemoving(bool isRemoving);
    int GetDeviceStateInt() const;
    bool GetRequestPermissionFlag() const;
    void SetRequestPermissionFlag(bool flag);
    void RequestPermissionTimeout();
    void StopRequestPermissionTimer() const;
    void AddDeferredMessage(const PbapPseMessage &msg);
    void ProcessDeferredMessage();
    std::string GetDeviceAddress();
    void NotifyStateTransitions();
    static std::string GetEventName(int what);

    bool ProcessConnectEvent();
    bool ProcessDisConnectEvent();
    void RequestPhonebookPermission(std::string &address);
    void RemoveStateMachine(const std::string &address);
    void RejectConnection();

private:
    std::shared_ptr<ObexSocketDevice> socketDevice_;
    bool isRemoving_ = false;
    int preState_ = 0;
    bool isRequestPermission_ = false;
    std::list<PbapPseMessage> deferMsgs_ {};
    std::shared_ptr<utility::Timer> requestPermissionTimer_ = nullptr;

    std::shared_ptr<ObexServerTransport> obexTransport_ = nullptr;
    std::shared_ptr<PbapPseObexServer> pbapPseObexServer_ = nullptr;
    std::shared_ptr<ObexRejectServer> obexRejectServer_ = nullptr;
    std::shared_ptr<ObexServerSession> obexServerSession_ = nullptr;
    PbapPseServiceImpl *pbapPseServiceImpl_ = nullptr;

    BT_DISALLOW_COPY_AND_ASSIGN(PbapPseStateMachine);
};

class PbapPseState : public utility::StateMachine::State {
public:
    PbapPseState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, statemachine, parent), stateInt_(stateInt),
        stateMachine_(static_cast<PbapPseStateMachine &>(statemachine))
    {}

    PbapPseState(const std::string &name, utility::StateMachine &statemachine, int stateInt)
        : State(name, statemachine), stateInt_(stateInt),
        stateMachine_(static_cast<PbapPseStateMachine &>(statemachine))
    {}

    ~PbapPseState() override
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_ {PBAP_PSE_STATE_WAITFORAUTH};
    PbapPseStateMachine &stateMachine_;
};

class PbapPseWaitForAuthState : public PbapPseState {
public:
    PbapPseWaitForAuthState(const std::string &name, utility::StateMachine &statemachine)
        : PbapPseState(name, statemachine, PBAP_PSE_STATE_WAITFORAUTH)
    {}
    ~PbapPseWaitForAuthState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class PbapPseConnectedState : public PbapPseState {
public:
    PbapPseConnectedState(const std::string &name, utility::StateMachine &statemachine)
        : PbapPseState(name, statemachine, PBAP_PSE_STATE_CONNECTED)
    {}
    ~PbapPseConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class PbapPseDisconnectedState : public PbapPseState {
public:
    PbapPseDisconnectedState(const std::string &name, utility::StateMachine &statemachine)
        : PbapPseState(name, statemachine, PBAP_PSE_STATE_DISCONNECTED)
    {}
    ~PbapPseDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_ {false};
};

}  // namespace bluetooth
}
#endif  // PBAP_PSE_STATEMACHINE_H
