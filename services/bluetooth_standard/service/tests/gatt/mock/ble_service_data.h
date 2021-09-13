/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
 * @brief Defines ble advertiser, peripheral deviceand central manager functions,
 *  including scan settings and filters, advertise settings and data etc.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file ble_data.cpp
 *
 * @brief Ble data class.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef BLE_SERVICE_DATA_H
#define BLE_SERVICE_DATA_H

#include "bt_uuid.h"
#include "raw_address.h"
#include "../../common/bt_def.h"
#include <vector>

namespace bluetooth {
/**
 * @brief Represents scan settings.
 *
 * @since 1.0
 * @version 1.0
 */
class BleScanSettingsImpl {
public:
    /**
     * @brief A constructor used to create a <b>BleScanSettingsInternal</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    BleScanSettingsImpl(){};

    /**
     * @brief A destructor used to delete the <b>BleScanSettingsInternal</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    ~BleScanSettingsImpl(){};

    /**
     * @brief Set repport delay time.
     *
     * @param reportDelayMillis Repport delay time.
     * @since 1.0
     * @version 1.0
     */
    void SetReportDelay(long reportDelayMillis = BLE_REPORT_DELAY_MILLIS);

    /**
     * @brief Get repport delay time.
     *
     * @return Returns Repport delay time.
     * @since 1.0
     * @version 1.0
     */
    long GetReportDelayMillisValue() const;

private:
    /// delay millistime
    long reportDelayMillis_ = BLE_REPORT_DELAY_MILLIS;
};

/**
 * @brief Represents advertise settings.
 *
 * @since 1.0
 * @version 1.0
 */
class BleAdvertiserSettingsImpl {
public:
    /**
     * @brief A constructor used to create a <b>BleAdvertiseSettingsInternal</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    BleAdvertiserSettingsImpl(){};

    /**
     * @brief A destructor used to delete the <b>BleAdvertiseSettingsInternal</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    ~BleAdvertiserSettingsImpl(){};

    /**
     * @brief Check if device service is connctable.
     *
     * @return Returns <b>true</b> if device service is connctable;
     *         returns <b>false</b> if device service is not connctable.
     * @since 1.0
     * @version 1.0
     */
    bool IsConnectable() const;

    /**
     * @brief Set connectable.
     *
     * @param connectable Whether it is connectable.
     * @since 1.0
     * @version 1.0
     */
    void SetConnectable(bool connectable);
    /**
     * @brief Check if advertiser is legacy mode.
     *
     * @return Returns <b>true</b> if advertiser is legacy mode;
     *         returns <b>false</b> if advertiser is not legacy mode.
     * @since 1.0
     * @version 1.0
     */
    bool IsLegacyMode() const;

    /**
     * @brief Set legacyMode.
     *
     * @param connectable Whether it is legacyMode.
     * @since 1.0
     * @version 1.0
     */
    void SetLegacyMode(bool legacyMode);

    /**
     * @brief Get advertise interval.
     *
     * @return Returns advertise interval.
     * @since 1.0
     * @version 1.0
     */
    int GetInterval() const;

    /**
     * @brief Set advertise interval.
     *
     * @param interval Advertise interval.
     * @since 1.0
     * @version 1.0
     */
    void SetInterval(int interval = BLE_ADV_DEFAULT_INTERVAL);

    /**
     * @brief Get advertiser Tx power.
     *
     * @return Returns advertiser Tx power.
     * @since 1.0
     * @version 1.0
     */
    int GetTxPower() const;

    /**
     * @brief Set advertiser Tx power.
     *
     * @param txPower Advertiser Tx power.
     * @since 1.0
     * @version 1.0
     */
    int SetTxPower(int txPower);

    /**
     * @brief Get primary phy.
     *
     * @return Returns primary phy.
     * @since 1.0
     * @version 1.0
     */
    int GetPrimaryPhy() const;

    /**
     * @brief Set primary phy.
     *
     * @param primaryPhy Primary phy.
     * @since 1.0
     * @version 1.0
     */
    void SetPrimaryPhy(int primaryPhy);

