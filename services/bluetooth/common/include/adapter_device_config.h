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

#ifndef ADAPTER_DEVICE_CONFIG_H
#define ADAPTER_DEVICE_CONFIG_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "base/base_def.h"

/*
 * @brief The Bluetooth subsystem.
 */
namespace OHOS {
namespace bluetooth {
constexpr const char *SECTION_HOST = "Host";
constexpr const char *PROPERTY_CLASS_OF_DEVICE = "ClassOfDevice";
constexpr const char *PROPERTY_IO_CAPABILITY = "IOCapability";
constexpr const char *PROPERTY_RANDOM_ADDR = "RandomAddr";

constexpr const char *SECTION_BREDR_PAIRED_LIST = "Classic Paired Device List";
constexpr const char *PROPERTY_PBAP_PSE_SHARETYPE = "PbapPseShareType";
constexpr const char *PROPERTY_PBAP_PSE_PERMISSION = "PbapPsePermission";
constexpr const char *PROPERTY_PBAP_PSE_REJECTCOUNT = "PbapPseRejectCount";
constexpr const char *PROPERTY_MAP_PERMISSION = "MapPermission";
constexpr const char *AUTO_PLAY_VALUE = "AutoPlayValue";
constexpr const char *RESTRICTED_DURATION = "RestrictedDuration";
constexpr const char *PROPERTY_HFP_AG_VGS_SUPPORT = "HfpAgVgsSupport";
constexpr const int32_t ACCESS_UNKNOWN = 0;
constexpr const int32_t ACCESS_ALLOWED = 1;
enum class BTShareType : int32_t {
    SHARE_NAME_AND_PHONE_NUMBER = 0,
    SHARE_ALL = 1,
    SHARE_NOTHING = 2,
};

/**
 * @brief BREDR/BLE config.
 */
class IAdapterDeviceConfig {
public:
    virtual ~IAdapterDeviceConfig() = default;
    /**
     * @brief Load XML Document from specified path.
     * @return true Success Load XML Document.
     * @return false Failed Load XML Document.
     */
    virtual bool Load() = 0;

    /**
     * @brief Reload XML Document from specified path.
     * @return true Success reload XML Document.
     * @return false Failed reload XML Document.
     */
    virtual bool Reload() = 0;

    /**
     * @brief Load XML Document from specified path.
     * @param[in] path XML Document path.
     * @return true Success Load XML Document.
     * @return false Failed Load XML Document.
     */
    virtual bool Save() = 0;

    /**
     * @brief Get specified property value.
     *        Value type is int.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[out] value Value type is int.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(
        const std::string &section, const std::string &subSection, const std::string &property, int &value) = 0;

    /**
     * @brief Get specified property value.
     *        Value type is string.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[out] value Value type is string.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(
        const std::string &section, const std::string &subSection, const std::string &property, std::string &value) = 0;

    /**
     * @brief Get specified property value.
     *        Value type is bool.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[out] value Value type is bool.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(
        const std::string &section, const std::string &subSection, const std::string &property, bool &value) = 0;

    /**
     * @brief Set specified property value.
     *        Value type is int.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[in] value Value type is const int.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &subSection, const std::string &property,
        const int &value, bool isAutoSave = true) = 0;

    /**
     * @brief Set specified property value.
     *        Value type is string.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[in] value Value type is const string.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &subSection, const std::string &property,
        const std::string &value, bool isAutoSave = true) = 0;

    /**
     * @brief Set specified property value.
     *        Value type is bool.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[in] value Value type is const bool.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &subSection, const std::string &property,
        const bool &value, bool isAutoSave = true) = 0;

    /**
     * @brief Get Address
     * @param[in] section
     * @param[out] subSections
     * @return true Specified section has one or Multiple subSections.
     * @return false Specified section do not has any subSection.
     */
    virtual bool GetSubSections(const std::string &section, std::vector<std::string> &subSections) = 0;

    /**
     * @brief Remove XML document specified section.
     * @param[in] section
     * @param[in] subSection
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success remove XML document specified section.
     * @return false Failed remove XML document specified section.
     */
    virtual bool RemoveSection(const std::string &section, const std::string &subSection, bool isAutoSave = true) = 0;

    /**
     * @brief Get specified property value.
     * @param[in] section
     * @param[in] property
     * @param[out] value Int type value.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(const std::string &section, const std::string &property, int &value) = 0;

    /**
     * @brief Get specified property value.
     * @param[in] section
     * @param[in] property
     * @param[out] value String type value.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(const std::string &section, const std::string &property, std::string &value) = 0;

    /**
     * @brief Get specified property value.
     * @param[in] section
     * @param[in] property
     * @param[out] value Bool type value.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(const std::string &section, const std::string &property, bool &value) = 0;

    /**
     * @brief Set specified property value.
     *        Value type is int.
     * @param[in] section
     * @param[in] property
     * @param[in] value Value type is const int.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &property, const int &value,
        bool isAutoSave = true) = 0;

    /**
     * @brief Set specified property value.
     *        Value type is string.
     * @param[in] section
     * @param[in] property
     * @param[in] value Value type is const string.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &property, const std::string &value,
        bool isAutoSave = true) = 0;
};

class AdapterDeviceConfig : public IAdapterDeviceConfig {
public:
    /**
     * @brief Get the Instance object
     * @return IAdapterConfig*
     */
    static IAdapterDeviceConfig *GetInstance();

