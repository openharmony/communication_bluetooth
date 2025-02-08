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
 * @brief Defines central manager, including scan filter and settings, and common functions.
 *
 * @since 6
 */

/**
 * @file bluetooth_ble_central_manager.h
 *
 * @brief Central manager common functions.
 *
 * @since 6
 */

#ifndef BLUETOOTH_BLE_CENTRAL_MANAGER_H
#define BLUETOOTH_BLE_CENTRAL_MANAGER_H

#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_ble_advertiser.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief Represents scan result.
 *
 * @since 6
 */
class BLUETOOTH_API BleScanResult {
public:
    /**
     * @brief A constructor used to create a <b>BleScanResult</b> instance.
     *
     * @since 6
     */
    BleScanResult();

    /**
     * @brief A destructor used to delete the <b>BleScanResult</b> instance.
     *
     * @since 6
     */
    ~BleScanResult();

    /**
     * @brief Get service uuids.
     *
     * @return Returns service uuids.
     * @since 6
     */
    std::vector<UUID> GetServiceUuids() const;

    /**
     * @brief Get manufacture data.
     *
     * @return Returns manufacture data.
     * @since 6
     */
    std::map<uint16_t, std::string> GetManufacturerData() const;

    /**
     * @brief Get service data.
     *
     * @return Returns service data.
     * @since 6
     */
    std::map<UUID, std::string> GetServiceData() const;

    /**
     * @brief Get peripheral device.
     *
     * @return Returns peripheral device pointer.
     * @since 6
     */
    BluetoothRemoteDevice GetPeripheralDevice() const;

    /**
     * @brief Get peer device rssi.
     *
     * @return Returns peer device rssi.
     * @since 6
     */
    int8_t GetRssi() const;

    /**
     * @brief Check if device is connctable.
     *
     * @return Returns <b>true</b> if device is connctable;
     *         returns <b>false</b> if device is not connctable.
     * @since 6
     */
    bool IsConnectable() const;

    /**
     * @brief Get advertiser flag.
     *
     * @return Returns advertiser flag.
     * @since 6
     */
    uint8_t GetAdvertiseFlag() const;

    /**
     * @brief Get payload.
     *
     * @return Returns payload.
     * @since 6
     */
    std::vector<uint8_t> GetPayload() const;
    /**
     * @brief Add manufacture data.
     *
     * @param manufacturerId Manufacture Id which addad data.
     * @param data Manufacture data
     * @since 6
     */

    void AddManufacturerData(uint16_t manufacturerId, const std::string &data);

    /**
     * @brief Add service data.
     *
     * @param uuid Uuid of service data.
     * @param serviceData Service data.
     * @since 6
     */
    void AddServiceData(const UUID &uuid, const std::string &serviceData);

    /**
     * @brief Add service uuid.
     *
     * @param serviceUuid Service uuid.
     * @since 6
     */
    void AddServiceUuid(const UUID &serviceUuid);

    /**
     * @brief Add data to payload.
     *
     * @param payload Added payload.
     * @since 6
     */
    void SetPayload(std::string payload);

    /**
     * @brief Set peripheral device.
     *
     * @param device Remote device.
     * @since 6
     */
    void SetPeripheralDevice(const BluetoothRemoteDevice &device);

    /**
     * @brief Set peer device rssi.
     *
     * @param rssi Peer device rssi.
     * @since 6
     */
    void SetRssi(int8_t rssi);

    /**
     * @brief Set connectable.
     *
     * @param connectable Whether it is connectable.
     * @since 6
     */
    void SetConnectable(bool connectable);

    /**
     * @brief Set advertiser flag.
     *
     * @param flag Advertiser flag.
     * @since 6
     */
    void SetAdvertiseFlag(uint8_t flag);

    void SetName(const std::string &name);
    std::string GetName(void);

    void SetEventType(uint16_t eventType);
    uint16_t GetEventType(void) const;

private:
    std::vector<UUID> serviceUuids_ {};
    std::map<uint16_t, std::string> manufacturerSpecificData_ {};
    std::map<UUID, std::string> serviceData_ {};
    BluetoothRemoteDevice peripheralDevice_ {};
    int8_t rssi_ {};
    bool connectable_ {};
    uint8_t advertiseFlag_ {};
    std::vector<uint8_t> payload_ {};
    std::string name_ {};
    uint16_t eventType_ {};
};
/**
 * @brief Represents central manager callback.
 *
 * @since 6
 */
