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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines adapter ble, including observer, callbacks and common functions.
 *
 * @since 6
 */

/**
 * @file interface_adapter_ble.h
 *
 * @brief Adapter ble interface.
 *
 * @since 6
 */

#ifndef INTERFACE_ADAPTER_BLE_H
#define INTERFACE_ADAPTER_BLE_H

#include "interface_adapter.h"
#include "ble_service_data.h"
#include "bluetooth_ble_scan_result.h"
#include <memory>

namespace OHOS {
namespace bluetooth {
/**
 * @brief Represents central manager callbacks.
 *
 * @since 6
 */
class IBleCentralManagerCallback {
public:
    /**
     * @brief A destructor used to delete the <b>IBleCentralManagerCallback</b> instance.
     *
     * @since 6
     */
    virtual ~IBleCentralManagerCallback() = default;

    /**
     * @brief Scan callback.
     *
     * @param result Scan result.
     * @since 6
     */
    virtual void OnScanCallback(const BleScanResultImpl &result) = 0;

    /**
     * @brief Scan callback for found mode.
     *
     * @param scanId Scan id.
     * @param advertiserState advertiser state.
     * @param result Scan result.
     * @since 11
     */
    virtual void OnFoundOrLostCallback(uint8_t scanId, uint8_t advertiserState, const BleScanResultImpl &result) = 0;

    /**
     * @brief Scan results event callback.
     *
     * @param results Scan results.
     * @since 6
     */
    virtual void OnBleBatchScanResultsEvent(int clientIf, std::vector<Bluetooth::BluetoothBleScanResult> &results) = 0;

    /**
     * @brief Start or Stop scan event callback.
     *
     * @param resultCode Start scan result code.
     * @since 6
     */
    virtual void OnStartOrStopScanEvent(int resultCode, bool isStartScan) = 0;

    virtual void OnNotifyMsgReportFromLpDevice(const FilterIdxInfo &info, uint8_t msgType,
        const std::vector<uint8_t> &notifyValue) = 0;
};

/**
 * @brief Represents advertise callbacks.
 *
 * @since 6
 */
class IBleAdvertiserCallback {
public:
    virtual ~IBleAdvertiserCallback() = default;
    virtual void OnStartResultEvent(int result, uint8_t advHandle, int opcode = BLE_ADV_DEFAULT_OP_CODE) = 0;
    virtual void OnEnableResultEvent(int result, uint8_t advHandle) = 0;
    virtual void OnDisableResultEvent(int result, uint8_t advHandle) = 0;
    virtual void OnStopResultEvent(int result, uint8_t advHandle) = 0;
    virtual void OnAutoStopAdvEvent(uint8_t advHandle) = 0;
    virtual void OnSetAdvDataEvent(int result, uint8_t advHandle, SetAdvDataType type = SET_ADV_DATA_BOTH) = 0;
    virtual void OnChangeAdvResultEvent(int result, uint8_t advHandle) = 0;
};

/**
 * @brief Represents ble adapter observer.
 *
 * @since 6
 */
class IAdapterBleObserver {
public:
    /**
     * @brief A destructor used to delete the <b>IBleAdapterObserver</b> instance.
     *
     * @since 6
     */
    virtual ~IAdapterBleObserver() = default;

    /**
     * @brief Discovery state changed observer.
     *
     * @param status Device discovery status.
     * @since 6
     */
    virtual void OnDiscoveryStateChanged(const int status) = 0;

    /**
     * @brief Discovery result observer.
     *
     * @param device Remote device.
     * @param rssi Rssi of device.
     * @param deviceName Name of device.
     * @param deviceClass Class of device.
     * @since 6
     */
    virtual void OnDiscoveryResult(
        const RawAddress &device, int rssi, const std::string deviceName, int deviceClass) = 0;

    /**
     * @brief Pair request observer.
     *
     * @param device Remote device.
     * @since 6
     */
    virtual void OnPairRequested(const BTTransport transport, const RawAddress &device) = 0;

    /**
     * @brief Pair confirmed observer.
     *
     * @param device Remote device.
     * @param reqType Pair type.
     * @param number Paired passkey.
     * @since 6
     */
    virtual void OnPairConfirmed(
        const BTTransport transport, const RawAddress &device, const int reqType, const int number) = 0;

