/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_BLUETOOTH_ACCESS_OBSERVER_H
#define TAIHE_BLUETOOTH_ACCESS_OBSERVER_H

#include <shared_mutex>
#include "stdexcept"

#include "ohos.bluetooth.access.proj.hpp"
#include "ohos.bluetooth.access.impl.hpp"
#include "taihe/runtime.hpp"

#include "bluetooth_host.h"

#ifndef BT_MODULE_NAME
#define BT_MODULE_NAME "default"
#endif

namespace OHOS {
namespace Bluetooth {

class TaiheBluetoothAccessObserver : public BluetoothHostObserver {
public:
    TaiheBluetoothAccessObserver();
    ~TaiheBluetoothAccessObserver() override = default;

    static std::shared_ptr<TaiheBluetoothAccessObserver> GetInstance(void);

    void OnStateChanged(const int transport, const int status) override;
    void OnDiscoveryStateChanged(int status) override {};
    void OnDiscoveryResult(
        const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass) override {};
    void OnPairRequested(const BluetoothRemoteDevice &device) override {};
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) override {};
    void OnScanModeChanged(int mode) override {};
    void OnDeviceNameChanged(const std::string &deviceName) override {};
    void OnDeviceAddrChanged(const std::string &address) override {};

    std::vector<::taihe::optional<::taihe::callback<void(
        ::ohos::bluetooth::access::BluetoothState data)>>> stateChangedObserverVec = {};
    std::shared_mutex stateChangedObserverLock;
private:
    void DealStateChange(const int transport, const int status, BluetoothState &state);
    void GetBrStateByStatus(const int status, BluetoothState &state);
    void GetBleStateByStatus(const int status, BluetoothState &state);
};

class TaiheBluetoothAccessInit {
public:
    static void RegisterAccessObserverToHost();
};

} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_ACCESS_OBSERVER_H