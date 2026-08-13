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

#ifndef BAS_SERVICE_H
#define BAS_SERVICE_H

#include "log.h"
#include <atomic>
#include "base_def.h"
#include "bas_service_impl_interface.h"
#include "bluetooth_observer_list.h"
#include "btcommon/dynamic_library_loader.h"
#include "context.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;

class BasService : public IProfileBas, public utility::Context {
public:
    explicit BasService();
    ~BasService() override;
    utility::Context *GetContext() override;
    static BasService *GetService();
    void Enable(void) override;
    void Disable(void) override;
    void RegisterObserver(std::shared_ptr<IBasObserver> observer) override;
    void DeregisterObserver(std::shared_ptr<IBasObserver> observer) override;
    void HandleAclStateChanged(const RawAddress &device, int state,
        unsigned int reason, int pairStatus);
    void NotifyReadBatteryLevelEvent(const RawAddress &device, int batteryLevel);
    void NotifyBatteryLevelChanged(const RawAddress &device, int batteryLevel);
    int Connect(const RawAddress &device) override;
    int Disconnect(const RawAddress &device) override;
    int SetConnectStrategy(const RawAddress &device, int strategy) override;
    int GetConnectStrategy(const RawAddress &device) override;
    int GetBatteryLevel(const RawAddress &device) override;
    int GetDeviceState(const RawAddress &device) override;
    std::list<RawAddress> GetConnectDevices() override;
    int GetConnectState(void) override;
    int GetMaxConnectNum(void) override;
    std::vector<RawAddress> GetDevicesByStates(const std::vector<int> &states) override;
    std::map<RawAddress, int32_t> GetConnectedDeviceBatteryInfos() override;

private:
    void StartUp(void);
    void ShutDown(void);
    bool IsBasSupportedByRemoteDevice(const RawAddress &device);
    void LoadBasServiceInterfaceLib(void);
    void UnloadBasServiceInterfaceLib(void);

    CxxDynamicLibraryLoader<BasServiceImplInterface> loader_;
    std::atomic_bool isLoaded_ = false;

    class BasObserver;
    std::shared_ptr<BasObserver> basObserverImpl_ {nullptr};
    BluetoothObserverList<IBasObserver> basObservers_ {};

    class BlePeripheralCallback;
    std::unique_ptr<BlePeripheralCallback> bleObserverImpl_ {nullptr};

    bool isStarted_{false};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // BAS_SERVICE_H