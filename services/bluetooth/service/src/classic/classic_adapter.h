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

#ifndef CLASSIC_ADAPTER_H
#define CLASSIC_ADAPTER_H

#include <map>
#include <vector>
#include <set>
#include <atomic>

#include "base_def.h"
#include "bt_uuid.h"
#include "remote_device_properties.h"
#include "adapter_properties.h"
#include "bluetooth.h"
#include "types/bt_transport.h"
#include "service_util.h"
#include "safe_map.h"

#include "context.h"
#include "interface_adapter_classic.h"
#include "log.h"
#include "raw_address.h"
#include "interface_profile_a2dp_src.h"
#ifdef BLUETOOTH_HFP_HF_ENABLE
#include "interface_profile_hfp_hf.h"
#endif
#include "interface_profile_hfp_ag.h"
#include "interface_profile_hid_host.h"
#include "interface_profile_hid_device.h"
#include "interface_profile_pbap_pse.h"
#include "interface_profile_map_mse.h"
#include "interface_profile_hearing_aid.h"
#include "interface_profile_pan.h"
#include "interface_profile_bas.h"


/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines adapter classic common functions.
 *
 */

/**
 * @file classic_adapter.h
 *
 * @brief Adapter classic.
 *
 */

namespace OHOS {
namespace bluetooth {
/**
 * @brief Represents classic adapter.
 *
 */
class ClassicAdapter : public IAdapterClassic, public utility::Context {
public:
    /**
     * @brief A constructor used to create a <b>ClassicAdapter</b> instance.
     *
     */
    ClassicAdapter();

    /**
     * @brief A destructor used to delete the <b>ClassicAdapter</b> instance.
     *
     */
    ~ClassicAdapter() override;

    /**
     * @brief Get local device address.
     *
     * @return Returns local device address.
     */
    std::string GetLocalAddress() const override;

    /**
     * @brief Generate local OOB data.
     *
     * @param transport BR or BLE.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 23
     */
    int32_t GenerateLocalOobData(int32_t transport) const override;

    /**
     * @brief Get local device name.
     *
     * @return Returns local device name.
     */
    std::string GetLocalName() const override;

    /**
     * @brief Set local device name.
     *
     * @param name Device name.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetLocalName(const std::string &name) const override;

    /**
     * @brief Set bondable mode.
     *
     * @param mode Bondable mode.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetBondableMode(int mode) const override;

    /**
     * @brief Get bondable mode.
     *
     * @return Returns bondable mode.
     */
    int GetBondableMode() const override;

    /**
     * @brief Get remote device name.
     *
     * @param device Remote device.
     * @param alias Indicates whether to obtain the device alias.
     * @return Returns device name.
     */
    std::string GetDeviceName(const RawAddress &device, bool alias = true) const override;

    /**
     * @brief Get remote device uuids.
     *
     * @param device Remote device.
     * @return Returns device uuids.
     */
    std::vector<std::string> GetDeviceUuids(const RawAddress &device) const override;

    /**
     * @brief Get local profile uuids.
     *
     * @return Returns local uuids.
     */
    std::vector<std::string> GetLocalProfileUuids() const override;

    /**
     * @brief Get paired devices.
     *
     * @param transport transport
     * @return Returns device list.
     */
    std::vector<RawAddress> GetPairedDevices() const override;

    int StartPairInner(const RawAddress &device, int32_t transport, const Bluetooth::BluetoothOobData &oobData,
        const BtInterface *btInterface);

    /**
     * @brief Local device start pair.
     *
     * @param transport transport
     * @param device Remote device.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    /**
     * @brief Local device start pair.
     *
     * @param transport transport
     * @param device Remote device.
     * @param oobData OOB data used for pairing with a remote Bluetooth device using the Out Of Band mechanism.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 23
     */
    bool StartPair(int32_t transport, const RawAddress &device, const Bluetooth::BluetoothOobData &oobData) override;