    /**
     * @brief Get second phy.
     *
     * @return Returns primary phy.
     * @since 1.0
     * @version 1.0
     */
    int GetSecondaryPhy() const;

    /**
     * @brief Set second phy.
     *
     * @param secondaryPhy Second phy.
     * @since 1.0
     * @version 1.0
     */
    void SetSecondaryPhy(int secondaryPhy);

private:
    /// Advertising interval.
    int interval_ = BLE_ADV_DEFAULT_INTERVAL;
    /// Advertising connectable.
    bool connectable_ = false;
    /// Advertising txPower.
    int txPower_ = -1;
    /// Advertising legacyMode.
    bool legacyMode_ = true;
    /// Advertising primaryPhy.
    int primaryPhy_ = 0;
    /// Advertising secondaryPhy.
    int secondaryPhy_ = 0;
};

/**
 * @brief Represents advertise data.
 *
 * @since 1.0
 * @version 1.0
 */
class BleAdvertiserDataImpl {
public:
    /**
     * @brief A constructor used to create a <b>BleAdvertiseDataInternal</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    BleAdvertiserDataImpl();

    /**
     * @brief A destructor used to delete the <b>BleAdvertiseDataInternal</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    ~BleAdvertiserDataImpl()
    {}

    /**
     * @brief Add manufacture data.
     *
     * @param manufacturerId Manufacture Id which addad data.
     * @param data Manufacture data
     * @since 1.0
     * @version 1.0
     */
    int AddManufacturerData(uint16_t manufacturerId, const std::string &data);

    /**
     * @brief Add service data.
     *
     * @param uuid Uuid of service data.
     * @param data Service data.
     * @since 1.0
     * @version 1.0
     */
    void AddServiceData(Uuid uuid, std::string data);

    /**
     * @brief Add service uuid.
     *
     * @param uuid Service uuid.
     * @since 1.0
     * @version 1.0
     */
    void AddServiceUuid(Uuid uuid);

    /**
     * @brief Set device appearance.
     *
     * @param appearance Device appearance.
     * @since 1.0
     * @version 1.0
     */
    void SetAppearance(uint16_t appearance);

    /**
     * @brief Set complete services.
     *
     * @param uuid Service uuid.
     * @since 1.0
     * @version 1.0
     */
    void SetCompleteServices(Uuid uuid);

    /**
     * @brief Set advertiser flag.
     *
     * @param flag Advertiser flag.
     * @since 1.0
     * @version 1.0
     */
    void SetFlags(uint8_t flag);

    /**
     * @brief Set manufacture data.
     *
     * @param data Manufacture data.
     * @since 1.0
     * @version 1.0
     */
    void SetManufacturerData(const std::string &data);

    /**
     * @brief Set device name.
     *
     * @param name Device name.
     * @since 1.0
     * @version 1.0
     */
    void SetDeviceName(const std::string &name);

    /**
     * @brief Set partial services.
     *
     * @param uuid Service uuid.
     * @since 1.0
     * @version 1.0
     */
    void SetPartialServices(Uuid uuid);
    /**
     * @brief Set Tx power level.
     *
     * @param txPowerLevel Tx power level.
     * @since 1.0
     * @version 1.0
     */
    void SetTxPowerLevel(uint8_t txPowerLevel);

    /**
     * @brief Add service data.
     *
     * @param data Service data.
     * @since 1.0
     * @version 1.0
     */
    void AddData(std::string data);

    /**
     * @brief Get advertiser data packet.
     *
     * @return Returns advertiser data packet.
     * @since 1.0
     * @version 1.0
     */
    std::string GetPayload() const;

private:
    /// Advertiser data packet
    std::string payload_;

    /**
     * @brief Set advertiser data long name.
     *
     * @param name Bluetooth device name.
     * @since 1.0
     * @version 1.0
     */
    void SetLongName(const std::string &name);

