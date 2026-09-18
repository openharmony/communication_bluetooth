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

#ifndef ADAPTER_MANAGER_H
#define ADAPTER_MANAGER_H

#include <memory>
#include <string>
#include <vector>
#include "adapter_properties.h"
#include "bluetooth.h"
#include "bluetooth_errorcode.h"
#include <set>

#include "bt_def.h"
#include "interface_adapter.h"
#include "interface_adapter_manager.h"
#include "util/context.h"
#include "classic_adapter.h"
#include "ble/ble_adapter.h"
#include "btcommon/timer_manager.h"
#include <power_mgr_client.h>
#include "service_util.h"
#include "param_update_event_manager.h"
#include "switch_dependency.h"
#include "dialog/dialog_result.h"
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Represents adapter manager.
 *
 * @since 6
 */
const static int CLASSIC_ENABLE_STATE_BIT = 0x04;
class AdapterManager : public IAdapterManager {
public:
    /**
     * @brief Get adapter manager singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 6
     */
    static AdapterManager *GetInstance();

    /**
     * @brief Get classic adapter interface, called in bluetooth_server.
     *
     * @return Returns IAdapterClassic pointer.
     * @since 6
     */
    std::shared_ptr<IAdapterClassic> GetClassicAdapterInterface(void) const override;

    /**
     * @brief Get classic adapter, called in btservice.
     *
     * @return Returns ClassicAdapter pointer.
     * @since 6
     */
    std::shared_ptr<ClassicAdapter> GetClassicAdapter(void) const;

    /**
     * @brief Get ble adapter interface, called in bluetooth_server..
     *
     * @return Returns IAdapterBle pointer.
     * @since 6
     */
    std::shared_ptr<IAdapterBle> GetBleAdapterInterface(void) const override;

    /**
     * @brief Get ble adapter, called in btservice.
     *
     * @return Returns BleAdapter pointer.
     * @since 6
     */
    std::shared_ptr<BleAdapter> GetBleAdapter(void) const;

    /**
     * @brief bluetooth adapter start.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool Start() override;

    /**
     * @brief Stop bluetooth service.
     *
     * @since 6
     */
    void Stop() const override;

    /**
     * @brief Reset bluetooth service.
     *
     * @since 6
     */
    void Reset() const override;

    /**
     * @brief Factory reset bluetooth service.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool FactoryReset() const override;

    /**
     * @brief Enable bluetooth service.
     *
     * @param transport Adapter transport.
     * @param isAsync Indicate Async function flag.
     * @param callingName Indicate calling application
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 6
     */
    int32_t Enable(BTTransport transport, bool isAsync = false, std::string callingName = "",
        bool isUserTriggered = false) const override;

    /**
     * @brief Disable bluetooth service.
     *
     * @param transport Adapter transport.
     * @param isAsync Indicate Async function flag.
     * @param callingName Indicate calling application
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 6
     */
    int32_t Disable(BTTransport transport, bool isAsync = false, std::string callingName = "",
        bool isUserTriggered = false) const override;

    /**
     * @brief Get adapter enable/disable state.
     *
     * @param transport Adapter transport.
     * @return Returns adapter enable/disable state.
     * @since 6
     */
    BTStateID GetState(BTTransport transport) const override;

    /**
     * @brief Get adapter connects state.
     *
     * @return Returns adapter connects state.
     * @since 6
     */
    BTConnectState GetAdapterConnectState() const override;

    /**
     * @brief Register adapter state observer.
     *
     * @param observer Class IAdapterStateObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool RegisterStateObserver(IAdapterStateObserver &observer) const override;

    /**
     * @brief Deregister adapter state observer.
     *
     * @param observer Class IAdapterStateObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool DeregisterStateObserver(IAdapterStateObserver &observer) const override;

    /**
     * @brief Register system state observer.
     *
     * @param observer Class ISystemStateObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool RegisterSystemStateObserver(ISystemStateObserver &observer) const override;

    /**
     * @brief Deregister system state observer.
     *
     * @param observer Class ISystemStateObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool DeregisterSystemStateObserver(ISystemStateObserver &observer) const override;

    /**
     * @brief Notify bluetooth the result of bluetooth dialog.
     *
     * @param dialogType The type of bluetooth dialog.
     * @param dialogResult The result of bluetooth dailog.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 20
     */
    int32_t NotifyDialogResult(uint32_t dialogType, bool dialogResult) const override;