    /**
     * @brief Credible Device start pair.
     *
     * @param transport transport
     * @param device Remote device.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool StartCrediblePair(int32_t transport, const RawAddress &device) override;

    /**
     * @brief Check if remote device was bonded from local.
     *
     * @param device Remote device.
     * @return Returns <b>true</b> if device was bonded from local;
     *         returns <b>false</b> if device was not bonded from local.
     */
    bool IsBondedFromLocal(const RawAddress &device) const override;

    /**
     * @brief Cancel pair operation.
     *
     * @param device Remote device.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool CancelPairing(const RawAddress &device) override;

    /**
     * @brief Remove pair.
     *
     * @param transport transport
     * @param device Remote device.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool RemovePair(int32_t transport, const RawAddress &device) override;

    /**
     * @brief Get device pair state.
     *
     * @param device Remote device.
     * @return Returns device pair state.
     */
    int GetPairState(const RawAddress &device) const override;

    /**
     * @brief Set device pairing confirmation.
     *
     * @param device Remote device.
     * @param accept Set gap accept flag.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetDevicePairingConfirmation(const RawAddress &device, bool accept) const override;

    /**
     * @brief Set device pair passkey.
     *
     * @param device Remote device.
     * @param passkey Device passkey.
     * @param accept Set gap accept flag.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetDevicePasskey(const RawAddress &device, int passkey, bool accept) const override;

    /**
     * @brief Check remote device pair request reply.
     *
     * @param device Remote device.
     * @param accept Set gap accept flag.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool PairRequestReply(const RawAddress &device, bool accept) const override;

    /**
     * @brief Check if acl connected with remote device.
     *
     * @param device Remote device.
     * @return Returns <b>true</b> if device acl connected;
     *         returns <b>false</b> if device does not acl connect.
     */
    bool IsAclConnected(const RawAddress &device) const override;

    /**
     * @brief Check if remote device acl Encrypted.
     *
     * @return Returns <b>true</b> if device acl Encrypted;
     *         returns <b>false</b> if device does not acl Encrypt.
     */
    bool IsAclEncrypted(const RawAddress &device) const override;

    /**
     * @brief Get the instance of the ClassicAdapter.
     *
     * @return Returns the instance of the ClassicAdapter.
     */
    utility::Context *GetContext() override;

    /**
     * @brief Enable classic service.
     *
     */
    void Enable() override;

    /**
     * @brief Disable classic service.
     *
     */
    void Disable() override;

    /**
     * @brief PostEnable classic service.
     *
     */
    void PostEnable() override;

    /**
     * @brief Get remote device type.
     *
     * @param device Remote device.
     * @return Returns remote device type.
     */
    int GetDeviceType(const RawAddress &device) const override;

    /**
     * @brief Get remote device class.
     *
     * @param device Remote device.
     * @return Returns remote device class.
     */
    int GetDeviceClass(const RawAddress &device) const override;

    /**
     * @brief Get remote device alias name.
     *
     * @param device Remote device
     * @return Returns remote device alias name.
     */
    std::string GetAliasName(const RawAddress &device) const override;

    /**
     * @brief Set remote device alias name.
     *
     * @param device Remote device which setted alias name.
     * @param name Alias name.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetAliasName(const RawAddress &device, const std::string &name) override;

    /**
     * @brief Get remote device Hw RemoteDeviceType.
     *
     * @param device Remote device address
     * @return Remote device Hw RemoteDeviceType.
     */
    int GetHwRemoteDeviceType(const RawAddress &device) const override;