class BleCentralManagerCallback {
public:
    /**
     * @brief A destructor used to delete the <b>BleCentralManagerCallback</b> instance.
     *
     * @since 6
     */
    virtual ~BleCentralManagerCallback() = default;

    /**
     * @brief Scan result callback.
     *
     * @param result Scan result.
     * @since 6
     */
    virtual void OnScanCallback(const BleScanResult &result) = 0;

    /**
     * @brief Scan result for found or lost callback type.
     *
     * @param result Scan result.
     * @param callbackType callback Type.
     * @since 12
     */
    virtual void OnFoundOrLostCallback(const BleScanResult &result, uint8_t callbackType) = 0;

    /**
     * @brief Batch scan results event callback.
     *
     * @param results Scan results.
     * @since 6
     */
    virtual void OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results) = 0;

    /**
     * @brief Start or Stop scan event callback.
     *
     * @param resultCode Scan result code.
     * @param isStartScan true->start scan, false->stop scan.
     * @since 6
     */
    virtual void OnStartOrStopScanEvent(int resultCode, bool isStartScan) = 0;

    /**
     * @brief low power device msg report event callback.
     *
     * @param uuid Service uuid.
     * @param msgType Report msg type.
     * @param value Msg data.
     * @since 6
     */
    virtual void OnNotifyMsgReportFromLpDevice(const UUID &uuid, int msgType, const std::vector<uint8_t> &value) {};
};

/**
 * @brief Represents Scan settings.
 *
 * @since 6
 */
class BLUETOOTH_API BleScanSettings {
public:
    /**
     * @brief A constructor used to create a <b>BleScanSettings</b> instance.
     *
     * @since 6
     */
    BleScanSettings();

    /**
     * @brief A destructor used to delete the <b>BleScanSettings</b> instance.
     *
     * @since 6
     */
    ~BleScanSettings();

    /**
     * @brief Set repport delay time.
     *
     * @param reportDelayMillis Repport delay time.
     * @since 6
     */
    void SetReportDelay(long reportDelayMillis = 0);

    /**
     * @brief Get repport delay time.
     *
     * @return Returns Repport delay time.
     * @since 6
     */
    long GetReportDelayMillisValue() const;

    /**
     * @brief Set scan mode.
     *
     * @param scanMode Scan mode.
     * @return Returns <b>true</b> if set scanMode successfully;
     *         returns <b>false</b> if set scanMode failed.
     * @since 6
     */
    int SetScanMode(int scanMode);

    /**
     * @brief Get scan mode.
     *
     * @return Scan mode.
     * @since 6
     */
    int GetScanMode() const;

    /**
     * @brief Set legacy flag.
     *
     * @param legacy Legacy value.
     * @since 6
     */
    void SetLegacy(bool legacy);

    /**
     * @brief Get legacy flag.
     *
     * @return Legacy flag.
     * @since 6
     */
    bool GetLegacy() const;

    /**
     * @brief Set phy value.
     *
     * @param phy Phy value.
     * @since 6
     */
    void SetPhy(int phy);

    /**
     * @brief Get phy value.
     *
     * @return Phy value.
     * @since 6
     */
    int GetPhy() const;

    /**
     * @brief Set callback type.
     *
     * @param callbackType callback type.
     * @since 12
     */
    void SetCallbackType(uint8_t callbackType);

    /**
     * @brief Get callback type.
     *
     * @return callback type value.
     * @since 12
     */
    uint8_t GetCallbackType() const;

    /**
     * @brief Set sensitivity mode.
     *
     * @param sensitivityMode sensitivity mode.
     * @since 15
     */
    void SetSensitivityMode(uint8_t sensitivityMode);

    /**
     * @brief Get sensitivity mode.
     *
     * @return sensitivity mode value.
     * @since 15
     */
    uint8_t GetSensitivityMode() const;

    /**
     * @brief Set match track adv type for total number of advertisers to track per filter.
     *
     * @param matchTrackAdvType match track adv type value.
     * @since 12
     */
    void SetMatchTrackAdvType(uint8_t matchTrackAdvType);
    /**
     * @brief Get match track adv type.
     *
     * @return match track adv type value.
     * @since 12
     */
    uint8_t GetMatchTrackAdvType() const;

private:
    long reportDelayMillis_ = 0;
    int scanMode_ = SCAN_MODE_LOW_POWER;
    bool legacy_ = true;
    int phy_ = PHY_LE_1M;
    uint8_t callbackType_ = BLE_SCAN_CALLBACK_TYPE_ALL_MATCH;
    uint8_t sensitivityMode_ = SENSITIVITY_MODE::SENSITIVITY_MODE_HIGH;
    uint8_t matchTrackAdvType_ = MAX_MATCH_TRACK_ADV;
};

