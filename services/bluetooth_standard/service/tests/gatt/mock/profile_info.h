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

#ifndef PROFILE_INFO_H
#define PROFILE_INFO_H

#include "base_def.h"
#include <string>
#include <vector>
#include "adapter_config.h"

namespace bluetooth {

struct ProfileInfo {
    ProfileInfo() = delete;
    ProfileInfo(std::string name, uint32_t id, std::string uuid)
    {
        name_ = name;
        id_ = id;
        uuid_ = uuid;
    }
    ~ProfileInfo()
    {}
    std::string name_;
    uint32_t id_;
    std::string uuid_;
};

class SupportProfilesInfo {
public:
    SupportProfilesInfo()
    {}
    ~SupportProfilesInfo()
    {}
    static SupportProfilesInfo *GetInstance()
    {
        static SupportProfilesInfo instance;
        if (nullptr == instance_) {
            instance_ = &instance;
        }

        return instance_;
    }
    const std::vector<ProfileInfo> &GetSupportProfiles()
    {
        return supportProfiles_;
    }
    std::string IdToName(uint32_t id)
    {
        std::string name;
        for (auto &it : supportProfiles_) {
            if (it.id_ == id) {
                return it.name_;
            }
        }
        return name;
    }
    const std::vector<ProfileInfo> GetConfigSupportProfiles(BTTransport transport)
    {
        std::vector<ProfileInfo> retProfiles;
        retProfiles.clear();
        std::string section = (transport == BTTransport::ADAPTER_BREDR) ? SECTION_CLASSIC_ADAPTER : SECTION_BLE_ADAPTER;
        for (auto &sp : supportProfiles_) {
            bool value = false;
            AdapterConfig::GetInstance()->GetValue(section, sp.name_, value);
            if (value) {
                retProfiles.push_back(sp);
            }
        }
        return retProfiles;
    }

private:
    static SupportProfilesInfo *instance_;
    const std::vector<ProfileInfo> supportProfiles_ = {
        ProfileInfo(PROFILE_NAME_GATT_CLIENT, PROFILE_ID_GATT_CLIENT, BLUETOOTH_UUID_GATT),
        ProfileInfo(PROFILE_NAME_GATT_SERVER, PROFILE_ID_GATT_SERVER, BLUETOOTH_UUID_GATT),
        ProfileInfo(PROFILE_NAME_A2DP_SRC, PROFILE_ID_A2DP_SRC, BLUETOOTH_UUID_A2DP_SRC),
        ProfileInfo(PROFILE_NAME_A2DP_SINK, PROFILE_ID_A2DP_SINK, BLUETOOTH_UUID_A2DP_SINK),
        ProfileInfo(PROFILE_NAME_AVRCP_CT, PROFILE_ID_AVRCP_CT, BLUETOOTH_UUID_AVRCP_CT),
        ProfileInfo(PROFILE_NAME_AVRCP_TG, PROFILE_ID_AVRCP_TG, BLUETOOTH_UUID_AVRCP_TG),
        ProfileInfo(PROFILE_NAME_HFP_AG, PROFILE_ID_HFP_AG, BLUETOOTH_UUID_HFP_AG),
        ProfileInfo(PROFILE_NAME_HFP_HF, PROFILE_ID_HFP_HF, BLUETOOTH_UUID_HFP_HF),
        ProfileInfo(PROFILE_NAME_MAP_MCE, PROFILE_ID_MAP_MCE, ""),
        ProfileInfo(PROFILE_NAME_MAP_MSE, PROFILE_ID_MAP_MSE, ""),
        ProfileInfo(PROFILE_NAME_PBAP_PCE, PROFILE_ID_PBAP_PCE, BLUETOOTH_UUID_PBAP_PCE),
        ProfileInfo(PROFILE_NAME_PBAP_PSE, PROFILE_ID_PBAP_PSE, BLUETOOTH_UUID_PBAP_PSE),
        ProfileInfo(PROFILE_NAME_SPP, PROFILE_ID_SPP, BLUETOOTH_UUID_SPP),
    };
};
SupportProfilesInfo *SupportProfilesInfo::instance_ = nullptr;

#define GET_CONFIG_PROFILES(transport) SupportProfilesInfo::GetInstance()->GetConfigSupportProfiles(transport)
#define GET_SUPPORT_PROFILES SupportProfilesInfo::GetInstance()->GetSupportProfiles()

};  // namespace bluetooth

#endif  // PROFILE_CONFIG_H