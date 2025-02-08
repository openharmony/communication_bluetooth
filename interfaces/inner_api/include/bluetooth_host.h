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
 * @brief Defines bluetooth host, including observer and common functions.
 *
 * @since 6
 */

/**
 * @file bluetooth_host.h
 *
 * @brief Framework bluetooth host interface.
 *
 * @since 6
 */

#ifndef BLUETOOTH_HOST_H
#define BLUETOOTH_HOST_H

#include <string>

#include "bluetooth_battery_info.h"
#include "bluetooth_def.h"
#include "bluetooth_types.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_device_class.h"
#include "refbase.h"
#include "bluetooth_no_destructor.h"

namespace OHOS { class IRemoteObject; }
namespace OHOS {
namespace Bluetooth {
/**
 * @brief Represents framework host device basic observer.
 *
 * @since 6
 */
class BluetoothHostObserver {
public:
    /**
     * @brief A destructor used to delete the <b>BluetoothHostObserver</b> instance.
     *
     * @since 6
     */
    virtual ~BluetoothHostObserver() = default;

    // common
    /**
     * @brief Adapter state change function.
     *
     * @param transport Transport type when state change.
     *        BTTransport::ADAPTER_BREDR : classic;
     *        BTTransport::ADAPTER_BLE : ble.
     * @param state Change to the new state.
     *        BTStateID::STATE_TURNING_ON;
     *        BTStateID::STATE_TURN_ON;
     *        BTStateID::STATE_TURNING_OFF;
     *        BTStateID::STATE_TURN_OFF.
     * @since 6
     */
    virtual void OnStateChanged(const int transport, const int status) = 0;

    // gap
    /**
     * @brief Discovery state changed observer.
     *
     * @param status Device discovery status.
     * @since 6
     */
    virtual void OnDiscoveryStateChanged(int status) = 0;

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
        const BluetoothRemoteDevice &device, int rssi, const std::string deviceName, int deviceClass) = 0;

    /**
     * @brief Pair request observer.
     *
     * @param device Remote device.
     * @since 6
     */
    virtual void OnPairRequested(const BluetoothRemoteDevice &device) = 0;

    /**
     * @brief Pair confirmed observer.
     *
     * @param device Remote device.
     * @param reqType Pair type.
     * @param number Paired passkey.
     * @since 6
     */
    virtual void OnPairConfirmed(const BluetoothRemoteDevice &device, int reqType, int number) = 0;

    /**
     * @brief Scan mode changed observer.
     *
     * @param mode Device scan mode.
     * @since 6
     */
    virtual void OnScanModeChanged(int mode) = 0;

    /**
     * @brief Device name changed observer.
     *
     * @param deviceName Device name.
     * @since 6
     */
    virtual void OnDeviceNameChanged(const std::string &deviceName) = 0;

    /**
     * @brief Device address changed observer.
     *
     * @param address Device address.
     * @since 6
     */
    virtual void OnDeviceAddrChanged(const std::string &address) = 0;
};

/**
 * @brief Represents remote device observer.
 *
 * @since 6
 */
class BluetoothRemoteDeviceObserver {
public:
    /**
     * @brief A destructor used to delete the <b>BluetoothRemoteDeviceObserver</b> instance.
     *
     * @since 6
     */
    virtual ~BluetoothRemoteDeviceObserver() = default;

    /**
     * @brief Acl state changed observer.
     *
     * @param device Remote device.
     * @param state Remote device acl state.
     * @param reason Remote device reason.
     * @since 6
     */
    virtual void OnAclStateChanged(const BluetoothRemoteDevice &device, int state, unsigned int reason) = 0;

    /**
     * @brief Pair status changed observer.
     *
     * @param device Remote device.
     * @param status Remote device pair status.
     * @param cause Pair fail cause.
     * @since 12
     */
    virtual void OnPairStatusChanged(const BluetoothRemoteDevice &device, int status, int cause) = 0;

    /**
     * @brief Remote uuid changed observer.
     *
     * @param device Remote device.
     * @param uuids Remote device uuids.
     * @since 6
     */
    virtual void OnRemoteUuidChanged(const BluetoothRemoteDevice &device, const std::vector<ParcelUuid> &uuids) = 0;

