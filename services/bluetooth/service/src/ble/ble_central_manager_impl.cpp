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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_ble_central_manager"
#endif

#include "ble_central_manager_impl.h"

#include <array>
#include <future>
#include <sstream>

#include "base/bind.h"
#include "base/callback.h"
#include "gatt/bluetooth_gatt_interface.h"
#include "common/bluetooth_hw_interface.h"
#include "common/bluetooth_channel_rssi_manager.h"
#include "bt_def.h"
#include "btif_gatt.h"
#include "log.h"
#include "raw_address.h"
#include "service_util.h"
#include "thread_util.h"
#include "adapter_properties.h"
#include "bluetooth_errorcode.h"
#include "bt_chr_dft_exception.h"

namespace OHOS {
namespace bluetooth {
struct BleCentralManagerImpl::impl :
    public BluetoothGattInterface::ScannerObserver, public BluetoothHwInterface::SensorhubObserver {
    explicit impl(BleCentralManagerImpl &bleCentralManager) : self_(bleCentralManager), isAddObserver_(false),
        isEnableSyncDataToLpDevice(false), isScanStartInLpDevice(false), isLpDeviceReset(false), scanIntervalInLp(0),
        scanWindowInLp(0)
    {}
    ~impl() override
    {
        // Clean up resources when users forget
        if (isAddObserver_.load()) {
            BluetoothGattInterface::GetInstance()->RemoveScannerObserver(*this);
            BluetoothHwInterface::GetInstance()->RemoveSensorhubObserver(*this);
            isAddObserver_ = false;
        }
    }

    void CreateBlePeripheralDevice(BlePeripheralDevice &outDevice,
        uint16_t eventType, uint8_t addrType, const RawAddress &addr, int8_t rssi);
    void ScanResultCallback(uint16_t eventType, uint8_t addrType, const STACK::RawAddress *addr, int8_t rssi,
        std::vector<uint8_t> advData) override;
    void BatchScanReportCallback(int clientIf, int status, int reportFormat, int numRecords,
            std::vector<uint8_t> data) override;
    void BatchScanThresholdCallback(int clientIf) override;
    void TrackAdvFoundLostCallback(const TrackAdvBaseInfo &info, const STACK::RawAddress &addr,
        std::vector<uint8_t> advData) override;
    void SensorhubDevInfoCallback(uint8_t *buffer, int length) override;
    void SensorhubResetCallback(uint32_t state) override;

    void GetScanParams(const BleScanSettingsImpl &setting, uint16_t &outScanInterval, uint16_t &outScanWindow,
        int &outPhy) const;

    void AppendUuidToByteArray(const Uuid &uuid, std::vector<uint8_t> &retValue);
    void AppendDataToByteArray(int32_t data, uint8_t dataLen, std::vector<uint8_t> &retValue);
    void AppendDeviceAddressToByteArray(const std::vector<BleScanFilterImpl> &filters, std::vector<uint8_t> &retValue);
    void AppendSrvUuidListToByteArray(const std::vector<BleScanFilterImpl> &filters, std::vector<uint8_t> &retValue);
    void AppendUuidAndMaskToByteArray(const Uuid &uuid, const Uuid &uuidMask, bool isMaskExist,
        std::vector<uint8_t> &retValue);
    void AppendSolicUuidListToByteArray(const std::vector<BleScanFilterImpl> &filters, std::vector<uint8_t> &retValue);
    void AppendLocalNameToByteArray(const std::vector<BleScanFilterImpl> &filters, std::vector<uint8_t> &retValue);
    void AppendManufDataListToByteArray(const std::vector<BleScanFilterImpl> &filters, std::vector<uint8_t> &retValue);
    void AppendSrvDataListToByteArray(const std::vector<BleScanFilterImpl> &filters, std::vector<uint8_t> &retValue);
    std::vector<uint8_t> GetAdvHandleMsgData(const Uuid &uuid, int advHandle);
    std::vector<uint8_t> GetSoftFilterMsgData(uint8_t idx, int deliveryMode, const Uuid &uuid,
         const std::vector<BleScanFilterImpl> &filters);
    std::vector<uint8_t> GetAdvParamMsgData(uint8_t idx, const Uuid &uuid, int advHandle, int duration,
        const BleAdvertiserSettingsImpl &settings);
    std::vector<uint8_t> GetAdvDataMsgData(uint8_t idx, int advHandle, const Uuid &uuid,
        const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData);
    std::vector<uint8_t> GetRespDataMsgData(uint8_t idx, int advHandle, const Uuid &uuid,
        const BleAdvertiserDataImpl &respData);
    std::vector<uint8_t> GetUuidDataMsgData(uint8_t idx, const Uuid &uuid);
    std::vector<uint8_t> GetAdvDeviceInfoMsgData(uint8_t idx, const std::vector<BleActiveDeviceInfoImpl> &infoImpls);

    BleCentralManagerImpl &self_;
    std::atomic_bool isAddObserver_;

    bool isEnableSyncDataToLpDevice;
    bool isScanStartInLpDevice;
    bool isLpDeviceReset; // Whether the LpDevice is being reset. False means work normally.
    uint32_t scanIntervalInLp;
    uint32_t scanWindowInLp;
};

BleCentralManagerImpl::BleCentralManagerImpl(IBleCentralManagerCallback &callback)
    : callback_(&callback), status_(SCAN_NOT_STARTED),
      pimpl(std::make_unique<BleCentralManagerImpl::impl>(*this))
{
    btifBleScanner_ = get_ble_scanner_instance();
    bthwInterface_ = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    currentUsedTrackAdvs_ = 0;
}

BleCentralManagerImpl::~BleCentralManagerImpl()
{}

const bthwif_interface_t* BleCentralManagerImpl::GetBtHwInterface()
{
    if (bthwInterface_ != nullptr) {
        return bthwInterface_;
    }
    bthwInterface_ = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    return bthwInterface_;
}

void BleCentralManagerImpl::Start(void)
{
    // Add observer
    if (!pimpl->isAddObserver_.load()) {
        BluetoothGattInterface::GetInstance()->AddScannerObserver(*pimpl.get());
        BluetoothHwInterface::GetInstance()->AddSensorhubObserver(*pimpl.get());
        pimpl->isAddObserver_ = true;
    }

    if (btifBleScanner_) {
        btifBleScanner_->Scan(true);
    }

    status_ = SCAN_FAILED_ALREADY_STARTED;
    ReportStartScanEvent(SCAN_SUCCESS);

#ifdef COMM_PROTOCOL_RESOURCE_MGR_ENABLE
    BluetoothChannelRssiManager::GetInstance()->SendChannelRssiReadCommand();
#endif
}

void BleCentralManagerImpl::SetScanParametersCallback(uint8_t status)
{
    HILOGD("status: %{public}u", status);

    DoInBleThread([this, status]() {
        if (status != BT_STATUS_SUCCESS) {
            HILOGE("failed, status: %{public}u", status);
            status_ = SCAN_FAILED_INTERNAL_ERROR;
            // In jni thread, need switch to inner thread ?
            ReportStartScanEvent(SCAN_FAILED_INTERNAL_ERROR);
            return;
        }
        // Enable scan
        Start();
    });
}

void BleCentralManagerImpl::SetScanParameters(uint16_t intervalMSecs, uint16_t windowMSecs, bool legacy, int phy)
{
    int interval = intervalMSecs / BLE_SCAN_UNIT_TIME;
    int window = windowMSecs / BLE_SCAN_UNIT_TIME;
    if (btifBleScanner_) {
        btifBleScanner_->SetScanParameters(interval, window, legacy, phy,
            base::Bind(&BleCentralManagerImpl::SetScanParametersCallback, base::Unretained(this)));
    }
}

std::shared_ptr<std::promise<uint8_t>> g_promise = nullptr; // 全局变量

void InitializeBatchScanOperation()
{
    g_promise = std::make_shared<std::promise<uint8_t>>();
}

void PromiseCallback(std::weak_ptr<std::promise<uint8_t>> promiseWpr, uint8_t btmStatus)
{
    auto promise(promiseWpr.lock()); // Check whether the object exists.
    if (promise) {
        promise->set_value(btmStatus);
    }
}

static void WaitForBatchScanOperationEnd()
{
    auto future = g_promise->get_future();
    int waitTime = 1000;
    if (future.wait_for(std::chrono::milliseconds(waitTime)) != std::future_status::ready) {
        HILOGE("Timeout");
        return;
    }
}

void BleCentralManagerImpl::BatchscanConfigStorage(int clientIf, int batchScanFullMax, int batchScanTruncMax,
    int batchScanNotifyThreshold)
{
    InitializeBatchScanOperation();
    btifBleScanner_->BatchscanConfigStorage(clientIf, batchScanFullMax, batchScanTruncMax, batchScanNotifyThreshold,
        base::Bind(PromiseCallback, g_promise));
    WaitForBatchScanOperationEnd();
}

void BleCentralManagerImpl::BatchscanEnable(int scanMode, int scanInterval, int scanWindow, int addrType,
    int discardRule)
{
    InitializeBatchScanOperation();
    btifBleScanner_->BatchscanEnable(scanMode, scanInterval, scanWindow, addrType, discardRule,
        base::Bind(PromiseCallback, g_promise));
    WaitForBatchScanOperationEnd();
}

void BleCentralManagerImpl::BatchscanDisable()
{
    InitializeBatchScanOperation();
    btifBleScanner_->BatchscanDisable(base::Bind(PromiseCallback, g_promise));
    WaitForBatchScanOperationEnd();
}

void BleCentralManagerImpl::BatchscanReadReports(int clientIf, int scanMode)
{
    btifBleScanner_->BatchscanReadReports(clientIf, scanMode);
}

void BleCentralManagerImpl::StartScan(const BleScanSettingsImpl &setting)
{
    HILOGD("legacy: %{public}d, scanPhy: %{public}d, callbackType: %{public}d, sensitivityMode: %{public}d, "
        "scanInterval: %{public}d, scanWindow: %{public}d", setting.GetLegacy(), setting.GetPhy(),
        setting.GetCallbackType(), setting.GetSensitivityMode(), setting.GetScanInterval(), setting.GetScanWindow());

    if (IsScanStarted()) {
        HILOGE("Scan already started");
        ReportStartScanEvent(SCAN_FAILED_ALREADY_STARTED);
        return;
    }

    // Change scan mode to scan parameters
    uint16_t scanInterval = 0;
    uint16_t scanWindow = 0;
    int phy = 0;
    pimpl->GetScanParams(setting, scanInterval, scanWindow, phy);
    auto resourceMgr = BluetoothResourceManager::GetInstance();
    if (resourceMgr) {
        resourceMgr->UpdateScanRecord(setting.GetScanMode());
    }
    SetScanParameters(scanInterval, scanWindow, setting.GetLegacy(), phy);
    NotifyScanStatusToLpDevice(true);
    NotifyScanParamsToLpDevice(scanWindow, scanInterval);
}

void BleCentralManagerImpl::StopScan(void)
{
    if (!IsScanStarted()) {
        HILOGE("Scan not started");
        ReportStopScanEvent(SCAN_NOT_STARTED);
        return;
    }

    if (btifBleScanner_ != nullptr) {
        btifBleScanner_->Scan(false);
    }

    status_ = SCAN_NOT_STARTED;
    ReportStopScanEvent(SCAN_SUCCESS);
    NotifyScanStatusToLpDevice(false);
}

void BleCentralManagerImpl::StopScanWithDisabling(void)
{
    if (IsScanStarted()) {
        StopScan();
    }
    // BLE is disabled, clear scan filter status and delete scan filter indexs.
    std::lock_guard<std::mutex> lock(filterMuteLock_);
    for (auto &filter : filters_) {
        filter.DeleteScanFilters();
    }
    filters_.clear();
    scanFilterEnabled_ = false;
    ClearScannerId();
    HILOGI("filters_ size:%{public}zu", filters_.size());
    // When the filter client is disabled, the filter client is not deleted,
    // because the application still holds the clientId.
}

void BleCentralManagerImpl::Reset(void)
{
    std::lock_guard<std::mutex> lock(filterMuteLock_);
    // Clear resources
    filters_.clear();
    ClearScannerId();
    scanFilterEnabled_ = false;
    status_ = SCAN_NOT_STARTED;
    currentUsedTrackAdvs_ = 0;

    ClearShFilterIdx();
    pimpl->scanWindowInLp = 0;
    pimpl->scanIntervalInLp = 0;
    pimpl->isLpDeviceReset = false; // false means LpDevice work normally
    pimpl->isScanStartInLpDevice = false;
    pimpl->isEnableSyncDataToLpDevice = false;
}

void BleCentralManagerImpl::impl::GetScanParams(const BleScanSettingsImpl &setting,
    uint16_t &outScanInterval, uint16_t &outScanWindow, int &outPhy) const
{
    switch (setting.GetPhy()) {
        case PHY_LE_CODED:
            outPhy = BTM_PHY_LE_CODED;
            break;
        case PHY_LE_ALL_SUPPORTED:
            outPhy = BTM_PHY_LE_1M | BTM_PHY_LE_CODED;
            break;
        case PHY_LE_1M:
        case PHY_LE_2M:
        default:
            outPhy = BTM_PHY_LE_1M;
            break;
    }
    outScanInterval = setting.GetScanInterval();
    outScanWindow = setting.GetScanWindow();
}

int BleCentralManagerImpl::GetScanStatus() const
{
    return status_.load();
}

void BleCentralManagerImpl::DeregisterCallbackToGap(void)
{
    if (pimpl->isAddObserver_.load()) {
        BluetoothGattInterface::GetInstance()->RemoveScannerObserver(*pimpl.get());
        BluetoothHwInterface::GetInstance()->RemoveSensorhubObserver(*pimpl.get());
        pimpl->isAddObserver_ = false;
    }
}

void BleCentralManagerImpl::impl::CreateBlePeripheralDevice(BlePeripheralDevice &outDevice,
    uint16_t eventType, uint8_t addrType, const RawAddress &addr, int8_t rssi)
{
    outDevice.SetAddress(addr);
    outDevice.SetAddressType(addrType);
    outDevice.SetRSSI(rssi);
    if (!self_.IsConnectable(eventType)) { // default value is connectable, so set false
        outDevice.SetConnectable(false);
    }
    outDevice.SetEventType(eventType);
}

// called in jni thread
void BleCentralManagerImpl::impl::ScanResultCallback(uint16_t eventType, uint8_t addrType,
    const STACK::RawAddress *addr, int8_t rssi, std::vector<uint8_t> advData)
{
    if (addr == nullptr) {
        HILOGE("addr is empty");
        return;
    }

    // Bluedroid has combined advertising and scanning packages.
    DoInBleThread([this, eventType, addrType, rssi, advDataMove = std::move(advData),
        addr = ServiceUtil::AddrFromStack(*addr)]() {
            BlePeripheralDevice device;
            CreateBlePeripheralDevice(device, eventType, addrType, addr, rssi);
            if (advDataMove.size() > 0) {
                BlePeripheralDeviceParseAdvData parseAdvData = {
                    .payload = const_cast<uint8_t*>(advDataMove.data()),
                    .length = advDataMove.size(),
                };
                device.ParseAdvertiserment(parseAdvData);
            }

            BleScanResultImpl res;
            res.SetPeripheralDevice(device);
            if (self_.callback_) {
                self_.callback_->OnScanCallback(res);
            }
        });
}

void ParseBatchScanFullModeHead(RawAddress &address, uint8_t &addrType, int8_t &rssi,
    uint16_t &timestamp, uint8_t* &pos)
{
    STACK::RawAddress addr;
    STREAM_TO_BDADDR(addr, const_cast<const uint8_t *&>(pos));
    address = ServiceUtil::AddrFromStack(addr);
    STREAM_TO_UINT8(addrType, pos);
    pos++; // Tx_power
    STREAM_TO_INT8(rssi, pos);
    STREAM_TO_UINT16(timestamp, pos);
}

void BuildBlePeripheralDevice(BlePeripheralDevice& outDevice, const RawAddress &address,
    const uint8_t &addrType, const int8_t &rssi, const uint16_t &timestamp)
{
    outDevice.SetAddress(address);
    outDevice.SetAddressType(addrType);
    outDevice.SetRSSI(rssi);
    outDevice.SetTimestamp(timestamp);
}

void AddBatchScanResult(std::vector<Bluetooth::BluetoothBleScanResult> &batchResults, BlePeripheralDevice& device,
    std::vector<uint8_t> &payloadData)
{
    if (payloadData.size() > 0) {
        BlePeripheralDeviceParseAdvData parseAdvData = {
            .payload = const_cast<uint8_t*>(payloadData.data()),
            .length = payloadData.size(),
        };
        device.ParseAdvertiserment(parseAdvData);
    }
    BleScanResultImpl res;
    res.SetPeripheralDevice(device);
    batchResults.emplace_back(res);
}

std::vector<Bluetooth::BluetoothBleScanResult> ParseFullResults(int numRecords, std::vector<uint8_t> data)
{
    uint8_t* pos = data.data();
    uint8_t* end = data.data() + data.size();
    std::vector<Bluetooth::BluetoothBleScanResult> batchResults;
    for (int countRecords = 0; countRecords < numRecords; countRecords++) {
        RawAddress address;
        uint8_t addrType = 0;
        int8_t rssi = 0;
        uint8_t advPacketLen = 0;
        uint8_t scanDataRespLen = 0;
        uint16_t timestamp = 0;
        int headLen = 11;
        int maxPacketLen = 31;
        std::vector<uint8_t> payloadData;

        if ((end - pos) < headLen) {
            break;
        }
        ParseBatchScanFullModeHead(address, addrType, rssi, timestamp, pos);

        if ((end - pos) < 1) {
            break;
        }
        STREAM_TO_UINT8(advPacketLen, pos);
        if (advPacketLen > maxPacketLen || (end - pos) < advPacketLen) {
            break;
        }
        if (advPacketLen == 0) {
            continue; // such packet may cause problem on 06C
        }
        payloadData.resize(advPacketLen);
        for (int i = 0; i < advPacketLen; i++) {
            STREAM_TO_UINT8(payloadData[i], pos);
        }

        if ((end - pos) < 1) {
            break;
        }
        STREAM_TO_UINT8(scanDataRespLen, pos);
        if (scanDataRespLen > maxPacketLen || (end - pos) < scanDataRespLen) {
            break;
        }
        payloadData.resize(advPacketLen + scanDataRespLen);
        for (int i = 0; i < scanDataRespLen; i++) {
            STREAM_TO_UINT8(payloadData[i + advPacketLen], pos);
        }
        BlePeripheralDevice device;
        BuildBlePeripheralDevice(device, address, addrType, rssi, timestamp);
        AddBatchScanResult(batchResults, device, payloadData);
    }
    return batchResults;
}

void BleCentralManagerImpl::impl::BatchScanReportCallback(int clientIf, int status, int reportFormat, int numRecords,
    std::vector<uint8_t> data)
{
    DoInBleThread([this, clientIf, reportFormat, numRecords, data]() {
        int fullMode = BATCH_SCAN_FORMAT_FULL; // only support full mode currently
        if (reportFormat == fullMode) {
            std::vector<Bluetooth::BluetoothBleScanResult> batchResults = ParseFullResults(numRecords, data);
            if (self_.callback_) {
                self_.callback_->OnBleBatchScanResultsEvent(clientIf, batchResults);
            }
        }
    });
}

void BleCentralManagerImpl::impl::BatchScanThresholdCallback(int clientIf)
{
    // BT chip actively request to report results when storage reaches threshold
    DoInBleThread([clientIf, this]() {
        HILOGI("BT chip actively request to report results, storage reaches threshold.");
        int scanMode = BATCH_SCAN_FORMAT_FULL;
        self_.BatchscanReadReports(clientIf, scanMode);
    });
}

void BleCentralManagerImpl::impl::TrackAdvFoundLostCallback(const TrackAdvBaseInfo &info,
    const STACK::RawAddress &addr, std::vector<uint8_t> advData)
{
    DoInBleThread([this, advDataMove = std::move(advData), scanId = info.scanId, advertiserState = info.advertiserState,
        addrType = info.addrType, rssi = info.rssi, addr = ServiceUtil::AddrFromStack(addr)]() {
            BlePeripheralDevice device;
            CreateBlePeripheralDevice(device, 0, addrType, addr, rssi);
            if (advDataMove.size() > 0) {
                BlePeripheralDeviceParseAdvData parseAdvData = {
                    .payload = const_cast<uint8_t*>(advDataMove.data()),
                    .length = advDataMove.size(),
                };
                device.ParseAdvertiserment(parseAdvData);
            }
            BleScanResultImpl res;
            res.SetPeripheralDevice(device);
            if (self_.callback_) {
                self_.callback_->OnFoundOrLostCallback(scanId, advertiserState, res);
            }
        });
}

int32_t BleCentralManagerImpl::ConfigScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
    const std::vector<BleScanFilterImpl> &filters)
{
    HILOGD("scannerId: %{public}d, callbackType: %{public}d, sensitivityMode: %{public}d, \
        matchAdv: %{public}d, size:%{public}d", scannerId, settings.GetCallbackType(), settings.GetSensitivityMode(),
        settings.GetMatchTrackAdvType(), filters.size());
    uint16_t maxTotal = AdapterProperties::GetInstance()->GetTotalNumOfTrackableAdvertisements();
    uint16_t trackAdvs = GetNumOfTrackAdvs(settings.GetMatchTrackAdvType());
    if (GetDeliveryMode(settings) == BleScanFilter::DELIVERY_MODE_ON_FOUND) {
        if (trackAdvs * static_cast<uint16_t>(filters.size()) > (maxTotal - currentUsedTrackAdvs_)) {
            HILOGE("No hardware resources for onfound/onlost filter %{public}d", trackAdvs);
            return Bluetooth::BT_ERR_BLE_SCAN_NO_RESOURCE;
        }
        currentUsedTrackAdvs_ += trackAdvs * filters.size();
    }
    std::lock_guard<std::mutex> lock(filterMuteLock_);
    auto iter = GetScanFilterIter(scannerId);
    if (iter == filters_.end()) {
        iter = filters_.emplace(filters_.end(), scannerId, btifBleScanner_);
    } else {
        HILOGE("Already config filter scannerId :%{public}d", scannerId);
        return Bluetooth::BT_ERR_BLE_SCAN_ALREADY_STARTED;
    }

    auto ret = iter->AddScanFilters(filters, GetDeliveryMode(settings),
        settings.GetSensitivityMode(), trackAdvs);
    if (ret != GATT_SUCCESS) {
        HILOGE("Add scan filter failed");
        iter->DeleteScanFilters();
        filters_.erase(iter);
        return ret;
    }

    // Enable scan filter if not
    if (!scanFilterEnabled_.load() && btifBleScanner_) {
        btifBleScanner_->ScanFilterEnable(true, base::Bind(
            [](BleCentralManagerImpl *self, uint8_t action, uint8_t btmStatus) {
                HILOGI("ScanFilterEnable: action: %{public}u, btmStatus: %{public}u", action, btmStatus);
                self->scanFilterEnabled_ = true;
                if (btmStatus != BTM_SUCCESS) {
                    BtChrBtExcpEvent("", BTOPT_BLE_SCAN_FILTER_FAIL, btmStatus);
                }
            },
            base::Unretained(this)));
    }

    return GATT_SUCCESS;
}

void BleCentralManagerImpl::RemoveScanFilter(int32_t scannerId,
    const BleScanSettingsImpl &settings, uint16_t filtersSize)
{
    std::lock_guard<std::mutex> lock(filterMuteLock_);
    auto iter = GetScanFilterIter(scannerId);
    if (iter != filters_.end()) {
        if (GetDeliveryMode(settings) == BleScanFilter::DELIVERY_MODE_ON_FOUND) {
            currentUsedTrackAdvs_ -= GetNumOfTrackAdvs(settings.GetMatchTrackAdvType()) * filtersSize;
        }
        iter->DeleteScanFilters();
        filters_.erase(iter);
    }
}

int32_t BleCentralManagerImpl::AllocScannerId()
{
    int32_t scannerId = 0;
    std::lock_guard<std::mutex> lk(scannerIdsMutex_);
    for (int32_t i = 1; i <= BLE_SCAN_MAX_NUM; i++) {
        if (scannerIds_.find(i) == scannerIds_.end()) {
            scannerId = i;
            scannerIds_.insert(scannerId);
            break;
        }
    }
    HILOGI("scannerId: %{public}d", scannerId);
    return scannerId;
}

void BleCentralManagerImpl::RemoveScannerId(int32_t scannerId)
{
    std::lock_guard<std::mutex> lk(scannerIdsMutex_);
    HILOGI("RemoveScannerId. scannerId: %{public}d", scannerId);
    scannerIds_.erase(scannerId);
}

void BleCentralManagerImpl::ClearScannerId()
{
    std::lock_guard<std::mutex> lk(scannerIdsMutex_);
    HILOGI("ClearScannerId.");
    scannerIds_.clear();
}

void ConvertBleScanMode(int scanMode, uint16_t &outScanWindow, uint16_t &outScanInterval)
{
    // scanMode <-> { interval, window }
    static std::map<int, std::pair<uint16_t, uint16_t>> table {
        { SCAN_MODE_LOW_POWER, { BLE_SCAN_MODE_LOW_POWER_INTERVAL_MS, BLE_SCAN_MODE_LOW_POWER_WINDOW_MS }},
        { SCAN_MODE_BALANCED, { BLE_SCAN_MODE_BALANCED_INTERVAL_MS, BLE_SCAN_MODE_BALANCED_WINDOW_MS }},
        { SCAN_MODE_LOW_LATENCY, { BLE_SCAN_MODE_LOW_LATENCY_INTERVAL_MS, BLE_SCAN_MODE_LOW_LATENCY_WINDOW_MS }},
        { SCAN_MODE_OP_P2_60_3000, { BLE_SCAN_MODE_OP_P2_60_3000_INTERVAL_MS, BLE_SCAN_MODE_OP_P2_60_3000_WINDOW_MS }},
        { SCAN_MODE_OP_P10_60_600, { BLE_SCAN_MODE_OP_P10_60_600_INTERVAL_MS, BLE_SCAN_MODE_OP_P10_60_600_WINDOW_MS }},
        { SCAN_MODE_OP_P25_60_240, { BLE_SCAN_MODE_OP_P25_60_240_INTERVAL_MS, BLE_SCAN_MODE_OP_P25_60_240_WINDOW_MS }},
        { SCAN_MODE_OP_P100_1000_1000,
            { BLE_SCAN_MODE_OP_P100_1000_1000_INTERVAL_MS, BLE_SCAN_MODE_OP_P100_1000_1000_WINDOW_MS }},
        { SCAN_MODE_OP_P50_100_200,
            {BLE_SCAN_MODE_OP_P50_100_200_INTERVAL_MS, BLE_SCAN_MODE_OP_P50_100_200_WINDOW_MS}},
        { SCAN_MODE_OP_P10_30_300, { BLE_SCAN_MODE_OP_P10_30_300_INTERVAL_MS, BLE_SCAN_MODE_OP_P10_30_300_WINDOW_MS }},
        { SCAN_MODE_OP_P2_30_1500, {BLE_SCAN_MODE_OP_P2_30_1500_INTERVAL_MS, BLE_SCAN_MODE_OP_P2_30_1500_WINDOW_MS }},
        { SCAN_MODE_OP_P75_30_40, {BLE_SCAN_MODE_OP_P75_30_40_INTERVAL_MS, BLE_SCAN_MODE_OP_P75_30_40_WINDOW_MS }},
        { SCAN_MODE_OP_P50_30_60, {BLE_SCAN_MODE_OP_P50_30_60_INTERVAL_MS, BLE_SCAN_MODE_OP_P50_30_60_WINDOW_MS }},
        { SCAN_MODE_OP_P10_25_250, {BLE_SCAN_MODE_OP_P10_25_250_INTERVAL_MS, BLE_SCAN_MODE_OP_P10_25_250_WINDOW_MS }}
    };
    auto iter = table.find(scanMode);
    if (iter != table.end()) {
        auto [interval, window] = iter->second;
        outScanInterval = interval;
        outScanWindow = window;
    } else {
        HILOGW("Unknown scan mode: %{public}d, set default balanced mode", scanMode);
        outScanInterval = BLE_SCAN_MODE_BALANCED_INTERVAL_MS;
        outScanWindow = BLE_SCAN_MODE_BALANCED_WINDOW_MS;
    }
}

void BleCentralManagerImpl::impl::SensorhubDevInfoCallback(uint8_t *buffer, int length)
{
    HILOGD("SensorhubDevInfoCallback.");
    if (buffer == nullptr || length < BLE_DATA_LEN_1) {
        HILOGE("buffer is null or length is wrong");
        return;
    }

    uint8_t filterIdx = 0;
    uint8_t msgType = 0;
    int32_t msgDataLen = 0;
    int pos = 0;

    uint8_t noticeType = buffer[pos++];
    if (noticeType == BLE_LPDEVICE_MSG_TYPE_NOTICE_UP_DATA) {
        if (length < BLE_DATA_LEN_4) {
            HILOGE("SensorhubDevInfoCallback, length wrong.");
            return;
        }
        filterIdx = buffer[pos++];
        msgType = buffer[pos++];
        msgDataLen = length - pos;
    } else {
        HILOGE("SensorhubDevInfoCallback, noticeType not found.");
        return;
    }
    std::vector<uint8_t> notifyValue(buffer[pos], buffer[pos] + msgDataLen);
    auto [info, isExist] = self_.GetShFilterIdx(filterIdx);
    if (!isExist) {
        HILOGE("SensorhubDevInfoCallback, filterIdx not found.");
        return;
    }
    HILOGD("SensorhubDevInfoCallback msgType: %{public}d, filterIdx: %{public}d", msgType, filterIdx);
    DoInBleThread([this, msgType, infoCopy = info, notifyValue = std::move(notifyValue)]() {
        if (self_.callback_) {
            self_.callback_->OnNotifyMsgReportFromLpDevice(infoCopy, msgType, notifyValue);
        }
    });
}

void BleCentralManagerImpl::impl::SensorhubResetCallback(uint32_t state)
{
    HILOGI("SensorhubResetCallback state: %{public}d", state);
    if (state == BLE_LPDEVICE_RECOVERY_DOING) {
        isLpDeviceReset = true;
    } else if (state == BLE_LPDEVICE_RECOVERY_DONE) {
        isLpDeviceReset = false;
    } else {
        HILOGE("[SensorhubResetCallback], state error.");
    }
    self_.ClearShFilterIdx();
}

void BleCentralManagerImpl::impl::AppendUuidToByteArray(const Uuid &uuid, std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> tmpvec {};
    std::array<uint8_t, BLE_UUID_LEN> uuidData = uuid.ConvertTo128Bits();

    tmpvec.insert(tmpvec.end(), uuidData.begin(), uuidData.end());
    std::reverse(tmpvec.begin(), tmpvec.begin() + BLE_UUID_HALF_LEN);
    std::reverse(tmpvec.begin() + BLE_UUID_HALF_LEN, tmpvec.end());
    retValue.insert(retValue.end(), tmpvec.begin(), tmpvec.end());
}

void BleCentralManagerImpl::impl::AppendDataToByteArray(int32_t data, uint8_t dataLen, std::vector<uint8_t> &retValue)
{
    for (int i = 0; i < dataLen; i++) {
        retValue.push_back(static_cast<uint8_t>((data >> (i * BLE_ONE_BYTE_LEN)) & 0xFF));
    }
}

void BleCentralManagerImpl::impl::AppendDeviceAddressToByteArray(const std::vector<BleScanFilterImpl> &filters,
    std::vector<uint8_t> &retValue)
{
    int deviceNameNums = 0;
    std::vector<uint8_t> tempVec;
    for (auto filter : filters) {
        std::string addr = filter.GetDeviceId();
        if (addr.empty()) {
            HILOGD("addr is empty.");
            continue;
        }
        STACK::RawAddress rawAddress = ServiceUtil::AddrToStack(RawAddress(addr));
        if (rawAddress.IsEmpty()) {
            HILOGI("rawAddress is empty.");
            continue;
        }
        tempVec.push_back(BLE_LPDEVICE_DEVICE_TYPE_ALL);
        tempVec.insert(tempVec.end(), rawAddress.address, rawAddress.address + BLE_DEVICE_ADDR_LEN);
        deviceNameNums++;
    }
    HILOGD("deviceNameNums: %{public}d", deviceNameNums);
    retValue.push_back(deviceNameNums);
    retValue.insert(retValue.end(), tempVec.begin(), tempVec.end());
}

void BleCentralManagerImpl::impl::AppendUuidAndMaskToByteArray(const Uuid &uuid, const Uuid &uuidMask,
    bool isMaskExist, std::vector<uint8_t> &retValue)
{
    int typeLen = uuid.GetUuidType();
    retValue.push_back(typeLen + typeLen); // length
    std::array<uint8_t, BLE_UUID_LEN> uuidData = uuid.ConvertTo128Bits();
    std::array<uint8_t, BLE_UUID_LEN> uuidMaskData = uuidMask.ConvertTo128Bits();
    retValue.insert(retValue.end(), uuidData.begin(), uuidData.begin() + typeLen);
    if (!isMaskExist) {
        retValue.insert(retValue.end(), typeLen, 0);
    } else {
        retValue.insert(retValue.end(), uuidMaskData.begin(),
            uuidMaskData.begin() + typeLen);
    }
}

void BleCentralManagerImpl::impl::AppendSrvUuidListToByteArray(const std::vector<BleScanFilterImpl> &filters,
    std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> tempVec {};
    int srvUuidNum = 0;
    for (auto filter : filters) {
        if (!filter.HasServiceUuid()) {
            HILOGD("not has ServiceUuid.");
            continue;
        }
        AppendUuidAndMaskToByteArray(filter.GetServiceUuid(), filter.GetServiceUuidMask(),
            filter.HasServiceUuidMask(), tempVec);
        srvUuidNum++;
    }
    HILOGD("srvUuidNum: %{public}d", srvUuidNum);
    retValue.push_back(srvUuidNum);
    retValue.insert(retValue.end(), tempVec.begin(), tempVec.end());
}

void BleCentralManagerImpl::impl::AppendSolicUuidListToByteArray(const std::vector<BleScanFilterImpl> &filters,
    std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> tempVec {};
    int solicUuidNum = 0;
    for (auto filter : filters) {
        if (!filter.HasSolicitationUuid()) {
            HILOGD("not has SolicitationUuid.");
            continue;
        }
        AppendUuidAndMaskToByteArray(filter.GetServiceSolicitationUuid(), filter.GetServiceSolicitationUuidMask(),
            filter.HasSolicitationUuidMask(), tempVec);
        solicUuidNum++;
    }
    HILOGD("solicUuidNum: %{public}d", solicUuidNum);
    retValue.push_back(solicUuidNum);
    retValue.insert(retValue.end(), tempVec.begin(), tempVec.end());
}

void BleCentralManagerImpl::impl::AppendLocalNameToByteArray(const std::vector<BleScanFilterImpl> &filters,
    std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> tempVec {};
    int localNameNum = 0;
    for (auto filter : filters) {
        std::string localName = filter.GetName();
        if (localName.empty()) {
            HILOGD("localName is empty.");
            continue;
        }
        tempVec.push_back(localName.length());
        tempVec.insert(tempVec.end(), localName.begin(), localName.end());
        localNameNum++;
    }
    HILOGD("localNameNum: %{public}d", localNameNum);
    retValue.push_back(localNameNum);
    retValue.insert(retValue.end(), tempVec.begin(), tempVec.end());
}

void BleCentralManagerImpl::impl::AppendManufDataListToByteArray(const std::vector<BleScanFilterImpl> &filters,
    std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> tempVec {};
    int manufDataNum = 0;
    std::vector<uint8_t> ManufactureData {};
    std::vector<uint8_t> ManufactureDataMask {};

    for (auto filter : filters) {
        uint16_t manufacturerId = filter.GetManufacturerId();
        if (manufacturerId == 0) {
            HILOGD("manufacturerId is 0.");
            continue;
        }
        ManufactureData = filter.GetManufactureData();
        ManufactureDataMask = filter.GetManufactureDataMask();
        int len = BLE_DATA_LEN_4; // manufacturerId 和mask需填充id长度
        if (!ManufactureData.empty()) {
            if (!ManufactureDataMask.empty()) {
                len += ManufactureData.size() + ManufactureDataMask.size();
            } else {
                len += ManufactureData.size() + ManufactureData.size();
            }
        }
        tempVec.push_back(len);
        AppendDataToByteArray(manufacturerId, BLE_DATA_LEN_2, tempVec);
        if (!ManufactureData.empty()) {
            tempVec.insert(tempVec.end(), ManufactureData.begin(), ManufactureData.end());
        }
        tempVec.push_back(0xFF);
        tempVec.push_back(0xFF);
        if (!ManufactureData.empty()) {
            if (!ManufactureDataMask.empty()) {
                tempVec.insert(tempVec.end(), ManufactureDataMask.begin(),
                    ManufactureDataMask.end());
            } else {
                tempVec.insert(tempVec.end(), ManufactureData.size(), 0xFF);
            }
        }
        manufDataNum++;
    }
    HILOGD("manufDataNum: %{public}d", manufDataNum);
    retValue.push_back(manufDataNum);
    retValue.insert(retValue.end(), tempVec.begin(), tempVec.end());
}

void BleCentralManagerImpl::impl::AppendSrvDataListToByteArray(const std::vector<BleScanFilterImpl> &filters,
    std::vector<uint8_t> &retValue)
{
    std::vector<uint8_t> tempVec;
    int SrvDataNum = 0;
    std::vector<uint8_t> serviceData {};
    std::vector<uint8_t> serviceDataMask {};

    for (auto filter : filters) {
        serviceData = filter.GetServiceData();
        serviceDataMask = filter.GetServiceDataMask();
        if (serviceData.empty()) {
            HILOGD("serviceData is empty.");
            continue;
        }
        int len = serviceData.size();
        if (!serviceDataMask.empty()) {
            len += serviceDataMask.size();
        } else {
            len += serviceData.size();
        }
        tempVec.push_back(len);
        tempVec.insert(tempVec.end(), serviceData.begin(), serviceData.end());
        if (!filter.GetServiceDataMask().empty()) {
            tempVec.insert(tempVec.end(), serviceDataMask.begin(), serviceDataMask.end());
        } else {
            tempVec.insert(tempVec.end(), serviceData.size(), 0xFF);
        }
        SrvDataNum++;
    }
    HILOGD("SrvDataNum: %{public}d", SrvDataNum);
    retValue.push_back(SrvDataNum);
    retValue.insert(retValue.end(), tempVec.begin(), tempVec.end());
}

std::vector<uint8_t> BleCentralManagerImpl::impl::GetAdvHandleMsgData(const Uuid &uuid, int advHandle)
{
    std::vector<uint8_t> retValue {};
    AppendUuidToByteArray(uuid, retValue);
    AppendDataToByteArray(advHandle, BLE_DATA_LEN_4, retValue);
    return retValue;
}

std::vector<uint8_t> BleCentralManagerImpl::impl::GetSoftFilterMsgData(uint8_t idx, int deliveryMode, const Uuid &uuid,
    const std::vector<BleScanFilterImpl> &filters)
{
    HILOGI("size: %{public}zu", filters.size());
    std::vector<uint8_t> retValue {};
    AppendUuidToByteArray(uuid, retValue);
    retValue.push_back(0x01); // 1 byte filter item num. default value 1
    retValue.push_back(idx); // 1 byte filter_index
    AppendDataToByteArray(0x1000, BLE_DATA_LEN_2, retValue); // 2 bytes apcf_feature_selection
    AppendDataToByteArray(0, BLE_DATA_LEN_2, retValue); // 2 bytes apcf_list_logic_type mScanSettings.getListLogicType()
    retValue.push_back(0); // 1 byte apcf_logic_type  mScanSettings.getFilterLogicType()
    retValue.push_back(0); // 1 byte rssi_high_htresh  mScanSettings.getRssiHighValue()
    retValue.push_back(deliveryMode); // 1 byte delivery_mode
    AppendDataToByteArray(0, BLE_DATA_LEN_2, retValue); // 2 bytes onfound_timeout
    retValue.push_back(0); // 1 byte onfound_timeout_cnt
    retValue.push_back(0); // 1 byte rssi_low_thresh
    AppendDataToByteArray(0, BLE_DATA_LEN_2, retValue); // 2 bytes onlost_timeout
    AppendDataToByteArray(0, BLE_DATA_LEN_2, retValue); // 2 bytes num_of_tracking_entries
    AppendDeviceAddressToByteArray(filters, retValue);
    AppendSrvUuidListToByteArray(filters, retValue);
    AppendSolicUuidListToByteArray(filters, retValue);
    AppendLocalNameToByteArray(filters, retValue);
    AppendManufDataListToByteArray(filters, retValue);
    AppendSrvDataListToByteArray(filters, retValue);
    return retValue;
}

std::vector<uint8_t> BleCentralManagerImpl::impl::GetAdvParamMsgData(uint8_t idx, const Uuid &uuid, int advHandle,
    int duration, const BleAdvertiserSettingsImpl &settings)
{
    std::vector<uint8_t> retValue {};
    AppendUuidToByteArray(uuid, retValue);
    retValue.push_back(0x01); // 1 byte filter item num. default value 1
    retValue.push_back(idx); // 1 byte filter_index
    retValue.push_back(advHandle);  // 1 byte advertising handle
    short advEvtProp = 0;
    if (settings.IsConnectable()) {
        advEvtProp |= 0x01;
    }
    if (settings.IsLegacyMode()) {
        advEvtProp |= 0x10;
    }
    if (settings.GetTxPower()) {
        advEvtProp |= 0x40;
    }
    AppendDataToByteArray(advEvtProp, BLE_DATA_LEN_2, retValue); // advertising_event_properties
    AppendDataToByteArray(settings.GetInterval(), BLE_DATA_LEN_3, retValue); // min
    AppendDataToByteArray(settings.GetInterval(), BLE_DATA_LEN_3, retValue); // max is same as min
    retValue.push_back(0x07); // primary_advertising_channel_map 0x07-all channels
    retValue.push_back(0x01); // own_address_type--BLE_ADDR_RANDOM
    retValue.push_back(0x00); // peer_address_type
    retValue.insert(retValue.end(), BLE_DEVICE_ADDR_LEN, 0x00); // peer_address
    retValue.push_back(0x00); // advertising_filter_poliy
    retValue.push_back(settings.GetTxPower()); // advertising_tx_power
    retValue.push_back(settings.GetPrimaryPhy()); // primary_advertising_phy
    retValue.push_back(0x00); // secondary_advertising_max_skip
    retValue.push_back(settings.GetSecondaryPhy()); // secondary_advertising_phy
    retValue.push_back(0x01); // advertising_sid
    retValue.push_back(0x00); // scan_request_notification_enable
    AppendDataToByteArray(duration, BLE_DATA_LEN_2, retValue);
    return retValue;
}

std::vector<uint8_t> BleCentralManagerImpl::impl::GetAdvDataMsgData(uint8_t idx, int advHandle, const Uuid &uuid,
    const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData)
{
    std::vector<uint8_t> retValue {};
    AppendUuidToByteArray(uuid, retValue);
    retValue.push_back(0x01); // 1 byte filter item num. default value 1
    retValue.push_back(idx); // 1 byte filter_index
    retValue.push_back(advHandle); // 1 byte advertising handle
    retValue.push_back(0x03); // operation 3--COMPLETE
    retValue.push_back(0x01); // fragmen_preference,
    std::string payload = advData.GetPayload();
    if (settings.IsConnectable()) {
        retValue.push_back(payload.length() + BLE_DATA_LEN_3);
        retValue.push_back(BLE_DATA_LEN_2); // len 2 bytes
        retValue.push_back(BLE_AD_TYPE_FLAG); // HCI_EIR_FLAGS_TYPE
        retValue.push_back(BLE_ADV_FLAG_GEN_DISC); // BTM_GENERAL_DISCOVERABLE
    } else {
        retValue.push_back(payload.length());
    }
    retValue.insert(retValue.end(), payload.begin(), payload.end());
    return retValue;
}

std::vector<uint8_t> BleCentralManagerImpl::impl::GetRespDataMsgData(uint8_t idx, int advHandle, const Uuid &uuid,
    const BleAdvertiserDataImpl &respData)
{
    std::vector<uint8_t> retValue {};
    AppendUuidToByteArray(uuid, retValue);
    retValue.push_back(0x01); // 1 byte filter item num. default value 1
    retValue.push_back(idx); // 1 byte filter_index
    retValue.push_back(advHandle); // 1 byte advertising handle
    retValue.push_back(0x03); // operation 3--COMPLETE
    retValue.push_back(0x01); // fragmen_preference,
    std::string payload = respData.GetPayload();
    retValue.push_back(payload.length());
    retValue.insert(retValue.end(), payload.begin(), payload.end());
    return retValue;
}

std::vector<uint8_t> BleCentralManagerImpl::impl::GetUuidDataMsgData(uint8_t idx, const Uuid &uuid)
{
    std::vector<uint8_t> retValue {};
    AppendUuidToByteArray(uuid, retValue);
    retValue.push_back(idx); // 1 byte filter_index
    return retValue;
}

std::vector<uint8_t> BleCentralManagerImpl::impl::GetAdvDeviceInfoMsgData(uint8_t idx,
    const std::vector<BleActiveDeviceInfoImpl> &infoImpls)
{
    std::vector<uint8_t> retValue {};
    retValue.push_back(0x01); // 1 byte filter item num. default value 1
    retValue.push_back(idx); // 1 byte filter_index
    retValue.push_back(infoImpls.size());
    for (auto info : infoImpls) {
        retValue.insert(retValue.end(), info.deviceId.begin(), info.deviceId.end());
        retValue.push_back(info.status);
        AppendDataToByteArray(info.timeOut, BLE_DATA_LEN_2, retValue);
    }
    return retValue;
}

void BleCentralManagerImpl::SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval,
    int advHandle)
{
    HILOGI("SetLpDeviceAdvParam. duration: %{public}d, maxExtAdvEvents: %{public}d, Window: %{public}d,\
        interval: %{public}d, advHandle: %{public}d", duration, maxExtAdvEvents, window, interval, advHandle);
    if (GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return;
    }
    GetBtHwInterface()->setBurstParam(duration, maxExtAdvEvents, window, interval, advHandle);
}

