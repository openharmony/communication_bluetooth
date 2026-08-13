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

#ifndef BLUETOOTH_BLE_SCANNER_APPLICATION_CONTAINER_H
#define BLUETOOTH_BLE_SCANNER_APPLICATION_CONTAINER_H

#include "ble_scanner_state_machine.h"
#include "ble_service_data.h"
#include "bluetooth_application_container.h"

namespace OHOS {
namespace Bluetooth {

struct BleScanFilterContainer {
    bool isFiltersEnable = false;
    std::vector<bluetooth::BleScanFilterImpl> bleScanFilters {};
};
struct BluetoothBleScannerApplication : public BluetoothApplication {
    BluetoothBleScannerApplication(int pid, int uid, const sptr<IRemoteObject> &remote)
        : BluetoothApplication(pid, uid, remote) {}
    ~BluetoothBleScannerApplication() override = default;

    uint64_t tokenId = 0;
    int32_t scannerId = 0;
    int sdkVersion = 0;
    bool isUseRealAddrFlag = false;
    BleScanFilterContainer bleScanFilterContainer;

    bool isRssFreeze = false; // turn true only when app is freezed, turn false when stop freeze
    bool isAppActiveScan = false; // turn true when start scan, turn false only when stop scan
    bool isBatchScan = false; // turn true only when start batchscan, turn false only when stop batchscan
    bluetooth::BleScanSettingsImpl settings;
    std::string callingName;
};

class BluetoothBleScannerApplicationContainer : public BluetoothApplicationContainer<BluetoothBleScannerApplication> {
public:
    using ResetBatchScanCallback = std::function<bool(int32_t, int32_t, int32_t)>;
    explicit BluetoothBleScannerApplicationContainer(std::weak_ptr<BleScannerStateMachine> sm,
        ResetBatchScanCallback resetBatchScanCb)
        : BluetoothApplicationContainer("BluetoothBleScannerApplicationContainer"), sm_(sm),
        resetBatchScanCb_(resetBatchScanCb) {}
    void AddTokenId(const wptr<IRemoteObject> &remote, uint64_t tokenId);
    uint64_t GetTokenId(const wptr<IRemoteObject> &remote) const;
    void AddSdkVersion(const wptr<IRemoteObject> &remote, int sdkVersion);
    int GetSdkVersion(const wptr<IRemoteObject> &remote) const;

    void SetScannerId(const wptr<IRemoteObject> &remote, int pid, int uid, int scannerId);
    void ClearScannerId(void);
    int32_t GetScannerId(const wptr<IRemoteObject> &remote) const;
    void SetUseRealAddrFlag(const wptr<IRemoteObject> &remote, bool flag);
    bool GetUseRealAddrFlag(const wptr<IRemoteObject> &remote) const;
    bool GetUseRealAddrFlag(int32_t scannerId) const;
    uint16_t GetBleScanFilterSize(const int32_t scannerId) const;
    bool IsBleScanFilterMatched(const int32_t scannerId, const BluetoothBleScanResult &bleScanResult,
        bluetooth::BleScanFilterImpl &matchedFilter) const;
    void SetLpDeviceFlag(int32_t scannerId, bool enable);
    void SetBleScanFilter(const int32_t scannerId, std::vector<bluetooth::BleScanFilterImpl> bleScanFilters);
    void ClearBleScanFilter(const int32_t scannerId);
    void RemoveBleScanFilter(const int32_t scannerId, std::vector<bluetooth::BleScanFilterImpl> bleScanFilters);
    void AppendBleScanFilter(const int32_t scannerId, std::vector<bluetooth::BleScanFilterImpl> bleScanFilters);
    void GetBatchScanApps(std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue);
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    bool IsPidUidScannerIdMatched(const int32_t pid, const int32_t uid, const int32_t scannerId);
    void SetScanIdSetting(const int32_t pid, const int32_t uid, const int32_t scannerId,
        bluetooth::BleScanSettingsImpl settings);
    bluetooth::BleScanSettingsImpl GetScanSetting(uint8_t scannerId);
    void SetBatchScanFlag(const int32_t pid, const int32_t uid, const int32_t scannerId, bool isBatchScan);
    long GetMinReportDelayMillisValue(std::list<BleScannerStateMachine::ScannerAppInfo> &batchscanAppQueue);
    bool IsBatchScanApp(const int32_t pid, const int32_t uid, const int32_t scannerId);
    void SetScanIdState(const int32_t pid, const int32_t uid, const int32_t scannerId, bool isAppActiveScan);
    bool GetScanIdState(const int32_t pid, const int32_t uid, int32_t scannerId);
    bool IsValidScannerId(const int32_t pid, const int32_t uid, int32_t scannerId);
    void SetFreezeState(const int32_t pid, const int32_t uid, const int32_t scannerId, bool isRssFreeze);
    std::string DumpScanFreezeAppInfo();
    std::vector<BluetoothBleScannerApplication> FindFreezeInfoByPid(int uid);
    sptr<IRemoteObject> GetRemote(uint8_t scannerId);
    void SetCallingName(const int32_t scannerId, std::string name);
    void ChrReportScanFilter(int32_t subErrCode);
private:
    std::weak_ptr<BleScannerStateMachine> sm_;
    ResetBatchScanCallback resetBatchScanCb_;
    auto GetApplicationIterByScanId(uint8_t scannerId)
    {
        return std::find_if(container_.begin(), container_.end(),
            [scannerId](const auto &app) { return app.scannerId == scannerId; });
    }
    auto GetApplicationIterByPidScanId(int pid, int uid, int scannerId)
    {
        return std::find_if(container_.begin(), container_.end(),
            [pid, uid, scannerId](const auto &app) {
                return app.pid == pid && app.uid == uid && app.scannerId == scannerId;
            });
    }
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // BLUETOOTH_BLE_SCANNER_APPLICATION_CONTAINER_H