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

#ifndef OHOS_BLUETOOTH_STANDARD_HOST_SERVER_H
#define OHOS_BLUETOOTH_STANDARD_HOST_SERVER_H

#include <map>

#include "safe_map.h"
#include "bt_def.h"
#include "bluetooth_host_stub.h"
#include "bluetooth_types.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"
#include "bluetooth_remote_device_info.h"
#include "datetime_ex.h"

namespace OHOS {
namespace Bluetooth {
enum class ServiceRunningState { STATE_IDLE, STATE_RUNNING };

struct PairRecord {
    int64_t timestamp = {};
    std::string appIdentifier = {};
    std::string pkgName = {};
    bool isSatisfyTimeAndAppLimit = true;
};

class BluetoothHostServer : public SystemAbility, public BluetoothHostStub {
    DECLARE_SYSTEM_ABILITY(BluetoothHostServer);

public:
    explicit BluetoothHostServer();
    ~BluetoothHostServer() override;

    static sptr<BluetoothHostServer> GetInstance();

    void OnStart() override;
    void OnStop() override;

    void RegisterObserver(const sptr<IBluetoothHostObserver> &observer) override;
    void DeregisterObserver(const sptr<IBluetoothHostObserver> &observer) override;
    int32_t EnableBt() override;
    int32_t DisableBt(bool isAsync = false, const std::string &callingName = "") override;
    int32_t GetBtState(int32_t &state) override;
    int32_t BluetoothFactoryReset() override;
    int32_t GetDeviceType(int32_t transport, const std::string &address) override;

    int32_t GetLocalAddress(std::string &addr) override;
    int32_t GenerateLocalOobData(int32_t transport, const sptr<IBluetoothOobObserver> &observer) override;
    sptr<IRemoteObject> GetProfile(const std::string &name) override;
    sptr<IRemoteObject> GetBleRemote(const std::string &name) override;

    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }

    bool Start();
    void Stop();
    int32_t DisableBle() override;
    int32_t EnableBle(bool noAutoConnect, bool isAsync = false, const std::string &callingName = "") override;
    std::vector<uint32_t> GetProfileList() override;
    int32_t GetMaxNumConnectedAudioDevices() override;
    int32_t GetBtConnectionState(int32_t &state) override;
    int32_t GetBtProfileConnState(uint32_t profileId, int &state) override;
    int32_t GetLocalDeviceClass() override;
    bool SetLocalDeviceClass(const int32_t &deviceClass) override;
    int32_t GetLocalName(std::string &name) override;
    int32_t SetLocalName(const std::string &name) override;
    int32_t GetBtScanMode(int32_t &scanMode) override;
    int32_t SetBtScanMode(int32_t mode, int32_t duration) override;
    int32_t GetBondableMode(const int32_t transport) override;
    bool SetBondableMode(int32_t transport, int32_t mode) override;
    int32_t StartBtDiscovery() override;
    int32_t CancelBtDiscovery() override;
    int32_t IsBtDiscovering(bool &isDiscovering, const int32_t transport) override;
    long GetBtDiscoveryEndMillis() override;
    int32_t GetPairedDevices(std::vector<BluetoothRawAddress> &pairedAddr) override;
    int32_t RemovePair(const int32_t transport, const sptr<BluetoothRawAddress> &device) override;
    bool RemoveAllPairs() override;
    void RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer) override;
    void DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver> &observer) override;
    int32_t GetBleMaxAdvertisingDataLength() override;
    int32_t GetPhonebookPermission(const std::string &address) override;
    bool SetPhonebookPermission(const std::string &address, int32_t permission) override;
    int32_t GetMessagePermission(const std::string &address) override;
    bool SetMessagePermission(const std::string &address, int32_t permission) override;
    int32_t GetPowerMode(const std::string &address) override;
    int32_t GetDeviceName(int32_t transport, const std::string &address, std::string &name, bool alias = true) override;
    std::string GetDeviceAlias(const std::string &address) override;
    int32_t SetDeviceAlias(const std::string &address, const std::string &aliasName) override;
    int32_t GetRemoteDeviceBatteryInfo(const std::string &address, BluetoothBatteryInfo &batteryInfo) override;
    int32_t SetRemoteDeviceBatteryInfo(const std::string &address, const BluetoothBatteryInfo &batteryInfo) override;
    int32_t GetPairState(int32_t transport, const std::string &address, int32_t &pairState) override;
    int32_t StartPair(int32_t transport, const BluetoothRawAddress &bluetoothRawAddress,
        const BluetoothOobData &oobData) override;
    int32_t StartCrediblePair(int32_t transport, const std::string &address) override;
    bool CancelPairing(int32_t transport, const std::string &address) override;
    bool IsBondedFromLocal(int32_t transport, const std::string &address) override;
    bool IsAclConnected(int32_t transport, const std::string &address) override;
    bool IsAclEncrypted(int32_t transport, const std::string &address) override;
    int32_t GetDeviceClass(const std::string &address, int32_t &cod) override;
    int32_t SetDevicePin(const std::string &address, const std::string &pin) override;
    int32_t SetDevicePairingConfirmation(int32_t transport, const std::string &address, bool accept) override;
    bool SetDevicePasskey(int32_t transport, const std::string &address, int32_t passkey, bool accept) override;
    bool PairRequestReply(int32_t transport, const std::string &address, bool accept) override;
    bool ReadRemoteRssiValue(const std::string &address) override;
    void RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer) override;
    void DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver> &observer) override;
    void RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer) override;
    void DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver> &observer) override;
    void GetLocalSupportedUuids(std::vector<std::string> &uuids) override;
    int32_t GetDeviceUuids(const std::string &address, std::vector<std::string> &uuids) override;
    int32_t GetConnectedBLEDevices(int32_t bleProfile, std::vector<std::string> &connectedDevices) override;
    int32_t GetLocalProfileUuids(std::vector<std::string> &uuids) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    int32_t SetFastScan(bool isEnable) override;
    int32_t GetRandomAddress(const std::string &realAddr, std::string &randomAddr, uint64_t tokenId = 0) override;
    int32_t GetRealAddress(const std::string &randomAddr, std::string &realAddr) override;
    int32_t SyncRandomAddress(const std::string &realAddr, const std::string &randomAddr) override;
    int32_t ConnectAllowedProfiles(const std::string &address) override;
    int32_t DisconnectAllowedProfiles(const std::string &address) override;
    int32_t SetDeviceCustomType(const std::string &address, int32_t deviceCustomType) override;
    int32_t GetRemoteDeviceInfo(const std::string &address,
        std::shared_ptr<BluetoothRemoteDeviceInfo> &deviceInfo, int type) override;
    int32_t SatelliteControl(int type, int state) override;
    void UpdateVirtualDevice(int32_t action, const std::string &address) override;
    void RegisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver> &observer) override;
    void DeregisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver> &observer) override;
    int32_t IsSupportVirtualAutoConnect(const std::string &address, bool &support) override;
    int32_t SetVirtualAutoConnectType(const std::string &address, int connType, int businessType) override;
    int32_t SetFastScanLevel(int32_t level) override;
    int32_t EnableBluetoothToRestrictMode(const std::string &callingName) override;
    int32_t EnableBluetoothToHalfAppRegisteredMode(const std::string &callingName) override;
    int32_t ControlDeviceAction(const std::string &deviceId, uint32_t controlType,
        uint32_t controlTypeVal, uint32_t controlObject) override;
    int32_t GetLastConnectionTime(const std::string &address, int64_t& connectionTime) override;
    int32_t UpdateCloudBluetoothDevice(std::vector<BluetoothTrustPairDevice> &cloudDevices) override;
    int32_t GetCloudBondState(const std::string &address, int32_t& cloudBondState) override;
    int32_t GetDeviceTransport(const std::string &address, int32_t &transport) override;
    int32_t UpdateRefusePolicy(const int32_t protocolType,
        const int32_t pid, const int64_t prohibitedSecondsTime) override;
    int32_t GetVirtualAddressByHash(int hashAlgorithmType,
        const std::string &hashValue, std::string &virtualAddress) override;
    int32_t ProcessRandomDeviceIdCommand(
        int32_t command, std::vector<std::string> &deviceIdVec, bool &isValid) override;
    int32_t IsProfileExist(const std::string &profileName, bool &isProfileExist) override;
    int32_t OnSvcCmd(int32_t fd, const std::vector<std::u16string>& args) override;
    int32_t GetCarKeyDfxData(std::string &dfxData) override;
    int32_t SetCarKeyCardData(const std::string &address, int32_t action) override;
    int32_t NotifyDialogResult(uint32_t dialogType, bool dialogResult) override;
    void SetCallingPackageName(const std::string &address, const std::string &packageName) override;
    int32_t StartRemoteSdpSearch(const std::string &address, const std::string &uuid) override;
    int32_t GetRemoteServices(const std::string &address) override;
    int32_t SetConnectionPriority(const std::string &address, int32_t priority) override;
    int32_t VerifyMultiPermissions(bool systemHapNeeded, const std::set<std::string> &permissions) override;
    int32_t UpdateSecondaryPhonePairMode(int32_t mode) override;
    int32_t SetBtChannelScan(bool isEnable, uint32_t interval) override;
private:
    bool Init();
    bool IsBtEnabled();
    bool IsAllowedConnectProfile(const std::string &address);
    bool IsAllowedDisconnectProfile(bool isSystemHap, const std::string &address);
    std::string GetPairedAppIdentifier(uint32_t tokenId);
    static sptr<BluetoothHostServer> instance;
    static std::mutex instanceLock;
    int32_t transport_ = bluetooth::BT_TRANSPORT_BREDR;

    bool registeredToService_ = false;
    std::atomic<ServiceRunningState> state_ = ServiceRunningState::STATE_IDLE;
    const size_t SUBSTRING_SIZE = 32;
    const size_t HASH256_SIZE = 64;
    const int SHA256_TYPE = 0;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothHostServer);
    BLUETOOTH_DECLARE_IMPL();
public:
    std::string DumpRssFreezeInfo();
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_HOST_SERVER_H
