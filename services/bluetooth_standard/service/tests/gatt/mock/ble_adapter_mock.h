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

#ifndef BLE_ADAPTER_H
#define BLE_ADAPTER_H

#include "interface_adapter_ble.h"
#include "context.h"

#include "ble_properties.h"
#include "raw_address.h"
#include "bt_uuid.h"
#include "ble_advertiser_impl.h"
#include "ble_central_manager_impl.h"
#include "btm.h"
#include "ble_security.h"
#include "base_observer_list.h"
#include <memory>
#include <map>

/*
 * @brief The Bluetooth subsystem.
 */
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
    ~BleAdapter();

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

    /**
     *  @brief Get local host bluetooth address
     *
     *  @return @c Local host bluetooth address
     */
    std::string GetLocalAddress() const override;

    /**
     *  @brief Get local host bluetooth name
     *
     *  @return @c Local host bluetooth name
     */
    std::string GetLocalName() const override;

    /**
     *  @brief Set local host bluetooth name
     *
     *  @param [in] name Device name.
     *  @return @c true success
     *          @c false failure
     */
    bool SetLocalName(const std::string &name) const override;

    /**
     *  @brief Set local host ble roles
     *
     *  @param [in] ble roles.
     *  @return @c true success
     *          @c false failure
     */
    bool SetBleRoles(uint8_t roles);

    /**
     *  @brief Get local host ble roles
     *
     *  @return @c ble roles
     */
    int GetBleRoles() const;

    /// add adapter manager common api
    std::string GetDeviceName(const RawAddress &device) const override;
    std::vector<Uuid> GetDeviceUuids(const RawAddress &device) const override;
    std::vector<RawAddress> GetPairedDevices() const override;
    std::vector<RawAddress> GetConnectedDevices() const;
    bool StartPair(const RawAddress &device) override;
    bool CancelPairing(const RawAddress &device) override;
    bool RemovePair(const RawAddress &device) override;
    bool RemoveAllPairs() override;
    bool IsBondedFromLocal(const RawAddress &device) const override;
    bool SetDevicePasskey(const RawAddress &device, int passkey, bool accept) const override;
    bool PairRequestReply(const RawAddress &device, bool accept) const override;
    bool IsAclConnected(const RawAddress &device) const override;
    bool IsAclEncrypted(const RawAddress &device) const override;
    int GetPairState(const RawAddress &device) const override;
    int GetBondableMode() const override;
    bool SetBondableMode(int mode) const override;
    bool SetDevicePairingConfirmation(const RawAddress &device, bool accept) const override;
    int GetDeviceType(const RawAddress &device) const override;
    int GetBleMaxAdvertisingDataLength() const override;
    int GetIoCapability() const override;
    bool SetIoCapability(int ioCapability) const override;
    bool IsBleEnabled() const;
    bool IsBtDiscovering() const override;

    /// FW api passthrough from service
    void StartAdvertising(const BleAdvertiserSettingsImpl &settings, const BleAdvertiserDataImpl &advData,
        const BleAdvertiserDataImpl &scanResponse, uint8_t advHandle)  const override;
    void StopAdvertising(uint8_t advHandle)  const override;
    void Close(uint8_t advHandle) const override;
    void StartScan() const override;
    void StartScan(const BleScanSettingsImpl &setting)  const override;
    void StopScan() const override;
    int GetAdvertisingStatus() const override;
    bool IsLlPrivacySupported() const override;
    void AddCharacteristicValue(uint8_t adtype, const std::string &data)  const override;

    void RegisterBleAdvertiserCallback(IBleAdvertiserCallback &callback) override;
    void DeregisterBleAdvertiserCallback() const override;
    void RegisterBleCentralManagerCallback(IBleCentralManagerCallback &callback) override;
    void DeregisterBleCentralManagerCallback() const override;
    void RegisterBlePeripheralCallback(IBlePeripheralCallback &callback) const override;
    void DeregisterBlePeripheralCallback(IBlePeripheralCallback &callback) const override;
    void RegisterBleSecurityCallback(BaseObserverList<IAdapterBleObserver> &callback);
    void DeregisterBleSecurityCallback();

    bool RegisterBleAdapterObserver(IAdapterBleObserver &observer) const override;
    bool DeregisterBleAdapterObserver(IAdapterBleObserver &observer) const override;
    int GetPeerDeviceAddrType(const RawAddress &device) const override;

    /// pair
    void LePairComplete(const RawAddress &device, const int status);
    void LePairingStatus(const RawAddress &device);
    void EncryptionComplete(const RawAddress &device);
    void BtmDisable();
    bool IsRemovePairedDevice(const RawAddress &device);

    void OnStartAdvertisingEvt();
    void OnStopAdvertisingEvt();

    /**
     * @brief Read Remote Rssi Value.
     *
     * @return @c true sucessfull otherwise false.
     */
    bool ReadRemoteRssiValue(const RawAddress &device) const override;
    uint8_t GetAdvertiserHandle() const override;

private:
    int RegisterCallbackToBtm();
    int DeregisterCallbackToBtm();
    int DeregisterAllCallback();
    bool EnableTask();
    bool DisableTask();
    bool PostEnableTask();
    bool StopAdvAndScan();
    bool LoadConfig();
    void LeConnectionCompleteTask(uint8_t status, uint16_t connectionHandle, const BtAddr &addr, uint8_t role);
    void LeDisconnectionCompleteTask(uint8_t status, uint16_t connectionHandle, uint8_t reason);
    void OnReadRemoteRssiEventTask(uint8_t status, const BtAddr &addr, int8_t rssi);
    // pair
    void ReadPeerDeviceInfoFromConf(const std::vector<std::string> &pairedAddrList);
    bool SavePeerDeviceInfoToConf(const std::map<std::string, BlePeripheralDevice> &peerConnDeviceList);
    void SavePeerDevices2BTM(const std::map<std::string, BlePeripheralDevice> &peerConnDeviceList);
    void ClearPeerDeviceInfo();
    int SetRpaAddrAndTypeToBtm();
    int InitBtmAndGap();

    /// BTM ACL Change status callback.
    static void LeConnectionComplete(
        uint8_t status, uint16_t connectionHandle, const BtAddr *addr, uint8_t role, void *context);
    static void LeDisconnectionComplete(uint8_t status, uint16_t connectionHandle, uint8_t reason, void *context);
    static void OnReadRemoteRssiEvent(uint8_t status, const BtAddr *addr, int8_t rssi, void *context);

    bool btmEnableFlag_ = false;
    bool isDisable_ = false;

    DISALLOW_COPY_AND_ASSIGN(BleAdapter);
    DECLARE_IMPL();
};
};  // namespace bluetooth

#endif  /// BLE_ADAPTER_H
