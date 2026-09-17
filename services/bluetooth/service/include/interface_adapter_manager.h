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
 * @brief Defines adapter manager, including observer and common functions.
 *
 * @since 6
 */

/**
 * @file interface_adapter_manager.h
 *
 * @brief Adapter manager interface.
 *
 * @since 6
 */

#ifndef INTERFACE_ADAPTER_MANAGER
#define INTERFACE_ADAPTER_MANAGER

#include "interface_adapter.h"
#include "interface_adapter_ble.h"
#include "interface_adapter_classic.h"
#include <memory>

namespace OHOS {
namespace bluetooth {
enum Priority {
    PRIORITY_BEGIN = 1,
    PRIORITY_HIGH = PRIORITY_BEGIN,
    PRIORITY_LOW,
    PRIORITY_END,
};

/**
 * @brief Represents adapter state change observer during enable/disable.
 *
 * @since 6
 */
class IAdapterStateObserver {
public:

    explicit IAdapterStateObserver(int priority = PRIORITY_LOW) : priority_(priority) {}
    /**
     * @brief A destructor used to delete the <b>IAdapterStateObserver</b> instance.
     *
     * @since 6
     */
    virtual ~IAdapterStateObserver() = default;

    /**
     * @brief IAdapterStateObserver state change function.
     *
     * @param transport Transport type when state change.
     * @param state Change to the new state.
     * @since 6
     */
    virtual void OnStateChange(const BTTransport transport, const BTStateID state) = 0;

    /**
     * @brief bluetooth state change new callback.
     *
     * @param state Change to the new state.
     *      BluetoothSwitchState::STATE_ON,
     *      BluetoothSwitchState::STATE_OFF,
     *      BluetoothSwitchState::STATE_HALF,
     *
     * @since 12
     */
    virtual void OnBluetoothStateChanged(int state) = 0;

    /**
     * @brief KIA refuse policy.
     *
     * @param pid Restricting processes using Bluetooth.
     * @param prohibitedSecondsTime Restricted use time point.
     *
     * @since 16
     */
    virtual void OnRefusePolicyChanged(const int32_t pid, const int64_t prohibitedSecondsTime) {};

    int GetPriority(void)
    {
        return priority_;
    }

private:
    int priority_ = PRIORITY_LOW;
};

/**
 * @brief System state define, using to OnSystemChange()...
 */
enum class BTSystemState : int {
    ON,
    OFF,
};

/**
 * @brief Represents system state change observer during start/stop/factoryReset/reset.
 *
 * @since 6
 */
class ISystemStateObserver {
public:
    /**
     * @brief A destructor used to delete the <b>ISystemStateObserver</b> instance.
     *
     * @since 6
     */
    virtual ~ISystemStateObserver() = default;

    /**
     * @brief OnSystemStateChange state change function.
     *
     * @param transport Transport type when state change.
     * @param state Change to
     * @since 6
     */
    virtual void OnSystemStateChange(const BTSystemState state) = 0;
};

/**
 * @brief Represents interface adapter manager.
 *
 * @since 6
 */
class BLUETOOTH_API IAdapterManager {
public:
    /**
     * @brief A destructor used to delete the <b>IAdapterManager</b> instance.
     *
     * @since 6
     */
    virtual ~IAdapterManager() = default;

    /**
     * @brief Get adapter manager singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 6
     */
    static IAdapterManager *GetInstance();

    /**
     * @brief Reset bluetooth service.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual void Reset() const = 0;

    /**
     * @brief Start bluetooth service.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool Start() = 0;

    /**
     * @brief Stop bluetooth service.
     *
     * @since 6
     */
    virtual void Stop() const = 0;

    /**
     * @brief Factory reset bluetooth service.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool FactoryReset() const = 0;

    /**
     * @brief Enable bluetooth service.
     *
     * @param transport Enable classic or ble.
     * @param isAsync Indicate Async function flag.
     * @param callingName Indicate calling application
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 6
     */
    virtual int32_t Enable(const BTTransport transport, bool isAsync, std::string callingName,
        bool isUserTriggered) const = 0;

    /**
     * @brief BT switch core when satellite launches.
     *
     * @param state <b>True or false if bluetooth is restricte
     * @since 12
     */
    virtual int32_t SatelliteControl(int type, int state) const = 0;