    /**
     * @brief Scan mode changed observer.
     *
     * @param mode Device scan mode.
     * @since 6
     */
    virtual void OnScanModeChanged(const int mode) = 0;

    /**
     * @brief Device name changed observer.
     *
     * @param deviceName Device name.
     * @since 6
     */
    virtual void OnDeviceNameChanged(const std::string deviceName) = 0;

    /**
     * @brief Device address changed observer.
     *
     * @param address Device address.
     * @since 6
     */
    virtual void OnDeviceAddrChanged(const std::string address) = 0;

    /**
     * @brief Advertising state changed observer.
     *
     * @param state Advertising state.
     * @since 6
     */
    virtual void OnAdvertisingStateChanged(const int state) = 0;
};

/**
 * @brief Represents peripheral callback.
 *
 * @since 6
 */
class IBlePeripheralCallback {
public:
    /**
     * @brief A destructor used to delete the <b>IBlePeripheralCallback</b> instance.
     *
     * @since 6
     */
    virtual ~IBlePeripheralCallback() = default;

    /**
     * @brief Read remote rssi event callback.
     *
     * @param device Remote device.
     * @param rssi Remote device rssi.
     * @param status Read status.
     * @since 6
     */
    virtual void OnReadRemoteRssiEvent(const RawAddress &device, int rssi, int status) = 0;

    /**
     * @brief Pair status changed callback.
     *
     * @param device Remote device.
     * @param status Pair status.
     * @param cause Pair fail cause.
     * @since 12
     */
    virtual void OnPairStatusChanged(
        BTTransport transport, const RawAddress &device, int status, int cause, const std::string &causeMessage) = 0;

    /**
     * @brief Acl state changed callback.
     *
     * @param device Remote device.
     * @param state Acl state.
     * @param reason Reason.
     * @since 6
     */
    virtual void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) = 0;
};

/**
 * @brief Represents ble adapter interface.
 *
 * @since 6
 */
class IAdapterBle {
public:
    /**
     * @brief Register central manager callback.
     *
     * @param callback Class IBleCentralManagerCallback pointer to register callback.
     * @since 6
     */
    virtual void RegisterBleCentralManagerCallback(IBleCentralManagerCallback &callback) = 0;

    /**
     * @brief Deregister central manager callback.
     *
     * @since 6
     */
    virtual void DeregisterBleCentralManagerCallback() const = 0;

    /**
     * @brief Register advertiser callback.
     *
     * @param callback Class IBleAdvertiseCallback pointer to register callback.
     * @since 6
     */
    virtual void RegisterBleAdvertiserCallback(IBleAdvertiserCallback &callback) = 0;

    /**
     * @brief Deregister advertiser callback.
     *
     * @since 6
     */
    virtual void DeregisterBleAdvertiserCallback() const = 0;

    /**
     * @brief Read remote device rssi value.
     *
     * @param device Remote device
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool ReadRemoteRssiValue(const RawAddress &device) const = 0;

    /**
     * @brief Register ble adapter observer.
     *
     * @param observer Class IBleAdapterObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool RegisterBleAdapterObserver(IAdapterBleObserver &observer) const = 0;

    /**
     * @brief Deregister ble adapter observer.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool DeregisterBleAdapterObserver(IAdapterBleObserver &observer) const = 0;

    /**
     * @brief Register peripheral callback.
     *
     * @param callback Class IBlePeripheralCallback pointer to register callback.
     * @since 6
     */
    virtual void RegisterBlePeripheralCallback(IBlePeripheralCallback &callback) const = 0;

    /**
     * @brief Deregister peripheral callback.
     *
     * @param callback Class IBlePeripheralCallback pointer to register callback.
     * @since 6
     */
    virtual void DeregisterBlePeripheralCallback(IBlePeripheralCallback &callback) const = 0;

    /**
     * @brief Get device IO capability.
     *
     * @return Returns device IO capability.
     * @since 6
     */
    virtual int GetIoCapability() const = 0;