    /**
     * @brief Register remote device observer.
     *
     * @param observer Class IClassicRemoteDeviceObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool RegisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const override;

    /**
     * @brief Deregister remote device observer.
     *
     * @param observer Class IClassicRemoteDeviceObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool DeregisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const override;

    /**
     * @brief Register classic adapter observer.
     *
     * @param observer Class IAdapterClassicObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool RegisterClassicAdapterObserver(IAdapterClassicObserver &observer) const override;

    /**
     * @brief Deregister classic adapter observer.
     *
     * @param observer Class IAdapterClassicObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool DeregisterClassicAdapterObserver(IAdapterClassicObserver &observer) const override;

    /**
     * @brief Remove all pairs.
     *
     * @param transport transport
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool RemoveAllPairs() override;

    /**
     * @brief Set device pair pin.
     *
     * @param device Remote device address.
     * @param pin Pin code.
     * @param length Pin code length.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetDevicePin(const RawAddress &device, const std::string &pinCode) const override;

    /**
     * @brief Set device scan mode.
     *
     * @param mode Scan mode.
     * @param duration Scan time.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetBtScanMode(int mode, int duration) override;

    /**
     * @brief Get device scan mode.
     *
     * @return Returns bluetooth scan mode.
     */
    int GetBtScanMode() const override;

    /**
     * @brief Get local device class.
     *
     * @return Returns local device class.
     */
    int GetLocalDeviceClass() const override;

    /**
     * @brief Set local device class.
     *
     * @param deviceClass Device class.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetLocalDeviceClass(int deviceClass) const override;

    /**
     * @brief Start device discovery.
     *
     * @return Returns operation result.
     */
    int32_t StartBtDiscovery() override;

    /**
     * @brief Cancel device discovery.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool CancelBtDiscovery() override;

    /**
     * @brief Check if device is discovering.
     *
     * @return Returns <b>true</b> if device is discovering;
     *         returns <b>false</b> if device is not discovering.
     */
    bool IsBtDiscovering() const override;

    void SendRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo) override;

    void SendRemoteEchoInfo(const RawAddress &device, const std::vector<uint8_t> &value) override;

    /**
     * @brief Send acl state changed.
     *
     * @param device Remote device.
     * @param state Acl state.
     * @param reason Reason.
     */
    void SendAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus);

    /**
     * @brief Send remote device cod changed.
     *
     * @param device Remote device.
     * @param cod Remote device cod.
     */
    void SendRemoteCodChanged(const RawAddress &device, int cod) const;

    /**
     * @brief Send remote device name changed.
     *
     * @param device Remote device.
     * @param deviceName Remote device name.
     */
    void SendRemoteNameChanged(const RawAddress &device, const std::string &deviceName) const;

    /**
     * @brief Save remote device uuids.
     *        If uuids is changed, send OnRemoteUuidChanged to client.
     *
     * @param remote Remote device.
     * @param uuids Remote device uuids.
     */
    void SaveRemoteDeviceUuids(std::shared_ptr<BluetoothDevice> remote, const std::vector<Uuid> &uuids);

    /**
     * @brief Send discovery result.
     *
     * @param device Remote device.
     * @param rssi Rssi of device.
     * @param deviceName Name of device.
     * @param deviceClass Class of device.
     */
    void SendDiscoveryResult(
        const RawAddress &device, int rssi, const std::string deviceName, int deviceClass) const;

    /**
     * @brief DisconnectAcl when reach max br link.
     *
     * @param address disconnect address.
     */
    void DisconnectAcl(const std::string &address);

    /**
     * @brief CreateAclConnect when auto link pagetiomeout retry.
     *
     * @param address CreateAclConnect address.
     */
    void CreateAclConnect(const std::string &address);

