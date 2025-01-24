/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_PROXY_H
#define OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_PROXY_H

#include "i_bluetooth_ble_central_manager.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleCentralManagerProxy : public IRemoteProxy<IBluetoothBleCentralManager> {
public:
    BluetoothBleCentralManagerProxy() = delete;
    explicit BluetoothBleCentralManagerProxy(const sptr<IRemoteObject> &impl);
    ~BluetoothBleCentralManagerProxy() override;
    DISALLOW_COPY_AND_MOVE(BluetoothBleCentralManagerProxy);

    void RegisterBleCentralManagerCallback(int32_t &scannerId, bool enableRandomAddrMode,
        const sptr<IBluetoothBleCentralManagerCallback> &callback) override;
    void DeregisterBleCentralManagerCallback(int32_t scannerId,
        const sptr<IBluetoothBleCentralManagerCallback> &callback) override;
    int StartScan(int32_t scannerId, const BluetoothBleScanSettings &settings,
        const std::vector<BluetoothBleScanFilter> &filters) override;
    int StopScan(int32_t scannerId) override;
    void RemoveScanFilter(int32_t scannerId) override;
    bool FreezeByRss(std::set<int> pidSet, bool isProxy) override;
    bool ResetAllProxy() override;
    int SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle) override;
    int SetScanReportChannelToLpDevice(int32_t scannerId, bool enable) override;
    int EnableSyncDataToLpDevice() override;
    int DisableSyncDataToLpDevice() override;
    int SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type) override;
    bool IsLpDeviceAvailable() override;
    int SetLpDeviceParam(const BluetoothLpDeviceParamSet &paramSet) override;
    int RemoveLpDeviceParam(const bluetooth::Uuid &uuid) override;
    int ChangeScanParams(int32_t scannerId, const BluetoothBleScanSettings &settings,
        const std::vector<BluetoothBleScanFilter> &filters, uint32_t filterAction) override;

private:
    ErrCode InnerTransact(uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply);
    static inline BrokerDelegator<BluetoothBleCentralManagerProxy> delegator_;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_PROXY_H