/**
 * @brief Represents Scan filter.
 *
 */
class BLUETOOTH_API BleScanFilter {
public:
    /**
     * @brief A constructor used to create a <b>BleScanFilter</b> instance.
     *
     */
    BleScanFilter();

    /**
     * @brief A destructor used to delete the <b>BleScanFilter</b> instance.
     *
     */
    ~BleScanFilter();

    void SetDeviceId(std::string deviceId);

    std::string GetDeviceId() const;

    void SetName(std::string name);

    std::string GetName() const;

    void SetServiceUuid(const UUID &uuid);

    bool HasServiceUuid();

    UUID GetServiceUuid() const;

    void SetServiceUuidMask(const UUID &serviceUuidMask);

    bool HasServiceUuidMask();

    UUID GetServiceUuidMask() const;

    void SetServiceSolicitationUuid(const UUID &serviceSolicitationUuid);

    bool HasSolicitationUuid();

    UUID GetServiceSolicitationUuid() const;

    void SetServiceSolicitationUuidMask(const UUID &erviceSolicitationUuidMask);

    bool HasSolicitationUuidMask();

    UUID GetServiceSolicitationUuidMask() const;

    void SetServiceData(std::vector<uint8_t> serviceData);

    std::vector<uint8_t> GetServiceData() const;

    void SetServiceDataMask(std::vector<uint8_t> serviceDataMask);

    std::vector<uint8_t> GetServiceDataMask() const;

    void SetManufacturerId(uint16_t manufacturerId);

    uint16_t GetManufacturerId() const;

    void SetManufactureData(std::vector<uint8_t> manufactureData);

    std::vector<uint8_t> GetManufactureData() const;

    void SetManufactureDataMask(std::vector<uint8_t> manufactureDataMask);

    std::vector<uint8_t> GetManufactureDataMask() const;

    void SetAdvIndReportFlag(bool advIndReprot);

    bool GetAdvIndReportFlag() const;

    void SetFilterIndex(uint8_t index);

    uint8_t GetFilterIndex() const;
    /**
     * @brief Compare two BleScanFilter whether are same or not.
     *
     * @param rhs Compared BleScanFilter instance.
     * @return Returns <b>true</b> if this BleScanFilter is the same as compared BleScanFilter;
     *         returns <b>false</b> if this BleScanFilter is not the same as compared BleScanFilter.
     */
    bool operator==(const BleScanFilter &rhs) const
    {
        return (deviceId_ == rhs.deviceId_) &&
            (name_ == rhs.name_) &&
            (serviceUuid_ == rhs.serviceUuid_) &&
            (serviceUuidMask_ == rhs.serviceUuidMask_) &&
            (serviceSolicitationUuid_ == rhs.serviceSolicitationUuid_) &&
            (serviceSolicitationUuidMask_ == rhs.serviceSolicitationUuidMask_) &&
            (hasServiceUuid_ == rhs.hasServiceUuid_) &&
            (hasServiceUuidMask_ == rhs.hasServiceUuidMask_) &&
            (hasSolicitationUuid_ == rhs.hasSolicitationUuid_) &&
            (hasSolicitationUuidMask_ == rhs.hasSolicitationUuidMask_) &&
            (serviceData_ == rhs.serviceData_) &&
            (serviceDataMask_ == rhs.serviceDataMask_) &&
            (manufacturerId_ == rhs.manufacturerId_) &&
            (manufactureData_ == rhs.manufactureData_) &&
            (manufactureDataMask_ == rhs.manufactureDataMask_) &&
            (advIndReprot_ == rhs.advIndReprot_) &&
            (filterIndex_ == rhs.filterIndex_);
    }

    private:
        std::string deviceId_;
        std::string name_;

        UUID serviceUuid_;
        UUID serviceUuidMask_;
        UUID serviceSolicitationUuid_;
        UUID serviceSolicitationUuidMask_;
        bool hasServiceUuid_ = false;
        bool hasServiceUuidMask_ = false;
        bool hasSolicitationUuid_ = false;
        bool hasSolicitationUuidMask_ = false;

        std::vector<uint8_t> serviceData_;
        std::vector<uint8_t> serviceDataMask_;

