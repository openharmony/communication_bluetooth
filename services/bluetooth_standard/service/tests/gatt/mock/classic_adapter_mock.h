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

#ifndef CLASSIC_ADAPTER_H
#define CLASSIC_ADAPTER_H

#include <vector>
#include <map>

#include "interface_adapter_classic.h"
#include "context.h"

#include "raw_address.h"
#include "base_def.h"
#include "bt_uuid.h"
#include "btm.h"
#include "gap_if.h"
#include "sdp.h"
#include "log.h"

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines adapter classic common functions.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file classic_adapter.h
 *
 * @brief Adapter classic.
 *
 * @since 1.0
 * @version 1.0
 */

namespace bluetooth {
/**
 * @brief Represents classic adapter.
 *
 * @since 1.0
 * @version 1.0
 */
class ClassicAdapter : public IAdapterClassic, public utility::Context {
public:
    /**
     * @brief A constructor used to create a <b>ClassicAdapter</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    ClassicAdapter();

    /**
     * @brief A destructor used to delete the <b>ClassicAdapter</b> instance.
     *
     * @since 1.0
     * @version 1.0
     */
    ~ClassicAdapter();

    /**
     * @brief Get local device address.
     *
     * @return Returns local device address.
     * @since 1.0
     * @version 1.0
     */
    std::string GetLocalAddress() const override;

    /**
     * @brief Get local device name.
     *
     * @return Returns local device name.
     * @since 1.0
     * @version 1.0
     */
    std::string GetLocalName() const override;

    /**
     * @brief Set local device name.
     *
     * @param name Device name.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetLocalName(const std::string &name) const override;

    /**
     * @brief Set bondable mode.
     *
     * @param mode Bondable mode.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetBondableMode(int mode) const override;

    /**
     * @brief Get bondable mode.
     *
     * @return Returns bondable mode.
     * @since 1.0
     * @version 1.0
     */
    int GetBondableMode() const override;

    /**
     * @brief Get remote device name.
     *
     * @param device Remote device.
     * @return Returns device name.
     * @since 1.0
     * @version 1.0
     */
    std::string GetDeviceName(const RawAddress &device) const override;

    /**
     * @brief Get remote device uuids.
     *
     * @param device Remote device.
     * @return Returns device uuids.
     * @since 1.0
     * @version 1.0
     */
    std::vector<Uuid> GetDeviceUuids(const RawAddress &device) const override;

    /**
     * @brief Get paired devices.
     *
     * @return Returns device list.
     * @since 1.0
     * @version 1.0
     */
    std::vector<RawAddress> GetPairedDevices() const override;

    /**
     * @brief Local device start pair.
     *
     * @param device Remote device.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool StartPair(const RawAddress &device) override;

    /**
     * @brief Check if remote device was bonded from local.
     *
     * @param device Remote device.
     * @return Returns <b>true</b> if device was bonded from local;
     *         returns <b>false</b> if device was not bonded from local.
     * @since 1.0
     * @version 1.0
     */
    bool IsBondedFromLocal(const RawAddress &device) const override;

    /**
     * @brief Cancel pair operation.
     *
     * @param device Remote device.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool CancelPairing(const RawAddress &device) override;

    /**
     * @brief Remove pair.
     *
     * @param device Remote device.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool RemovePair(const RawAddress &device) override;

    /**
     * @brief Get device pair state.
     *
     * @param device Remote device.
     * @return Returns device pair state.
     * @since 1.0
     * @version 1.0
     */
    int GetPairState(const RawAddress &device) const override;

    /**
     * @brief Set device pairing confirmation.
     *
     * @param device Remote device.
     * @param accept Set gap accept flag.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
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
     * @since 1.0
     * @version 1.0
     */
    bool SetDevicePasskey(const RawAddress &device, int passkey, bool accept) const override;

    /**
     * @brief Check remote device pair request reply.
     *
     * @param device Remote device.
     * @param accept Set gap accept flag.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool PairRequestReply(const RawAddress &device, bool accept) const override;

    /**
     * @brief Check if acl connected with remote device.
     *
     * @param device Remote device.
     * @return Returns <b>true</b> if device acl connected;
     *         returns <b>false</b> if device does not acl connect.
     * @since 1.0
     * @version 1.0
     */
    bool IsAclConnected(const RawAddress &device) const override;

