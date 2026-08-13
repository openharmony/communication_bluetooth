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

#ifndef CALL_STATE_MACHINE
#define CALL_STATE_MACHINE

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "base_def.h"
#include "btcommon/state_machine.h"
#include "btcommon/timer_manager.h"
#include "hfp_ag_defines.h"
#include "hfp_ag_system_event_processer.h"

namespace OHOS {
namespace bluetooth {
constexpr const char *DIALING_STATE = "Dialing";
constexpr const char *ALERTING_STATE = "Alerting";
constexpr const char *CALL_ACTIVE_STATE = "CallActive";
constexpr const char *CALL_END_STATE = "CallEnd";

class CallStateMachine : public utility::StateMachine {
public:
    enum CALL_STATE_MACHINE_EVENT {
        CALL_ALERT_EVENT = 0,
        CALL_ACTIVE_EVENT = 1,
        CALL_END_EVENT = 2,
    };
    explicit CallStateMachine(const std::string &address);
    ~CallStateMachine() override = default;

    void ReInit(void);
    static const char* GetEventName(int event);
    void DealVoipCallStateChanged(const HfpAgMessage &event);
    void DealCallStateChangeWhileDialing(HfpAgPhoneState &phoneState);
    void DealCallStateChangeWhileAlerting(HfpAgPhoneState &phoneState);
    void ProcessAlertingCallState(const HfpAgPhoneState &phoneState);
    void ProcessIdleCallState(const HfpAgPhoneState &phoneState);
    int GetActiveNum(void);
    int GetCallState(void);
    void SetPhoneState(const HfpAgPhoneState &phoneState);
    void ProcessPhoneStateChange(void);
    void TransitionToTargetState(int stateEvent);
    void RemoveTask(void);
    std::string GetDeviceAddr() const;
private:
    HfpAgSystemEventProcesser eventProcessor_;
    HfpAgPhoneState phoneState_ {};
    std::string address_;

    // The delay time of update alerting state 800ms
    inline static constexpr int VIRTUAL_CALL_ALERTING_DELAY_TIME {800};
    // The delay time of update avtive call state 810ms
    inline static constexpr int VIRTUAL_CALL_ACTIVE_STATE_DELAY_TIME {810};
    // The delay time of update active call state 10ms
    inline static constexpr int VIRTUAL_CALL_IDLE_DELAY_TIME {10};
    // Solve the problem of no sound from the device and send call status without delay
    inline static constexpr int NO_DELAYED {0};

    friend class DialingState;
    friend class AlertingState;
    friend class ActiveState;
    friend class EndState;

    BT_DISALLOW_COPY_AND_ASSIGN(CallStateMachine);
};

class CallState : public utility::StateMachine::State {
public:
    CallState(const std::string &name, CallStateMachine &stateMachine, int stateInt)
        : State(name, stateMachine), sm_(stateMachine), stateInt_(stateInt) {}
    virtual ~CallState() = default;

    int GetStateInt() const
    {
        return stateInt_;
    }
protected:
    CallStateMachine &sm_;
    int stateInt_ {HFP_AG_CALL_STATE_IDLE};
};

class DialingState : public CallState {
public:
    explicit DialingState(CallStateMachine &stateMachine)
        : CallState(DIALING_STATE, stateMachine, HFP_AG_CALL_STATE_DIALING) {}

    ~DialingState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class AlertingState : public CallState {
public:
    explicit AlertingState(CallStateMachine &stateMachine)
        : CallState(ALERTING_STATE, stateMachine, HFP_AG_CALL_STATE_ALERTING) {}

    ~AlertingState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class ActiveState : public CallState {
public:
    explicit ActiveState(CallStateMachine &stateMachine)
        : CallState(CALL_ACTIVE_STATE, stateMachine, HFP_AG_CALL_STATE_IDLE) {}

    ~ActiveState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};

class EndState : public CallState {
public:
    explicit EndState(CallStateMachine &stateMachine)
        : CallState(CALL_END_STATE, stateMachine, HFP_AG_CALL_STATE_IDLE) {}

    ~EndState() override = default;

    void Entry() override;
    void Exit() override;
    bool Dispatch(const utility::Message &msg) override;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // CALL_STATE_MACHINE