    /**
     * @brief Remote name changed observer.
     *
     * @param device Remote device.
     * @param deviceName Remote device name.
     * @since 6
     */
    virtual void OnRemoteNameChanged(const BluetoothRemoteDevice &device, const std::string &deviceName) = 0;

    /**
     * @brief Remote alias changed observer.
     *
     * @param device Remote device.
     * @param alias Remote device alias.
     * @since 6
     */
    virtual void OnRemoteAliasChanged(const BluetoothRemoteDevice &device, const std::string &alias) = 0;

    /**
     * @brief Remote cod changed observer.
     *
     * @param device Remote device.
     * @param cod Remote device cod.
     * @since 6
     */
    virtual void OnRemoteCodChanged(const BluetoothRemoteDevice &device, const BluetoothDeviceClass &cod) = 0;

    /**
     * @brief Remote battery level changed observer.
     *
     * @param device Remote device.
     * @param cod Remote device battery Level.
     * @since 6
     */
    virtual void OnRemoteBatteryLevelChanged(const BluetoothRemoteDevice &device, int batteryLevel) = 0;

    /**
     * @brief Remote rssi event observer.
     *
     * @param device Remote device.
     * @param rssi Remote device rssi.
     * @param status Read status.
     * @since 6
     */
    virtual void OnReadRemoteRssiEvent(const BluetoothRemoteDevice &device, int rssi, int status) = 0;

    /**
     * @brief Remote device battery info observer.
     *
     * @param device Remote device.
     * @param batteryInfo Remote device batteryInfo
     * @since 12
     */
    virtual void OnRemoteBatteryChanged(const BluetoothRemoteDevice &device, const DeviceBatteryInfo &batteryInfo)
    {};

    /**
     * @brief Remote device common value observer.
     *
     * @param device Remote device.
     * @param value Remote device report info
     * @since 12
     */
    virtual void OnRemoteDeviceCommonInfoReport(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &value)
    {};
};

/**
 * @brief Represents bluetooth resource manager observer.
 *
 * @since 12
 */
class BluetoothResourceManagerObserver {
public:
    /**
     * @brief A destructor used to delete the <b>BluetoothResourceManagerObserver</b> instance.
     *
     * @since 12
     */
    virtual ~BluetoothResourceManagerObserver() = default;

    /**
     * @brief sensing state changed observer.
     *
     * @param eventId bluetooth resource manager event id.
     * @param info bluetooth sensing information.
     * @since 12
     */
    virtual void OnSensingStateChanged(uint8_t eventId, const SensingInfo &info)
    {};

    /**
     * @brief bluetooth resource decision observer.
     *
     * @param eventId bluetooth resource manager event id.
     * @param info bluetooth sensing information.
     * @param result bluetooth resource decision result.
     * @since 12
     */
    virtual void OnBluetoothResourceDecision(uint8_t eventId, const SensingInfo &info, uint32_t &result)
    {};
};

/**
 * @brief Represents framework host device.
 *
 * @since 6
 */
class BLUETOOTH_API BluetoothHost {
public:
    // common
    /**
     * @brief Get default host device.
     *
     * @return Returns the singleton instance.
     * @since 6
     */
    static BluetoothHost &GetDefaultHost();

    /**
     * @brief Get remote device instance.
     *
     * @param addr Remote device address.
     * @param transport Adapter transport.
     * @return Returns remote device instance.
     * @since 6
     */
    BluetoothRemoteDevice GetRemoteDevice(const std::string &addr, int transport) const;

    /**
     * @brief Register observer.
     *
     * @param observer Class BluetoothHostObserver pointer to register observer.
     * @since 6
     */
    void RegisterObserver(std::shared_ptr<BluetoothHostObserver> observer);

    /**
     * @brief Deregister observer.
     *
     * @param observer Class BluetoothHostObserver pointer to deregister observer.
     * @since 6
     */
    void DeregisterObserver(std::shared_ptr<BluetoothHostObserver> observer);

    /**
     * @brief Enable classic.
     *
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 6
     */
    int EnableBt();

    /**
     * @brief Disable classic.
     *
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 6
     */
    int DisableBt();

