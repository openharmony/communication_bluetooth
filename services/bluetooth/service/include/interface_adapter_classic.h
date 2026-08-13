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
 * @brief Defines adapter classic, including observer and common functions.
 *
 * @since 6
 */

/**
 * @file interface_adapter_classic.h
 *
 * @brief Adapter classic interface.
 *
 * @since 6
 */

#ifndef INTERFACE_ADAPTER_CLASSIC_H
#define INTERFACE_ADAPTER_CLASSIC_H

#include "interface_adapter.h"
#include "bt_battery_info.h"
#include "bluetooth_remote_device_info.h"
#include "bluetooth_trust_pair_device.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Represents adapter classic observer.
 *
 * @since 6
 */
class IAdapterClassicObserver {
public:
    /**
     * @brief A destructor used to delete the <b>IAdapterClassicObserver</b> instance.
     *
     * @since 6
     */
    virtual ~IAdapterClassicObserver() = default;

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
        const RawAddress &device, int rssi, const std::string deviceName, int deviceClass) = 0;

    /**
     * @brief Pair requester observer.
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
    virtual void OnPairConfirmed(const BTTransport transport, const RawAddress &device, int reqType, int number) = 0;

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

    /**
     * @brief Generate local OOB data callback.
     *
     * @param ret Return of OnGenerateLocalOobData.
     * @param oobData OOB data of local device.
     * @since 23
     */
    virtual void OnGenerateLocalOobData(int32_t ret, const Bluetooth::BluetoothOobData &oobData) = 0;
};

/**
 * @brief Represents remote device observer.
 *
 * @since 6
 */
class IClassicRemoteDeviceObserver {
public:
    /**
     * @brief A destructor used to delete the <b>IClassicRemoteDeviceObserver</b> instance.
     *
     * @since 6
     */
    virtual ~IClassicRemoteDeviceObserver() = default;

    /**
     * @brief Acl state changed observer.
     *
     * @param device Remote device.
     * @param state Remote device acl state.
     * @param reason Remote device reason.
     * @since 6
     */
    virtual void OnAclStateChanged(const RawAddress &device, int state, unsigned int reason, int pairStatus) = 0;

    /**
     * @brief Pair status changed observer.
     *
     * @param device Remote device.
     * @param status Remote device pair status.
     * @param cause Pair fail cause.
     * @since 12
     */
    virtual void OnPairStatusChanged(const BTTransport transport, const RawAddress &device, int status, int cause,
        const std::string &causeMessage) = 0;

    /**
     * @brief Remote uuid changed observer.
     *
     * @param device Remote device.
     * @param uuids Remote device uuids.
     * @since 6
     */
    virtual void OnRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids) = 0;

    /**
     * @brief Remote name changed observer.
     *
     * @param device Remote device.
     * @param deviceName Remote device name.
     * @since 6
     */
    virtual void OnRemoteNameChanged(const RawAddress &device, const std::string &deviceName) = 0;

    /**
     * @brief Remote alias changed observer.
     *
     * @param device Remote device.
     * @param alias Remote device alias.
     * @since 6
     */
    virtual void OnRemoteAliasChanged(const RawAddress &device, const std::string &alias) = 0;

    /**
     * @brief Remote cod changed observer.
     *
     * @param device Remote device.
     * @param cod Remote device cod.
     * @since 6
     */
    virtual void OnRemoteCodChanged(const RawAddress &device, int cod) = 0;

    /**
     * @brief Remote battery info changed observer.
     *
     * @param device Remote device.
     * @param batteryInfo Remote device battery info.
     * @since 12
     */
    virtual void OnRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo) = 0;

    /**
     * @brief Remote device common info changed observer.
     *
     * @param device Remote device.
     * @param value Remote common info.
     * @since 12
     */
    virtual void OnRemoteDeviceCommonInfoReport(const RawAddress &device, const std::vector<uint8_t> &value) = 0;
};

/**
 * @brief Represents classic adapter interface.
 *
 * @since 6
 */
class IAdapterClassic : public IAdapter {
public:
    /**
     * @brief A destructor used to delete the <b>IAdapterClassic</b> instance.
     *
     * @since 6
     */
    ~IAdapterClassic() override = default;

    /**
     * @brief Get remote device class.
     *
     * @param device Remote device.
     * @return Returns remote device class.
     * @since 6
     */
    virtual int GetDeviceClass(const RawAddress &device) const = 0;

    /**
     * @brief Get remote device alias name.
     *
     * @param device Remote device
     * @return Returns remote device alias name.
     * @since 6
     */
    virtual std::string GetAliasName(const RawAddress &device) const = 0;