    /**
     * @brief Set advertiser data short name
     *
     * @param name Bluetooth device name.
     * @since 1.0
     * @version 1.0
     */
    void SetShortName(const std::string &name);
};
struct BlePeripheralDeviceParseAdvData {
    uint8_t *payload = nullptr;
    size_t length = 0;
};

/**
 * @brief Represents peripheral device.
 *
 * @since 1.0
 * @version 1.0
 */
class BlePeripheralDevice {
public:
    /**
     * @brief A constructor used to create a <b>BlePeripheralDevice</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    BlePeripheralDevice();

    /**
     * @brief A destructor used to delete the <b>BlePeripheralDevice</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    ~BlePeripheralDevice();

    /**
     * @brief Get device address.
     *
     * @return Returns device address.
     * @since 1.0
     * @version 1.0
     */
    RawAddress GetRawAddress() const;

    /**
     * @brief Get device Appearance.
     *
     * @return Returns device Appearance.
     * @since 1.0
     * @version 1.0
     */
    uint16_t GetAppearance() const;

    /**
     * @brief Get Manufacturer Data.
     *
     * @return Returns Manufacturer Data.
     * @since 1.0
     * @version 1.0
     */
    std::string GetManufacturerData() const;

    /**
     * @brief Get device Name.
     *
     * @return Returns device Name.
     * @since 1.0
     * @version 1.0
     */
    std::string GetName() const;

    /**
     * @brief Get device RSSI.
     *
     * @return Returns device RSSI.
     * @since 1.0
     * @version 1.0
     */
    int8_t GetRSSI() const;

    /**
     * @brief Get service Data.
     *
     * @return Returns service data.
     * @since 1.0
     * @version 1.0
     */
    std::vector<std::string> GetServiceData() const;

    /**
     * @brief Get Service Data.
     *
     * @param index Service data index.
     * @return Returns service data.
     * @since 1.0
     * @version 1.0
     */
    std::string GetServiceData(int index) const;

    /**
     * @brief Get service data UUID.
     *
     * @return Returns service data UUID.
     * @since 1.0
     * @version 1.0
     */
    std::vector<Uuid> GetServiceDataUUID() const;

    /**
     * @brief Get service data UUID.
     *
     * @param index Service data index.
     * @return Returns service data UUID.
     * @since 1.0
     * @version 1.0
     */
    Uuid GetServiceDataUUID(int index);

    /**
     * @brief Get serviceU UUID.
     *
     * @return Returns service UUID.
     * @since 1.0
     * @version 1.0
     */
    std::vector<Uuid> GetServiceUUID() const;

    /**
     * @brief Get service UUID.
     *
     * @param index Service UUID index.
     * @return Return service UUID.
     * @since 1.0
     * @version 1.0
     */
    Uuid GetServiceUUID(int index) const;

    /**
     * @brief Get service data count.
     *
     * @return Returns service data count.
     * @since 1.0
     * @version 1.0
     */
    int GetServiceDataCount() const;

    /**
     * @brief Get Tx power.
     *
     * @return Returns Tx power.
     * @since 1.0
     * @version 1.0
     */
    int8_t GetTXPower() const;

    /**
     * @brief Get advertiser data packet.
     *
     * @return Returns advertiser data packet.
     * @since 1.0
     * @version 1.0
     */
    uint8_t *GetPayload() const;

    /**
     * @brief Get advertising packet length.
     *
     * @return Returns advertising packet length.
     * @since 1.0
     * @version 1.0
     */
    size_t GetPayloadLen() const;

    /**
     * @brief Get address type.
     *
     * @return Returns address type.
     * @since 1.0
     * @version 1.0
     */
    int GetAddressType() const;

    /**
     * @brief Set address type.
     *
     * @param type Address type.
     * @since 1.0
     * @version 1.0
     */
    void SetAddressType(int type);

    /**
     * @brief Check if it is advertising service.
     *
     * @param uuid Service uuid.
     * @return Returns <b>true</b> if it is advertising service;
     *         returns <b>false</b> if it is not advertising service.
     * @since 1.0
     * @version 1.0
     */
    bool IsAdvertisingService(Uuid uuid);

    /**
     * @brief Check if include appearance.
     *
     * @return Returns <b>true</b> if include appearance;
     *         returns <b>false</b> if do not include appearance.
     * @since 1.0
     * @version 1.0
     */
    bool IsAppearance() const;

