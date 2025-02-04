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

#ifndef OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_INTERFACE_H

#include <set>
#include "../parcel/bluetooth_ble_scan_filter.h"
#include "../parcel/bluetooth_ble_scan_settings.h"
#include "../parcel/bluetooth_ble_advertiser_data.h"
#include "../parcel/bluetooth_ble_advertiser_settings.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_ble_central_manager_callback.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothActiveDeviceInfo {
    std::vector<int8_t> deviceId;
    int32_t status;
    int32_t timeOut;
};
struct BluetoothLpDeviceParamSet {
    BluetoothBleScanSettings btScanSettings;
    std::vector<BluetoothBleScanFilter> btScanFilters;
    BluetoothBleAdvertiserSettings btAdvSettings;
    BluetoothBleAdvertiserData btAdvData;
    BluetoothBleAdvertiserData btRespData;
    bluetooth::Uuid uuid;
    std::vector<BluetoothActiveDeviceInfo> activeDeviceInfos;
    int32_t deliveryMode;
    int32_t advHandle;
    int32_t duration;
    uint32_t fieldValidFlagBit;
};

class IBluetoothBleCentralManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetootBleCentralManager");
    virtual void RegisterBleCentralManagerCallback(int32_t &scannerId, bool enableRandomAddrMode,
        const sptr<IBluetoothBleCentralManagerCallback> &callback) = 0;
    virtual void DeregisterBleCentralManagerCallback(int32_t scannerId,
        const sptr<IBluetoothBleCentralManagerCallback> &callback) = 0;
    virtual int StartScan(int32_t scannerId, const BluetoothBleScanSettings &settings,
        const std::vector<BluetoothBleScanFilter> &filters) = 0;
    virtual int StopScan(int32_t scannerId) = 0;
    virtual void RemoveScanFilter(int32_t scannerId) = 0;
    virtual bool FreezeByRss(std::set<int> pidSet, bool isProxy) = 0;
    virtual bool ResetAllProxy() = 0;
    virtual int SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle) = 0;
    virtual int SetScanReportChannelToLpDevice(int32_t scannerId, bool enable) = 0;
    virtual int EnableSyncDataToLpDevice() = 0;
    virtual int DisableSyncDataToLpDevice() = 0;
    virtual int SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type) = 0;
    virtual bool IsLpDeviceAvailable() = 0;
    virtual int SetLpDeviceParam(const BluetoothLpDeviceParamSet &paramSet) = 0;
    virtual int RemoveLpDeviceParam(const bluetooth::Uuid &uuid) = 0;
    virtual int ChangeScanParams(int32_t scannerId, const BluetoothBleScanSettings &settings,
        const std::vector<BluetoothBleScanFilter> &filters, uint32_t filterAction) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_INTERFACE_H