    /**
     * @brief Get max audio connected devices number.
     *
     * @return Returns max device number that audio can connect.
     * @since 6
     */
    int GetMaxNumConnectedAudioDevices() const override;

    /**
     * @brief Set phonebook permission for device.
     *
     * @param address Device address which is setted permission.
     * @param permission permission grade.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetPhonebookPermission(const std::string &address, BTPermissionType permission) const override;

    /**
     * @brief Get phonebook permission for device.
     *
     * @param address Device address which is setted permission.
     * @return Returns permission grade.
     * @since 6
     */
    BTPermissionType GetPhonebookPermission(const std::string &address) const override;

    /**
     * @brief Set message permission for device.
     *
     * @param address Device address which is setted permission.
     * @param permission permission grade.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool SetMessagePermission(const std::string &address, BTPermissionType permission) const override;

    /**
     * @brief Get message permission for device.
     *
     * @param address Device address which is setted permission.
     * @return Returns Permission grade.
     * @since 6
     */
    BTPermissionType GetMessagePermission(const std::string &address) const override;

    /**
     * @brief Get power mode.
     *
     * @param address Device address.
     * @return Returns power mode grade.
     *         BTPowerMode::MODE_INVALID = 0x00,
     *         BTPowerMode::MODE_ACTIVE = 0x100,
     *         BTPowerMode::MODE_SNIFF_LEVEL_LOW = 0x201,
     *         BTPowerMode::MODE_SNIFF_LEVEL_MID = 0x202,
     *         BTPowerMode::MODE_SNIFF_LEVEL_HIG = 0x203,
     * @since 6
     */
    int GetPowerMode(const std::string &address) const override;

    /**
     * @brief Stop bluetooth adapter and profile service.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool AdapterStop() const;

    /**
     * @brief Clear all storage.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    bool ClearAllStorage() const;

    /**
     * @brief System state change.
     *
     * @param state Change to a new state.
     * @since 6
     */
    void OnSysStateChange(const std::string &state) const;

    /**
     * @brief System state exit.
     *
     * @param state Exit the old state.
     * @since 6
     */
    void OnSysStateExit(const std::string &state) const;

    /**
     * @brief Adapter state change.
     *
     * @param transport Adapter transport.
     * @param state Change to a new state.
     * @since 6
     */
    void OnAdapterStateChange(const BTTransport transport, const BTStateID state) const;

    /**
     * @brief Turn on Bluetooth High Power.
     * @since 6
     */
    void SetHighPower() const;

    /**
     * @brief Profile services enable complete notify.
     *
     * @param transport Adapter transport.
     * @param ret Profile services enable operation result.
     * @since 6
     */
    void OnProfileServicesEnableComplete(const BTTransport transport, const bool ret) const;

    /**
     * @brief Profile services disable complete notify.
     *
     * @param transport Adapter transport.
     * @param ret Profile services disable operation result.
     * @since 6
     */
    void OnProfileServicesDisableComplete(const BTTransport transport, const bool ret) const;

