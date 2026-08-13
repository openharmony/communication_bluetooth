/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef PAN_SERVICE_IMPL_H
#define PAN_SERVICE_IMPL_H

#include "pan_message.h"
#include "pan_statemachine.h"
#include "interface_profile_pan.h"
#include "base_observer_list.h"
#include "bluetooth.h"
#include "raw_address.h"
#include "pan_service_impl_interface.h"
#include "bt_pan.h"

#include <map>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <net/if.h>
#include "safe_map.h"

namespace OHOS {
namespace Bluetooth {

class PanServiceImpl : public PanServiceImplInterface {
public:
    PanServiceImpl();
    ~PanServiceImpl() override;
    
    int BringUpNetwork() override;
    std::vector<bluetooth::RawAddress> GetDevicesByStates(std::vector<int> states) override;
    int GetConnectState(void) override;
    void ProcessConnect(const PanMessage &msg) override;
    void ProcessDisconnect(const PanMessage &msg) override;
    void ProcessConnectStateChange(const PanMessage &msg) override;
    void ProcessConnectionTimeout(const PanMessage &msg) override;
    
    void RemoveStateMachine(const std::string &device) override;
    bool IsStateMachineEmpty() override;
    void ClearStatemachine() override;
    btpan_interface_t* GetPanInterface();
    std::string GetIfname();

    void DllIsTetheringOnFunc(const std::function<bool()> IsTetheringOnFunc) override;
    void DllNotifyStateChangedFunc(const std::function<void(
        const bluetooth::RawAddress &device, int state, int role)> NotifyStateChangedFunc) override;
    void DllPostEventFunc(const std::function<void(const PanMessage &event)> PostEventFunc) override;
    void DllGetDeviceStateFunc(const std::function<int(
        const bluetooth::RawAddress &device)> GetDeviceStateFunc) override;
    void DllSetDevProactiveDisconnectFlagFunc(
        const std::function<void(const std::string &addr, uint32_t profileId, bool isProactive)>
        SetDevProactiveDisconnectFlagFunc) override;

    std::function<bool()> IsTetheringOnFunc_ = nullptr;
    std::function<void(const bluetooth::RawAddress &device, int state, int role)> NotifyStateChangedFunc_ = nullptr;
    std::function<void(const PanMessage &event)> PostEventFunc_ = nullptr;
    std::function<int(const bluetooth::RawAddress &device)> GetDeviceStateFunc_ = nullptr;
    std::function<void(const std::string &addr, uint32_t profileId, bool isProactive)>
        SetDevProactiveDisconnectFlagFunc_ = nullptr;

private:
    int InitIfreq(struct ifreq &ifr);
    int SetNetworkAddress(int inetSocket);
    int SetInterfaceUp(int inetSocket);

    SafeMap<const std::string, std::shared_ptr<PanStateMachine>> stateMachines_ {};
    std::string ifname_ {"bt-pan"};
    btpan_interface_t* panInterface_ = nullptr;
    const std::map<const int, const int> stateMap_ = {
        {PAN_STATE_DISCONNECTED, static_cast<int>(bluetooth::BTConnectState::DISCONNECTED)},
        {PAN_STATE_CONNECTING, static_cast<int>(bluetooth::BTConnectState::CONNECTING)},
        {PAN_STATE_DISCONNECTING, static_cast<int>(bluetooth::BTConnectState::DISCONNECTING)},
        {PAN_STATE_CONNECTED, static_cast<int>(bluetooth::BTConnectState::CONNECTED)},
        {PAN_STATE_NETWORK_ESTABLISHED, static_cast<int>(bluetooth::BTConnectState::CONNECTED)}
    };
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // PAN_SERVICE_IMPL_H