    /**
     * @brief Get classic enable/disable state.
     *
     * @return Returns classic enable/disable state.
     *         BTStateID::STATE_TURNING_ON;
     *         BTStateID::STATE_TURN_ON;
     *         BTStateID::STATE_TURNING_OFF;
     *         BTStateID::STATE_TURN_OFF.
     * @since 6
     * @deprecated since 14
     * @useinstead BluetoothHost#GetBluetoothState
     */
    int GetBtState() const;

    /**
     * @brief Get classic enable/disable state.
     *
     * @param Returns classic enable/disable state.
     *         BTStateID::STATE_TURNING_ON;
     *         BTStateID::STATE_TURN_ON;
     *         BTStateID::STATE_TURNING_OFF;
     *         BTStateID::STATE_TURN_OFF.
     * @since 6
     * @deprecated since 14
     * @useinstead BluetoothHost#GetBluetoothState
     */
    int GetBtState(int &state) const;

    /**
     * @brief Get the current state of the local Bluetooth adapter.
     *
     * @return current state of Bluetooth adapter.
     *         BluetoothState::STATE_TURN_OFF.
     *         BluetoothState::STATE_TURNING_ON;
     *         BluetoothState::STATE_TURN_ON;
     *         BluetoothState::STATE_TURNING_OFF;
     *         BluetoothState::STATE_BLE_TURNING_ON;
     *         BluetoothState::STATE_BLE_ON;
     *         BluetoothState::STATE_BLE_TURNING_OFF;
     * @since 14
     */
    BluetoothState GetBluetoothState(void) const;

    /**
     * @brief Disable ble.
     *
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 6
     */
    int DisableBle();

    /**
     * @brief Enable ble.
     *
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 6
     */
    int EnableBle();

    /**
     * @brief Enable bluetooth without AutoConnect.
     *
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 16
     */
    int EnableBluetoothNoAutoConnect();

    /**
     * @brief Enable bluetooth to restrict mode.
     *
     * @return Returns BT_NO_ERROR if the operation is accepted;
     *         returns others if the operation is rejected.
     * @since 12
     */
    int EnableBluetoothToRestrictMode(void);

    /**
     * @brief Get br/edr enable/disable state.
     *
     * @return Returns <b>true</b> if br is enabled;
     *         returns <b>false</b> if br is not enabled.
     * @since 6
     */
    bool IsBrEnabled() const;

    /**
     * @brief Get ble enable/disable state.
     *
     * @return Returns <b>true</b> if ble is enabled;
     *         returns <b>false</b> if ble is not enabled.
     * @since 6
     */
    bool IsBleEnabled() const;

    /**
     * @brief Factory reset bluetooth service.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int BluetoothFactoryReset();

    /**
     * @brief Get profile service ID list.
     *
     * @return Returns vector of enabled profile services ID.
     * @since 6
     */
    std::vector<uint32_t> GetProfileList() const;

    /**
     * @brief Get max audio connected devices number.
     *
     * @return Returns max device number that audio can connect.
     * @since 6
     */
    int GetMaxNumConnectedAudioDevices() const;

    /**
     * @brief Get bluetooth connects state.
     *
     * @return Returns bluetooth connects state.
     *         BTConnectState::CONNECTING;
     *         BTConnectState::CONNECTED;
     *         BTConnectState::DISCONNECTING;
     *         BTConnectState::DISCONNECTED.
     * @since 6
     */
    int GetBtConnectionState() const;

    /**
     * @brief Get bluetooth connects state.
     *
     * @return Returns bluetooth connects state.
     *         BTConnectState::CONNECTING;
     *         BTConnectState::CONNECTED;
     *         BTConnectState::DISCONNECTING;
     *         BTConnectState::DISCONNECTED.
     * @since 6
     */
    int GetBtConnectionState(int &state) const;

    /**
     * @brief Get profile service connect state.
     *
     * @param profileID Profile service ID.
     * @return Returns connect state for designated profile service.
     *         BTConnectState::CONNECTING;
     *         BTConnectState::CONNECTED;
     *         BTConnectState::DISCONNECTING;
     *         BTConnectState::DISCONNECTED.
     * @since 6
     */
    int GetBtProfileConnState(uint32_t profileId, int &state) const;

    /**
     * @brief Get local device supported uuids.
     *
     * @param[out] Vector which use to return support uuids.
     * @since 6
     */
    void GetLocalSupportedUuids(std::vector<ParcelUuid> &uuids);