    /**
     * @brief Pair devices remove notify.
     *
     * @param transport Adapter transport.
     * @param devices The vector of removed devices.
     * @since 6
     */
    void OnPairDevicesRemoved(const std::vector<RawAddress> &devices) const;
    BtInterface* getBluetoothInterface() const;
    void setBluetoothInterface(BtInterface* interface) const;
    static void AdapterStateChangedCb(BtState state);
    static void DeviceFoundCb(int numProperties, BtProperty* properties);
    static void DiscoveryStateChangedCb(BtDiscoveryState state);
    static void BondStateChangedCb(BtStackStatus status, OHOS::bluetooth::RawAddress* bdAddr, BtBondState state);
    static void SspRequestCb(OHOS::bluetooth::RawAddress* remoteBdAddr, BtBdname* bdName, uint32_t cod,
                             BtSspVariant pairingVariant, uint32_t passKey);
    static void AclStateChangedCb(BtStackStatus status, OHOS::bluetooth::RawAddress* remoteBdAddr, BtAclState state,
                                  BtHciErrorCode hciReason, BtTransport linkTypeCallback);
    static void AdapterPropertiesCb(BtStackStatus status, int numProperties, BtProperty* properties);
    static void RemoteDevicePropertiesCb(BtStackStatus status, OHOS::bluetooth::RawAddress* bdAddr,
                                         int numProperties, BtProperty* properties);
    static void ThreadEvtCb(BtCbThreadEvt evt);
    static void PinRequestCb(OHOS::bluetooth::RawAddress* remoteBdAddr, BtBdname* bdName, uint32_t cod,
        bool min16Digit);
    static void DutModeRecvCb(uint16_t opcode, uint8_t* buf, uint8_t len);
    static void EnergyInfoCb(BtActivityEnergyInfo* energyInfo, BtUidTraffic* uidData);
    static void SensingStateChangedCb(uint8_t eventId, BtSensingInfo* info);
    static void GenerateLocalOobDataCb(BtTransport t, BtStackOobData oobdata);
    void UnLoadBluetoothSystemAbility(const BTTransport transport, const BTStateID state) const;
    void UpdateBluetoothSwitchStatus(const BTTransport transport, const BTStateID state) const;
    bool IsBluetoothSwitchEnableFromSystemParameter() const;
    void StartUnloadBluetoothSaTimer() const;
    void StopUnloadBluetoothSaTimer() const;
    bool IsUnloadBluetoothSaTimerStarted() const;
    bool IsBluetoothSysStateStarted() const;
    int32_t SatelliteControl(int type, int state) const override;
    bool IsBluetoothRestricted() const override;
    void SetBluetoothRestrictedFlag(bool isBluetoothRestricted) const override;
    void SetBluetoothRestrictedFlagOnly(bool isBluetoothRestricted) const;
    BTStateID GetRestrictedState(BTTransport transport)  const override;
    int32_t EnablebluetoothFromRestricted(std::string callingName = "", bool isAsync = false,
        bool isUserTriggered = false) const override;
    int32_t EnableBluetoothToRestrictMode(std::string callingName = "",
        bool isUserTriggered = false) override;
    int32_t EnableBluetoothToBleOwnerOnlyMode(int32_t pid, std::string callingName = "",
        bool isUserTriggered = false) override;
    int32_t EnableBluetoothFromBleOwnerOnlyMode(std::string callingName = "") const override;
    bool IsBleOwnerOnlyMode() const override;
    bool IsBleAccessible(int32_t pid) const override;
    bool IsBrAllowed() const override;
    std::set<int32_t> GetOwnerPids() const override;
    bool IsBetaVersion() const;
    void EnableHisiPcm(bool state) const;
    bool IsCollaborationModeOn(void) const;
    int EnableBluetoothFromOffToRestrictMode(const std::string &callingName) const;
    bool IsFactoryReset() const;
    void SetApplicationDisableBluetoothFlag(bool isAppCloseBt) override;
    bool IsSupportVirtualAutoConnect(const std::string &address) const override;
    int SetVirtualAutoConnectType(const std::string &address, int connType, int businessType) const override;
    bool GetQuietMode() const;
    void SetQuietMode(bool isQuietMode) const override;
    int32_t UpdateRefusePolicy(const int32_t protocolType,
        const int32_t pid, const int64_t prohibitedSecondsTime) const override;
    int32_t SetTmpAdvName(const uint32_t tokenId, const std::string &name) override;
    std::string GetTmpAdvName(const uint32_t tokenId) override;
    void EraseTmpAdvName(const uint32_t tokenId) override;
    static Bluetooth::BluetoothOobData BuildBluetoothOobData(const BtStackOobData &data);
#ifdef COMMUNICATION_L2
    static int AcquireWakeLock(const char* lockName);
    static int ReleaseWakeLock(const char* lockName);
#endif
    static void ResetCloudBondingState(const RawAddress &device);
    static bool StartPairIfInCloudCreateBondState(const RawAddress &device);
    static void HandleCloudBondWhenAclStateChange(const RawAddress &device, int connectionState);
    static int HandleAclStateChanged(std::shared_ptr<BluetoothDevice> remoteDevice, const RawAddress &device,
        OHOS::bluetooth::RawAddress *remoteBdAddr, BtAclState state);