    /**
     * @brief Load XML Document from specified path.
     * @return true Success Load XML Document.
     * @return false Failed Load XML Document.
     */
    virtual bool Load() override;

    /**
     * @brief Reload XML Document from specified path.
     * @return true Success reload XML Document.
     * @return false Failed reload XML Document.
     */
    virtual bool Reload() override;

    /**
     * @brief Load XML Document from specified path.
     * @param[in] path XML Document path.
     * @return true Success Load XML Document.
     * @return false Failed Load XML Document.
     */
    virtual bool Save() override;

    /**
     * @brief Get specified property value.
     *        Value type is int.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[out] value Value type is int.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(
        const std::string &section, const std::string &subSection, const std::string &property, int &value) override;

    /**
     * @brief Get specified property value.
     *        Value type is string.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[out] value Value type is string.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(const std::string &section, const std::string &subSection, const std::string &property,
        std::string &value) override;

    /**
     * @brief Get specified property value.
     *        Value type is bool.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[out] value Value type is bool.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(
        const std::string &section, const std::string &subSection, const std::string &property, bool &value) override;

    /**
     * @brief Set specified property value.
     *        Value type is int.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[in] value Value type is const int.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &subSection, const std::string &property,
        const int &value, bool isAutoSave = true) override;

    /**
     * @brief Set specified property value.
     *        Value type is string.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[in] value Value type is const string.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &subSection, const std::string &property,
        const std::string &value, bool isAutoSave = true) override;

    /**
     * @brief Set specified property value.
     *        Value type is bool.
     * @param[in] section
     * @param[in] subSection
     * @param[in] property
     * @param[in] value Value type is const bool.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &subSection, const std::string &property,
        const bool &value, bool isAutoSave = true) override;

    /**
     * @brief Get Address
     * @param[in] section
     * @param[out] subSections
     * @return true Specified section has one or multiple subSections.
     * @return false Specified section do not has any subSection.
     */
    virtual bool GetSubSections(const std::string &section, std::vector<std::string> &subSections) override;

    /**
     * @brief Remove XML document specified section.
     * @param[in] section
     * @param[in] subSection
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success remove XML document specified section.
     * @return false Failed remove XML document specified section.
     */
    virtual bool RemoveSection(const std::string &section, const std::string &subSection,
        bool isAutoSave = true) override;

    /**
     * @brief Get specified property value.
     * @param[in] section
     * @param[in] property
     * @param[out] value Int type value.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(const std::string &section, const std::string &property, int &value) override;

    /**
     * @brief Get specified property value.
     * @param[in] section
     * @param[in] property
     * @param[out] value String type value.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(const std::string &section, const std::string &property, std::string &value) override;

    /**
     * @brief Get specified property value.
     * @param[in] section
     * @param[in] property
     * @param[out] value Bool type value.
     * @return true Success get specified property's value.
     * @return false Failed get specified property's value.
     */
    virtual bool GetValue(const std::string &section, const std::string &property, bool &value) override;

    /**
     * @brief Set specified property value.
     *        Value type is int.
     * @param[in] section
     * @param[in] property
     * @param[in] value Value type is const int.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &property, const int &value,
        bool isAutoSave = true) override;

    /**
     * @brief Set specified property value.
     *        Value type is string.
     * @param[in] section
     * @param[in] property
     * @param[in] value Value type is const string.
     * @param[in] isAutoSave indicate whether to save after Set specified property value, default is true.
     * @return true Success set specified property's value.
     * @return false Failed set specified property's value.
     */
    virtual bool SetValue(const std::string &section, const std::string &property, const std::string &value,
        bool isAutoSave = true) override;

private:
    /**
     * @brief Construct a new Adapter Config object
     */
    AdapterDeviceConfig();

    /**
     * @brief Destroy the Adapter Config object
     */
    ~AdapterDeviceConfig();

    std::mutex mutex_ {};
    static AdapterDeviceConfig *g_instance;
    DECLARE_IMPL();
};

/******************************************* Interface *****************************************************/
bool HasPbapOrMapPermission();
bool SetPbapShareType(const std::string &address, int32_t shareType);
bool GetPbapShareType(const std::string &address, int32_t &shareType);
bool SetPbapPermission(const std::string &address, int32_t permission);
bool GetPbapPermission(const std::string &address, int32_t &permission);
bool SetPbapRejectCount(const std::string &address, int32_t rejectCount);
bool GetPbapRejectCount(const std::string &address, int32_t &rejectCount);
bool SetMessagePermission(const std::string &address, int32_t permission);
bool GetMessagePermission(const std::string &address, int32_t &permission);
bool SaveAutoPlayValue(const std::string &address, int autoPlayValue);
bool GetAutoPlayValue(const std::string &address, int &autoPlayValue);
bool SetRestrictedDuration(const std::string &address, int duration);
bool GetRestrictedDuration(const std::string &address, int &duration);
bool SetHfpAgVgsSupport(const std::string &address, bool vgsSupport);
bool GetHfpAgVgsSupport(const std::string &address, bool &vgsSupport);

}  // namespace bluetooth
}  // namespace OHOS

#endif // ADAPTER_DEVICE_CONFIG_H
