/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
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
#ifndef OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_SERVER_H
#define OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_SERVER_H

#include <cstdint>
#include <mutex>
#include <set>

#include "bluetooth_ble_central_manager_stub.h"
#include "bluetooth_types.h"
#include "i_bluetooth_ble_central_manager.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleCentralManagerServer : public BluetoothBleCentralManagerStub {
public:
    BluetoothBleCentralManagerServer();
    ~BluetoothBleCentralManagerServer() override;

    enum FreezeType : uint8_t {
        DEFAULT_TYPE = 0x00,
        LOW_POWER_TYPE = 0x01,
        UNSET_TYPE = 0xFF,
    };
    enum WakeType : uint8_t {
        DEFAULT_WAKE_TYPE = 0x00,
        BLE_SCAN_WAKE_TYPE = 0x01,
        ACL_STATE_WAKE_TYPE = 0x02,
        UNSET_WAKE_TYPE = 0xFF,
    };
    void RegisterBleCentralManagerCallback(int32_t &scannerId, bool enableRandomAddrMode,
        const sptr<IBluetoothBleCentralManagerCallback> &callback) override;
    void DeregisterBleCentralManagerCallback(int32_t scannerId,
        const sptr<IBluetoothBleCentralManagerCallback> &callback) override;
    int StartScan(int32_t scannerId, const BluetoothBleScanSettings &settings,
        const std::vector<BluetoothBleScanFilter> &filters, bool isNewApi) override;
    int StopScan(int32_t scannerId) override;
    void RemoveScanFilter(int32_t scannerId) override;
    bool FreezeByRss(std::set<int> pidSet, bool isProxy, uint8_t freezeType) override;
    bool ResetAllProxy() override;
    static bool IsResourceScheduleApp(int32_t pid, uint8_t wakeType = WakeType::DEFAULT_WAKE_TYPE);
    void Init(const wptr<BluetoothBleCentralManagerServer> &wptr);
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
    int IsValidScannerId(int32_t scannerId, bool &isValid) override;
    int FlushBatchScanResults(int32_t scannerId) override;
private:
    BLUETOOTH_DECLARE_IMPL();
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleCentralManagerServer);
    static std::mutex proxyMutex_;
    static std::map<int32_t, uint8_t> proxyPids_;
    void SetScanParams(const BluetoothBleScanSettings &settings);
    void SetWindowAndInterval(const int mode, uint16_t &window, uint16_t &interval);
    bool IsNewScanParams();
    bool IsAllStop();
    void AdjustScanModeForHi1112Chip(bluetooth::BleScanSettingsImpl &settingsImpl);
    bool IsWifiConnected();
    int ConfigScanFilterInner(int32_t scannerId, const bluetooth::BleScanSettingsImpl &settings,
        const std::vector<BluetoothBleScanFilter> &filters);
    void RemoveScanFilterByRss(int32_t scannerId);
    bool StartFreezeByRss(int32_t pid);
    bool StopFreezeByRss(int32_t pid);
    void ReportStartedOrStoppedEvent(int pid, int uid, int scannerId, int resultCode, bool isStartScan);
    int UpdateScanFilterInner(int32_t scannerId, const bluetooth::BleScanSettingsImpl &settings,
        const std::vector<BluetoothBleScanFilter> &filters, uint32_t filterAction);
    int UpdateScanSettingsInner(int32_t pid, int32_t uid, int32_t scannerId,
        const bluetooth::BleScanSettingsImpl &settings);
    void SetScanSettingsInfo(bluetooth::BleScanSettingsImpl &settingsImpl, const BluetoothBleScanSettings &settings);
    void StartNormalScan(int32_t pid, int32_t uid, int32_t scannerId, bluetooth::BleScanSettingsImpl &settingsImpl);
    void ReportStartScanHiSysEvent(int32_t pid, int32_t uid, std::string callingName,
        const BluetoothBleScanSettings &settings);
    bool IsValidBleScanEnhanceMode(bool isEnable, bluetooth::BleScanEnhanceMode &enhanceMode);
    int SetBleScanEnhanceMode(bool isEnable, bluetooth::BleScanEnhanceMode enhanceMode);
public:
    std::string DumpRssFreezeInfo();
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_BLE_CENTRAL_MANAGER_SERVER_H