void BleCentralManagerImpl::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    HILOGD("scannerId: %{public}d, enable: %{public}d", scannerId, enable);
    if (GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return;
    }
    std::lock_guard<std::mutex> lock(filterMuteLock_);
    auto iter = GetScanFilterIter(scannerId);
    if (iter == filters_.end()) {
        HILOGE("filters_ not exist, scannerId: %{public}d", scannerId);
        return;
    }
    // 0:report msg to low power device; 1:report msg to AP
    uint8_t reportChannel = 1;
    if (enable) {
        reportChannel = 0;
    }

    std::set<int> filterIndexs = iter->GetFilterIndexs();
    std::stringstream ss;
    for (int index : filterIndexs) {
        ss << index << " ";
        GetBtHwInterface()->setScanReportChannelSensorHub(index, reportChannel);
    }
    HILOGD("filterIndex: %{public}s", ss.str().c_str());
}

void BleCentralManagerImpl::EnableSyncDataToLpDevice()
{
    HILOGI("EnableSyncDataToLpDevice.");
    pimpl->isEnableSyncDataToLpDevice = true;
}

void BleCentralManagerImpl::DisableSyncDataToLpDevice()
{
    pimpl->isEnableSyncDataToLpDevice = false;
}

void BleCentralManagerImpl::SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type)
{
    HILOGD("SendParamsToLpDevice. type: %{public}d, dataLen: %{public}zu", type, dataValue.size());
    if (GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return;
    }
    GetBtHwInterface()->sendParamsToSensorhub(reinterpret_cast<const char*>(dataValue.data()), type,
        dataValue.size());
}

