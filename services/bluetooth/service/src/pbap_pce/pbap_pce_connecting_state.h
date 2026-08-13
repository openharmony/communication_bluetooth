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

#ifndef PBAP_PCE_CONNECTING_STATE_H
#define PBAP_PCE_CONNECTING_STATE_H

#include <cstring>
#include "interface_profile_pbap_pce.h"
#include "btcommon/message.h"
#include "btcommon/timer_manager.h"
#include "pbap_pce_base_state.h"
#include "pbap_pce_header_msg.h"
#include "pbap_pce_state_machine.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief connecting statemachine
 * control connecting state
 */
class PceConnectingState : public PceBaseState {
public:
    explicit PceConnectingState(
        const std::string &name, PbapPceStateMachine &sm, std::shared_ptr<IPbapPceObserver> observer);

    /**
     * @brief destructor
     * @details destructor
     */
    ~PceConnectingState() override = default;

    /**
     * @brief entry
     * @details when become connecting, call this
     * @return void
     */
    void Entry() override;

    /**
     * @brief exit
     * @details when leave connecting, call this
     * @return void
     */
    void Exit() override;

    /**
     * @brief Dispatch
     * @details when Dispatch PceConnectingState, call this
     * @param msg message
     * @return bool true:success false:failure
     */
    bool Dispatch(const utility::Message &msg) override;

private:
    /**
     * @brief connect to obex
     * @details connect to obex
     * @param sdpMsg pass obex message
     * @return int @c  0 success
     *              @c -1 failure
     */
    int CreateObexClient(const PbapPceHeaderSdpMsg &sdpMsg);

    /**
     * @brief set Obex Client Config
     * @details set Obex Client Config
     * @param obexConfig Client Config
     * @return
     */
    void SetObexClientConfigDetail(PbapPceObexConfig &obexConfig) const;

    /**
     * @brief InitAuth
     * @details Initialize Auth data
     */
    void InitAuth();

    /**
     * @brief Process ObexConnected
     * @details Process ObexConnected
     * @param msg
     */
    void ProcessObexConnected(const utility::Message &msg);

    /**
     * @brief Process SdpFinish
     * @details Process SdpFinish
     * @param msg
     */
    void ProcessSdpFinish(const utility::Message &msg);

    /**
     * @brief Process ObexConnectFailed
     * @details Process ObexConnectFailed
     * @param msg
     */
    void ProcessObexConnectFailed(const utility::Message &msg);

    /**
     * @brief Process ObexTransportFailed
     * @details Process ObexTransportFailed
     * @param msg
     */
    void ProcessObexTransportFailed(const utility::Message &msg);

    /**
     * @brief Process PasswordInput
     * @details Process PasswordInput
     * @param msg
     */
    void ProcessPasswordInput(const utility::Message &msg) const;

    void OnConnectTimeout();

    PbapPceObexConfig obexConfig_ {};
    std::vector<uint8_t> authDescription_ {};
    std::vector<uint8_t> authNonce_ {};
    uint8_t authUserCharset_ = 0;
    bool authFullAccess_ = false;
    bool authNeedUser_ = false;
    bool authChallenge_ = false;
    std::unique_ptr<ObexHeader> header_ {nullptr};
    std::shared_ptr<utility::Timer> connectTimer_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // PBAP_PCE_CONNECTING_STATE_H