    /**
     * @brief Get device discovery end time.
     *
     * @return Returns device discovery end time.
     */
    long GetBtDiscoveryEndMillis() const override;
    int GetRemoteDeviceBatteryInfo(const RawAddress &device, BatteryInfo &batteryInfo) const override;
    int SetRemoteDeviceBatteryInfo(const RawAddress &device, const BatteryInfo &batteryInfo) const override;
    void DiscoveryStateChanged(BtDiscoveryState state);
    void BondStateChanged(BtStackStatus status, OHOS::bluetooth::RawAddress* bdAddr, BtBondState state);
    void SspRequest(OHOS::bluetooth::RawAddress* remoteBdAddr, BtBdname* bdName, uint32_t cod,
                    BtSspVariant pairingVariant, uint32_t passKey);
    void PinRequest(OHOS::bluetooth::RawAddress* remoteBdAddr, BtBdname* bdName, uint32_t cod, bool min16Digit);
    void GenerateLocalOobDataCb(int32_t status, const Bluetooth::BluetoothOobData &data);
    void UpdateDiscovertState(int discoveryState);
    void ResumeBleStackDevice(void);
    void CancelDiscoveryIfInCloudBonding(const RawAddress &device);

    /**
     * @brief Set fast scan status.
     *
     * @param isEnable Fast scan enable or disable.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetFastScan(bool isEnable) override;

    /**
     * @brief Connects all allowed bluetooth profiles between the local and remote device.
     *        Currently, only the hfpAg, a2dpsrc, and hid services support connections.
     *        If other services support connections, need to add corresponding connections.
     *
     * @param device Remote device.
     */
    void ConnectAllowedProfiles(
        const RawAddress &device, bool isConnectAllEnabled = true) override;

    /**
     * @brief set connect strategy
     *
     * @param device Remote device.
     */
    void SetConnectStrategy(
        const RawAddress &device, bool isConnectAllEnabled = true);

    /**
     * @brief  Disconnects all connected bluetooth profiles between the local and remote device.
     *         Currently, only the hfpAg, a2dpsrc, and hid services support disconnections.
     *         If other services support connections, need to add corresponding disconnections.
     *
     * @param device Remote device.
     */
    void DisconnectAllowedProfiles(const RawAddress &device, bool isDisconnectAllEnabled = true) override;

    /**
     * @brief  Disconnects all connected bluetooth profiles between the local and remote device.
     *         Only disconnect profiles, not set virtual connection switch.
     *
     * @param device Remote device.
     */
    void DisconnectAllowedProfilesNotSetVirtualConnSwitch(const RawAddress &device) override;

    /**
    * @brief Perform the factoryreset operation
    *
    * @return Returns {@link BT_NO_ERROR} if factoryreset success;
    * returns an error code defined in {@link BtErrCode} otherwise.
    */
    int32_t FactoryReset() override;

    /**
     * @brief Get the model ID of a remote device.
     *
     * @param device Remote device.
     * @return Model Id
     * @since 11
     */
    std::string GetDeviceModelId(const RawAddress &device) override;

    /**
     * @brief Set the custom type of a remote device.
     *
     * @param device Remote device.
     * @param customType custom type.
     * @return Returns {@link BT_NO_ERROR} if SetDeviceCustomType success;
     * @return Returns an error code defined in {@link BtErrCode} otherwise.
     * @since 12
     */
    int32_t SetDeviceCustomType(const RawAddress &device, int32_t customType) const override;

    /**
     * @brief Get remote device information.
     *
     * @param device Remote device.
     * @param deviceInfo[out] device information.
     * @return Returns {@link BT_NO_ERROR} if GetDeviceCustomType success;
     * @return Returns an error code defined in {@link BtErrCode} otherwise.
     * @since 12
     */
    int32_t GetRemoteDeviceInfo(const RawAddress &device,
        Bluetooth::BluetoothRemoteDeviceInfo &deviceInfo) override;

    /**
     * @brief  Disconnects all Paired Device connected bluetooth profiles between the local and remote device.
     *         Currently, only the hfpAg, a2dpsrc, and hid services support disconnections.
     *         If other services support connections, need to add corresponding disconnections.
     *
     */
    void DisconnectPairedDeviceProfiles() override;

	/**
     * @brief get the property info of a remote device.
     *
     * @param device Remote device.
     * @param type Property type.
     * @param type Property value.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 12
     */
    bool GetRemoteDevicePropertyInfo(const RawAddress &device, BtPropertyType type, std::string &property);