bool BleCentralManagerImpl::IsLpDeviceAvailable()
{
    HILOGI("IsLpDeviceAvailable.");
    if (GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return false;
    }
    bool isSupp = GetBtHwInterface()->getSensorhubCap();
    HILOGI("isSupp: %{public}d, isLpDeviceReset: %{public}d", isSupp, pimpl->isLpDeviceReset);
    if (isSupp && !pimpl->isLpDeviceReset) {
        return true;
    }
    return false;
}

void BleCentralManagerImpl::NotifyScanStatusToLpDevice(bool isStart)
{
    HILOGD("NotifyScanStatusToLpDevice.isStart: %{public}d", isStart);
    if (!pimpl->isEnableSyncDataToLpDevice) {
        HILOGD("isEnableSyncDataToLpDevice is false.");
        return;
    }
    if (isStart == pimpl->isScanStartInLpDevice) {
        HILOGD("isScan: %{public}d, isScanStartInLpDevice: %{public}d", isStart, pimpl->isScanStartInLpDevice);
        return;
    }
    pimpl->isScanStartInLpDevice = isStart;
    int8_t scanStatus = isStart ? BLE_LPDEVICE_ENABLE_SCAN_IN_LPDEVICE : BLE_LPDEVICE_DISABLE_SCAN_IN_LPDEVICE;

    std::vector<uint8_t> scanStatusValue {};
    scanStatusValue.push_back(scanStatus);
    SendParamsToLpDevice(scanStatusValue, BLE_LPDEVICE_MSG_SCAN_STATUS);
}