    /**
     * @brief Check if remote device acl Encrypted.
     *
     * @return Returns <b>true</b> if device acl Encrypted;
     *         returns <b>false</b> if device does not acl Encrypt.
     * @since 1.0
     * @version 1.0
     */
    bool IsAclEncrypted(const RawAddress &device) const override;

    /**
     * @brief Get the instance of the ClassicAdapter.
     *
     * @return Returns the instance of the ClassicAdapter.
     * @since 1.0
     * @version 1.0
     */
    utility::Context *GetContext() override;

    /**
     * @brief Enable classic service.
     *
     * @since 1.0
     * @version 1.0
     */
    void Enable() override;

    /**
     * @brief Disable classic service.
     *
     * @since 1.0
     * @version 1.0
     */
    void Disable() override;

    /**
     * @brief PostEnable classic service.
     *
     * @since 1.0
     * @version 1.0
     */
    void PostEnable() override;

    /**
     * @brief Get remote device type.
     *
     * @param device Remote device.
     * @return Returns remote device type.
     * @since 1.0
     * @version 1.0
     */
    int GetDeviceType(const RawAddress &device) const override;

    /**
     * @brief Get remote device class.
     *
     * @param device Remote device.
     * @return Returns remote device class.
     * @since 1.0
     * @version 1.0
     */
    int GetDeviceClass(const RawAddress &device) const override;

    /**
     * @brief Get remote device alias name.
     *
     * @param device Remote device
     * @return Returns remote device alias name.
     * @since 1.0
     * @version 1.0
     */
    std::string GetAliasName(const RawAddress &device) const override;

    /**
     * @brief Get remote device flags.
     *
     * @param device Remote device
     * @return Returns remote flags.
     * @since 1.0
     * @version 1.0
     */
    uint8_t GetFlags(const RawAddress &device) const;

    /**
     * @brief Set remote device alias name.
     *
     * @param device Remote device which setted alias name.
     * @param name Alias name.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetAliasName(const RawAddress &device, const std::string &name) const override;

    /**
     * @brief Register remote device observer.
     *
     * @param observer Class IClassicRemoteDeviceObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool RegisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const override;

    /**
     * @brief Deregister remote device observer.
     *
     * @param observer Class IClassicRemoteDeviceObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool DeregisterRemoteDeviceObserver(IClassicRemoteDeviceObserver &observer) const override;

    /**
     * @brief Register classic adapter observer.
     *
     * @param observer Class IAdapterClassicObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool RegisterClassicAdapterObserver(IAdapterClassicObserver &observer) const override;

    /**
     * @brief Deregister classic adapter observer.
     *
     * @param observer Class IAdapterClassicObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool DeregisterClassicAdapterObserver(IAdapterClassicObserver &observer) const override;

    /**
     * @brief Remove all pairs.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
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
     * @since 1.0
     * @version 1.0
     */
    bool SetDevicePin(const RawAddress &device, const std::string &pinCode) const override;

    /**
     * @brief Set device scan mode.
     *
     * @param mode Scan mode.
     * @param duration Scan time.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetBtScanMode(int mode, int duration) override;

    /**
     * @brief Get device scan mode.
     *
     * @return Returns bluetooth scan mode.
     * @since 1.0
     * @version 1.0
     */
    int GetBtScanMode() const override;

    /**
     * @brief Get local device class.
     *
     * @return Returns local device class.
     * @since 1.0
     * @version 1.0
     */
    int GetLocalDeviceClass() const override;

    /**
     * @brief Set local device class.
     *
     * @param deviceClass Device class.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetLocalDeviceClass(int deviceClass) const override;

    /**
     * @brief Get device address.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool StartBtDiscovery() override;

    /**
     * @brief Cancel device discovery.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool CancelBtDiscovery() override;

    /**
     * @brief Check if device is discovering.
     *
     * @return Returns <b>true</b> if device is discovering;
     *         returns <b>false</b> if device is not discovering.
     * @since 1.0
     * @version 1.0
     */
    bool IsBtDiscovering() const override;

