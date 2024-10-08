/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef BLUETOOTH_ACCESS_IMPL_H
#define BLUETOOTH_ACCESS_IMPL_H

#include "bluetooth_host.h"
#include "napi_bluetooth_utils.h"

#include <cstdint>

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothAccess {
using Bluetooth::BluetoothHostObserver;
using Bluetooth::BluetoothRemoteDevice;
using Bluetooth::BluetoothState;

class AccessImpl {
public:
    AccessImpl() = default;
    ~AccessImpl() = default;

    static void EnableBluetooth(int32_t* errCode);
    static void DisableBluetooth(int32_t* errCode);
    static int32_t GetState(int32_t* errCode);
    static void RegisterAccessObserver(int32_t callbackType, void (*callback)(), int32_t* errCode);
};

class CjBluetoothAccessObserver : public BluetoothHostObserver {
    public:
    CjBluetoothAccessObserver();
    ~CjBluetoothAccessObserver() override = default;

    void OnStateChanged(const int transport, const int status) override;
    void OnDiscoveryStateChanged(int status) override{};
    void OnDiscoveryResult(
        const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass) override{};
    void OnPairRequested(const BluetoothRemoteDevice &device) override{};
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) override{};
    void OnScanModeChanged(int mode) override{};
    void OnDeviceNameChanged(const std::string &deviceName) override{};
    void OnDeviceAddrChanged(const std::string &address) override{};

    void RegisterStateChangeFunc(std::function<void(int32_t)> cjCallback);

private:
    bool DealStateChange(const int transport, const int status, BluetoothState &state);
    void GetBrStateByStatus(const int status, BluetoothState &state, bool &isCallback);
    void GetBleStateByStatus(const int status, BluetoothState &state);
    std::function<void(int32_t)> stateChangeFunc{nullptr};
};
} // namespace BluetoothAccess
} // namespace CJSystemapi
} // namespace OHOS

#endif // BLUETOOTH_ACCESS_IMPL_H