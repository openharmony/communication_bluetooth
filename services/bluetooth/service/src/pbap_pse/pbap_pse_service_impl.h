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

#ifndef PBAP_PSE_SERVICE_IMPL_H
#define PBAP_PSE_SERVICE_IMPL_H

#include <cstring>
#include <list>
#include <map>
#include <vector>
#include "base_def.h"
#include "base_observer_list.h"
#include "context.h"
#include "btcommon/message.h"
#include "pbap_pse_def.h"
#include "pbap_pse_message.h"
#include "pbap_pse_statemachine.h"
#include "raw_address.h"
#include "pbap_pse_service_impl_interface.h"
#include "common_util.h"
#include "thread_util.h"
#include "adapter_device_config.h"


namespace OHOS {
namespace bluetooth {
class PbapPseServiceImpl : public PbapPseServiceImplInterface {
public:
    PbapPseServiceImpl() = default;
    ~PbapPseServiceImpl() override;

    void RegisterObserver(std::shared_ptr<IPbapPseObserver> &observer) override;

    void DeregisterObserver(std::shared_ptr<IPbapPseObserver> &observer) override;

    int32_t Disconnect(const RawAddress &device) override;

    std::vector<RawAddress> GetDevicesByStates(const std::vector<int32_t> &states) override;

    int32_t GetDeviceState(const RawAddress &device) override;

    int32_t GetConnectState(void) override;

    void SetPhoneBookAccessAuthorization(const RawAddress& device, int32_t accessAuthorization) override;

    void HandlerConnectEvt(std::shared_ptr<ObexSocketDevice> &socketDevice) override;

    void DllRegisterFunc(const std::function<bool(const std::string&, std::shared_ptr<utility::Timer>,
        bool)> &dialogFunc, std::shared_ptr<SafeMap<const std::string, int64_t>> permissionRequestTimeMap) override;

    void PostEvent(const PbapPseMessage &event);

    void ProcessEvent(const PbapPseMessage &event);

    void RemoveStateMachine(const std::string &device);

    void CheckOrGetPermission(const std::string &address, std::shared_ptr<utility::Timer> timer);

    void ProcessReqPermissionTimeOut(const PbapPseMessage &event);
private:
    void ProcessConnectEvent(const PbapPseMessage &event);

    void ProcessRemoveStateMachine(const PbapPseMessage &event);

    void ProcessCommmonEvent(const PbapPseMessage &event);

    void ProcessPermissionResultEvent(const PbapPseMessage &event);

    bool IsNeedSetPermissionToUnknown(const std::string &address, int permission);

    void VerifyPermissionResult(const std::string &address, bool isAccess);

    void NotifyStateChanged(const RawAddress &device, int state);

    BtRecursiveMutex mutex_ {};
    // The map of the device and sate machine.
    std::map<const std::string, std::unique_ptr<PbapPseStateMachine>> stateMachines_ {};
    std::shared_ptr<SafeMap<const std::string, int64_t>> permissionRequestTimeMap_;
    std::shared_ptr<IPbapPseObserver> pbapPseConnectionObserver_ = nullptr;
    std::function<bool(const std::string&, std::shared_ptr<utility::Timer>, bool)> dialogFunc_ = nullptr;

    const std::map<const int32_t, const int32_t> stateMap_ = {
        {PBAP_PSE_STATE_WAITFORAUTH, static_cast<int>(BTConnectState::CONNECTING)},
        {PBAP_PSE_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED)},
        {PBAP_PSE_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED)}
    };
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PBAP_PSE_SERVICE_IMPL_H
