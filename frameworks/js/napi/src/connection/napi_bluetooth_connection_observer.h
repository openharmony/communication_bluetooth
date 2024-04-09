/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef NAPI_BLUETOOTH_CONNECTION_OBSERVER_H
#define NAPI_BLUETOOTH_CONNECTION_OBSERVER_H

#include "bluetooth_host.h"
#include "napi_async_callback.h"
#include "napi_bluetooth_utils.h"
#include "napi_event_subscribe_module.h"

namespace OHOS {
namespace Bluetooth {
class NapiBluetoothConnectionObserver : public BluetoothHostObserver {
public:
    NapiBluetoothConnectionObserver();
    ~NapiBluetoothConnectionObserver() override = default;

    void OnStateChanged(const int transport, const int status) override{};
    void OnDiscoveryStateChanged(int status) override;
    void OnDiscoveryResult(
        const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass) override;
    void OnPairRequested(const BluetoothRemoteDevice &device) override;
    void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) override;
    void OnScanModeChanged(int mode) override;
    void OnDeviceNameChanged(const std::string &deviceName) override;
    void OnDeviceAddrChanged(const std::string &address) override;

    NapiEventSubscribeModule eventSubscribe_;

private:
    void OnPairConfirmedCallBack(const std::shared_ptr<PairConfirmedCallBackInfo> &pairConfirmInfo);
    void OnDiscoveryResultCallBack(const BluetoothRemoteDevice &device);
    void OnDiscoveryResultCallBack(
        const BluetoothRemoteDevice &device, int rssi, const std::string &deviceName, int deviceClass);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_BLUETOOTH_HOST_OBSERVER_H