    /**
     * @brief Set device IO capability.
     *
     * @param ioCapability IO capability.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetIoCapability(int ioCapability) const = 0;

    /**
     * @brief Get max advertising data length.
     *
     * @return Returns max advertising data length.
     * @since 6
     */
    virtual int GetBleMaxAdvertisingDataLength() const = 0;

    /**
     * @brief Get peer device address type.
     *
     * @param device Remote device.
     * @return Returns peer device address type.
     * @since 6
     */
    virtual int GetPeerDeviceAddrType(const RawAddress &device) const = 0;

    /**
     * @brief Get advertiser id.
     *
     * @return Returns advertiser handle.
     * @since 6
     */
    virtual uint8_t GetAdvertiserHandle(int &status) const = 0;

    /**
     * @brief Unregister advertising handle.
     *
     * @param advHandle Advertising handle.
     * @since 6
     */
    virtual void UnregisterAdvertisingHandle(int advHandle) const = 0;

    /**
     * @brief Get advertiser status.
     *
     * @return Returns advertiser status.
     * @since 6
     */
    virtual int GetAdvertisingStatus() const = 0;

    /**
     * @brief Get Link Layer Privacy Supported.
     *
     * @return True:supported; False:not supported.
     * @since 6
     */
    virtual bool IsLlPrivacySupported() const = 0;

    /**
     * @brief Add characteristic value.
     *
     * @param adtype Type of the field.
     * @param data Field data.
     * @since 6
     */
    virtual void AddCharacteristicValue(uint8_t adtype, const std::string &data) const = 0;

    /**
     * @brief Start advertising.
     *
     * @param settings Advertise settings.
     * @param advData Advertise data.
     * @param scanResponse Scan response data
     * @param advHandle Advertise handle
     * @param duration Advertise duration
     * @since 6
     */
    virtual void StartAdvertising(const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData,
        const BleAdvertiserDataImpl &scanResponse, uint8_t advHandle, uint16_t duration) const = 0;

    /**
     * @brief Enable advertising.
     *
     * @param advHandle Advertise handle
     * @param duration Advertise duration
     * @since 11
     */
    virtual void EnableAdvertising(uint8_t advHandle, uint16_t duration) const = 0;

    /**
     * @brief Disable advertising.
     *
     * @param advHandle Advertise handle
     * @since 11
     */
    virtual void DisableAdvertising(uint8_t advHandle) const = 0;

    /**
     * @brief Set advertising data when advertising is started.
     *
     * @param advData Advertise data.
     * @param scanResponse Scan response data
     * @param advHandle Advertise handle
     * @since 6
     */
    virtual void SetAdvertisingData(const BleAdvertiserDataImpl &advData, const BleAdvertiserDataImpl &scanResponse,
        uint8_t advHandle) const = 0;
    
    /**
     * @brief Set advertising data or Scan response data when advertising is started.
     *
     * @param data Advertise data or Scan response data
     * @param isAdv true means Advertise data, false means Scan response data
     * @param advHandle Advertise handle
     * @since 22
     */
    virtual void SetAdvOrRspData(const BleAdvertiserDataImpl &data, bool isAdv, uint8_t advHandle) const = 0;

    /**
     * @brief Stop advertising.
     *
     * @param advHandle Advertise handle
     * @since 6
     */
    virtual void StopAdvertising(uint8_t advHandle) const = 0;

    /**
     * @brief Cleans up advertisers.
     *
     * @since 6
     */
    virtual void Close(uint8_t advHandle) const = 0;

    /**
     * @brief Start scan
     *
     * @param setting Scan setting.
     * @since 6
     */
    virtual void StartScan(const BleScanSettingsImpl &setting) const = 0;

    virtual void BatchscanConfigStorage(int clientIf,
                                        int batchScanFullMax,
                                        int batchScanTruncMax,
                                        int batchScanNotifyThreshold) = 0;

    virtual void BatchscanEnable(int scanMode,
                                int scanInterval,
                                int scanWindow,
                                int addrType,
                                int discardRule) = 0;

    virtual void BatchscanDisable() = 0;

    virtual void BatchscanReadReports(int clientIf, int scanMode) = 0;

    /**
     * @brief Stop scan.
     *
     * @since 6
     */
    virtual void StopScan() const = 0;