    /**
     * @brief Check if include manufacture data.
     *
     * @return Returns <b>true</b> if include manufacture data;
     *         returns <b>false</b> if do not include manufacture data.
     * @since 1.0
     * @version 1.0
     */
    bool IsManufacturerData() const;

    /**
     * @brief Check if include device name.
     *
     * @return Returns <b>true</b> if include device name;
     *         returns <b>false</b> if do not include device name.
     * @since 1.0
     * @version 1.0
     */
    bool IsName() const;

    /**
     * @brief Check if include device rssi.
     *
     * @return Returns <b>true</b> if include device rssi;
     *         returns <b>false</b> if do not include device rssi.
     * @since 1.0
     * @version 1.0
     */
    bool IsRSSI() const;

    /**
     * @brief Check if include service data.
     *
     * @return Returns <b>true</b> if include service data;
     *         returns <b>false</b> if do not include service data.
     * @since 1.0
     * @version 1.0
     */
    bool IsServiceData() const;

    /**
     * @brief Check if include service UUID.
     *
     * @return Returns <b>true</b> if include service UUID;
     *         returns <b>false</b> if do not include service UUID.
     * @since 1.0
     * @version 1.0
     */
    bool IsServiceUUID() const;

    /**
     * @brief Check if include TX power.
     *
     * @return Returns <b>true</b> if include TX power;
     *         returns <b>false</b> if do not include TX power.
     * @since 1.0
     * @version 1.0
     */
    bool IsTXPower() const;

    /**
     * @brief set device address.
     *
     * @param address device address.
     * @since 1.0
     * @version 1.0
     */
    void SetAddress(const RawAddress &address);

    /**
     * @brief set rssi value.
     *
     * @param rssi rssi value.
     * @since 1.0
     * @version 1.0
     */
    void SetRSSI(int8_t rssi);

    /**
     * @brief set rssi value.
     *
     * @param [in] rssi value.
     */
    bool IsConnectable() const;

    /**
     * @brief set rssi value.
     *
     * @param [in] rssi value.
     */
    void SetConnectable(bool connectable);

    /**
     * @brief Parse advertisement data.
     *
     * @param payload Advertisement packet.
     * @param total_len Advertisement packet len.
     * @since 1.0
     * @version 1.0
     */
    void ParseAdvertiserment(BlePeripheralDeviceParseAdvData &parseAdvData);

    /**
     * @brief Build advertisement data.
     *
     * @param payload Advertisement packet.
     * @param total_len Advertisement packet len.
     *
     * @since 1.0
     * @version 1.0
     */
    void BuildAdvertiserData(uint8_t adv_type, BlePeripheralDeviceParseAdvData &parseAdvData);

    /**
     * @brief Set service uuid 16 bit data.
     *
     * @param payload Advertisement packet.
     * @param total_len Advertisement packet len.
     * @since 1.0
     * @version 1.0
     */
    void SetServiceUUID16Bits(BlePeripheralDeviceParseAdvData &parseAdvData);

    /**
     * @brief Set service uuid 32 bit data.
     *
     * @param payload Advertisement packet.
     * @param total_len Advertisement packet len.
     * @since 1.0
     * @version 1.0
     */
    void SetServiceUUID32Bits(BlePeripheralDeviceParseAdvData &parseAdvData);

    /**
     * @brief Set service data uuid 16 bit data.
     *
     * @param payload Advertisement packet.
     * @param total_len Advertisement packet len.
     * @since 1.0
     * @version 1.0
     */
    void SetServiceDataUUID16Bits(BlePeripheralDeviceParseAdvData &parseAdvData);

    /**
     * @brief Set service data uuid 32 bit data.
     *
     * @param payload Advertisement packet.
     * @param total_len Advertisement packet len.
     * @since 1.0
     * @version 1.0
     */
    void SetServiceDataUUID32Bits(BlePeripheralDeviceParseAdvData &parseAdvData);

    /**
     * @brief Set service data uuid 128 bit data.
     *
     * @param payload Advertisement packet.
     * @param total_len Advertisement packet len.
     * @since 1.0
     * @version 1.0
     */
    void SetServiceDataUUID128Bits(BlePeripheralDeviceParseAdvData &parseAdvData);

