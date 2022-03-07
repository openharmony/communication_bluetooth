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

#include "state_machine.h"
#include "hid_host_defines.h"
#include "hid_host_l2cap_connection.h"
#include "hid_host_sdp_client.h"
#include "hid_host_uhid.h"
#include "timer.h"

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
    ~HidHostStateMachine() = default;

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

    uint16_t GetDeviceControlLcid();

    uint16_t GetDeviceInterruptLcid();

    void ProcessL2capConnectionEvent(const HidHostMessage &event);

    void ConnectionTimeout() const;
    void DisonnectionTimeout() const;
    void StartConnectionTimer() const;
    void StopConnectionTimer() const;
    void StartDisconnectionTimer() const;
    void StopDisconnectionTimer() const;
    void AddDeferredMessage(const HidHostMessage &msg);
    void ProcessDeferredMessage();
    std::string GetDeviceAdress();
    void NotifyStateTransitions();
    static std::string GetEventName(int what);

    void ProcessStartSdp(const HidHostMessage &msg);
    void ProcessOpenDevice(const HidHostMessage &msg);
    void ProcessCloseDeviceReq(const HidHostMessage &msg);
    void ProcessCloseDevice(const HidHostMessage &msg);
    void ProcessReciveData(const HidHostMessage &msg);
    void ProcessReciveControlData(const HidHostMessage &msg);
    void ProcessReciveHandshake(const HidHostMessage &msg);
    void ProcessWriteData(const HidHostMessage &msg);
    void ProcessSdpComplete(const HidHostMessage &msg);
    void ProcessOpenComplete(const HidHostMessage &msg);

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

    HidHostL2capConnection l2capConnection_;
    HidHostUhid uhid_;
    HidHostSdpClient sdpClient_;

    DISALLOW_COPY_AND_ASSIGN(HidHostStateMachine);
};

class HidHostState : public utility::StateMachine::State {
public:
    HidHostState(const std::string &name, utility::StateMachine &statemachine, int stateInt,
        utility::StateMachine::State &parent)
        : State(name, statemachine, parent), stateInt_(stateInt), stateMachine_((HidHostStateMachine &)statemachine)
    {}

    HidHostState(const std::string &name, utility::StateMachine &statemachine, int stateInt)
        : State(name, statemachine), stateInt_(stateInt), stateMachine_((HidHostStateMachine &)statemachine)
    {}

    virtual ~HidHostState()
    {}
    int GetStateInt() const
    {
        return stateInt_;
    }

protected:
    int stateInt_ {HID_HOST_STATE::HID_HOST_STATE_DISCONNECTED};
    HidHostStateMachine &stateMachine_;
};

class HidHostDisconnectedState : public HidHostState {
public:
    HidHostDisconnectedState(const std::string &name, utility::StateMachine &statemachine)
        : HidHostState(name, statemachine, HID_HOST_STATE_DISCONNECTED)
    {}
    ~HidHostDisconnectedState() = default;
    virtual void Entry() override;
    virtual void Exit() override;
    virtual bool Dispatch(const utility::Message &msg) override;

private:
    bool isReentry_ {false};
};

class HidHostConnectingState : public HidHostState {
public:
    HidHostConnectingState(const std::string &name, utility::StateMachine &statemachine)
        : HidHostState(name, statemachine, HID_HOST_STATE_CONNECTING)
    {}
    ~HidHostConnectingState() = default;
    virtual void Entry() override;
    virtual void Exit() override;
    virtual bool Dispatch(const utility::Message &msg) override;
};

class HidHostDisconnectingState : public HidHostState {
public:
    HidHostDisconnectingState(const std::string &name, utility::StateMachine &statemachine)
        : HidHostState(name, statemachine, HID_HOST_STATE_DISCONNECTING)
    {}
    ~HidHostDisconnectingState() = default;
    virtual void Entry() override;
    virtual void Exit() override;
    virtual bool Dispatch(const utility::Message &msg) override;
};

class HidHostConnectedState : public HidHostState {
public:
    HidHostConnectedState(const std::string &name, utility::StateMachine &statemachine)
        : HidHostState(name, statemachine, HID_HOST_STATE_CONNECTED)
    {}
    ~HidHostConnectedState() = default;
    virtual void Entry() override;
    virtual void Exit() override;
    virtual bool Dispatch(const utility::Message &msg) override;
};
}  // namespace bluetooth
#endif // HID_HOST_STATEMACHINE_H