    /**
     * @brief passive pairing mode, a profile connection request needs to be sent to the peer end three seconds
     * @since 12
     */
    void PassivePairToDelayConnect(const RawAddress &device) override;

    /**
     * @brief need delay connect or not
     * @since 12
     */
    bool IsNeedDelayConnect(const RawAddress &device) override;

    /**
     * @brief device connection is inconing
     * @return Returns <b>true</b> if the connection is incoming;
     * @since 12
     */
    bool IsIncomingConnection(const RawAddress &device);

    /**
     * @brief Start BtScanTimer
     * @since 12
     */
    void StartBtScanModeTimer(int mode, int duration);

    /**
     * @brief if BtScanTimer started to stop
     * @since 12
     */
    void StopBtScanModeTimer();

    /**
     * @brief Set fast scan status.
     *
     * @param level Fast scan level.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetFastScanLevel(int level) override;

    /**
     * @brief Control the actions of Bluetooth peripherals.
     *
     * @param deviceId Address of the peripheral.
     * @param controlType The control type.
     * @param controlTypeVal The value of control type.
     * @param controlObject The control object.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 15
     */
    int32_t ControlDeviceAction(const std::string &deviceId, uint32_t controlType,
        uint32_t controlTypeVal, uint32_t controlObject) override;

    /**
     * @brief Get latest connection time of device.
     *
     * @param device Remote device.
     * @param connectionTime connection time.
     * @since 15
     */
    int32_t GetLastConnectionTime(const RawAddress &device, int64_t &connectionTime) override;

    /**
     * @brief Set latest connection time of device.
     *
     * @param device Remote device.
     * @param connectionTime connection time.
     * @since 15
     */
    void SetConnectionTime(const RawAddress &device, int64_t connectionTime);

    /**
     * @brief GetCloudBondState
     * @since 16
     */
    int32_t GetCloudBondState(const RawAddress &device, int32_t &cloudBondState) override;

    /**
     * @brief GetDeviceTransport
     * @since 20
     */
    int32_t GetDeviceTransport(const RawAddress &device, int32_t &transport) override;

    /**
     * @brief UpdateCloudBluetoothDevice
     * @since 16
     */
    int32_t UpdateCloudBluetoothDevice(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList) override;

    /**
     * @brief StartCloudPair
     * @since 16
     */
    bool StartCloudPair(const RawAddress &device);

    /**
     * @brief SetCloudBondState
     * @since 16
     */
    void SetCloudBondState(const RawAddress &device, int32_t cloudBondState);

    /**
     * @brief GetCloudBondState
     * @since 16
     */
    int32_t GetCloudBondState(const RawAddress &device) const;

    /**
     * @brief IsCloudDevice
     * @since 16
     */
    bool IsCloudDevice(const RawAddress &device);

    /**
     * @brief IsStartCloudPair
     * @since 16
     */
    bool IsStartCloudPair(const RawAddress &device);

    void ClearCloudDevice(std::shared_ptr<BluetoothDevice> &device);
    bool IsInCloudBondingState(std::shared_ptr<BluetoothDevice> &device);

    void NotifyDisconnectState(const RawAddress &device, int32_t cause = 0);
    void SendRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) const;
    void SendScanModeChanged(const int scanMode) const;
    bool CheckBondStateAndReturn(std::shared_ptr<BluetoothDevice> &remoteDevice, int unbondCause);
    void SetBondState(BtStackStatus status, OHOS::bluetooth::RawAddress bdAddr, BtBondState state);
    int32_t UpdateCloudBluetoothDeviceInner(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList);
	std::string GetCarKeyDfxData() const override;
    void SetCarKeyCardData(const std::string &address, int32_t action) override;
    void InitCarKeyInfo() override;
    void AddPendingConnectDevice(const RawAddress &address, uint32_t profileId);
    void RemovePendingConnectDevice(const RawAddress &address);
    void NotifyBondStateChanged(BtStackStatus status, OHOS::bluetooth::RawAddress bdAddr, BtBondState state);
    bool StartRemoteSdpSearch(const std::string &address, const std::string &uuid) override;
    bool GetRemoteServices(const std::string &address) override;
    int32_t SetConnectionPriority(const std::string &address, int32_t priority) override;
