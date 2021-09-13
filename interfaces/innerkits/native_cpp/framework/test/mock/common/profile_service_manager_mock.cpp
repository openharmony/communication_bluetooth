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
#include <memory>
#include <map>
#include "profile_service_manager.h"
#include "../gatt/gatt_server_service.h"
#include "../gatt/gatt_client_service.h"
#include "../sock/socket_service.h"
#include "bluetooth_log.h"

using namespace std;

namespace bluetooth {

static std::unique_ptr<ProfileServiceManager> g_profileManager = nullptr;
map<string, IProfile*> startedProfiles = {};

IProfileManager *IProfileManager::GetInstance()
{
    return ProfileServiceManager::GetInstance();
}

ProfileServiceManager *ProfileServiceManager::GetInstance()
{
    HILOGI("ProfileServiceManager::GetInstance start");
    if (!g_profileManager) {
        g_profileManager = std::make_unique<ProfileServiceManager>();
        HILOGI("ProfileServiceManager::GetInstance： g_profileManager generated");
    }
    return g_profileManager.get();
}

IProfile *ProfileServiceManager::GetProfileService(const string &name) const
{
    HILOGI("ProfileServiceManager::GetProfileService start");
    return startedProfiles[name];
}

ProfileServiceManager::ProfileServiceManager()
{
    HILOGI("ProfileServiceManager::ProfileServiceManager profiles started");
    startedProfiles[PROFILE_NAME_GATT_CLIENT] = new GattClientService();
    startedProfiles[PROFILE_NAME_GATT_SERVER] = new GattServerService();
    startedProfiles[PROFILE_NAME_SPP] = new SocketService();
}

}  // namespace bluetooth
