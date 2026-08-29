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

#ifndef BLE_CENTRAL_MANAGER_IMPL_H
#define BLE_CENTRAL_MANAGER_IMPL_H

#include <algorithm>
#include <atomic>
#include <list>

#include "base_def.h"
#include "ble_defs.h"
#include "ble_scan_filter.h"
#include "bt_ble_interface.h"
#include "interface_adapter_ble.h"
#include "hw_bt_hwif.h"
#include "bluetooth_resource_manager.h"

namespace OHOS {
namespace bluetooth {
class BleCentralManagerImpl {
public:
    static constexpr int UNDEFINED_CLIENT_ID = 0;

    class ShFilterIdx {
    public:
        static ShFilterIdx *GetInstance(void);
        std::pair<uint8_t, bool> GetOne(void);
        void Release(uint8_t index);
        void Clear();

    private:
        static constexpr uint8_t MAX_SH_FILTERS = 100;
        std::mutex mutex_ {};
        std::queue<uint8_t> releasedIndexs_ {};
        uint8_t currentIndex_ = 0;
    };

    /**
     * @brief Constructor.
     */
    explicit BleCentralManagerImpl(IBleCentralManagerCallback &callback);

    /**
     * @brief Destructor.
     */
    ~BleCentralManagerImpl();

    void StartScan(const BleScanSettingsImpl &setting);
    void BatchscanConfigStorage(int clientIf, int batchScanFullMax, int batchScanTruncMax,
        int batchScanNotifyThreshold);
    void BatchscanEnable(int scanMode, int scanInterval, int scanWindow, int addrType, int discardRule);
    void BatchscanDisable();
    void BatchscanReadReports(int clientIf, int scanMode);
    void StopScan();
    void StopScanWithDisabling(void);
    int GetScanStatus() const;

    /**
     * @brief config scan filter.
     *
     * @param scannerId indicate one scan.
     * @param settings scan settings
     * @param [in] filter filter list
     * @return @c client id <-> success or not
     */
    int32_t ConfigScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
        const std::vector<BleScanFilterImpl> &filters);

    /**
     * @brief remove scan filter.
     *
     * @param [in] scannerId scanner id
     * @param settings scan settings
     * @param filtersSize scan filters size
     */
    void RemoveScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings, uint16_t filtersSize);

    void DeregisterCallbackToGap(void);

    int32_t AllocScannerId();
    void RemoveScannerId(int32_t scannerId);
    void ClearScannerId();

    void SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle);
    void SetScanReportChannelToLpDevice(int32_t scannerId, bool enable);
    void EnableSyncDataToLpDevice();
    void DisableSyncDataToLpDevice();
    void SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type);
    bool IsLpDeviceAvailable();
    void SetLpDeviceParam(const FilterIdxInfo &filterIdx, const BleLpDeviceParamSetImpl &paramImpl);
    void RemoveLpDeviceParam(const FilterIdxInfo &filterIdx);
    uint8_t GetLpDeviceAdvHandle();
    void SendAdvHandleDataToLpDevice(uint8_t idx, const Uuid &uuid, const BleLpDeviceParamSetImpl &paramImpl);
    void SendScanDataToLpDevice(uint8_t idx, const Uuid &uuid, const BleLpDeviceParamSetImpl &paramImpl);
    void SendAdvertiserDataToLpDevice(uint8_t idx, const Uuid &uuid, const BleLpDeviceParamSetImpl &paramImpl);
    void SendUuidDataToLpDevice(uint8_t idx, const Uuid &uuid);
    int32_t UpdateScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
        const std::vector<BleScanFilterImpl> &filters, uint32_t filterAction);
    void ResetBatchScan();
    /**
     * @brief: Reset ble central manager object, used to clear resources.
     *
     * @return void
     */
    void Reset(void);

