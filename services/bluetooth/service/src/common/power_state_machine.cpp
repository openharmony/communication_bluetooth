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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_power_state"
#endif

#include "power_state_machine.h"
#include <cstring>
#include "log.h"
#include "power_spec.h"

namespace OHOS {
namespace bluetooth {
void PowerStateMachine::Init(PowerDevice &pd)
{
    std::unique_ptr<utility::StateMachine::State> powerActiveState = std::make_unique<PowerActiveState>(*this, pd);
    std::unique_ptr<utility::StateMachine::State> powerAaState =
        std::make_unique<PowerActiveActivingState>(*this, pd, *powerActiveState.get());
    std::unique_ptr<utility::StateMachine::State> powerAsState =
        std::make_unique<PowerActiveSniffingState>(*this, pd, *powerActiveState.get());

    std::unique_ptr<utility::StateMachine::State> powerSniffState = std::make_unique<PowerSniffState>(*this, pd);
    std::unique_ptr<utility::StateMachine::State> powerSaState =
        std::make_unique<PowerSniffActivingState>(*this, pd, *powerSniffState.get());
    std::unique_ptr<utility::StateMachine::State> powerSsState =
        std::make_unique<PowerSniffSniffingState>(*this, pd, *powerSniffState.get());

    Move(powerActiveState);
    Move(powerAaState);
    Move(powerAsState);

    Move(powerSniffState);
    Move(powerSaState);
    Move(powerSsState);

    InitState(ACTIVE_STATE);
}

void PowerActiveState::Entry()
{
}

void PowerActiveState::Exit()
{
}

bool PowerActiveState::Dispatch(const utility::Message &msg)
{
    switch (msg.what_) {
        case PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF: {
            HILOGI("msg: MSG_PM_MODE_CHANGE_SNIFF");
            Transition(SNIFF_STATE);
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_SNIFF: {
            HILOGI("msg: MSG_PM_SET_SNIFF");
            Transition(ACTIV_SNIFFING_STATE);
            return true;
        }
        default:
            return false;
    }
}

void PowerActiveActivingState::Entry()
{
    int exitSniffRet = pd_.BtmExitSniffMode();
    HILOGI("BtmExitSniffMode ret val: %{public}d", exitSniffRet);
}

void PowerActiveActivingState::Exit()
{
}

bool PowerActiveActivingState::Dispatch(const utility::Message &msg)
{
    switch (msg.what_) {
        case PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE: {
            HILOGI("msg: MSG_PM_MODE_CHANGE_ACTIVE");
            Transition(ACTIVE_STATE);
            return true;
        }
        case PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF: {
            HILOGI("msg: MSG_PM_MODE_CHANGE_SNIFFn");
            return false;
        }
        case PowerStateMachine::MSG_PM_SET_ACTIVE: {
            HILOGI("msg: MSG_PM_SET_ACTIVE");
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_SNIFF: {
            HILOGI("msg: MSG_PM_SET_SNIFF");
            Transition(ACTIV_SNIFFING_STATE);
            return true;
        }
        default:
            return false;
    }
}

void PowerActiveSniffingState::Entry()
{
    auto powerLevel = pd_.GetRequestPowerLevel();
    auto controlPowerLevel = pd_.GetControlPowerLevel();
    if (powerLevel.first != PowerSsrLevel::NO_ACTION && powerLevel.first != controlPowerLevel.first) {
        int ssrRet = pd_.BtmSetSniffSubrating(PowerSpec::GetPowerSsrParam(powerLevel.first));
        HILOGI("BtmSetSniffSubrating ret val: %{public}d", ssrRet);
    } else if (powerLevel.second != PowerModeLevel::NO_ACTION && powerLevel.second != controlPowerLevel.second) {
        int sniffRet = pd_.BtmEnterSniffMode(PowerSpec::GetPowerParam(powerLevel.second));
        HILOGI("BtmEnterSniffMode ret val: %{public}d", sniffRet);
    } else {
    }
}

void PowerActiveSniffingState::Exit()
{
}

bool PowerActiveSniffingState::Dispatch(const utility::Message &msg)
{
    switch (msg.what_) {
        case PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE: {
            HILOGI("msg: MSG_PM_MODE_CHANGE_ACTIVE");
            Transition(ACTIVE_STATE);
            return true;
        }
        case PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF: {
            HILOGI("msg: MSG_PM_MODE_CHANGE_SNIFF");
            return false;
        }
        case PowerStateMachine::MSG_PM_SET_ACTIVE: {
            HILOGI("msg: MSG_PM_SET_ACTIVE");
            Transition(ACTIV_ACTIVING_STATE);
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_SNIFF: {
            HILOGI("msg: MSG_PM_SET_SNIFF");
            auto powerLevel = pd_.GetRequestPowerLevel();
            auto controlPowerLevel = pd_.GetControlPowerLevel();
            if (powerLevel != controlPowerLevel) {
                if (powerLevel.first != PowerSsrLevel::NO_ACTION && powerLevel.first != controlPowerLevel.first) {
                    int ssrRet = pd_.BtmSetSniffSubrating(PowerSpec::GetPowerSsrParam(powerLevel.first));
                    HILOGI("BtmSetSniffSubrating ret val: %{public}d", ssrRet);
                } else if (powerLevel.second != PowerModeLevel::NO_ACTION &&
                           powerLevel.second != controlPowerLevel.second) {
                    int sniffRet = pd_.BtmEnterSniffMode(PowerSpec::GetPowerParam(powerLevel.second));
                    HILOGI("BtmEnterSniffMode ret val: %{public}d", sniffRet);
                } else {
                    HILOGI("nothing to do in PowerActiveSniffingState while receive message MSG_PM_SET_SNIFF");
                }
            }
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE: {
            HILOGI("msg: MSG_PM_SET_SUBRATING_COMPLETE");
            auto powerLevel = pd_.GetRequestPowerLevel();
            auto controlPowerLevel = pd_.GetControlPowerLevel();
            if (powerLevel.second != PowerModeLevel::NO_ACTION && powerLevel.second != controlPowerLevel.second) {
                int sniffRet = pd_.BtmEnterSniffMode(PowerSpec::GetPowerParam(powerLevel.second));
                HILOGI("BtmEnterSniffMode ret val: %{public}d", sniffRet);
            } else {
                Transition(SNIFF_STATE);
            }
            return true;
        }
        default:
            return false;
    }
}

void PowerSniffState::Entry()
{
}

void PowerSniffState::Exit()
{
}

bool PowerSniffState::Dispatch(const utility::Message &msg)
{
    switch (msg.what_) {
        case PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE: {
            HILOGI("msg: MSG_PM_MODE_CHANGE_ACTIVE");
            Transition(ACTIVE_STATE);
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_SNIFF: {
            HILOGI("msg: MSG_PM_SET_SNIFF");
            if (pd_.GetControlPowerLevel() != pd_.GetRequestPowerLevel()) {
                Transition(SNIFF_SNIFFING_STATE);
                HILOGI("requestPowerLevel = %{public}d, controlPowerLevel = %{public}d",
                    pd_.GetRequestPowerLevel().second,
                    pd_.GetControlPowerLevel().second);
                HILOGI("requestSSRLevel = %{public}d, controlSSRLevel = %{public}d",
                    pd_.GetRequestPowerLevel().first,
                    pd_.GetControlPowerLevel().first);
            }
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_ACTIVE: {
            HILOGI("msg: MSG_PM_SET_ACTIVE");
            Transition(SNIFF_ACTIVING_STATE);
            return true;
        }
        default:
            return false;
    }
}

void PowerSniffActivingState::Entry()
{
    int exitSniffRet = pd_.BtmExitSniffMode();
    HILOGI("BtmExitSniffMode ret val: %{public}d", exitSniffRet);
}

void PowerSniffActivingState::Exit()
{
}

bool PowerSniffActivingState::Dispatch(const utility::Message &msg)
{
    switch (msg.what_) {
        case PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE: {
            HILOGI("msg: MSG_PM_MODE_CHANGE_ACTIVE");
            return false;
        }
        case PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF: {
            HILOGI("msg: MSG_PM_MODE_CHANGE_SNIFF");
            Transition(SNIFF_STATE);
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_ACTIVE: {
            HILOGI("msg: MSG_PM_SET_ACTIVE");
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_SNIFF: {
            HILOGI("msg: MSG_PM_SET_SNIFF");
            Transition(SNIFF_SNIFFING_STATE);
            HILOGI("requestPowerLevel = %{public}d, controlPowerLevel = %{public}d",
                pd_.GetRequestPowerLevel().second,
                pd_.GetControlPowerLevel().second);
            HILOGI("requestSSRLevel = %{public}d, controlSSRLevel = %{public}d",
                pd_.GetRequestPowerLevel().first,
                pd_.GetControlPowerLevel().first);
            return true;
        }
        default:
            return false;
    }
}

void PowerSniffSniffingState::Entry()
{
    auto powerLevel = pd_.GetRequestPowerLevel();
    auto controlPowerLevel = pd_.GetControlPowerLevel();
    if (powerLevel.first != PowerSsrLevel::NO_ACTION && powerLevel.first != controlPowerLevel.first) {
        int ssrRet = pd_.BtmSetSniffSubrating(PowerSpec::GetPowerSsrParam(powerLevel.first));
        HILOGI("BtmSetSniffSubrating ret val: %{public}d", ssrRet);
    } else if (powerLevel.second != PowerModeLevel::NO_ACTION && powerLevel.second != controlPowerLevel.second) {
        HILOGI("requestSniffLevel != controlLevel, request sniff level = %{public}d, controlLevel = %{public}d",
            powerLevel.second, controlPowerLevel.second);
    } else {
        HILOGI("PM_: nothing to do in PowerSniffSniffingState entry");
    }
}

void PowerSniffSniffingState::Exit()
{
}

bool PowerSniffSniffingState::Dispatch(const utility::Message &msg)
{
    HILOGI("msg: %{public}d", msg.what_);
    switch (msg.what_) {
        case PowerStateMachine::MSG_PM_MODE_CHANGE_ACTIVE: {
            return false;
        }
        case PowerStateMachine::MSG_PM_MODE_CHANGE_SNIFF: {
            Transition(SNIFF_STATE);
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_ACTIVE: {
            Transition(SNIFF_ACTIVING_STATE);
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_SNIFF: {
            auto powerLevel = pd_.GetRequestPowerLevel();
            auto controlPowerLevel = pd_.GetControlPowerLevel();
            if (powerLevel != controlPowerLevel) {
                if (powerLevel.first != PowerSsrLevel::NO_ACTION && powerLevel.first != controlPowerLevel.first) {
                    pd_.BtmSetSniffSubrating(PowerSpec::GetPowerSsrParam(powerLevel.first));
                } else if (powerLevel.second != PowerModeLevel::NO_ACTION &&
                           powerLevel.second != controlPowerLevel.second) {
                    HILOGI("requestSniffLevel != controlLevel, request sniff level = %{public}d, "
                        "controlLevel = %{public}d", powerLevel.second, controlPowerLevel.second);
                } else {
                }
            }
            return true;
        }
        case PowerStateMachine::MSG_PM_SET_SUBRATING_COMPLETE: {
            auto powerLevel = pd_.GetRequestPowerLevel();
            auto controlPowerLevel = pd_.GetControlPowerLevel();
            if (powerLevel.second != PowerModeLevel::NO_ACTION && powerLevel.second != controlPowerLevel.second) {
                HILOGI("requestSniffLevel != controlLevel, request sniff level = %{public}d, "
                    "controlLevel = %{public}d", powerLevel.second, controlPowerLevel.second);
            } else {
                Transition(SNIFF_STATE);
            }
            return true;
        }
        default:
            return false;
    }
}
}  // namespace bluetooth
}  // namespace OHOS