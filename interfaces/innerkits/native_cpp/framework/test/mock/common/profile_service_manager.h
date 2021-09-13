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

namespace bluetooth {
/**
 * @brief Represents profile service manager.
 *
 * @since 1.0
 * @version 1.0
 */
class ProfileServiceManager : public IProfileManager {
public:
    static ProfileServiceManager *GetInstance();
    IProfile *GetProfileService(const std::string &name) const override;
    std::vector<uint32_t> GetProfileServicesList() const override {std::vector<uint32_t> list; return list;}
    BTConnectState GetProfileServiceConnectState(const uint32_t profileID) const override {return BTConnectState::CONNECTED;}
    void GetProfileServicesSupportedUuids(std::vector<std::string> &uuids) const override {}

    ProfileServiceManager();
    ~ProfileServiceManager() = default;
};
}  // namespace bluetooth

#endif  // PROFILE_SERVICE_MANAGER_H