    /**
     * @brief Config scan filter
     *
     * @param scannerId indicate one scan.
     * @param settings Scan settings.
     * @param filters Scan filters.
     * @return ret
     */
    virtual int ConfigScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
        const std::vector<BleScanFilterImpl> &filters) = 0;

    /**
     * @brief Remove scan filter
     *
     * @param clientId client id.
     * @param settings Scan settings.
     * @param filtersSize Scan filters size.
     */
    virtual void RemoveScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings, uint16_t filtersSize) = 0;

    /**
     * @brief Alloc scan object id.
     *
     * @return scanner id
     */
    virtual int32_t AllocScannerId() = 0;

    /**
     * @brief Remove scan object id.
     *
     * @return scanner id
     */
    virtual void RemoveScannerId(int32_t scannerId) = 0;

    /**
     * @brief Set low power device adv Param
     *
     * @param duration duration.
     * @param maxExtAdvEvents maxExtAdvEvents.
     * @param window window.
     * @param interval interval.
     * @param advHandle advHandle.
     */
    virtual void SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle) = 0;

    /**
    * @brief Set scan result report channel.
    *
    * @param scannerId scanner id.
    * @param isToAp the switch of report.(true:report msg to low power device; false: not report;).
    */
    virtual void SetScanReportChannelToLpDevice(int32_t scannerId, bool enable) = 0;

    /**
    * @brief Enable sync data to low power device.
    *
    */
    virtual void EnableSyncDataToLpDevice() = 0;

    /**
    * @brief Disable sync data to low power device.
    *
    */
    virtual void DisableSyncDataToLpDevice() = 0;

    /**
    * @brief Translate ParamData to low power device.
    *
    * @param dataValue Indicates the data.
    * @param type data type.
    */
    virtual void SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type) = 0;

    /**
    * @brief Get whether low power device available.
    *
    * @return true: support; false: not support.
    */
    virtual bool IsLpDeviceAvailable() = 0;

    /**
    * @brief Set low power device data.
    *
    * @param filterIdx FilterIdx info.
    * @param paramImpl FilterParam info.
    */
    virtual void SetLpDeviceParam(const FilterIdxInfo &filterIdx, const BleLpDeviceParamSetImpl &paramImpl) = 0;

    /**
    * @brief Remove low power device data.
    *
    * @param filterIdx FilterIdx info.
    */
    virtual void RemoveLpDeviceParam(const FilterIdxInfo &filterIdx) = 0;

    /**
     * @brief Set advertising settings when advertising is disabled.
     *
     * @param advHandle Advertise handle
     * @param settings Advertise settings.
     * @return Returns change advertise parameters result.
     * @since 16
     */
    virtual int ChangeAdvertisingParams(uint8_t advHandle, const BleAdvertiserSettingsImpl &settings) = 0;

    /**
     * @brief Get advertising power for ranging business.
     *
     * @param appType Advertise application type
     * @return Returns adv power value.
     * @since 26
     */
    virtual int GetAdvPowerForRangingBusiness(bluetooth::BleAppType appType) = 0;

    /**
     * @brief ble stop range adv to send switch antenna msg.
     *
     * @param appType Advertise application type
     * @return Returns result the switchback.
     * @since 26
     */
    virtual int BleRestoreRangingAntSwitch(bluetooth::BleAppType appType) = 0;

    /**
     * @brief Update ble scan filters.
     *
     * @param scannerId scanner id.
     * @param settings scan settings.
     * @param filters Scan filters.
     * @param filterAction update filter action, include add,delete,modify.
     * @return Returns change scan parameters result.
     * @since 16
     */
    virtual int UpdateScanFilter(int32_t scannerId, const BleScanSettingsImpl &settings,
        const std::vector<BleScanFilterImpl> &filters, uint32_t filterAction) = 0;

    /**
    * @brief send ble scan message.
    *
    * @param isStarted true: start scan, false: stop scan.
    */
    virtual void SendBleScanMsg(bool isStarted) = 0;

    /**
     * @brief Get utility::Context pointer for adapter.
     *
     * @return Returns the pointer for adapter.
     * @since 6
     */
    virtual utility::Context *GetContext() = 0;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // INTERFACE_ADAPTER_BLE_H