private:
    static constexpr uint16_t DEFAULT_SCAN_WINDOW_MS = 512;
    static constexpr uint16_t DEFAULT_SCAN_INTERVAL_MS = 5120;
    static constexpr int DEFAULT_PHY = 0x01;
    static constexpr bool DEFAULT_LEGACY = true;
    static constexpr uint8_t BLE_SCAN_UPDATE_FILTER_NONE = 0;
    static constexpr uint8_t BLE_SCAN_UPDATE_FILTER_ADD = 1;
    static constexpr uint8_t BLE_SCAN_UPDATE_FILTER_DELETE = 2;
    static constexpr uint8_t BLE_SCAN_UPDATE_FILTER_MODIFY = 3;

    static constexpr uint8_t BLE_LEGACY_CONNECT_BIT = 0;
    inline bool IsConnectable(uint16_t eventType)
    {
        return eventType & (1 << BLE_LEGACY_CONNECT_BIT);
    }

    static constexpr bool IS_STARTING_SCAN = true;
    static constexpr bool IS_STOPPING_SCAN = false;
    inline void ReportStartScanEvent(int status) const
    {
        if (callback_) {
            callback_->OnStartOrStopScanEvent(status, IS_STARTING_SCAN);
            auto resourceMgr = BluetoothResourceManager::GetInstance();
            if (resourceMgr) {
                resourceMgr->SendScanStartEvent(status);
            }
        }
    }
    inline void ReportStopScanEvent(int status) const
    {
        if (callback_) {
            callback_->OnStartOrStopScanEvent(status, IS_STOPPING_SCAN);
            auto resourceMgr = BluetoothResourceManager::GetInstance();
            if (resourceMgr) {
                resourceMgr->SendScanStopEvent(status);
            }
        }
    }

    void SetScanParameters(uint16_t intervalMSecs, uint16_t windowMSecs, bool legacy, int phy);
    void SetScanParametersCallback(uint8_t status);
    void Start(void);
    bool IsScanStarted(void) const
    {
        return status_.load() == SCAN_FAILED_ALREADY_STARTED;
    }

    auto GetScanFilterIter(int scannerId)
    {
        return std::find_if(filters_.begin(), filters_.end(),
            [scannerId](const BleScanFilter &self) { return self.GetScannerId() == scannerId; });
    }

    void NotifyScanStatusToLpDevice(bool isStart);
    void NotifyScanParamsToLpDevice(uint32_t scanWindow, uint32_t scanInterval);
    std::pair<uint8_t, bool> FindShFilterIdx(const FilterIdxInfo &info);
    std::pair<uint8_t, bool> AddShFilterIdx(const FilterIdxInfo &info);
    std::pair<FilterIdxInfo, bool> GetShFilterIdx(uint8_t idx);
    void ReleaseShFilterIdx(uint8_t idx);
    void ClearShFilterIdx();
    const BthwifInterface* GetBtHwInterface();
    uint8_t GetDeliveryMode(const BleScanSettingsImpl &settings) const;
    uint16_t GetNumOfTrackAdvs(uint8_t matchTrackAdvType) const;
    int RemoveScanFilterInner(int32_t scannerId, const BleScanSettingsImpl &settings,
        const std::vector<BleScanFilterImpl> &filters);
    int AppendScanFilterInner(int32_t scannerId, const BleScanSettingsImpl &settings,
        const std::vector<BleScanFilterImpl> &filters);
    IBleCentralManagerCallback *callback_ = nullptr;
    BleScannerInterface *btBleScanner_ = nullptr;
    const BthwifInterface *bthwInterface_ = nullptr;
    // scan status
    std::atomic_int status_ {SCAN_NOT_STARTED};

    std::atomic_bool scanFilterEnabled_ {false};
    // clientId <-> BleScanFilter
    std::list<BleScanFilter> filters_ {};

    std::mutex shFilterIdxMapLock_;
    std::mutex filterMuteLock_;
    std::map<uint8_t, FilterIdxInfo> shFilterIndexMap;

    std::set<int32_t> scannerIds_ {};
    std::mutex scannerIdsMutex_;
    std::atomic_int currentUsedTrackAdvs_;

    BT_DISALLOW_COPY_AND_ASSIGN(BleCentralManagerImpl);
    DECLARE_IMPL();
};
std::vector<Bluetooth::BluetoothBleScanResult> ParseFullResults(int numRecords, std::vector<uint8_t> data);
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLE_CENTRAL_MANAGER_IMPL_H
