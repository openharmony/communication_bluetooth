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
 * @brief Defines classic config functions.
 *
 */

/**
 * @file classic_config.h
 *
 * @brief Classic config.
 *
 */

#ifndef CLASSIS_CONFIG_H
#define CLASSIS_CONFIG_H

#include <mutex>
#include <string>

#include "adapter_device_config.h"
#include "bt_def.h"
#include "string_ex.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief Represents classic config.
 *
 */
class ClassicConfig {
public:
    /**
     * @brief Get classic config singleton instance pointer.
     *
     * @return Returns the singleton instance pointer.
     */
    static ClassicConfig &GetInstance();

    /**
     * @brief Load config info.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool LoadConfigFile() const;

    /**
     * @brief Save config.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool Save() const;

    /**
     * @brief Get device Io capability.
     *
     * @return Returns device Io capability.
     */
    int GetIoCapability() const;

    /**
     * @brief Get local device class.
     *
     * @return Returns local device class.
     */
    int GetLocalDeviceClass() const;

    /**
     * @brief Set local device class.
     *
     * @param cod Device cod.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetLocalDeviceClass(int cod) const;

    /**
     * @brief Get paired device address list.
     *
     * @return Returns paired device address list.
     */
    std::vector<std::string> GetPairedAddrList() const;

    /**
     * @brief Remove paired device.
     *
     * @param subSection Device address.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool RemovePairedDevice(const std::string &subSection) const;

    /**
     * @brief Set remote device random address.
     *
     * @param subSection Device real address.
     * @param randomAddr Device random address.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetRemoteRandomAddr(const std::string &subSection, const std::string &randomAddr) const;

    /**
     * @brief Set remote device random address without save.
     *
     * @param subSection Device real address.
     * @param randomAddr Device random address.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    bool SetRemoteRandomAddrNoSave(const std::string &subSection, const std::string &randomAddr) const;

    /**
     * @brief Get remote device random address.
     *
     * @param subSection Device real address.
     * @return Returns remote device random address.
     */
    std::string GetRemoteRandomAddr(const std::string &subSection) const;
private:
    /**
     * @brief A constructor used to create a <b>ClassicConfig</b> instance.
     *
     */
    ClassicConfig();
    ClassicConfig(const ClassicConfig &config);
    ClassicConfig &operator=(const ClassicConfig &config);

    /**
     * @brief A destructor used to delete the <b>ClassicConfig</b> instance.
     *
     */
    ~ClassicConfig();

    IAdapterDeviceConfig *config_ {};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // CLASSIS_CONFIG_H