    /**
     * @brief Get device discovery end time.
     *
     * @return Returns device discovery end time.
     * @since 1.0
     * @version 1.0
     */
    long GetBtDiscoveryEndMillis() const override;

    /**
     * @brief Inquiry result callback.
     *
     * @param addr Device address.
     * @param classOfDevice Device class.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void InquiryResultCallback(const BtAddr *addr, uint32_t classOfDevice, void *context);

    /**
     * @brief Inquiry result rssi callback.
     *
     * @param addr Device address.
     * @param classOfDevice Device class.
     * @param rssi Device rssi.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void InquiryResultRssiCallback(const BtAddr *addr, uint32_t classOfDevice, int8_t rssi, void *context);

    /**
     * @brief Inquiry complete callback.
     *
     * @param status Inquire status.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void InquiryCompleteCallback(uint8_t status, void *context);

    /**
     * @brief User confirm request callback.
     *
     * @param addr Device address.
     * @param number Pass key.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void UserConfirmReqCallback(const BtAddr *addr, uint32_t number, void *context);

    /**
     * @brief User pass key request callback.
     *
     * @param addr Device address.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void UserPasskeyReqCallback(const BtAddr *addr, void *context);

    /**
     * @brief User pass key notification callback.
     *
     * @param addr Device address.
     * @param number Pass key.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void UserPasskeyNotificationCallback(const BtAddr *addr, uint32_t number, void *context);

    /**
     * @brief Remote oob key request callback.
     *
     * @param addr Device address.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void RemoteOobReqCallback(const BtAddr *addr, void *context);

    /**
     * @brief Pin code request callback.
     *
     * @param addr Device address.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void PinCodeReqCallback(const BtAddr *addr, void *context);

    /**
     * @brief Link key request callback.
     *
     * @param addr Device address.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void LinkKeyReqCallback(const BtAddr *addr, void *context);

    /**
     * @brief Io capability request callback.
     *
     * @param addr Device address.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void IoCapabilityReqCallback(const BtAddr *addr, void *context);

    /**
     * @brief Io capability response callback.
     *
     * @param addr Device address.
     * @param ioCapability Device iocapability.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void IoCapabilityRspCallback(const BtAddr *addr, uint8_t ioCapability, void *context);

    /**
     * @brief Link key notification.
     *
     * @param addr Device address.
     * @param linkkey Link key.
     * @param keyType Link key type.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void LinkKeyNotification(
        const BtAddr *addr, const uint8_t linkKey[GAP_LINKKEY_SIZE], uint8_t keyType, void *context);

    /**
     * @brief Simple pair complete callback.
     *
     * @param addr Device address.
     * @param status Pair status.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void SimplePairCompleteCallback(const BtAddr *addr, uint8_t status, void *context);

    /**
     * @brief Authentication complete callback.
     *
     * @param addr Device address.
     * @param status Authentication status.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void AuthenticationCompleteCallback(const BtAddr *addr, uint8_t status, void *context);

    /**
     * @brief Encryption change complete callback.
     *
     * @param addr Device address.
     * @param status Encryption status.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void EncryptionChangeCallback(const BtAddr *addr, uint8_t status, void *context);

    /**
     * @brief Authorize ind callback.
     *
     * @param addr Device address.
     * @param service Gap service.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void AuthorizeIndCallback(const BtAddr *addr, GAP_Service service, void *context);

    /// BTM ACL Change status callback.
    /**
     * @brief Acl connection complete callback.
     *
     * @param status Connection status.
     * @param connectionHandle Connection handle.
     * @param addr Device address.
     * @param encyptionEnabled Encyption enable status.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void ConnectionComplete(const BtmAclConnectCompleteParam *param, void *context);

    /**
     * @brief Acl disconnection complete callback.
     *
     * @param status Connection status.
     * @param connectionHandle Connection handle.
     * @param reason Disconnection reason.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void DisconnectionComplete(uint8_t status, uint16_t connectionHandle, uint8_t reason, void *context);

    /// SDP search remote device supported uuid callback.
    /**
     * @brief Service search attribute callback.
     *
     * @param addr Device address.
     * @param serviceArray Service array.
     * @param serviceNum Service number.
     * @param context Classic adapter object.
     * @since 1.0
     * @version 1.0
     */
    static void ServiceSearchAttributeCb(
        const BtAddr *addr, const SdpService *serviceArray, uint16_t serviceNum, void *context);

