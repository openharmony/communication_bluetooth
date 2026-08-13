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

#ifndef PAN_SERVICE_H
#define PAN_SERVICE_H

#include <cmath>
#include <cstring>
#include <list>
#include <memory.h>
#include <vector>

#include "adapter_config.h"
#include "base_def.h"
#include "base_observer_list.h"
#include "bluetooth.h"
#include "bt_pan.h"
#include "class_creator.h"
#include "context.h"
#include "interface_profile_pan.h"
#include "btcommon/dynamic_library_loader.h"
#include "log.h"
#include "pan_message.h"
#include "pan_service_impl_interface.h"
#include "profile_config.h"
#include "profile_service_manager.h"
#include "raw_address.h"
#include "securec.h"
#include "service_util.h"

namespace OHOS {
namespace Bluetooth {
class PanService : public IProfilePan, public utility::Context {
public:
    static constexpr char const *DEFAULT_LIB_NAME = "libbtpan.z.so";
    static constexpr char const *DEFAULT_LIB_CREATE_FUNC_NAME = "CreatePanServiceImplInterface";
    static constexpr char const *DEFAULT_LIB_DESTROY_FUNC_NAME = "DestroyPanServiceImplInterface";
    static constexpr uint32_t DEFAULT_UNLOAD_TIMER_MS = 600000;

    void LoadPanServiceManagerInterfaceLib(void);
    void UnloadPanServiceManagerInterfaceLib(void);
    /**
     * @brief Get the instance of the HfpHfService object.
     *
     * @return Returns the instance of the HfpHfService object.
     */
    static PanService *GetService();
    /**
     * @brief Construct a new Pan Service object
     *
     */
    PanService();
    /**
     * @brief Destroy the Pan Service object
     *
     */
    ~PanService() override;
    utility::Context *GetContext() override;
    void Enable(void) override;
    void Disable(void) override;
    int Connect(const bluetooth::RawAddress &device) override;
    std::list<bluetooth::RawAddress> GetConnectDevices() override;
    int GetConnectState(void) override;
    int GetMaxConnectNum(void) override;
    int Disconnect(const bluetooth::RawAddress &device) override;
    std::vector<bluetooth::RawAddress> GetDevicesByStates(std::vector<int> states) override;
    int GetDeviceState(const bluetooth::RawAddress &device) override;
    void RegisterObserver(IPanObserver &PanObserver) override;
    void DeregisterObserver(IPanObserver &PanObserver) override;
    int SetTethering(bool enable) override;
    bool IsTetheringOn() override;
    int SetConnectStrategy(const bluetooth::RawAddress &device, int strategy) override;
    int GetConnectStrategy(const bluetooth::RawAddress &device) override;
    void NotifyStateChanged(const bluetooth::RawAddress &device, int state, int role);
    void RemoveStateMachine(const std::string &device);
    
    btpan_interface_t *getBluetoothPanInterface() const;

    void ProcessEvent(const PanMessage &event);

    int BringUpNetwork();

    void ShutDownDone();

    /**
     * @brief Send the event of the Pan role.
     *
     * @param event The event of the Pan role.
     */
    void PostEvent(const PanMessage &event);

    static void ConnectionStateCallback(btpan_connection_state_t state,
        bt_status_t error, const BLUEDROID::RawAddress* addr, int localRole, int remoteRole);

    static void ControlStateCallback(btpan_control_state_t state,
        int localRole, bt_status_t error, const char* ifname);
private:
    btpan_interface_t* bluetoothPanInterface_ = nullptr;

    bool isStarted_ {false};

    bool isTetheringOn_ {false};

    bool isShuttingDown_ {false};

    std::string ifname_ {"bt-pan"};

    BaseObserverList<IPanObserver> panObservers_ {};

    const int INVALID_CONNECT_STATE = 0;

    // The maximum default number of connection devices, same with MAX_PAN_CONNS in stack
    static const int PAN_MAX_DEFAULT_CONNECTIONS_NUMR = 7;
    // the maximum number of connection devices.
    int maxConnectionsNum_ {PAN_MAX_DEFAULT_CONNECTIONS_NUMR};

    // const state map
    const std::map<const int, const int> stateMap_ = {
        {PAN_STATE_DISCONNECTED, static_cast<int>(bluetooth::BTConnectState::DISCONNECTED)},
        {PAN_STATE_CONNECTING, static_cast<int>(bluetooth::BTConnectState::CONNECTING)},
        {PAN_STATE_DISCONNECTING, static_cast<int>(bluetooth::BTConnectState::DISCONNECTING)},
        {PAN_STATE_CONNECTED, static_cast<int>(bluetooth::BTConnectState::CONNECTED)},
        {PAN_STATE_NETWORK_ESTABLISHED, static_cast<int>(bluetooth::BTConnectState::CONNECTED)}
    };

    bluetooth::CxxDynamicLibraryLoader<PanServiceImplInterface> loader_;
    std::shared_ptr<PanServiceImplInterface> serviceImpl_ = nullptr;
    bool isLoaded_ = false;

    void StartUp();

    void ShutDown();

    void ProcessControlStateChange(const PanMessage &msg);

    void ProcessDisconnectAll();

    void ProcessRemoveStateMachine(const PanMessage &msg);

    void ProcessConnectionTimeout(const PanMessage &msg);

    bool IsRemotePanSupported(const bluetooth::RawAddress &device);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // PAN_SERVICE_H
