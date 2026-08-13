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
#define LOG_TAG "bt_service_classic_config"
#endif

#include "classic_config.h"

#include <vector>
#include <map>
#include "classic_defs.h"
#include "log.h"
#include "parameter.h"
constexpr const char* DEVICE_CLASS = "const.product.devicetype";
const int32_t DEVICE_CLASS_SIZE = 16;
const int32_t SYSTEM_PARAMETER_ERROR_CODE = 0;
const static std::string DEVICE_CLASS_PAD_STR = "tablet";
const static std::string DEVICE_CLASS_COMPUTER_STR = "2in1";
const static std::string DEVICE_CLASS_PHONE_STR = "phone";

namespace OHOS {
namespace bluetooth {
ClassicConfig &ClassicConfig::GetInstance()
{
    static ClassicConfig instance;
    return instance;
}

ClassicConfig::ClassicConfig() : config_(AdapterDeviceConfig::GetInstance())
{}

ClassicConfig::~ClassicConfig()
{}

bool ClassicConfig::LoadConfigFile() const
{
    /// Load Device Config File.
    bool ret = config_->Load();
    if (!ret) {
        HILOGE("[ClassicConfig] failed!");
    }

    return ret;
}

bool ClassicConfig::Save() const
{
    bool ret = config_->Save();
    if (!ret) {
        HILOGE("[ClassicConfig] failed!");
    }

    return ret;
}

int ClassicConfig::GetLocalDeviceClass() const
{
    char deviceClassName[DEVICE_CLASS_SIZE] = {0};
    int errorCode = GetParameter(DEVICE_CLASS, NULL, deviceClassName, DEVICE_CLASS_SIZE - 1); // 1为了防止越界读非'\0'
    if (errorCode <= SYSTEM_PARAMETER_ERROR_CODE) {
        HILOGE("[ClassicConfig]:Failed to get device class name properties, errorCode=%{public}d", errorCode);
        return PHONE_CLASS_OF_DEVICE;
    }
    std::string devClassName(deviceClassName);
    std::map<std::string, int> table {
        {DEVICE_CLASS_PAD_STR, TABLET_CLASS_OF_DEVICE},
        {DEVICE_CLASS_COMPUTER_STR, COMPUTER_CLASS_OF_DEVICE},
        {DEVICE_CLASS_PHONE_STR, PHONE_CLASS_OF_DEVICE},
    };
    auto iter = table.find(devClassName);
    if (iter != table.end()) {
        return iter->second;
    } else {
        HILOGE("[ClassicConfig] default phone class");
        return PHONE_CLASS_OF_DEVICE;
    }
}

bool ClassicConfig::SetLocalDeviceClass(int cod) const
{
    if (!config_->SetValue(SECTION_HOST, PROPERTY_CLASS_OF_DEVICE, cod)) {
        HILOGW("[ClassicConfig] failed!");
        return false;
    }

    return true;
}

int ClassicConfig::GetIoCapability() const
{
    int io = 0;
    if (!config_->GetValue(SECTION_HOST, PROPERTY_IO_CAPABILITY, io)) {
        HILOGI("[ClassicConfig] failed!");
    }

    return io;
}

std::vector<std::string> ClassicConfig::GetPairedAddrList() const
{
    std::vector<std::string> pairedList;
    if (!config_->GetSubSections(SECTION_BREDR_PAIRED_LIST, pairedList)) {
        HILOGI("[ClassicConfig] failed!");
    }

    return pairedList;
}

bool ClassicConfig::RemovePairedDevice(const std::string &subSection) const
{
    bool result = true;
    if (!config_->RemoveSection(SECTION_BREDR_PAIRED_LIST, subSection)) {
        HILOGI("[ClassicConfig] failed!");
        result = false;
    }
    std::string lowerSection = LowerStr(subSection);
    if (!config_->RemoveSection(SECTION_BREDR_PAIRED_LIST, lowerSection)) {
        HILOGI("[ClassicConfig] lower section not exist or remove failed!");
    }
    return result;
}

bool ClassicConfig::SetRemoteRandomAddrNoSave(const std::string &subSection, const std::string &randomAddr) const
{
    if (GetRemoteRandomAddr(subSection) == randomAddr) {
        return true;
    }
    if (!config_->SetValue(SECTION_BREDR_PAIRED_LIST, subSection, PROPERTY_RANDOM_ADDR, randomAddr, false)) {
        HILOGW("SetRemoteRandomAddrNoSave [ClassicConfig] failed!");
        return false;
    }
    return true;
}

bool ClassicConfig::SetRemoteRandomAddr(const std::string &subSection, const std::string &randomAddr) const
{
    if (GetRemoteRandomAddr(subSection) == randomAddr) {
        return true;
    }
    if (!config_->SetValue(SECTION_BREDR_PAIRED_LIST, subSection, PROPERTY_RANDOM_ADDR, randomAddr)) {
        HILOGW("[ClassicConfig] failed!");
        return false;
    }
    return true;
}

std::string ClassicConfig::GetRemoteRandomAddr(const std::string &subSection) const
{
    std::string randomAddr = "";
    if (!config_->GetValue(SECTION_BREDR_PAIRED_LIST, subSection, PROPERTY_RANDOM_ADDR, randomAddr)) {
        HILOGW("[ClassicConfig] failed!");
        std::string lowerSection = LowerStr(subSection);
        if (!config_->GetValue(SECTION_BREDR_PAIRED_LIST, lowerSection, PROPERTY_RANDOM_ADDR, randomAddr)) {
            HILOGW("[ClassicConfig] failed!");
        }
    }
    return randomAddr;
}
}  // namespace bluetooth
}  // namespace OHOS