    /**
     * @brief Set bluetooth restricted state.
     *
     * @param isBluetoothRestricted <b>True or false if bluetooth is restricte
     * @since 12
     */
    virtual void SetBluetoothRestrictedFlag(bool isBluetoothRestricted) const = 0;

    /**
     * @brief Get bluetooth restricted state.
     *
     * @param transport Enable classic or ble.
     * @return Returns <b>True</b> if bluetooth is restricted;
     *         returns <b>False</b> if bluetooth is not restricted.
     * @since 12
     */
    virtual bool IsBluetoothRestricted() const = 0;

    /**
     * @brief Get Restricted bluetooth adapter enable/disable state.
     *
     * @param transport Disable classic or ble.
     * @return Returns adapter enable/disable state.
     * @since 12
     */
    virtual BTStateID GetRestrictedState(BTTransport transport) const = 0;

    /**
     * @brief Enable bluetooth from restricted mode.
     *
     * @param callingName Indicate calling application
     * @param isAsync Indicate Async function flag.
     * @return int32_t
     * @since 12
     */
    virtual int32_t EnablebluetoothFromRestricted(
        std::string callingName, bool isAsync, bool isUserTriggered) const = 0;

    /**
     * @brief Enable bluetooth from restricted mode.
     *
     * @param callingName Indicate calling application
     * @return void
     * @since 12
     */
    virtual int32_t EnableBluetoothToRestrictMode(std::string callingName, bool isUserTriggered) = 0;

    /**
     * @brief Enable bluetooth to half-app-registered mode: both stacks are enabled underneath,
     * but only BLE functions are allowed and only the owner app can use BLE.
     *
     * @param callingName Indicate the calling application, recorded as the owner.
     * @return int32_t
     */
    virtual int32_t EnableBluetoothToHalfAppRegisteredMode(std::string callingName, bool isUserTriggered) = 0;

    /**
     * @brief Enable bluetooth from half-app-registered mode to full-on state.
     *
     * @param callingName Indicate calling application
     * @return int32_t
     */
    virtual int32_t EnableBluetoothFromHalfAppRegisteredMode(std::string callingName) const = 0;

    /**
     * @brief Whether the bluetooth switch is in half-app-registered mode.
     */
    virtual bool IsHalfAppRegisteredMode() const = 0;

    /**
     * @brief Whether the caller can use BLE functions in half-app-registered mode.
     */
    virtual bool IsBleAccessible(const std::string &callingName) const = 0;

    /**
     * @brief Whether BR functions (discovery/pairing/connection) are allowed.
     */
    virtual bool IsBrAllowed() const = 0;

    /**
     * @brief Disable bluetooth service.
     *
     * @param isAsync Indicate Async function flag.
     * @param transport Disable classic or ble.
     * @param callingName Indicate calling application
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 6
     */
    virtual int32_t Disable(const BTTransport transport, bool isAsync, std::string callingName,
        bool isUserTriggered) const = 0;

    /**
     * @brief Get adapter enable/disable state.
     *
     * @param transport Disable classic or ble.
     * @return Returns adapter enable/disable state.
     * @since 6
     */
    virtual BTStateID GetState(const BTTransport transport) const = 0;

    /**
     * @brief Get adapter connects state.
     *
     * @return Returns adapter connects state.
     * @since 6
     */
    virtual BTConnectState GetAdapterConnectState() const = 0;

    /**
     * @brief Register adapter state observer.
     *
     * @param observer Class IAdapterStateObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool RegisterStateObserver(IAdapterStateObserver &observer) const = 0;

    /**
     * @brief Deregister adapter state observer.
     *
     * @param observer Class IAdapterStateObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool DeregisterStateObserver(IAdapterStateObserver &observer) const = 0;

    /**
     * @brief Register system state observer.
     *
     * @param observer Class ISystemStateObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool RegisterSystemStateObserver(ISystemStateObserver &observer) const = 0;

    /**
     * @brief Deregister system state observer.
     *
     * @param observer Class ISystemStateObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool DeregisterSystemStateObserver(ISystemStateObserver &observer) const = 0;

    /**
     * @brief Notify bluetooth the result of bluetooth dialog.
     *
     * @param dialogType The type of bluetooth dialog.
     * @param dialogResult The result of bluetooth dialog.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 20
     */
    virtual int32_t NotifyDialogResult(uint32_t dialogType, bool dialogResult) const = 0;

