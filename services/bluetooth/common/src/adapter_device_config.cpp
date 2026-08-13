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
#ifndef LOG_TAG
#define LOG_TAG "bt_common_adapter_device_config"
#endif

#include "adapter_device_config.h"

#include <fstream>

#include "common_util.h"
#include "log.h"
#include "xml_parse.h"

namespace OHOS {
namespace bluetooth {
AdapterDeviceConfig *AdapterDeviceConfig::g_instance = nullptr;

struct AdapterDeviceConfig::impl {
    utility::XmlParse parse_ {};
    std::string fileName_ {"bt_device_config.xml"};
    std::string filePath_ {BT_CONFIG_PATH + fileName_};
    std::string fileBasePath_ {BT_CONFIG_PATH_BASE + fileName_};
};

IAdapterDeviceConfig *AdapterDeviceConfig::GetInstance()
{
    if (g_instance == nullptr) {
        static AdapterDeviceConfig instance;
        g_instance = &instance;
    }

    return static_cast<IAdapterDeviceConfig *>(g_instance);
}

AdapterDeviceConfig::AdapterDeviceConfig() : pimpl(std::make_unique<impl>()){};

AdapterDeviceConfig::~AdapterDeviceConfig()
{}

bool AdapterDeviceConfig::Load()
{
    std::lock_guard<std::mutex> lg(mutex_);
    if (pimpl->parse_.Load(pimpl->filePath_)) {
        return true;
    } else {
        if (!Reload()) {
            return false;
        }
        return pimpl->parse_.Load(pimpl->filePath_);
    }
}

bool AdapterDeviceConfig::Reload()
{
    std::ifstream fin(pimpl->fileBasePath_, std::ios::in | std::ios::binary);
    if (!fin) {
        return false;
    }
    std::ofstream fout(pimpl->filePath_, std::ios::out | std::ios::trunc);
    if (!fout) {
        return false;
    }
    fout << fin.rdbuf();
    return true;
}

bool AdapterDeviceConfig::Save()
{
    std::lock_guard<std::mutex> lg(mutex_);
    return pimpl->parse_.Save();
}

bool AdapterDeviceConfig::SetValue(const std::string &section, const std::string &property, const int &value,
    bool isAutoSave)
{
    std::lock_guard<std::mutex> lg(mutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->parse_.SetValue(section, property, value), false,
        "SetValue fail property: %{public}s", property.c_str());
    if (isAutoSave) {
        return pimpl->parse_.Save();
    }
    return true;
}

bool AdapterDeviceConfig::SetValue(const std::string &section, const std::string &property, const std::string &value,
    bool isAutoSave)
{
    std::lock_guard<std::mutex> lg(mutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->parse_.SetValue(section, property, value), false,
        "SetValue fail property: %{public}s", property.c_str());
    if (isAutoSave) {
        return pimpl->parse_.Save();
    }
    return true;
}

bool AdapterDeviceConfig::GetValue(const std::string &section, const std::string &property, int &value)
{
    std::lock_guard<std::mutex> lg(mutex_);
    return pimpl->parse_.GetValue(section, property, value);
}

bool AdapterDeviceConfig::GetValue(const std::string &section, const std::string &property, std::string &value)
{
    std::lock_guard<std::mutex> lg(mutex_);

    return pimpl->parse_.GetValue(section, property, value);
}

bool AdapterDeviceConfig::GetValue(const std::string &section, const std::string &property, bool &value)
{
    std::lock_guard<std::mutex> lg(mutex_);
    return pimpl->parse_.GetValue(section, property, value);
}

bool AdapterDeviceConfig::SetValue(const std::string &section, const std::string &subSection,
    const std::string &property, const int &value, bool isAutoSave)
{
    std::lock_guard<std::mutex> lg(mutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->parse_.SetValue(section, subSection, property, value), false,
        "SetValue fail property: %{public}s", property.c_str());
    if (isAutoSave) {
        return pimpl->parse_.Save();
    }
    return true;
}
bool AdapterDeviceConfig::SetValue(const std::string &section, const std::string &subSection,
    const std::string &property, const std::string &value, bool isAutoSave)
{
    std::lock_guard<std::mutex> lg(mutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->parse_.SetValue(section, subSection, property, value), false,
        "SetValue fail property: %{public}s", property.c_str());
    if (isAutoSave) {
        return pimpl->parse_.Save();
    }
    return true;
}

bool AdapterDeviceConfig::SetValue(const std::string &section, const std::string &subSection,
    const std::string &property, const bool &value, bool isAutoSave)
{
    std::lock_guard<std::mutex> lg(mutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->parse_.SetValue(section, subSection, property, value), false,
        "SetValue fail property: %{public}s", property.c_str());
    if (isAutoSave) {
        return pimpl->parse_.Save();
    }
    return true;
}

bool AdapterDeviceConfig::GetValue(
    const std::string &section, const std::string &subSection, const std::string &property, int &value)
{
    std::lock_guard<std::mutex> lg(mutex_);
    return pimpl->parse_.GetValue(section, subSection, property, value);
}

bool AdapterDeviceConfig::GetValue(
    const std::string &section, const std::string &subSection, const std::string &property, std::string &value)
{
    std::lock_guard<std::mutex> lg(mutex_);
    return pimpl->parse_.GetValue(section, subSection, property, value);
}

bool AdapterDeviceConfig::GetValue(
    const std::string &section, const std::string &subSection, const std::string &property, bool &value)
{
    std::lock_guard<std::mutex> lg(mutex_);
    return pimpl->parse_.GetValue(section, subSection, property, value);
}

bool AdapterDeviceConfig::GetSubSections(const std::string &section, std::vector<std::string> &subSections)
{
    std::lock_guard<std::mutex> lg(mutex_);
    return pimpl->parse_.GetSubSections(section, subSections);
}

bool AdapterDeviceConfig::RemoveSection(const std::string &section, const std::string &subSection, bool isAutoSave)
{
    std::lock_guard<std::mutex> lg(mutex_);
    CHECK_AND_RETURN_LOG_RET(pimpl->parse_.RemoveSection(section, subSection), false,
        "RemoveSection fail");
    if (isAutoSave) {
        return pimpl->parse_.Save();
    }
    return true;
}

/******************************************* Interface *****************************************************/
bool HasPbapOrMapPermission()
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");
    std::vector<std::string> pairedList;
    if (!config->GetSubSections(SECTION_BREDR_PAIRED_LIST, pairedList)) {
        HILOGI("[ClassicConfig] failed!");
        return false;
    }
    for (std::string address : pairedList) {
        int32_t permission = ACCESS_UNKNOWN;
        if (config->GetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_PBAP_PSE_PERMISSION, permission) &&
            permission == ACCESS_ALLOWED) {
            return true;
        }
        if (config->GetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_MAP_PERMISSION, permission) &&
            permission == ACCESS_ALLOWED) {
            return true;
        }
    }
    return false;
}

