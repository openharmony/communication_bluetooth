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

#ifndef BAS_SERVICE_IMPL_H
#define BAS_SERVICE_IMPL_H

#ifndef LOG_TAG
#define LOG_TAG "bt_service_bas_impl"
#endif

#include "bas_service_impl_interface.h"
#include "bas_statemachine.h"
#include "bluetooth_observer_list.h"
#include "raw_address.h"
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

class BasServiceImpl : public BasServiceImplInterface {
public:
    BasServiceImpl();
    ~BasServiceImpl() override;
    void Init(void) override;
    void DeInit(void) override;
    void RegisterObserver(std::shared_ptr<IBasObserver> observer) override;
    void DeregisterObserver(std::shared_ptr<IBasObserver> observer) override;
    int Connect(const RawAddress &device) override;
    int Disconnect(const RawAddress &device) override;
    int GetBatteryLevel(const RawAddress &device) override;
    int GetDeviceState(const RawAddress &device) override;
    std::list<RawAddress> GetConnectDevices() override;
    int GetConnectState(void) override;
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) override;
    std::map<RawAddress, int32_t> GetConnectedDeviceBatteryInfos() override;
    void PostEvent(const BasMessage &event) override;
    void RemoveStateMachine(const std::string &device) override;

private:
    void StartUp(void);
    void ShutDown(void);
    void ShutDownDone(bool isAllDisconnected);
    void ProcessEvent(const BasMessage &event);
    void ProcessConnectEvent(const BasMessage &event);
    void ProcessDefaultEvent(const BasMessage &event);
    void ProcessRemoveStateMachine(const std::string &address);
    void ProcessBatteryLevelChanged(const BasMessage &event);
    void ProcessBatteryLevelRead(const BasMessage &event);
    void NotifyReadBatteryLevelEvent(const RawAddress &device, int batteryLevel);
    void NotifyBatteryLevelChanged(const RawAddress &device, int batteryLevel);
    int IsLocalDeviceConnectAllowed(const RawAddress &device);
    std::string GetEventName(int what);
    int GetConnectedDeviceNum();

    static const int BAS_MAX_DEFAULT_CONNECTIONS_NUM = 8;
    // the maximum number of connection devices.
    int maxConnectionsNum_ {BAS_MAX_DEFAULT_CONNECTIONS_NUM};
    BluetoothObserverList<IBasObserver> basObservers_ {};
    SafeMap<const std::string, std::shared_ptr<BasStateMachine>> stateMachines_ {};
    const std::map<const int, const int> stateMap_ = {
        {BAS_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED)},
        {BAS_STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING)},
        {BAS_STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING)},
        {BAS_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED)}
    };
    bool isStarted_ {false};
    bool isShuttingDown_ {false};

    BT_DISALLOW_COPY_AND_ASSIGN(BasServiceImpl);
};

}  // namespace bluetooth
}  // namespace OHOS
#endif  // BAS_SERVICE_IMPL_H