    /**
     * @brief Get remote device hw RemoteDeviceType.
     *
     * @param device Remote device
     * @return remote device hw RemoteDeviceType.
     * @since 21
     */
    virtual int GetHwRemoteDeviceType(const RawAddress &device) const = 0;

    /**
     * @brief Set remote device alias name.
     *
     * @param device Remote device which set alias name.
     * @param name Alias name.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetAliasName(const RawAddress &device, const std::string &name) = 0;

    /**
     * @brief Get remote device battery info.
     *
     * @param device Remote device
     * @param batteryInfo remote device battery info
     * @return Returns result.
     * @since 12
     */
    virtual int GetRemoteDeviceBatteryInfo(const RawAddress &device, BatteryInfo &batteryInfo) const = 0;

    /**
     * @brief Set remote device battery info.
     *
     * @param device Remote device
     * @param batteryInfo remote device battery info
     * @return Returns result.
     * @since 23
     */
    virtual int SetRemoteDeviceBatteryInfo(const RawAddress &device, const BatteryInfo &batteryInfo) const = 0;

    /**
     * @brief send remote device battery info for change .
     *
     * @param device Remote device
     * @param batteryInfo remote device battery info
     * @return Returns result.
     * @since 12
     */
    virtual void SendRemoteBatteryChanged(const RawAddress &device, const BatteryInfo &batteryInfo) = 0;

    /**
     * @brief send remote device echo info.
     *
     * @param device Remote device
     * @param value Info
     * @return Returns result.
     * @since 12
     */
    virtual void SendRemoteEchoInfo(const RawAddress &device, const std::vector<uint8_t> &value) = 0;

    /**
     * @brief Register remote device observer.
     *
     * @param observer Class IClassicRemoteDeviceObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool RegisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const = 0;

    /**
     * @brief Deregister remote device observer.
     *
     * @param observer Class IClassicRemoteDeviceObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool DeregisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const = 0;

    /**
     * @brief Set device scan mode.
     *
     * @param mode Scan mode.
     * @param duration Scan time.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetBtScanMode(int mode, int duration) = 0;

    /**
     * @brief Get device scan mode.
     *
     * @return Returns bluetooth scan mode.
     * @since 6
     */
    virtual int GetBtScanMode() const = 0;

    /**
     * @brief Get local device class.
     *
     * @return Returns local device class.
     * @since 6
     */
    virtual int GetLocalDeviceClass() const = 0;

    /**
     * @brief Set local device class.
     *
     * @param deviceClass Device class.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetLocalDeviceClass(int deviceClass) const = 0;

    /**
     * @brief Start device discovery.
     *
     * @return Returns operation result.
     * @since 6
     */
    virtual int32_t StartBtDiscovery() = 0;

    /**
     * @brief Cancel device discovery.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool CancelBtDiscovery() = 0;

    /**
     * @brief Check if device is discovering.
     *
     * @return Returns <b>true</b> if device is discovering;
     *         returns <b>false</b> if device is not discovering.
     * @since 6
     */
    virtual bool IsBtDiscovering() const = 0;

    /**
     * @brief Get device discovery end time.
     *
     * @return Returns device discovery end time.
     * @since 6
     */
    virtual long GetBtDiscoveryEndMillis() const = 0;

    /**
     * @brief Set device pair pin.
     *
     * @param device Remote device address.
     * @param pinCode Pin code.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetDevicePin(const RawAddress &device, const std::string &pinCode) const = 0;

    /**
     * @brief Get remote device uuids.
     *
     * @param device Remote device address.
     * @return Returns remote device uuids vector.
     * @since 6
     */
    virtual std::vector<std::string> GetDeviceUuids(const RawAddress &device) const = 0;

    /**
     * @brief Get local profile uuids.
     *
     * @return Returns local uuids.
     */
    virtual std::vector<std::string> GetLocalProfileUuids() const = 0;

    /**
     * @brief Register classic adapter observer.
     *
     * @param observer Class IAdapterClassicObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool RegisterClassicAdapterObserver(IAdapterClassicObserver &observer) const = 0;

    /**
     * @brief Deregister classic adapter observer.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool DeregisterClassicAdapterObserver(IAdapterClassicObserver &observer) const = 0;

    /**
     * @brief Set fast scan status.
     *
     * @param isEnable fast scan enable or disable.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetFastScan(bool isEnable) = 0;

    /**
     * @brief  Connects all allowed bluetooth profiles between the local and remote device.
     *
     * @param device Remote device.
     * @since 11
     */
    virtual void ConnectAllowedProfiles(
        const RawAddress &device, bool isConnectAllEnabled = true) = 0;