void BleCentralManagerImpl::NotifyScanParamsToLpDevice(uint32_t scanWindow, uint32_t scanInterval)
{
    HILOGD("NotifyScanParamsToLpDevice. scanWindow: %{public}d, scanInterval: %{public}d", scanWindow, scanInterval);
    if (!pimpl->isEnableSyncDataToLpDevice) {
        HILOGD("isEnableSyncDataToLpDevice is false.");
        return;
    }
    if (scanWindow == pimpl->scanWindowInLp && scanInterval == pimpl->scanIntervalInLp) {
        HILOGD("Window: %{public}d, Interval: %{public}d, WindowInLp: %{public}d, IntervalInLp: %{public}d",
            scanWindow, scanInterval, pimpl->scanWindowInLp, pimpl->scanIntervalInLp);
        return;
    }
    pimpl->scanWindowInLp = scanWindow;
    pimpl->scanIntervalInLp = scanInterval;

    std::vector<uint8_t> scanParamValue {};
    pimpl->AppendDataToByteArray(scanWindow, BLE_DATA_LEN_4, scanParamValue);
    pimpl->AppendDataToByteArray(scanInterval, BLE_DATA_LEN_4, scanParamValue);
    SendParamsToLpDevice(scanParamValue, BLE_LPDEVICE_MSG_SCAN_PARAM);
}