    /**
     * @brief Start adapter manager, passthrough only.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool Start();

    /**
     * @brief Stop adapter manager, passthrough only.
     *
     * @since 6
     */
    void Stop();

    // gap
    /**
     * @brief Get local device class.
     *
     * @return Returns local device class.
     * @since 6
     */
    BluetoothDeviceClass GetLocalDeviceClass() const;

    /**
     * @brief Set local device class.
     *
     * @param deviceClass Device class.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetLocalDeviceClass(const BluetoothDeviceClass &deviceClass);

    /**
     * @brief Get local device address.
     *
     * @param addr local address.
     * @return Returns {@link BT_NO_ERROR} if the operation is successful;
     *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 6
     */
    int GetLocalAddress(std::string &addr) const;

    /**
     * @brief Get local device name.
     *
     * @return Returns local device name.
     * @since 6
     */
    std::string GetLocalName() const;

    /**
     * @brief Get local device name.
     *
     * @return Returns local device name.
     * @since 6
     */
    int GetLocalName(std::string &name) const;

    /**
     * @brief Set local device name.
     *
     * @param name Device name.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int SetLocalName(const std::string &name);

    /**
     * @brief Get device scan mode.
     *
     * @return Returns bluetooth scan mode.
     * @since 6
     */
    int GetBtScanMode(int32_t &scanMode) const;

    /**
     * @brief Set device scan mode.
     *
     * @param mode Scan mode.
     * @param duration Scan time.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int SetBtScanMode(int mode, int duration);

    /**
     * @brief Get local device bondable mode.
     *
     * @param transport Adapter transport.
     * @return Returns local device bondable mode.
     * @since 6
     */
    int GetBondableMode(int transport) const;

    /**
     * @brief Set local device bondable mode.
     *
     * @param transport Adapter transport.
     * @param mode Device bondable mode.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetBondableMode(int transport, int mode);

    /**
     * @brief Get device address.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int StartBtDiscovery();

    /**
     * @brief Cancel device discovery.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int CancelBtDiscovery();

    /**
     * @brief Check if device is discovering.
     *
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>false</b> if device is not discovering.
     * @since 6
     */
    int IsBtDiscovering(bool &isDisCovering, int transport = BT_TRANSPORT_BREDR) const;

    /**
     * @brief Get device discovery end time.
     *
     * @return Returns device discovery end time.
     * @since 6
     */
    long GetBtDiscoveryEndMillis() const;

    /**
     * @brief Get paired devices.
     *
     * @param transport Adapter transport.
     * @return Returns paired devices vector.
     * @since 6
     */
    int32_t GetPairedDevices(int transport, std::vector<BluetoothRemoteDevice> &pairedDevices) const;

    /**
     * @brief Remove pair.
     *
     * @param device Remote device address.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    int32_t RemovePair(const BluetoothRemoteDevice &device);

    /**
     * @brief Remove all pairs.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool RemoveAllPairs();

    /**
     * @brief Check if bluetooth address is valid.
     *
     * @param addr Bluetooth address.
     * @return Returns <b>true</b> if bluetooth address is valid;
     *         returns <b>false</b> if bluetooth address is not valid.
     * @since 6
     */
    static bool IsValidBluetoothAddr(const std::string &addr);

    /**
     * @brief Register remote device observer.
     *
     * @param observer Class BluetoothRemoteDeviceObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    void RegisterRemoteDeviceObserver(std::shared_ptr<BluetoothRemoteDeviceObserver> observer);

    /**
     * @brief Deregister remote device observer.
     *
     * @param observer Class BluetoothRemoteDeviceObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    void DeregisterRemoteDeviceObserver(std::shared_ptr<BluetoothRemoteDeviceObserver> observer);

    /**
     * @brief Get max advertising data length.
     *
     * @return Returns max advertising data length.
     * @since 6
     */
    int GetBleMaxAdvertisingDataLength() const;

    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);

    void LoadSystemAbilityFail();

    void OnRemoveBluetoothSystemAbility();

    /**
     * @brief Get local profile uuids.
     *
     * @return Returns local profile uuids.
     * @since 10
     */
    int32_t GetLocalProfileUuids(std::vector<std::string> &uuids);