    /**
     * @brief Set device name.
     *
     * @param name Device name.
     * @since 1.0
     * @version 1.0
     */
    void SetName(const std::string &name);

    /**
     * @brief Set device roles.
     *
     * @param roles Device roles.
     * @since 1.0
     * @version 1.0
     */
    void SetRoles(uint8_t roles);

    /**
     * @brief Get device roles.
     *
     * @return Returns central device roles.
     * @since 1.0
     * @version 1.0
     */
    uint8_t GetRoles() const;

    /**
     * @brief Set bonded from local.
     *
     * @param flag Advertiser flag.
     * @since 1.0
     * @version 1.0
     */
    void SetBondedFromLocal(bool flag);

    /**
     * @brief Set acl connect state.
     *
     * @param connectState Acl connect state.
     * @since 1.0
     * @version 1.0
     */
    void SetAclConnectState(int connectState);

    /**
     * @brief Set acl connection handle.
     *
     * @param handle Acl connection handle.
     * @since 1.0
     * @version 1.0
     */
    void SetConnectionHandle(int handle);

    /**
     * @brief Check if device acl connected.
     *
     * @return Returns <b>true</b> if device acl connected;
     *         returns <b>false</b> if device does not acl connect.
     * @since 1.0
     * @version 1.0
     */
    bool IsAclConnected() const;

    /**
     * @brief Check if device acl Encrypted.
     *
     * @return Returns <b>true</b> if device acl Encrypted;
     *         returns <b>false</b> if device does not acl Encrypt.
     * @since 1.0
     * @version 1.0
     */
    bool IsAclEncrypted() const;

    /**
     * @brief Check if device was bonded from local.
     *
     * @return Returns <b>true</b> if device was bonded from local;
     *         returns <b>false</b> if device was not bonded from local.
     * @since 1.0
     * @version 1.0
     */
    bool IsBondedFromLocal() const;

    /**
     * @brief Get acl connection handle.
     *
     * @return Returns acl connection handle;
     * @since 1.0
     * @version 1.0
     */
    int GetConnectionHandle() const;

    /**
     * @brief Get device type.
     *
     * @return Returns device type.
     * @since 1.0
     * @version 1.0
     */
    uint8_t GetDeviceType() const;

    /**
     * @brief Get advertising flag.
     *
     * @return Returns advertising flag.
     * @since 1.0
     * @version 1.0
     */
    uint8_t GetAdFlag() const;

    /**
     * @brief Get manufacturer Id.
     *
     * @return Returns manufacturer Id.
     * @since 1.0
     * @version 1.0
     */
    uint16_t GetManufacturerId() const;

    /**
     * @brief Get paired status.
     *
     * @return Returns paired status.
     * @since 1.0
     * @version 1.0
     */
    uint8_t GetPairedStatus() const;

    /**
     * @brief Set paired status.
     *
     * @param status Paired status.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetPairedStatus(uint8_t status);

    /**
     * @brief Get paired flag.
     *
     * @return Returns paired flag.
     * @since 1.0
     * @version 1.0
     */
    bool GetPairFlag() const;

    /**
     * @brief Set alias name.
     *
     * @param name Device alias name.
     * @since 1.0
     * @version 1.0
     */
    void SetAliasName(const std::string &name);

    /**
     * @brief Get alias name.
     *
     * @return Returns alias name.
     * @since 1.0
     * @version 1.0
     */
    std::string GetAliasName() const;

    /**
     * @brief Set IO capability.
     *
     * @param io IO capability
     * @since 1.0
     * @version 1.0
     */
    void SetIoCapability(uint8_t io);

    /**
     * @brief Get IO capability.
     *
     * @return Returns IO capability.
     * @since 1.0
     * @version 1.0
     */
    uint8_t GetIoCapability() const;