uint8_t BleCentralManagerImpl::GetLpDeviceAdvHandle()
{
    if (GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return 0;
    }
    return GetBtHwInterface()->getSensorhubAdvHandle();
}

static void AdvFilterParamLog(const std::string &tag, const std::vector<uint8_t> &data)
{
    const std::string hex = "0123456789ABCDEF";
    const uint8_t sizeFour = 4;
    std::stringstream ss;

    ss << "0x";
    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t n = (data[i]);
        ss << hex[n >> sizeFour] << hex[n & 0xF];
    }

    HILOGD("%{public}s : %{public}s", tag.c_str(), ss.str().c_str());
}

void BleCentralManagerImpl::SetLpDeviceParam(const FilterIdxInfo &filterIdx, const BleLpDeviceParamSetImpl &paramImpl)
{
    HILOGI("PID:%{public}d, UID:%{public}d, uuid:%{public}s", filterIdx.pid, filterIdx.uid,
        filterIdx.uuid.ToString().c_str());
    if (pimpl->isLpDeviceReset) {
        HILOGE("low power device state is reset.");
        return;
    }
    auto [idx, isExist] = FindShFilterIdx(filterIdx);
    if (!isExist) {
        auto [newIdx, isOk] = AddShFilterIdx(filterIdx);
        if (!isOk) {
            HILOGE("AddShFilterIdx fail.");
            return;
        }
        idx = newIdx;
    }
    HILOGI("Idx:%{public}d", idx);
    SendAdvHandleDataToLpDevice(idx, filterIdx.uuid, paramImpl);
    SendScanDataToLpDevice(idx, filterIdx.uuid, paramImpl);
    SendAdvertiserDataToLpDevice(idx, filterIdx.uuid, paramImpl);
    SendUuidDataToLpDevice(idx, filterIdx.uuid);
}

