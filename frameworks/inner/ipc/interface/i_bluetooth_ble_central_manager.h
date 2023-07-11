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

#include "../parcel/bluetooth_ble_scan_filter.h"
#include "../parcel/bluetooth_ble_scan_settings.h"
#include "../parcel/bluetooth_ble_advertiser_data.h"
#include "../parcel/bluetooth_ble_advertiser_settings.h"
#include "i_bluetooth_ble_central_manager_callback.h"
#include "iremote_broker.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothAdvDeviceInfo {
    std::vector<int8_t> advDeviceId;
    int32_t status;
    int32_t timeOut;
};
struct BluetoothBleFilterParamSet {
    BluetoothBleScanSettings btScanSettings;
    std::vector<BluetoothBleScanFilter> btScanFilters;
    BluetoothBleAdvertiserSettings btAdvSettings;
    BluetoothBleAdvertiserData btAdvData;
    BluetoothBleAdvertiserData btRespData;
    bluetooth::Uuid uuid;
    std::vector<BluetoothAdvDeviceInfo> advDeviceInfos;
    int deliveryMode;
    int advHandle;
    int duration;
    int fieldValidFlagBit;
};

class IBluetoothBleCentralManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetootBleCentralManager");

    enum Code {
        BLE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK = 0,
        BLE_DE_REGISTER_BLE_CENTRAL_MANAGER_CALLBACK,
        BLE_START_SCAN,
        BLE_START_SCAN_WITH_SETTINGS,
        BLE_STOP_SCAN,
        BLE_CONFIG_SCAN_FILTER,
        BLE_REMOVE_SCAN_FILTER,
        BLE_PROXY_UID,
        BLE_RESET_ALL_PROXY,
        BLE_SET_BURST_PARAM,
        BLE_SET_SCAN_REPORT_CHANNEL_TO_SH,
        BLE_START_SCAN_IN_SH_SYNC,
        BLE_STOP_SCAN_IN_SH_SYNC,
        BLE_SEND_PARAMS_TO_SH,
        BLE_IS_SUPPORT_SENSOR_ADVERTISER_FILTER,
        BLE_SET_SH_ADV_FILTER_PARAM,
        BLE_REMOVE_SH_ADV_FILTER,
    };

    virtual void RegisterBleCentralManagerCallback(int32_t &scannerId,
        const sptr<IBluetoothBleCentralManagerCallback> &callback) = 0;
    virtual void DeregisterBleCentralManagerCallback(int32_t scannerId,
        const sptr<IBluetoothBleCentralManagerCallback> &callback) = 0;
    virtual int StartScan(int32_t scannerId) = 0;
    virtual int StartScan(int32_t scannerId, const BluetoothBleScanSettings &settings) = 0;
    virtual int StopScan(int32_t scannerId) = 0;
    virtual int ConfigScanFilter(int32_t scannerId, const std::vector<BluetoothBleScanFilter> &filters) = 0;
    virtual void RemoveScanFilter(int32_t scannerId) = 0;
    virtual bool ProxyUid(int32_t uid, bool isProxy) = 0;
    virtual bool ResetAllProxy() = 0;
    virtual int SetBurstParam(int duration, int maxExtAdvEvents, int burstWindow, int burstInterval, int advHandle) = 0;
    virtual int SetScanReportChannelToSensorHub(const int clientId, const int isToAp) = 0;
    virtual int StartScanInShSync() = 0;
    virtual int StopScanInShSync() = 0;
    virtual int SendParamsToSensorhub(const std::vector<uint8_t> &dataValue, int32_t type) = 0;
    virtual bool IsSupportSensorAdvertiseFilter() = 0;
    virtual int SetAdvFilterParam(const BluetoothBleFilterParamSet &paramSet) = 0;
    virtual int RemoveAdvFilter(const bluetooth::Uuid &uuid) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_INTERFACE_H