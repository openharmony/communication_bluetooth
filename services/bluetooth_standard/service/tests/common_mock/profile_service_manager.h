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
 * @brief Defines profile services manager for bluetooth system.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file profile_service_manager.h
 *
 * @brief system state machine interface.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef PROFILE_SERVICE_MANAGER_H
#define PROFILE_SERVICE_MANAGER_H

#include <map>
#include <vector>

#include "interface_profile.h"
#include "interface_profile_manager.h"
#include "dispatcher.h"

namespace bluetooth {
/**
 * @brief Represents profile service manager.
 *
 * @since 1.0
 * @version 1.0
 */
class ProfileServiceManager : public IProfileManager {
public:
    /**
     * @brief Get profile service manager singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     * @since 1.0
     * @version 1.0
     */
    static ProfileServiceManager *GetInstance();

    /// framework function
    /**
     * @brief Get profile service pointer.
     *
     * @param name Profile service name.
     * @return Returns the profile service pointer.
     * @since 1.0
     * @version 1.0
     */
    IProfile *GetProfileService(const std::string &name) const override;

    /**
     * @brief Create profile services according to config.xml.
     *
     * @since 1.0
     * @version 1.0
     */
    void Start();

    /**
     * @brief Delete all profile services when Start() create.
     *
     * @since 1.0
     * @version 1.0
     */
    void Stop();

    /**
     * @brief Enable profile services.
     *
     * @param transport Adapter transport.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool Enable(const BTTransport transport);

    /**
     * @brief Set all profile services states as trun on.
     *
     * @param transport Adapter transport.
     * @since 1.0
     * @version 1.0
     */
    void OnAllEnabled(const BTTransport transport);

    /**
     * @brief Disable profile services.
     *
     * @param transport Adapter transport.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 1.0
     * @version 1.0
     */
    bool Disable(const BTTransport transport);

    /**
     * @brief Set all profile services states as trun off.
     *
     * @param transport Adapter transport.
     * @since 1.0
     * @version 1.0
     */
    void OnAllDisabled(const BTTransport transport);

    /**
     * @brief Get profile service ID list.
     *
     * @return Returns vector of enabled profile services ID.
     * @since 1.0
     * @version 1.0
     */
    std::vector<uint32_t> GetProfileServicesList() const override;

    /**
     * @brief Get profile service connect state.
     *
     * @param profileID Profile service ID.
     * @return Returns connect state for designated profile service.
     * @since 1.0
     * @version 1.0
     */
    BTConnectState GetProfileServiceConnectState(const uint32_t profileID) const override;

    /**
     * @brief Get local device supported uuids.
     *
     * @param[out] Vector which use to return support uuids.
     * @since 1.0
     * @version 1.0
     */
    void GetProfileServicesSupportedUuids(std::vector<std::string> &uuids) const override;
    /**
     * @brief Get all profile services connect state.
     *
     * @return Returns profile services connect state.
     * @since 1.0
     * @version 1.0
     */
    BTConnectState GetProfileServicesConnectState();

    /**
     * @brief Profile service enable complete notify.
     *
     * @param profileID Profile service ID.
     * @param ret Profile service enable operation result.
     * @since 1.0
     * @version 1.0
     */
    void OnEnable(const std::string &name, bool ret);

    /**
     * @brief Profile service disable complete notify.
     *
     * @param profileID Profile service ID.
     * @param ret Profile service disable operation result.
     * @since 1.0
     * @version 1.0
     */
    void OnDisable(const std::string &name, bool ret);

    /**
     * @brief Set profile service manager dispatcher.
     *
     * @param dispatcher The pointer of dispatcher used in profile service manager.
     * @since 1.0
     * @version 1.0
     */
    void SetDispatcher(utility::Dispatcher *dispatcher);

private:
    ProfileServiceManager();
    ~ProfileServiceManager();

    void CreateConfigSupportProfiles();
    void CreateClassicProfileServices();
    void CreateBleProfileServices();

    void EnableProfiles(const BTTransport transport);
    void DisableProfiles(const BTTransport transport);
    void EnableCompleteProcess(const std::string name, bool ret);
    void DisableCompleteProcess(const std::string name, bool ret);
    void EnableCompleteNotify(const BTTransport transport);
    void DisableCompleteNotify(const BTTransport transport);
    bool IsAllEnabled(const BTTransport transport);
    bool IsProfilesTurning(const BTTransport transport);
    bool IsAllDisabled(const BTTransport transport);
    void CheckWaitEnableProfiles(const std::string &name, const BTTransport transport);

    DISALLOW_COPY_AND_ASSIGN(ProfileServiceManager);
    DECLARE_IMPL();
};
}  // namespace bluetooth

#endif  // PROFILE_SERVICE_MANAGER_H