void BleCentralManagerImpl::SendAdvHandleDataToLpDevice(uint8_t idx, const Uuid &uuid,
    const BleLpDeviceParamSetImpl &paramImpl)
{
    // BLE_LPDEVICE_MSG_ADV_HANDLE
    std::vector<uint8_t> handleValue {};
    if (paramImpl.advHandle == BLE_LPDEVICE_HANDLE_RESOURCE_NOT_APPLY) {
        uint8_t handle = GetLpDeviceAdvHandle();
        if (handle <= 1) {
            HILOGI("handle is error: %{public}d", handle);
            return;
        }
        handleValue.push_back(handle - 1);
        AdvFilterParamLog("advHandle2", handleValue);
        SendParamsToLpDevice(handleValue, BLE_LPDEVICE_MSG_ADV_HANDLE);

        if ((paramImpl.fieldValidFlagBit & BLE_LPDEVICE_ADV_DEVICEINFO_VALID_BIT) != 0) {
            std::vector<uint8_t> deviceInfoValue = pimpl->GetAdvDeviceInfoMsgData(idx, paramImpl.activeDeviceInfoImpls);
            AdvFilterParamLog("deviceInfo", deviceInfoValue);
            SendParamsToLpDevice(deviceInfoValue, BLE_LPDEVICE_MSG_DEV_INFO);
        }
    } else {
        handleValue = pimpl->GetAdvHandleMsgData(uuid, paramImpl.advHandle);
        AdvFilterParamLog("advHandle", handleValue);
        SendParamsToLpDevice(handleValue, BLE_LPDEVICE_MSG_ADV_HANDLE);
    }
}