    /**
     * @brief  Disconnects all allowed bluetooth profiles between the local and remote device.
     *
     * @param device Remote device.
     * @since 11
     */
    virtual void DisconnectAllowedProfiles(const RawAddress &device, bool isDisconnectAllEnabled = true) = 0;

    /**
     * @brief  Disconnects all connected bluetooth profiles between the local and remote device.
     *         Only disconnect profiles, not set virtual connection switch.
     *
     * @param device Remote device.
     * @since 20
     */
    virtual void DisconnectAllowedProfilesNotSetVirtualConnSwitch(const RawAddress &device) = 0;

    virtual int32_t FactoryReset() = 0;
    /**
     * @brief Get the model ID of a remote device.
     *
     * @param device Remote device.
     * @return Model Id
     * @since 11
     */
    virtual std::string GetDeviceModelId(const RawAddress &device) = 0;

    /**
     * @brief Set the custom type of a remote device.
     *
     * @param device Remote device.
     * @param customType custom type.
     * @return custom type
     * @since 12
     */
    virtual int32_t SetDeviceCustomType(const RawAddress &device, int32_t customType) const = 0;

    /**
     * @brief Get remote device information.
     *
     * @param device Remote device.
     * @param deviceInfo[out] device information.
     * @return Returns {@link BT_NO_ERROR} if GetDeviceCustomType success;
     * @return Returns an error code defined in {@link BtErrCode} otherwise.
     * @since 12
     */
    virtual int32_t GetRemoteDeviceInfo(const RawAddress &device,
        Bluetooth::BluetoothRemoteDeviceInfo &deviceInfo) = 0;

    /**
     * @brief  Disconnects all Paired Device connected bluetooth profiles between the local and remote device.
     * @since 12
     */
    virtual void DisconnectPairedDeviceProfiles() = 0;

    /**
     * @brief  Passiv ePair To Delay to Connect.
     * @since 12
     */
    virtual void PassivePairToDelayConnect(const RawAddress &device) = 0;

    /**
     * @brief need delay connect or not
     * @since 12
     */
    virtual bool IsNeedDelayConnect(const RawAddress &device) = 0;

    /**
     * @brief Set fast scan status.
     *
     * @param level fast scan enable level.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 12
     */
    virtual bool SetFastScanLevel(int level) = 0;

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
    virtual int32_t ControlDeviceAction(const std::string &deviceId, uint32_t controlType,
        uint32_t controlTypeVal, uint32_t controlObject) = 0;

    /**
     * @brief Get latest connection time of device.
     *
     * @param device Remote device.
	 * @param connectionTime[out] latest connection time.
     * @return Returns {@link BT_NO_ERROR} if GetLastConnectionTime success;
     * @param connectionTime connection time.
     * @since 15
     */
    virtual int32_t GetLastConnectionTime(const RawAddress &device, int64_t &connectionTime) = 0;

    virtual int32_t UpdateCloudBluetoothDevice(std::vector<Bluetooth::BluetoothTrustPairDevice> &cloudList) = 0;

    virtual int32_t GetCloudBondState(const RawAddress &device, int32_t &cloudBondState) = 0;

    /**
     * @brief Get device transport.
     *
     * @param device Remote device.
     * @param transport The transport of device.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 20
     */
    virtual int32_t GetDeviceTransport(const RawAddress &device, int32_t &transport) = 0;

    /**
     * @brief Get car key dfx data.
     *
     * @return car key dfx data.
     * @since 16
     */
    virtual std::string GetCarKeyDfxData() const = 0;

    /**
     * @brief Set car key card data.
     *
     * @param address address.
	 * @param action action: 0 delete; 1 add.
     * @since 16
     */
    virtual void SetCarKeyCardData(const std::string &address, int32_t action) = 0;

    /**
     * @brief Init car key info.
     *
     * @since 16
     */
    virtual void InitCarKeyInfo() = 0;

    /**
     * @brief search specific uuid of the remote with sdp
     *
     * @since 22
     */
    virtual bool StartRemoteSdpSearch(const std::string &address, const std::string &uuid) = 0;

    /**
     * @brief search all the services remote supported with sdp
     *
     * @since 22
     */
    virtual bool GetRemoteServices(const std::string &address) = 0;

    /**
     * @brief Set connection priority for incoming or existed connection.
     *
     * @param address The remote device address.
     * @param priority Connection priority.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 21
     */
    virtual int32_t SetConnectionPriority(const std::string &address, int32_t priority) = 0;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // INTERFACE_ADAPTER_CLASSIC_H