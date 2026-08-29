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

#ifndef BLE_ADAPTER_H
#define BLE_ADAPTER_H

#include <map>
#include <memory>

#include "adapter_properties.h"
#include "base_observer_list.h"
#include "bluetooth.h"
#include "bt_uuid.h"
#include "context.h"
#include "stack_adapter.h"
#include "interface_adapter_ble.h"
#include "raw_address.h"
#include "remote_device_properties.h"
#include "service_util.h"

/*
 * @brief The Bluetooth subsystem.
 */
namespace OHOS {
namespace bluetooth {
/**
 *  @brief BLE Adpter implementation class
 */
class BleAdapter : public IAdapterBle, public utility::Context {
public:
    /**
     * @brief Constructor.
     */
    BleAdapter();

    /**
     * @brief Destructor.
     */
    ~BleAdapter() override;

    utility::Context *GetContext() override;

    /**
     *  @brief Turn on the BLE Bluetooth adapter
     *
     *  @return @c true Turn on BLE Bluetooth successfully
     *          @c false Failed to turn on BLE Bluetooth
     */
    void Enable() override;

    /**
     *  @brief Turn off the BLE Bluetooth adapter
     *
     *  @return @c true Turn off BLE Bluetooth successfully
     *          @c false Failed to turn off BLE Bluetooth
     */
    void Disable() override;

    /**
     *  @brief Processing after Bluetooth startup
     *
     *  @return @c true success
     *          @c false failure
     */
    void PostEnable() override;

    /// add adapter manager common api
    std::vector<RawAddress> GetConnectedDevices() const;
    int GetBleMaxAdvertisingDataLength() const override;
    int GetIoCapability() const override;
    bool SetIoCapability(int ioCapability) const override;
    bool IsBleEnabled() const;

    /// FW api passthrough from service
    void StartAdvertising(const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData,
        const BleAdvertiserDataImpl &scanResponse, uint8_t advHandle, uint16_t duration) const override;
    void SetAdvertisingData(const BleAdvertiserDataImpl &advData, const BleAdvertiserDataImpl &scanResponse,
            uint8_t advHandle) const override;
    void EnableAdvertising(uint8_t advHandle, uint16_t duration) const override;
    void DisableAdvertising(uint8_t advHandle) const override;
    void StopAdvertising(uint8_t advHandle) const override;
    void Close(uint8_t advHandle) const override;
    void StartScan(const BleScanSettingsImpl &setting) const override;
    void BatchscanConfigStorage(int clientIf, int batchScanFullMax, int batchScanTruncMax,
        int batchScanNotifyThreshold) override;
    void BatchscanEnable(int scanMode, int scanInterval, int scanWindow, int addrType, int discardRule) override;
    void BatchscanDisable() override;
    void BatchscanReadReports(int clientIf, int scanMode) override;
    void StopScan() const override;
    int ConfigScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
        const std::vector<BleScanFilterImpl> &filters) override;
    void RemoveScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings, uint16_t filtersSize) override;
    int GetAdvertisingStatus() const override;
    bool IsLlPrivacySupported() const override;
    void AddCharacteristicValue(uint8_t adtype, const std::string &data) const override;

    void RegisterBleAdvertiserCallback(IBleAdvertiserCallback &callback) override;
    void DeregisterBleAdvertiserCallback() const override;
    void RegisterBleCentralManagerCallback(IBleCentralManagerCallback &callback) override;
    void DeregisterBleCentralManagerCallback() const override;
    void RegisterBlePeripheralCallback(IBlePeripheralCallback &callback) const override;
    void DeregisterBlePeripheralCallback(IBlePeripheralCallback &callback) const override;

    bool RegisterBleAdapterObserver(IAdapterBleObserver &observer) const override;
    bool DeregisterBleAdapterObserver(IAdapterBleObserver &observer) const override;
    int GetPeerDeviceAddrType(const RawAddress &device) const override;

    void OnStartAdvertisingEvt() const;
    void OnStopAdvertisingEvt() const;

    void BondStateChanged(BtStackStatus status, OHOS::bluetooth::RawAddress* bdAddr, BtBondState state);
    void BondStateChangedInner(BtStackStatus status, RawAddress bdAddr, BtBondState state);
    void SendPairStatusChanged(const BTTransport transport, const RawAddress &device, int status) const;
    void SendAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) const;

    /**
     * @brief Read Remote Rssi Value.
     *
     * @return @c true sucessfull otherwise false.
     */
    bool ReadRemoteRssiValue(const RawAddress &device) const override;
    uint8_t GetAdvertiserHandle(int &status) const override;
    void UnregisterAdvertisingHandle(int advHandle) const override;
    void SendDiscoveryResult(
        const RawAddress &device, int rssi, const std::string deviceName, int deviceClass) const;

    int32_t AllocScannerId() override;
    void RemoveScannerId(int32_t scannerId) override;

    void SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle) override;
    void SetScanReportChannelToLpDevice(int32_t scannerId, bool enable) override;
    void EnableSyncDataToLpDevice() override;
    void DisableSyncDataToLpDevice() override;
    void SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type) override;
    bool IsLpDeviceAvailable() override;
    void SetLpDeviceParam(const FilterIdxInfo &filterIdx, const BleLpDeviceParamSetImpl &paramImpl) override;
    void RemoveLpDeviceParam(const FilterIdxInfo &filterIdx) override;
    int ChangeAdvertisingParams(uint8_t advHandle, const BleAdvertiserSettingsImpl &settings) override;
    int UpdateScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
        const std::vector<BleScanFilterImpl> &filters, uint32_t filterAction) override;
    void SetAdvOrRspData(const BleAdvertiserDataImpl &data,
        bool isAdv, uint8_t advHandle) const override;
    int GetAdvPowerForRangingBusiness(bluetooth::BleAppType appType) override;
    int BleRestoreRangingAntSwitch(bluetooth::BleAppType appType) override;
    void SendBleScanMsg(bool isStarted) override;

private:
    bool EnableTask();
    bool DisableTask();

    AdapterProperties *adapterProperties_;
    RemoteDeviceProperties *remoteDeviceProperties_;

    BT_DISALLOW_COPY_AND_ASSIGN(BleAdapter);
    DECLARE_IMPL();
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  /// BLE_ADAPTER_H