void BleCentralManagerImpl::SendScanDataToLpDevice(uint8_t idx, const Uuid &uuid,
    const BleLpDeviceParamSetImpl &paramImpl)
{
    // BLE_LPDEVICE_MSG_SOFT_FILTER
    if ((paramImpl.fieldValidFlagBit & BLE_LPDEVICE_SCAN_SETTING_VALID_BIT) == 0 ||
        (paramImpl.fieldValidFlagBit & BLE_LPDEVICE_SCAN_FILTER_VALID_BIT) == 0) {
        return;
    }
    std::vector<uint8_t> filterValue = pimpl->GetSoftFilterMsgData(idx, paramImpl.deliveryMode, uuid,
        paramImpl.scanFliterImpls);
    AdvFilterParamLog("ScanData", filterValue);
    SendParamsToLpDevice(filterValue, BLE_LPDEVICE_MSG_SOFT_FILTER);
}

void BleCentralManagerImpl::SendAdvertiserDataToLpDevice(uint8_t idx, const Uuid &uuid,
    const BleLpDeviceParamSetImpl &paramImpl)
{
    // BLE_LPDEVICE_MSG_ADV_PARAM
    if ((paramImpl.fieldValidFlagBit & BLE_LPDEVICE_ADV_SETTING_VALID_BIT) != 0) {
        std::vector<uint8_t> advParamValue = pimpl->GetAdvParamMsgData(idx, uuid, paramImpl.advHandle,
            paramImpl.duration, paramImpl.advSettingsImpl);
        AdvFilterParamLog("advParam", advParamValue);
        SendParamsToLpDevice(advParamValue, BLE_LPDEVICE_MSG_ADV_PARAM);
    }

    // BLE_LPDEVICE_MSG_ADV_DATA
    if ((paramImpl.fieldValidFlagBit & BLE_LPDEVICE_ADV_SETTING_VALID_BIT) != 0 &&
        (paramImpl.fieldValidFlagBit & BLE_LPDEVICE_ADVDATA_VALID_BIT) != 0) {
        std::vector<uint8_t> advDataValue = pimpl->GetAdvDataMsgData(idx, paramImpl.advHandle, uuid,
            paramImpl.advSettingsImpl, paramImpl.advDataImpl);
        AdvFilterParamLog("advData", advDataValue);
        SendParamsToLpDevice(advDataValue, BLE_LPDEVICE_MSG_ADV_DATA);
    }

    // BLE_LPDEVICE_MSG_ADV_RSP_DATA
    if ((paramImpl.fieldValidFlagBit & BLE_LPDEVICE_RESPDATA_VALID_BIT) != 0) {
        std::vector<uint8_t> respDataValue = pimpl->GetRespDataMsgData(idx, paramImpl.advHandle, uuid,
            paramImpl.respDataImpl);
        AdvFilterParamLog("respData", respDataValue);
        SendParamsToLpDevice(respDataValue, BLE_LPDEVICE_MSG_ADV_RSP_DATA);
    }
}

void BleCentralManagerImpl::SendUuidDataToLpDevice(uint8_t idx, const Uuid &uuid)
{
    // BLE_LPDEVICE_MSG_UUID_DATA
    std::vector<uint8_t> uuidDataValue = pimpl->GetUuidDataMsgData(idx, uuid);
    AdvFilterParamLog("uuidData", uuidDataValue);
    SendParamsToLpDevice(uuidDataValue, BLE_LPDEVICE_MSG_UUID_DATA);
}

int32_t BleCentralManagerImpl::UpdateScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
    const std::vector<BleScanFilterImpl> &filters, uint32_t filterAction)
{
    int ret = Bluetooth::BT_ERR_INTERNAL_ERROR;
    switch (filterAction) {
        case BLE_SCAN_UPDATE_FILTER_NONE:
            break;
        case BLE_SCAN_UPDATE_FILTER_ADD:
            ret = AppendScanFilterInner(scannerId, settings, filters);
            break;
        case BLE_SCAN_UPDATE_FILTER_DELETE:
            ret = RemoveScanFilterInner(scannerId, settings, filters);
            break;
        case BLE_SCAN_UPDATE_FILTER_MODIFY: {
            ret = RemoveScanFilterInner(scannerId, settings, filters);
            ret = (ret == Bluetooth::BT_NO_ERROR) ? AppendScanFilterInner(scannerId, settings, filters) : ret;
            break;
        }
        default:
            break;
    }
    if (ret != Bluetooth::BT_NO_ERROR) {
        HILOGI("action:%{public}d, ret:%{public}d", filterAction, ret);
    }
    return ret;
}