bool SetPbapShareType(const std::string &address, int32_t shareType)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->SetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_PBAP_PSE_SHARETYPE, shareType)) {
        HILOGE("SetPbapShareType failed!");
        return false;
    }
    return true;
}

bool GetPbapShareType(const std::string &address, int32_t &shareType)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->GetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_PBAP_PSE_SHARETYPE, shareType)) {
        HILOGI("GetPbapShareType failed!");
        return false;
    }
    if (shareType < static_cast<int32_t>(BTShareType::SHARE_NAME_AND_PHONE_NUMBER) ||
        shareType > static_cast<int32_t>(BTShareType::SHARE_NOTHING)) {
        shareType = static_cast<int32_t>(BTShareType::SHARE_NAME_AND_PHONE_NUMBER);
    }
    return true;
}

bool SetPbapPermission(const std::string &address, int32_t permission)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->SetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_PBAP_PSE_PERMISSION, permission)) {
        HILOGE("SetPbapPermission failed!");
        return false;
    }
    return true;
}

bool GetPbapPermission(const std::string &address, int32_t &permission)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->GetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_PBAP_PSE_PERMISSION, permission)) {
        HILOGI("GetPbapPermission failed!");
        return false;
    }
    return true;
}

bool SetPbapRejectCount(const std::string &address, int32_t rejectCount)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->SetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_PBAP_PSE_REJECTCOUNT, rejectCount)) {
        HILOGE("SetPbapRejectCount failed!");
        return false;
    }
    return true;
}

bool GetPbapRejectCount(const std::string &address, int32_t &rejectCount)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->GetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_PBAP_PSE_REJECTCOUNT, rejectCount)) {
        HILOGI("GetPbapRejectCount failed!");
        return false;
    }
    return true;
}

bool SetMessagePermission(const std::string &address, int32_t permission)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->SetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_MAP_PERMISSION, permission)) {
        HILOGE("SetMessagePermission failed!");
        return false;
    }
    return true;
}

bool GetMessagePermission(const std::string &address, int32_t &permission)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->GetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_MAP_PERMISSION, permission)) {
        HILOGI("GetMessagePermission failed!");
        return false;
    }
    return true;
}

bool SaveAutoPlayValue(const std::string &address, int autoPlayValue)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->SetValue(SECTION_BREDR_PAIRED_LIST, address, AUTO_PLAY_VALUE, autoPlayValue)) {
        HILOGI("SaveAutoPlayValue failed!");
        return false;
    }
    return true;
}

bool GetAutoPlayValue(const std::string &address, int &autoPlayValue)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->GetValue(SECTION_BREDR_PAIRED_LIST, address, AUTO_PLAY_VALUE, autoPlayValue)) {
        HILOGI("GetAutoPlayValue failed!");
        return false;
    }
    return true;
}

bool SetRestrictedDuration(const std::string &address, int duration)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->SetValue(SECTION_BREDR_PAIRED_LIST, address, RESTRICTED_DURATION, duration)) {
        HILOGI("SetRestrictedDuration failed!");
        return false;
    }
    return true;
}

bool GetRestrictedDuration(const std::string &address, int &duration)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->GetValue(SECTION_BREDR_PAIRED_LIST, address, RESTRICTED_DURATION, duration)) {
        HILOGI("GetRestrictedDuration failed!");
        return false;
    }
    return true;
}

bool SetHfpAgVgsSupport(const std::string &address, bool vgsSupport)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->SetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_HFP_AG_VGS_SUPPORT, vgsSupport, true)) {
        HILOGE("SetValue failed!");
        return false;
    }
    return true;
}

bool GetHfpAgVgsSupport(const std::string &address, bool &vgsSupport)
{
    IAdapterDeviceConfig *config = AdapterDeviceConfig::GetInstance();
    CHECK_AND_RETURN_LOG_RET(config, false, "config is nullptr");

    if (!config->GetValue(SECTION_BREDR_PAIRED_LIST, address, PROPERTY_HFP_AG_VGS_SUPPORT, vgsSupport)) {
        HILOGI("GetValue failed!");
        return false;
    }
    return true;
}
}  // namespace bluetooth
}  // namespace OHOS