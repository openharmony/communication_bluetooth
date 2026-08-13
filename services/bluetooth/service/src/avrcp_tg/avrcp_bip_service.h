/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AVRCP_BIP_SERVICE_H
#define AVRCP_BIP_SERVICE_H

#include "safe_map.h"

#include "avrcp/avrcp.h"
#include "avrcp_tg_bip_statemachine.h"
#include "../obex/obex_socket_device.h"

namespace OHOS {
namespace bluetooth {
class BipService : public std::enable_shared_from_this<BipService> {
public:
    static std::shared_ptr<BipService> GetInstance();
    BipService();
    ~BipService();
    void SetBipClientStatus(const RawAddress &rawAddr, bool connected);
    void RemoveStateMachine(const std::string &address);
    void HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice);
    void PostEvent(const BipMessage &event);
    void ProcessEvent(const BipMessage &event);
    void ProcessConnectEvent(const BipMessage &event);
    void ProcessRemoveStateMachine(const BipMessage &event);
    void ProcessCommonEvent(const BipMessage &event);

private:
    std::mutex bipServiceLock_ {};
    SafeMap<const std::string, std::shared_ptr<BipStateMachine>> stateMachines_ {};
    ::bluetooth::avrcp::ServiceInterface *avrcpServiceInterface_{nullptr};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // AVRCP_BIP_SERVICE_H