void BleCentralManagerImpl::RemoveLpDeviceParam(const FilterIdxInfo &filterIdx)
{
    auto [idx, isExist] = FindShFilterIdx(filterIdx);
    if (!isExist) {
        HILOGE("fail, idx not exist.");
        return;
    }

    HILOGI("PID: %{public}d, UID: %{public}d, uuid: %{public}s, Idx: %{public}d",
        filterIdx.pid, filterIdx.uid, filterIdx.uuid.ToString().c_str(), idx);
    ReleaseShFilterIdx(idx);
    if (GetBtHwInterface() == nullptr) {
        HILOGE("bthwInterface_ is null.");
        return;
    }
    GetBtHwInterface()->deleteSensorhubFilter(idx);
}

std::pair<uint8_t, bool> BleCentralManagerImpl::FindShFilterIdx(const FilterIdxInfo &info)
{
    int idx = 0;
    bool isExist = false;
    std::lock_guard<std::mutex> lock(shFilterIdxMapLock_);
    auto it = shFilterIndexMap.begin();
    for (; it != shFilterIndexMap.end(); it++) {
        if (it->second == info) {
            idx = it->first;
            isExist = true;
            break;
        }
    }
    return {idx, isExist};
}

std::pair<FilterIdxInfo, bool> BleCentralManagerImpl::GetShFilterIdx(uint8_t idx)
{
    bool isExist = false;
    FilterIdxInfo info;
    std::lock_guard<std::mutex> lock(shFilterIdxMapLock_);
    auto iter = shFilterIndexMap.find(idx);
    if (iter == shFilterIndexMap.end()) {
        return {info, isExist};
    }
    info = iter->second;
    isExist = true;
    return {info, isExist};
}

std::pair<uint8_t, bool> BleCentralManagerImpl::AddShFilterIdx(const FilterIdxInfo &info)
{
    auto [idx, isOk] = ShFilterIdx::GetInstance()->GetOne();
    if (isOk) {
        std::lock_guard<std::mutex> lock(shFilterIdxMapLock_);
        shFilterIndexMap.insert(std::make_pair(idx, info));
    }
    return {idx, isOk};
}

void BleCentralManagerImpl::ReleaseShFilterIdx(uint8_t idx)
{
    ShFilterIdx::GetInstance()->Release(idx);
    std::lock_guard<std::mutex> lock(shFilterIdxMapLock_);
    shFilterIndexMap.erase(idx);
}

void BleCentralManagerImpl::ClearShFilterIdx()
{
    ShFilterIdx::GetInstance()->Clear();
    std::lock_guard<std::mutex> lock(shFilterIdxMapLock_);
    shFilterIndexMap.clear();
}

BleCentralManagerImpl::ShFilterIdx *BleCentralManagerImpl::ShFilterIdx::GetInstance(void)
{
    static ShFilterIdx singleton;
    return &singleton;
}

std::pair<uint8_t, bool> BleCentralManagerImpl::ShFilterIdx::GetOne(void)
{
    std::lock_guard<std::mutex> lock(mutex_);

    uint8_t index = 0;
    bool ret = true;
    if (!releasedIndexs_.empty()) {
        index = releasedIndexs_.front();
        releasedIndexs_.pop();
    } else if (currentIndex_ < MAX_SH_FILTERS) {
        index = currentIndex_++;
    } else {
        HILOGE("Not enough filter indexs");
        ret = false;
    }

    return {index, ret};
}

void BleCentralManagerImpl::ShFilterIdx::Release(uint8_t index)
{
    std::lock_guard<std::mutex> lock(mutex_);

    releasedIndexs_.push(index);
}

void BleCentralManagerImpl::ShFilterIdx::Clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    currentIndex_ = 0;
    while (!releasedIndexs_.empty()) {
        releasedIndexs_.pop();
    }
}

uint8_t BleCentralManagerImpl::GetDeliveryMode(const BleScanSettingsImpl &settings) const
{
    if ((settings.GetCallbackType() & BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH) != 0) {
        return BleScanFilter::DELIVERY_MODE_ON_FOUND;
    }

    if (settings.GetReportDelayMillisValue() > 0 && settings.GetReportMode() == REPORT_MODE_BATCH) {
        return BleScanFilter::DELIVERY_MODE_BATCHED;
    }

    return BleScanFilter::DELIVERY_MODE_IMMEDIATE;
}

uint16_t BleCentralManagerImpl::GetNumOfTrackAdvs(uint8_t matchTrackAdvType) const
{
    uint16_t num = 0;
    uint16_t maxTotal = AdapterProperties::GetInstance()->GetTotalNumOfTrackableAdvertisements();
    switch (matchTrackAdvType) {
        case bluetooth::ONE_MATCH_TRACK_ADV:
            num = BleScanFilter::NUM_OF_ONE_MATCH_TRACK_ADV_TYPE;
            break;
        case bluetooth::FEW_MATCH_TRACK_ADV:
            num = BleScanFilter::NUM_OF_FEW_MATCH_TRACK_ADV_TYPE;
            break;
        case bluetooth::MAX_MATCH_TRACK_ADV:
            num = maxTotal / 2; // 1/2 means half the maxTotal
            break;
        default:
            num = BleScanFilter::NUM_OF_ONE_MATCH_TRACK_ADV_TYPE;
            HILOGW("invalid matchTrackAdvType: %{public}d", matchTrackAdvType);
    }
    return num;
}

int BleCentralManagerImpl::RemoveScanFilterInner(int32_t scannerId, const BleScanSettingsImpl &settings,
    const std::vector<BleScanFilterImpl> &filters)
{

    std::lock_guard<std::mutex> lock(filterMuteLock_);
    auto iter = GetScanFilterIter(scannerId);
    if (iter == filters_.end()) {
        HILOGE("not found filter scannerId:%{public}d", scannerId);
        return GATT_NOT_FOUND;
    }
    int size = iter->RemoveScanFilters(filters);
    HILOGI("scannerId:%{public}d, callbackType:%{public}d, matchAdv:%{public}d, size:%{public}d, remove:%{public}d",
        scannerId, settings.GetCallbackType(), settings.GetMatchTrackAdvType(), filters.size(), size);
    if (GetDeliveryMode(settings) == BleScanFilter::DELIVERY_MODE_ON_FOUND) {
        currentUsedTrackAdvs_ -= GetNumOfTrackAdvs(settings.GetMatchTrackAdvType()) * size;
    }
    return Bluetooth::BT_NO_ERROR;
}

int BleCentralManagerImpl::AppendScanFilterInner(int32_t scannerId, const BleScanSettingsImpl &settings,
    const std::vector<BleScanFilterImpl> &filters)
{
    int maxTotal = AdapterProperties::GetInstance()->GetTotalNumOfTrackableAdvertisements();
    int trackAdvs = GetNumOfTrackAdvs(settings.GetMatchTrackAdvType());
    bool deliveryModeOnFound =
        (GetDeliveryMode(settings) == BleScanFilter::DELIVERY_MODE_ON_FOUND) ? true : false;
    int filterSize = static_cast<int>(filters.size());
    if (deliveryModeOnFound && (trackAdvs * filterSize > (maxTotal - currentUsedTrackAdvs_))) {
        HILOGE("No hardware resources for onfound/onlost filter %{public}d", trackAdvs);
        return Bluetooth::BT_ERR_BLE_SCAN_MAX_FILTER;
    }
    std::lock_guard<std::mutex> lock(filterMuteLock_);
    auto iter = GetScanFilterIter(scannerId);
    if (iter == filters_.end()) {
        HILOGE("not found filter scannerId:%{public}d", scannerId);
        return GATT_NOT_FOUND;
    }
    int appendSize = iter->AppendScanFilters(filters, GetDeliveryMode(settings),
        settings.GetMatchMode(), trackAdvs);
    HILOGI("scannerId: %{public}d, callbackType: %{public}d, matchAdv: %{public}d, size:%{public}d, append:%{public}d",
        scannerId, settings.GetCallbackType(), settings.GetMatchTrackAdvType(), filters.size(), appendSize);
    if (appendSize != filterSize) {
        return Bluetooth::BT_ERR_INTERNAL_ERROR;
    }
    if (deliveryModeOnFound && (appendSize > 0)) {
        currentUsedTrackAdvs_ += trackAdvs * appendSize;
    }
    return Bluetooth::BT_NO_ERROR;
}
}  // namespace bluetooth
}  // namespace OHOS