    /**
     * @brief ClassicAdater start up. Load config file and enable stack.
     *
     * @since 1.0
     * @version 1.0
     */
    void StartUp();

    /**
     * @brief ClassicAdater shut down. Save config file and disable stack.
     *
     * @since 1.0
     * @version 1.0
     */
    void ShutDown();

    /**
     * @brief Process post enable.
     *
     * @since 1.0
     * @version 1.0
     */
    void ProcessPostEnable();

    /**
     * @brief Set scan mode.
     *
     * @param mode Scan mode.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetScanMode(int mode);

    /**
     * @brief Set scan mode result callback.
     *
     * @param status Scan status.
     * @param context ClassicAdapterProperties object.
     * @since 1.0
     * @version 1.0
     */
    static void SetScanModeResultCallback(uint8_t status, void *context);

    /**
     * @brief Receive SetScanMode callback.
     *
     * @param status SetScanMode result.
     * @since 1.0
     * @version 1.0
     */
    void ReceiveSetScanModeCallback(uint8_t status);

    /**
     * @brief ScanMode timeout.
     *
     * @since 1.0
     * @version 1.0
     */
    void ScanModeTimeout();

    /**
     * @brief Hw(Controller) process timeout.
     *
     * @since 1.0
     * @version 1.0
     */
    void HwProcessTimeout();
    void HwTimeout();
    int GetDeviceBatteryLevel(const RawAddress &device) const override;
    void SetDeviceBatteryLevel(const RawAddress &device, int batteryLevel) const override;

private:
    /**
     * @brief Check the return value.
     *        If the ret is false, output the error log.
     *
     * @since 1.0
     * @version 1.0
     */
    inline void CheckReturnValue(const std::string &funcName, bool ret) const
    {
        if (!ret) {
            LOG_ERROR("[ClassicAdapter]:%s(), ret[%d]", funcName.c_str(), ret);
        }
    }

    /**
     * @brief Register callback.
     *
     * @since 1.0
     * @version 1.0
     */
    bool RegisterCallback();

    /**
     * @brief Deregister callback.
     *
     * @since 1.0
     * @version 1.0
     */
    bool DeregisterCallback();

    /**
     * @brief Get remote device name.
     *
     * @param addr Device address.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool GetRemoteName(const BtAddr &addr) const;

    /**
     * @brief Set link key.
     *
     * @param addr Device address.
     * @since 1.0
     * @version 1.0
     */
    void SetLinkKey(const BtAddr &addr);

    /**
     * @brief Set Io capability.
     *
     * @param addr Device address.
     * @since 1.0
     * @version 1.0
     */
    void SetIoCapability(const BtAddr &addr);

    /**
     * @brief Load paired device info from config file.
     *
     * @since 1.0
     * @version 1.0
     */
    void LoadPairedDeviceInfo();

    /**
     * @brief Save paired devices info.
     *
     * @since 1.0
     * @version 1.0
     */
    void SavePairedDevices();

    /**
     * @brief Get service uuid from device uuid.
     *
     * @param uuid Device uuid.
     * @return Returns service uuid.
     * @since 1.0
     * @version 1.0
     */
    Uuid GetUuidFromBtUuid(const BtUuid &uuid);

    /**
     * @brief Get local support uuids then update the uuids to EIR data.
     *
     * @since 1.0
     * @version 1.0
     */
    void UpdateSupportedUuids();

    /**
     * @brief Save remote device Io capability.
     *
     * @param addr Remote device address.
     * @param ioCapability Device Io capability
     * @since 1.0
     * @version 1.0
     */
    void SaveRemoteIoCapability(const BtAddr &addr, uint8_t ioCapability);

    /**
     * @brief Send scan mode changed.
     *
     * @param mode ScamMode.
     * @since 1.0
     * @version 1.0
     */
    void SendScanModeChanged(int mode);