    /**
     * @brief Set device type.
     *
     * @param type Device type.
     * @since 1.0
     * @version 1.0
     */
    void SetDeviceType(uint8_t type);

private:
    /**
     * @brief Set advertising flag.
     *
     * @param adFlag Advertising flag.
     * @since 1.0
     * @version 1.0
     */
    void SetAdFlag(uint8_t adFlag);

    /**
     * @brief Set appearance.
     *
     * @param appearance Appearance.
     * @since 1.0
     * @version 1.0
     */
    void SetAppearance(uint16_t appearance);

    /**
     * @brief Set manufacturer data.
     *
     * @param manufacturerData Manufacturer data.
     * @since 1.0
     * @version 1.0
     */
    void SetManufacturerData(std::string manufacturerData);

    /**
     * @brief Set service data
     *
     * @param data Service data.
     * @since 1.0
     * @version 1.0
     */
    void SetServiceData(std::string data);

    /**
     * @brief Set service data UUID.
     *
     * @param uuid Service data UUID.
     * @since 1.0
     * @version 1.0
     */
    void SetServiceDataUUID(Uuid uuid);

    /**
     * @brief Set service  UUID.
     *
     * @param serviceUUID Service  UUID.
     * @since 1.0
     * @version 1.0
     */
    void SetServiceUUID(Uuid serviceUUID);
    /**
     * @brief Set TX power.
     *
     * @param txPower TX power.
     * @since 1.0
     * @version 1.0
     */
    void SetTXPower(int8_t txPower);

    /// include appearance?
    bool isAppearance_ = false;
    /// include Manufacturer Data?
    bool isManufacturerData_ = false;
    /// include device name?
    bool isName_ = false;
    /// include rssi value?
    bool isRSSI_ = false;
    /// include service data?
    bool isServiceData_ = false;
    /// include service uuid?
    bool isServiceUUID_ = false;
    /// include tx power?
    bool isTXPower_ = false;
    /// peer roles
    uint8_t roles_ = 0;
    /// device address
    RawAddress address_ = RawAddress("00:00:00:00:00:00");
    /// advertising flag
    uint8_t adFlag_ = 0;
    /// appearance
    uint16_t appearance_ = 0;
    /// manufacturer Data
    std::string manufacturerData_;
    /// manufacturer ID
    uint16_t manufacturerId_ = 0;
    /// device name
    std::string name_;
    /// rssi value
    int8_t rssi_ = 0;
    /// service uuid
    std::vector<Uuid> serviceUUIDs_;
    /// tx power
    int8_t txPower_ = -1;
    /// service data
    std::vector<std::string> serviceData_;
    /// service data uuid
    std::vector<Uuid> serviceDataUUIDs_;
    /// advertiser packet
    uint8_t *payload_ = nullptr;
    /// advertiser packet len
    size_t payloadLen_ = 0;
    /// address type
    uint8_t addressType_ = BLE_ADDR_TYPE_RANDOM;
    int aclConnected_ = -1;
    int connectionHandle_ = -1;
    bool bondFlag_ = false;
    bool pairFlag_ = false;
    uint8_t pairState_ = BLE_PAIR_NONE;
    uint8_t ioCapability_ = BLE_IO_CAP_NONE;
    std::string aliasName_;
    bool connectable_ = true;
};

/**
 * @brief Represents scan result.
 *
 * @since 1.0
 * @version 1.0
 */
class BleScanResultImpl {
public:
    /**
     * @brief A constructor used to create a <b>BleScanResultInternal</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    BleScanResultImpl()
    {}

    /**
     * @brief A destructor used to delete the <b>BleScanResultInternal</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    ~BleScanResultImpl()
    {}

    /**
     * @brief Get peripheral device.
     *
     * @return Returns peripheral device pointer.
     * @since 1.0
     * @version 1.0
     */
    BlePeripheralDevice GetPeripheralDevice() const;

    /**
     * @brief Set peripheral device.
     *
     * @param dev Peripheral device.
     * @since 1.0
     * @version 1.0
     */
    void SetPeripheralDevice(const BlePeripheralDevice &dev);

private:
    /// scan device results
    BlePeripheralDevice peripheralDevice_;
};
}  // namespace bluetooth

#endif  /// BLE_SERVICE_DATA_H
