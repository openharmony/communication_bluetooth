/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_HOST_OBSERVER_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_HOST_OBSERVER_INTERFACE_H

#include "bluetooth_raw_address.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothHostObserver : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothHostObserver");

    virtual void OnStateChanged(int32_t transport, int32_t status) = 0;

    // ON_DIS_STA_CHANGE_CODE
    virtual void OnDiscoveryStateChanged(int32_t status) = 0;
    virtual void OnDiscoveryResult(
        const BluetoothRawAddress &device, int rssi, const std::string deviceName, int deviceClass) = 0;
    virtual void OnPairRequested(const int32_t transport, const BluetoothRawAddress &device) = 0;
    virtual void OnPairConfirmed(
        const int32_t transport, const BluetoothRawAddress &device, int reqType, int number) = 0;
    virtual void OnScanModeChanged(int32_t mode) = 0;
    virtual void OnDeviceNameChanged(const std::string &deviceName) = 0;
    virtual void OnDeviceAddrChanged(const std::string &address) = 0;
    virtual void OnBluetoothStateChanged(int32_t state) = 0;
    virtual void OnRefusePolicyChanged(const int32_t pid, const int64_t prohibitedSecondsTime) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_HOST_OBSERVER_INTERFACE_H