    /**
     * @brief Send discovery state changed.
     *
     * @since 1.0
     * @version 1.0
     */
    void SendDiscoveryStateChanged(int discoveryState);

    /**
     * @brief Send discovery result.
     *
     * @param device Remote device.
     * @since 1.0
     * @version 1.0
     */
    void SendDiscoveryResult(const RawAddress &device);

    /**
     * @brief Send remote device cod changed.
     *
     * @param device Remote device.
     * @param cod Remote device cod.
     * @since 1.0
     * @version 1.0
     */
    void SendRemoteCodChanged(const RawAddress &device, int cod);

    /**
     * @brief Send remote device battery level changed.
     *
     * @param device Remote device.
     * @param batteryLevel Remote device battery level.
     * @since 1.0
     * @version 1.0
     */
    void SendRemoteBatteryLevelChanged(const RawAddress &device, int batteryLevel);

    /**
     * @brief Send remote device name changed.
     *
     * @param device Remote device.
     * @param deviceName Remote device name.
     * @since 1.0
     * @version 1.0
     */
    void SendRemoteNameChanged(const RawAddress &device, const std::string &deviceName);

    /**
     * @brief Send pair status changed.
     *
     * @param device Remote device.
     * @param status Pair status.
     * @since 1.0
     * @version 1.0
     */
    void SendPairStatusChanged(const BTTransport transport, const RawAddress &device, int status);

    /**
     * @brief Send pair request.
     *
     * @param device Remote device.
     * @since 1.0
     * @version 1.0
     */
    void SendPairRequested(const BTTransport transport, const RawAddress &device);

    /**
     * @brief Send remote device uuid changed.
     *
     * @param device Remote device.
     * @param uuids Device uuid.
     * @since 1.0
     * @version 1.0
     */
    void SendRemoteUuidChanged(const RawAddress &device, const std::vector<Uuid> &uuids);

    /**
     * @brief Send remote alias name changed.
     *
     * @param device Remote device.
     * @param aliasName alias name.
     * @since 1.0
     * @version 1.0
     */
    void SendRemoteAliasChanged(const RawAddress &device, const std::string &aliasName);

    /**
     * @brief Handle inquire result.
     *
     * @param addr Device address.
     * @param classOfDevice Device class.
     * @param rssi Device rssi.
     * @param eir Eir data.
     * @since 1.0
     * @version 1.0
     */
    void HandleInquiryResult(const BtAddr &addr, uint32_t classOfDevice, int8_t rssi = 0, const uint8_t *eir = nullptr);

    /**
     * @brief Handle inquire complete.
     *
     * @param status Inquire status.
     * @since 1.0
     * @version 1.0
     */
    void HandleInquiryComplete(uint8_t status);

    /**
     * @brief Send pair confirmed.
     *
     * @param addr Remote device address.
     * @param reqType  Request type.
     * @param number Pass key.
     * @since 1.0
     * @version 1.0
     */
    void SendPairConfirmed(const BtAddr &addr, int reqType, int number);

    /**
     * @brief Set authorize response.
     *
     * @param addr Remote device address.
     * @param service Gap service.
     * @since 1.0
     * @version 1.0
     */
    void SetAuthorizeRes(const BtAddr &addr, GAP_Service service);

    /**
     * @brief Receive simple pair complete.
     *
     * @param addr Remote device address.
     * @param status Pair status.
     * @since 1.0
     * @version 1.0
     */
    void ReceiveSimplePairComplete(const BtAddr &addr, uint8_t status);

    /**
     * @brief Receive authentication complete.
     *
     * @param addr Remote device address.
     * @param status Authentication status.
     * @since 1.0
     * @version 1.0
     */
    void ReceiveAuthenticationComplete(const BtAddr &addr, uint8_t status);

    /**
     * @brief Receive encryption change.
     *
     * @param addr Remote device address.
     * @param status Encryption status.
     * @since 1.0
     * @version 1.0
     */
    void ReceiveEncryptionChange(const BtAddr &addr, uint8_t status);