        uint16_t manufacturerId_ = 0;
        std::vector<uint8_t> manufactureData_;
        std::vector<uint8_t> manufactureDataMask_;
        bool advIndReprot_ = false;
        uint8_t filterIndex_ = 0;
};

struct BleActiveDeviceInfo {
    std::vector<int8_t> deviceId;
    int32_t status;
    int32_t timeOut;
};

struct BleLpDeviceParamSet {
    BleScanSettings scanSettings;
    std::vector<BleScanFilter> scanFilters;
    BleAdvertiserSettings advSettings;
    std::vector<uint8_t> advData;
    std::vector<uint8_t> respData;
    UUID uuid;
    std::vector<BleActiveDeviceInfo> activeDeviceInfos;
    int32_t deliveryMode;
    int32_t advHandle;
    int32_t duration;
    uint32_t fieldValidFlagBit;
};

/**
 * @brief Represents central manager.
 *
 * @since 6
 */
class BLUETOOTH_API BleCentralManager {
public:
    /**
     * @brief A constructor used to create a <b>BleCentralManager</b> instance.
     *
     * @param cllback Central manager callback to create an <b>BleCentralManagerCallback</b> instance.
     * @since 6
     */
    explicit BleCentralManager(BleCentralManagerCallback &callback);

    /**
     * @brief A constructor used to create a <b>BleCentralManager</b> instance.
     *
     * @param cllback Central manager callback to create an <b>BleCentralManagerCallback</b> instance.
     * @param enableRandomAddrMode Indicates whether to use random address for interface interaction.
     * @since 6
     */
    explicit BleCentralManager(std::shared_ptr<BleCentralManagerCallback> callback, bool enableRandomAddrMode = true);

    /**
     * @brief A destructor used to delete the <b>BleCentralManager</b> instance.
     *
     * @since 6
     */
    ~BleCentralManager();

    /**
     * @brief Start scan.
     *
     * @param settings Scan settings.
     * @param filters Scan filters.
     * @since 11
     */
    int StartScan(const BleScanSettings &settings, const std::vector<BleScanFilter> &filters);

    /**
     * @brief Stop scan.
     *
     * @since 6
     */
    int StopScan();

    /**
    * @brief set low power device adv param.
    *
    * @param duration advertise duration.
    * @param maxExtAdvEvents maximum number of extended advertising events.
    * @param window work window.
    * @param interval work interval.
    * @param advHandle Indicates the advertisement handle.
    * @return Result.
    */
    int SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle);

    /**
    * @brief set scan result report channel.
    *
    * @param enable the switch of report.(true: report msg to low power device; false: not report).
    * @return Result.
    */
    int SetScanReportChannelToLpDevice(bool enable);

    /**
    * @brief Enable sync data to low power device.
    *
    * @return Result.
    */
    int EnableSyncDataToLpDevice();

    /**
    * @brief Disable sync data to low power device.
    *
    * @return Result.
    */
    int DisableSyncDataToLpDevice();

    /**
    * @brief Translate ParamData to low power device.
    *
    * @param data Indicates the pointer to the data.
    * @param dataSize Indicates the data size.
    * @param type Indicates the data type.
    * @return Result.
    */
    int SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type);

    /**
    * @brief Get whether support low power device.
    *
    * @return true: available; false: not available.
    * @since 6
    */
    bool IsLpDeviceAvailable();

    /**
    * @brief Set low power device param data.
    *
    * @param lpDeviceParamSet low power device param data.
    * @return Result
    * @since 6
    */
    int SetLpDeviceParam(const BleLpDeviceParamSet &lpDeviceParamSet);

    /**
    * @brief Remove low power device param data.
    *
    * @param uuid Uuid.
    * @return Result
    * @since 6
    */
    int RemoveLpDeviceParam(const UUID &uuid);

    /**
     * @brief Change a scan with BleScanConfigs and filter.
     * If don't change ble scan filter, set std::vector<BleScanFilter> empty or set filterAction none.
     *
     * @param settings Scan settings.
     * @param filters Scan filters.
     * @param filterAction Indicates change filter behavior. see {@link BleScanUpdateFilterAction}.
     * @return Returns {@link BT_NO_ERROR} if the scan is started.
     * returns an error code defined in {@link BtStatus} otherwise.
     * @since 16
     */
    int ChangeScanParams(const BleScanSettings &settings, const std::vector<BleScanFilter> &filter,
        uint32_t filterAction);

private:
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BleCentralManager);
    BLUETOOTH_DECLARE_IMPL();
};
} // namespace Bluetooth
} // namespace OHOS
#endif