    /**
    * @brief Set fast scan enable or disable.
    * @param isEnable set fast scan status flag.
    * @return Returns <b>true</b> if the operation is successful;
    *         returns <b>false</b> if the operation fails.
    */
    int SetFastScan(bool isEnable);

    /**
    * @brief Get the random address of a device.
    * If the address carried in the bluetooth interface is not obtained from the bluetooth,
    * the interface needs to be used for address translation.
    * @param realAddr real address.
    * @param[out] randomAddr random address.
    * @return Returns {@link BT_NO_ERROR} if get random address success;
    * returns an error code defined in {@link BtErrCode} otherwise.
    */
    int GetRandomAddress(const std::string &realAddr, std::string &randomAddr) const;

    /**
    * @brief Connects all allowed bluetooth profiles between the local and remote device.
    *
    * @param remoteAddr remote device addr.
    * @return Returns {@link BT_NO_ERROR} if the operation is successful;
    *         returns an error code defined in {@link BtErrCode} otherwise.
    * @since 11
    */
    int ConnectAllowedProfiles(const std::string &remoteAddr) const;

    /**
    * @brief Disconnects all allowed bluetooth profiles between the local and remote device.
    *
    * @param remoteAddr remote device addr.
    * @return Returns {@link BT_NO_ERROR} if the operation is successful;
    *         returns an error code defined in {@link BtErrCode} otherwise.
    * @since 11
    */
    int DisconnectAllowedProfiles(const std::string &remoteAddr) const;

    /**
    * @brief Restrict Bluetooth BR/EDR ability, just BLE ability available.
    *
    * @return Returns {@link BT_NO_ERROR} if the operation is successful;
    *         returns an error code defined in {@link BtErrCode} otherwise.
    * @since 12
    */
    int RestrictBluetooth();

    /**
    * @brief update virtual device
    *
    * @param action add or delete virtual device.
    * @param device device need to be operator.
    * @since 12
    */
    void UpdateVirtualDevice(int32_t action, const std::string &address);

    /**
    * @brief Restrict Bluetooth BR/EDR ability, just BLE ability available.
    *
    * @param type satellite control type.
    * @param state satellite state.
    * @return Returns {@link BT_NO_ERROR} if the operation is successful;
    *         returns an error code defined in {@link BtErrCode} otherwise.
    * @since 12
    */
    int SatelliteControl(int type, int state);

    /**
     * @brief Register bluetooth resource manager observer.
     *
     * @param observer Class RegisterBtResourceManagerObserver pointer to register observer.
     * @since 12
     */
    void RegisterBtResourceManagerObserver(std::shared_ptr<BluetoothResourceManagerObserver> observer);

    /**
     * @brief Deregister bluetooth resource manager observer.
     *
     * @param observer Class RegisterBtResourceManagerObserver pointer to deregister observer.
     * @since 12
     */
    void DeregisterBtResourceManagerObserver(std::shared_ptr<BluetoothResourceManagerObserver> observer);

    /**
     * @brief Set local adapter scan level.
     *
     * @param level Scan level.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 12
     */
    int SetFastScanLevel(int level);

    /**
     * @brief Close the bluetooth host to release resources, only called before the process exits.
     *
     * @since 13
     */
    void Close(void);

    int32_t UpdateCloudBluetoothDevice(const std::vector<TrustPairDeviceParam> &cloudDevices);
private:
    /**
     * @brief A constructor used to create a <b>BluetoothHost</b> instance.
     *
     * @since 6
     */
    BluetoothHost();

    /**
     * @brief A destructor used to delete the <b>BluetoothHost</b> instance.
     *
     * @since 6
     */
    ~BluetoothHost();

    /**
    * @brief Check whether bluetooth is prohibited by EDM.
    *
    * @return Returns <b>true</b> if bluetooth is prohibited, returns <b>false</b> otherwise.
    * @since 11
    */
    bool IsBtProhibitedByEdm(void);

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothHost);
    BLUETOOTH_DECLARE_IMPL();

#ifdef DTFUZZ_TEST
    friend class BluetoothNoDestructor<BluetoothHost>;
#endif
};
} // namespace Bluetooth
} // namespace OHOS

#endif  // BLUETOOTH_HOST_H
