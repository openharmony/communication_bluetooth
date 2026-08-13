/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PBAP_PCE_DISCONNECTING_STATE_H
#define PBAP_PCE_DISCONNECTING_STATE_H

#include <cstring>
#include "interface_profile_pbap_pce.h"
#include "btcommon/message.h"
#include "btcommon/timer_manager.h"
#include "pbap_pce_base_state.h"
#include "pbap_pce_state_machine.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief disconnecting statemachine
 * control disconnecting state
 */
class PceDisconnectingState : public PceBaseState {
public:
    explicit PceDisconnectingState(
        const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer);

    ~PceDisconnectingState() override = default;

    void Entry() override;

    void Exit() override;

    bool Dispatch(const utility::Message &msg) override;

private:
    void OnDisconnectTimeout();

    std::shared_ptr<utility::Timer> disconnectTimer_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // PBAP_PCE_DISCONNECTING_STATE_H