private:
    /**
     * @brief Connects only profiles that the connection strategy is allowed.
     *        Currently, only the hfpAg, hfphf, a2dpsrc, hid, and pan services support connections.
     *        If other services support connections, need to add corresponding connections.
     *
     * @param device Remote device.
     */
    void ConnectClassicProfiles(const RawAddress &device, bool isConnectMoreProfileEnabled,
        int32_t &numProfilesConnected) const;

    /**
     * @brief Connects only profiles that the connection strategy is allowed.
     *        Currently, only the aid and bas services support connections.
     *        If other services support connections, need to add corresponding connections.
     *
     * @param device Remote device.
     */
    void ConnectBleProfiles(const RawAddress &device, bool isConnectMoreProfileEnabled, 
        int32_t &numProfilesConnected) const;

    /**
     * @brief Connects only profiles that the connection strategy is allowed.
     *        Currently, only the hfpAg and hfphf services support connections.
     *        If other services support connections, need to add corresponding connections.
     *
     * @param device Remote device.
     */
    void ConnectHfpProfiles(const RawAddress &device, bool isConnectMoreProfileEnabled,
        int32_t &numProfilesConnected) const;

    /**
     * @brief Connects only profiles that the connection strategy is allowed.
     *        Currently, only the a2dpsrc services support connections.
     *        If other services support connections, need to add corresponding connections.
     *
     * @param device Remote device.
     */
    void ConnectA2dpProfiles(const RawAddress &device, bool isConnectMoreProfileEnabled,
        int32_t &numProfilesConnected) const;

    /**
     * @brief set classic connect strategy including hfpAg, hfphf, a2dpsrc, hid, and pan profiles.
     *
     * @param device Remote device.
     */
    void SetClassicConnectStrategy(const RawAddress &device, bool isConnectAllEnabled, 
        std::vector<Uuid> localUuids, std::vector<Uuid> remoteUuids, int32_t &numSetProfilesStrategies);

    /**
     * @brief set ble connect strategy including aid and bas profiles.
     *
     * @param device Remote device.
     */
    void SetBleConnectStrategy(const RawAddress &device, bool isConnectAllEnabled, 
        std::vector<Uuid> localUuids, std::vector<Uuid> remoteUuids, int32_t &numSetProfilesStrategies);

    /**
     * @brief set hfphf and hfpag connect strategy.
     *
     * @param device Remote device.
     */
    void SetHfpConnectStrategy(const RawAddress &device, bool isConnectAllEnabled, 
        std::vector<Uuid> localUuids, std::vector<Uuid> remoteUuids, int32_t &numSetProfilesStrategies);

    /**
     * @brief set a2dpsrc connect strategy.
     *
     * @param device Remote device.
     */
    void SetA2dpConnectStrategy(const RawAddress &device, bool isConnectAllEnabled, 
        std::vector<Uuid> localUuids, std::vector<Uuid> remoteUuids, int32_t &numSetProfilesStrategies);

    /**
     * @brief  Disconnect classic Profiles including hfpAg, hfphf, a2dpsrc, hid, pbappse, mappse and pan profiles.
     *
     * @param device Remote device.
     */
    void DisconnectClassicProfiles(const RawAddress &device, bool isDisconnectAllEnabled = true);

    /**
     * @brief  Disconnect ble Profiles including aid and bas profiles.
     *
     * @param device Remote device.
     */
    void DisconnectBleProfiles(const RawAddress &device, bool isDisconnectAllEnabled = true);

    /**
     * @brief  Disconnect hfphf and hfpag Profiles.
     *
     * @param device Remote device.
     */
    void DisconnectHfpProfiles(const RawAddress &device, bool isDisconnectAllEnabled = true);

    /**
     * @brief  Disconnect a2dpsrc Profiles.
     *
     * @param device Remote device.
     */
    void DisconnectA2dpProfiles(const RawAddress &device, bool isDisconnectAllEnabled = true);

    /**
     * @brief  Check and set virtual auto connect Switch.
     *
     * @param device Remote device.
     */
    void CheckAndSetVirtualAutoConnSwitch(const RawAddress &device);

    /**
     * @brief  Disconnect Profiles.
     *
     * @param device Remote device.
     */
    void DisconnectProfiles(const RawAddress &device, bool isDisconnectAllEnabled = true);

    /**
     * Check whether the remote device needs wait for sdp complete when paired
     *
     * @param remoteDevice the remote device
     * @return true if the remote device's uuid is empty, and it is br or dual type, false otherwise
     */
    bool NeedWaitForSdpComplete(std::shared_ptr<BluetoothDevice> remoteDevice);

    /**
     * Check whether the profile is supported by the local device and remote device
     *
     * @param profile is the profile we are checking for support
     * @param localUuids  is a vector of all supported profiles by the local device
     * @param remoteUuids is a vector of all supported profiles by the remote device
     * @return true if the profile is supported by both the local and remote device, false otherwise
     */
    bool IsProfileSupported(uint32_t profile,
        std::vector<Uuid> &localUuids, std::vector<Uuid> &remoteUuids) const;
    /**
     * @brief Check whether the remote device has a profile that the connection strategy is allowed.
     *        Currently, only the hfpAg, a2dpsrc, and hid services support connections.
     *
     * @param device Remote device.
     * @return true if any profile connection strategy is allowed, false otherwise.
     */
    bool IsAnyProfilesAllowed(const RawAddress &device) const;

    /**
     * @brief Connects only profiles that the connection strategy is allowed.
     *        Currently, only the hfpAg, a2dpsrc, and hid services support connections.
     *        If other services support connections, need to add corresponding connections.
     *
     * @param device Remote device.
     */
    void ConnectAnyAllowedProfiles(
        const RawAddress &device, bool isConnectMoreProfileEnabled = true) const;

    /**
     * @brief Send pair confirmed.
     *
     * @param device Remote device.
     * @param reqType confirmed type (SSP/Pin Code).
     * @param number confirmed number.
     */
    void SendPairConfirmed(const RawAddress &device, int reqType, int number);

    /**
     * @brief Send discovery state changed.
     *
     */
    void SendDiscoveryStateChanged(int discoveryState) const;

    /**
     * @brief Check uuids1 is equal with uuids2.
     *
     * @param uuids1 uuids.
     * @param uuids2 uuids.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool IsUuidsEqual(const std::vector<Uuid> &uuids1, const std::vector<Uuid> &uuids2) const;

    /**
     * @brief Send pair status changed.
     *
     * @param transport Remote device transport.
     * @param device Remote device.
     * @param status Pair status.
     * @param cause Pair fail cause.
     */
    void SendPairStatusChanged(const BTTransport transport, const RawAddress &device, int status, int cause,
        const std::string &causeMessage) const;

    /**
     * @brief User confirm auto reply.
     *
     * @param device Remote device.
     * @param reqType Request type.
     * @param accept Request accept or not.
     */
    void UserConfirmAutoReply(const RawAddress &device, int reqType, bool accept) const;

        /**
     * @brief Load paired device info from config file.
     *
     */
    void LoadPairedDeviceInfo();

    /**
     * @brief Process post enable.
     *
     */
    void ProcessPostEnable();
    void DiscoveryStateChangedInner(BtDiscoveryState state);
    void CovertUnbondCause(BtStackStatus status, int &unbondCause, std::string &causeMessage);
    void BondStateChangedInner(BtStackStatus status, OHOS::bluetooth::RawAddress bdAddr, BtBondState state);
    void SspRequestInner(OHOS::bluetooth::RawAddress remoteBdAddr, BtBdname* bdName, uint32_t cod,
        BtSspVariant pairingVariant, uint32_t passKey);
    void PinRequestInner(OHOS::bluetooth::RawAddress remoteBdAddr, BtBdname* bdName, uint32_t cod, bool min16Digit);
    void StartUp();
    void ShutDown();
    bool SetPairingConfirmationIfNeed(const std::string &address);
    void DeleteDeviceInfoFiles();
    bool FindPendingPairedDevice(const std::string device);
    void DeletePendingPairedDevice(const std::string device);
    void ReleaseBtChrInfos();
    bool IsCustomTypeSupported(const RawAddress &device, int32_t customType) const;
    bool GetHwPropertyInfo(const RawAddress &device, BtPropertyType type, std::string &property, bool toHex = true);
    bool IsNeededReturnCodForAudio(int32_t uid, int32_t customType) const;
    void GetPeripheralCod(const RawAddress &device, int32_t &cod) const;
    bool RemovePairInner(int32_t transport, const RawAddress &device) const;
    void CheckDeviceUuidsAndSendChr(std::shared_ptr<BluetoothDevice> remoteDevice);
    void HandleBondStateFailed(
        BtStackStatus status, std::shared_ptr<BluetoothDevice> remoteDevice, const RawAddress &device);
    void HandleBondStateBondNone(BtStackStatus status, OHOS::bluetooth::RawAddress bdAddr,
        std::shared_ptr<BluetoothDevice> remoteDevice, const RawAddress &device, BtBondState state);

    void HandleBondStateBonding(std::shared_ptr<BluetoothDevice> remoteDevice, const RawAddress &device);
    bool IsDisconnectAclBeforeBond(const std::string device) const;
    void AddDisconnectAclBeforeBondDevice(const std::string device);
    void DelDisconnectAclBeforeBondDevice(const std::string device);
    bool SetBtScanModeProperty(const BtInterface *btInterface, int mode);
    void IsDeviceReadyConnect(const RawAddress &address);
    bool SatisfyDisconnectAclCondition(const RawAddress &device, bool isSystemHap);
    std::string CovertUnbondMessage(BtStackStatus status);
    IProfile* GetProfileService(uint32_t profileId) const;
    bool IsProfileAllowed(uint32_t profileId, const RawAddress &device) const;
    bool GetProfileSupportState(uint32_t profileId, const RawAddress &device,
        bool isConnectMoreProfileEnabled) const;

    std::atomic_int discoveryState_ {};
    std::atomic_int scanMode_ {};
    std::atomic_long discoveryEndMs_ {};
    AdapterProperties *adapterProperties_;
    IProfileHfpAg *agService_ {nullptr};
#ifdef BLUETOOTH_HFP_HF_ENABLE
    IProfileHfpHf *hfService_ {nullptr};
#endif
    IProfileA2dp *a2dpSrcService_ {nullptr};
    IProfileHidHost *hidHostService_ {nullptr};
    IProfilePbapPse *pbapService_ {nullptr};
    IProfileMapMse *mapService_ {nullptr};
    IProfileHearingAid *hearingAidService_ {nullptr};
    Bluetooth::IProfilePan *panService_ {nullptr};
    IProfileBas *basService_ {nullptr};
    RemoteDeviceProperties *remoteDeviceProperties_;
    std::set<std::string> pendingPairedDevices_;
    std::set<std::string> disconnectAclBeforeBondDevices_;
    std::shared_ptr<utility::Timer> btScanTimeout_ = nullptr;
    mutable std::mutex btScanTimeoutMutex_;
    SafeMap<RawAddress, std::set<int>> deviceProfileAddressMap_ {};
    BT_DISALLOW_COPY_AND_ASSIGN(ClassicAdapter);
    DECLARE_IMPL();
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // CLASSIC_ADAPTER_H