    /**
     * @brief Receive acl connection complete.
     *
     * @param status Connection status.
     * @param connectionHandle Connection handle.
     * @param remoteAddr Device address.
     * @param encyptionEnabled Encyption enable status.
     * @since 1.0
     * @since 1.0
     * @version 1.0
     */
    void ReceiveConnectionComplete(uint8_t status, uint16_t connectionHandle, const RawAddress &remoteAddr,
        uint32_t classOfDevice, bool encyptionEnabled);

    /**
     * @brief Receive acl disconnection complete.
     *
     * @param status Connection status.
     * @param connectionHandle Connection handle.
     * @param reason Disconnection reason.
     * @since 1.0
     * @version 1.0
     */
    void ReceiveDisconnectionComplete(uint8_t status, uint16_t connectionHandle, uint8_t reason);

    /**
     * @brief User confirm auto reply.
     *
     * @param device Remote device.
     * @param reqType Request type.
     * @param accept Request accept or not.
     * @since 1.0
     * @version 1.0
     */
    void UserConfirmAutoReply(const RawAddress &device, int reqType, bool accept);

    /**
     * @brief Set pin code.
     *
     * @param device Remote device.
     * @param pin Pin code.
     * @param length Pin code length.
     * @param accept Request accept or not.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetPinCode(const RawAddress &device, uint8_t *pin, int length, bool accept);

    /**
     * @brief Check if auto reply.
     *
     * @param remoteIo Remote device Io capability.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool CheckAutoReply(int remoteIo);

    /**
     * @brief Check ssp confirm type.
     *
     * @param remoteIo Remote device Io capability.
     * @param type Confirm type.
     * @return Returns confirm type.
     * @since 1.0
     * @version 1.0
     */
    int CheckSspConfirmType(int remoteIo, int type);

    /**
     * @brief Parser service uuid from eir data.
     *
     * @param type type of Eir data structure.
     * @param value Eir data of Eir data structure.
     * @param length Eir data structure length.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    std::vector<Uuid> ParserUuidFromEir(int type, const std::vector<uint8_t> &value);

    /**
     * @brief Parser 16 bit service uuid from eir data.
     *
     * @param value Eir data of Eir data structure.
     * @param length Eir data structure length.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    std::vector<Uuid> Parser16BitUuidFromEir(const std::vector<uint8_t> &value);

    /**
     * @brief Parser 32 bit service uuid from eir data.
     *
     * @param value Eir data of Eir data structure.
     * @param length Eir data structure length.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    std::vector<Uuid> Parser32BitUuidFromEir(const std::vector<uint8_t> &value);

    /**
     * @brief Parser 128 bit service uuid from eir data.
     *
     * @param value Eir data of Eir data structure.
     * @param length Eir data structure length.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    std::vector<Uuid> Parser128BitUuidFromEir(const std::vector<uint8_t> &value);

    /**
     * @brief Check uuids1 is equal with uuids2.
     *
     * @param uuids1 uuids.
     * @param uuids2 uuids.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool IsUuidsEqual(const std::vector<Uuid> &uuids1, const std::vector<Uuid> &uuids2);

    void InitMode();
    void FreeMemory();
    void DisableBTM();
    bool DiscoverRemoteName();
    bool CancelGetRemoteName();
    void SearchRemoteUuids(const RawAddress &device, uint16_t uuid);
    void ResetScanMode();
    BtAddr ConvertToBtAddr(const RawAddress &rawAddr);
    int discoveryState_{DISCOVERY_STOPED};
    int scanMode_{SCAN_MODE_NONE};
    long discoveryEndMs_{};
    bool pinMode_{};
    bool btmEnableSuccess_{};
    bool isDisable_{};
    bool receiveInquiryComplete_{};
    bool cancelDiscovery_{};
    uint16_t searchUuid_{};
    std::vector<Uuid> uuids_{};
    std::string remoteNameAddr_{INVALID_MAC_ADDRESS};
    BtmAclCallbacks btmAclCbs_{};

    DECLARE_IMPL();
    DISALLOW_COPY_AND_ASSIGN(ClassicAdapter);
};
}  // namespace bluetooth
#endif  // CLASSIC_ADAPTER_H
