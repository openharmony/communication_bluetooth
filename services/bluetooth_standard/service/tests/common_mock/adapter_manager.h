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

#ifndef ADAPTER_MANAGER_H
#define ADAPTER_MANAGER_H

#include "interface_adapter.h"
#include "interface_adapter_manager.h"
#include "context.h"

namespace bluetooth {
/**
 * @brief Represents adapter manager.
 *
 * @since 1.0
 * @version 1.0
 */
class AdapterManager : public IAdapterManager {
public:
    /**
     * @brief Get adapter manager singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 1.0
     * @version 1.0
     */
    static AdapterManager *GetInstance();

    /// framework function
    /**
     * @brief Get classic adapter or ble adapter.
     *
     * @param transport Adapter transport.
     * @return Returns Basic adapter pointer.
     * @since 1.0
     * @version 1.0
     */
    IAdapter *GetAdapter(const BTTransport transport) const override;

    /**
     * @brief bluetooth adapter start.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool Start() override;

    /**
     * @brief Stop bluetooth service.
     *
     * @since 1.0
     * @version 1.0
     */
    void Stop() const override;

    /**
     * @brief Reset bluetooth service.
     *
     * @since 1.0
     * @version 1.0
     */
    void Reset() const override;

    /**
     * @brief Factory reset bluetooth service.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool FactoryReset() const override;

    /**
     * @brief Enable bluetooth service.
     *
     * @param transport Adapter transport.
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 1.0
     * @version 1.0
     */
    bool Enable(const BTTransport transport) const override;

    /**
     * @brief Disable bluetooth service.
     *
     * @param transport Adapter transport.
     * @return Returns <b>true</b> if the operation is accepted;
     *         returns <b>false</b> if the operation is rejected.
     * @since 1.0
     * @version 1.0
     */
    bool Disable(const BTTransport transport) const override;

    /**
     * @brief Get adapter enable/disable state.
     *
     * @param transport Adapter transport.
     * @return Returns adapter enable/disable state.
     * @since 1.0
     * @version 1.0
     */
    BTStateID GetState(const BTTransport transport) const override;

    /**
     * @brief Get adapter connects state.
     *
     * @return Returns adapter connects state.
     * @since 1.0
     * @version 1.0
     */
    BTConnectState GetAdapterConnectState() const override;

    /**
     * @brief Register adapter state observer.
     *
     * @param observer Class IAdapterStateObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool RegisterStateObserver(IAdapterStateObserver &observer) const override;

    /**
     * @brief Deregister adapter state observer.
     *
     * @param observer Class IAdapterStateObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool DeregisterStateObserver(IAdapterStateObserver &observer) const override;

    /**
     * @brief Register system state observer.
     *
     * @param observer Class ISystemStateObserver pointer to register observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool RegisterSystemStateObserver(ISystemStateObserver &observer) const override;

    /**
     * @brief Deregister system state observer.
     *
     * @param observer Class ISystemStateObserver pointer to deregister observer.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool DeregisterSystemStateObserver(ISystemStateObserver &observer) const override;

    /**
     * @brief Get max audio connected devices number.
     *
     * @return Returns max device number that audio can connect.
     * @since 1.0
     * @version 1.0
     */
    int GetMaxNumConnectedAudioDevices() const override;

    /**
     * @brief Set phonebook permission for device.
     *
     * @param address Device address which is setted permission.
     * @param permission permission grade.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetPhonebookPermission(const std::string &address, BTPermissionType permission) const override;

    /**
     * @brief Get phonebook permission for device.
     *
     * @param address Device address which is setted permission.
     * @return Returns permission grade.
     * @since 1.0
     * @version 1.0
     */
    BTPermissionType GetPhonebookPermission(const std::string &address) const override;

    /**
     * @brief Set message permission for device.
     *
     * @param address Device address which is setted permission.
     * @param permission permission grade.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool SetMessagePermission(const std::string &address, BTPermissionType permission) const override;

    /**
     * @brief Get message permission for device.
     *
     * @param address Device address which is setted permission.
     * @return Returns Permission grade.
     * @since 1.0
     * @version 1.0
     */
    BTPermissionType GetMessagePermission(const std::string &address) const override;

    /**
     * @brief Get power mode.
     *
     * @param address Device address.
     * @return
     * @since 1.0
     * @version 1.0
     */
    int GetPowerMode(const std::string &address) const override;

    /**
     * @brief Pair devices remove notify.
     *
     * @param transport Adapter transport.
     * @param devices The vector of removed devices.
     * @since 1.0
     * @version 1.0
     */
    void OnPairDevicesRemoved(const BTTransport transport, const std::vector<RawAddress> &devices);

private:
    AdapterManager();
    ~AdapterManager();

    DISALLOW_COPY_AND_ASSIGN(AdapterManager);
    DECLARE_IMPL();
};
};  // namespace bluetooth

#endif  // ADAPTER_MANAGER_H