    /**
     * @brief Get max audio connected devices number.
     *
     * @return Returns max device number that audio can connect.
     * @since 6
     */
    virtual int GetMaxNumConnectedAudioDevices() const = 0;

    /**
     * @brief Set phonebook permission for device.
     *
     * @param address Device address which is setted permission.
     * @param permission Permission grade.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetPhonebookPermission(const std::string &address, BTPermissionType permission) const = 0;

    /**
     * @brief Get phonebook permission for device.
     *
     * @param address Device address which is setted permission.
     * @return Returns Permission grade.
     * @since 6
     */
    virtual BTPermissionType GetPhonebookPermission(const std::string &address) const = 0;

    /**
     * @brief Set message permission for device.
     *
     * @param address Device address which is setted permission.
     * @param permission Permission grade.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    virtual bool SetMessagePermission(const std::string &address, BTPermissionType permission) const = 0;

    /**
     * @brief Get message permission for device.
     *
     * @param address Device address which is setted permission.
     * @return Returns Permission grade.
     * @since 6
     */
    virtual BTPermissionType GetMessagePermission(const std::string &address) const = 0;

    /**
     * @brief Get classic adapter.
     *
     * @return Returns IAdapterClassic pointer.
     * @since 6
     */
    virtual std::shared_ptr<IAdapterClassic> GetClassicAdapterInterface(void) const = 0;

    /**
     * @brief Get ble adapter.
     *
     * @return Returns IAdapterBle pointer.
     * @since 6
     */
    virtual std::shared_ptr<IAdapterBle> GetBleAdapterInterface(void) const = 0;

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
    virtual int GetPowerMode(const std::string &address) const = 0;

    /**
     * @brief Set application disable bluetooth
     *
     * @param isAppCloseBt If true, the bluetooth will unload immediatelly.
     * @return void
     */
    virtual void SetApplicationDisableBluetoothFlag(bool isAppCloseBt) = 0;

    /**
     * @brief Get remote device is supported virtual auto connect.
     *
     * @param[out] support the remote device configure auto connect value.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 12
     */
    virtual bool IsSupportVirtualAutoConnect(const std::string &address) const = 0;

    /**
     * @brief Set remote device auto connect type and business type.
     *
     * @param connType auto connect type value.
     * @param businessType business connect type value.
     * @return Returns <b>BT_NO_ERROR</b> if the operation is successful;
     *         returns <b>Other code</b> if the operation fails.
     * @since 12
     */
    virtual int SetVirtualAutoConnectType(const std::string &address, int connType, int businessType) const = 0;

    /**
     * @brief Set quietMode for preventing AutoConnect after bluetooth is enabled
     *
     * @param isQuietMode  quietMode = true means users who enable bluetooth do not
     *                     want to autoConnect the last connective/active device.
     * @return void
     * @since 16
     */
    virtual void SetQuietMode(bool isQuietMode) const = 0;

    /**
     * @brief update KIA refuse policy.
     *
     * @param protocolType protocol type
     * @param pid process PID
     * @param prohibitedSecondsTime Restricted use time point.
     *
     * @return Returns {@link BT_NO_ERROR} if the operation is successful;
    *         returns an error code defined in {@link BtErrCode} otherwise.
     * @since 16
     */
    virtual int32_t UpdateRefusePolicy(const int32_t protocolType,
        const int32_t pid, const int64_t prohibitedSecondsTime) const = 0;

    virtual int32_t SetTmpAdvName(const uint32_t tokenId, const std::string &name) = 0;

    virtual std::string GetTmpAdvName(const uint32_t tokenId) = 0;

    virtual void EraseTmpAdvName(const uint32_t tokenId) = 0;

    virtual void RegisterVirtualDeviceIdManagerFunc(
        std::function<int32_t(const RawAddress &realAddr, RawAddress &randomAddr)> func) = 0;
#ifdef FEATURE_MOUSE_FAST_CONNECTION
    virtual void SetNeedReconnectActiveDevice() const = 0;
#endif
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // INTERFACE_ADAPTER_MANAGER