    void RegisterVirtualDeviceIdManagerFunc(
        std::function<int32_t(const RawAddress &realAddr, RawAddress &randomAddr)> func) override;
    int32_t GetDeviceRandomAddr(const RawAddress &realAddr, RawAddress &randomAddr);
#ifdef FEATURE_MOUSE_FAST_CONNECTION
    void SetNeedReconnectActiveDevice() const override;
#endif
private:
    AdapterManager();
    ~AdapterManager() override;
    void Initialize() const;
    void CreateAdapters() const;
    std::string GetSysState() const;
    bool OutputSetting() const;
    void RegisterHciResetCallback();
    void DeregisterHciResetCallback() const;
    void RemoveDeviceProfileConfig(const std::vector<RawAddress> &devices) const;

    static void HciFailedReset(void *context);
    void AdapterStateChangedInner(BtState state);
    void ProcessStackEnableCmpMsg();
    void ProcessStackDisableCmpMsg();
    int StackInit();
    void UnloadBluetoothSaTimerCallback() const;
    bool UpdateBluetoothState(const BTTransport transport, const BTStateID state) const;
    void PostAutoConnectTask(const int32_t delayMs) const;
    void PromptUnloadBluetoothSystemAbility() const;
    bool IsThirdpartyNeedBluetoothSwitchDialog() const;
    bool IsAdpaterNullptrWhenEnable(BTTransport transport) const;
    bool IsAdpaterNullptrWhenDisable(BTTransport transport) const;
    void ExecuteTaskWhenBluetoothOn(void) const;

#ifdef COMMUNICATION_L2
    int AcquireWakeLockCallout(const char* lockName);
    int ReleaseWakeLockCallout(const char* lockName);
    std::shared_ptr<PowerMgr::RunningLock> runningLock_;
#endif
#ifdef BLUETOOTH_PLUGGABLE_SUPPORTED
    bool CheckBluetoothPluggableState() const;
#endif

    void WaitForAllSwitchDependency(void);
    void InitializeAfterAllDependencyOn(void);
    int32_t EnableInner(const std::string &callingName, const BTTransport transport, bool isAsync,
        bool isUserTriggered) const;
    int32_t DisableInner(BTTransport transport, bool isAsync, std::string callingName) const;
    static void SaveConnectionTime(const RawAddress &device);

    static std::shared_ptr<utility::Timer> UnloadSaTimeout_;
    mutable std::mutex unloadSaTimeoutMutex_;
    inline static constexpr int TIMEOUT_UNLOAD_BLUETOOTH_SA {10000};
    std::shared_ptr<ParamUpdateEventManager> paramUpdateManager_ {nullptr};
    std::shared_ptr<SwitchDependency> switchDependency_ { nullptr };
    BT_DISALLOW_COPY_AND_ASSIGN(AdapterManager);
    DECLARE_IMPL();
    std::function<int32_t(const RawAddress &realAddr, RawAddress &randomAddr)> virtualDeviceIdManageFunc_;
    std::shared_ptr<DialogResult> dialogResult_ {nullptr};
    SafeMap<int32_t, std::string> tmpAdvNameMap_;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // ADAPTER_MANAGER_H
