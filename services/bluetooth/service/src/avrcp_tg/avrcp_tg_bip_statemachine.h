/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AVRCP_TG_BIP_STATEMACHINE_H
#define AVRCP_TG_BIP_STATEMACHINE_H

#include <list>
#include <memory>
#include <string>
#include "base_def.h"
#include "bluetooth.h"
#include "btcommon/state_machine.h"
#include "avrcp_tg_bip_obex_server.h"
#include "avrcp_tg_bip_message.h"
#include "../obex/obex_server_transport.h"
#include "../obex/obex_server_session.h"

namespace OHOS {
namespace bluetooth {

class BipStateMachine : public utility::StateMachine {
public:
    explicit BipStateMachine(std::shared_ptr<ObexSocketDevice> socketDevice,
        const std::weak_ptr<BipService> bipService);
    ~BipStateMachine();
    void Init();
    bool IsRemoving() const;
    void SetRemoving(bool isRemoving);
    std::string GetDeviceAddress();
    static std::string GetEventName(int what);

    void ProcessConnectEvent();
    void ProcessDisConnectEvent();
    void RemoveStateMachine(const std::string &address);
    int GetDeviceStateInt() const;

private:
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
    bool isRemoving_ = false;
    int preState_ = 0;
    std::shared_ptr<ObexServerTransport> obexTransport_ = nullptr;
    std::shared_ptr<AvrcpBipObexServer> bipObexServer_ = nullptr;
    std::shared_ptr<ObexServerSession> obexServerSession_ = nullptr;
    std::weak_ptr<BipService> bipService_ = {};
    BT_DISALLOW_COPY_AND_ASSIGN(BipStateMachine);
};

class BipState : public utility::StateMachine::State {
public:
    BipState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, statemachine, parent), stateInt_(stateInt),
        stateMachine_(static_cast<BipStateMachine &>(statemachine))
    {}

    BipState(const std::string &name, utility::StateMachine &statemachine, int stateInt)
        : State(name, statemachine), stateInt_(stateInt),
        stateMachine_(static_cast<BipStateMachine &>(statemachine))
    {}

    ~BipState() override
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_ {BIP_STATE_CONNECTED};
    BipStateMachine &stateMachine_;
};

class BipConnectedState : public BipState {
public:
    BipConnectedState(const std::string &name, utility::StateMachine &statemachine)
        : BipState(name, statemachine, BIP_STATE_CONNECTED)
    {}
    ~BipConnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class BipDisconnectedState : public BipState {
public:
    BipDisconnectedState(const std::string &name, utility::StateMachine &statemachine)
        : BipState(name, statemachine, BIP_STATE_DISCONNECTED)
    {}
    ~BipDisconnectedState() override = default;
    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_ {false};
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_TG_BIP_STATEMACHINE_H