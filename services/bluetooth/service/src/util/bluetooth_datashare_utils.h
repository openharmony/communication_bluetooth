/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_DATASHARE_UTILS_H
#define BLUETOOTH_DATASHARE_UTILS_H

#include <memory>
#include <utility>

#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "system_ability_definition.h"
#include "data_ability_observer_stub.h"
#include "uri.h"

namespace OHOS {
namespace bluetooth {

constexpr const char *SETTINGS_DATASHARE_URL_AIRPLANE_MODE =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=airplane_mode";
constexpr const char *SETTINGS_DATASHARE_KEY_AIRPLANE_MODE = "settings.telephony.airplanemode";

constexpr const char *SETTINGS_DATASHARE_URL_DEVICE_NAME =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_SECURE_";
constexpr const char *SETTINGS_DATASHARE_KEY_DEVICE_NAME = "settings.general.display_device_name";

constexpr const char *SETTINGS_DATASHARE_URI_COLLABORATION_SWITCH =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=";
constexpr const char *SETTINGS_DATASHARE_KEY_COLLABORATION_SWITCH =
    "settings.collaboration.multi_device_collaboration_service_switch";

constexpr const char *SETTINGS_DATASHARE_URI_DEFAULT_DEVICE_NAME =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=settings.general.device_name";
constexpr const char *SETTINGS_DATASHARE_KEY_DEFAULT_DEVICE_NAME = "settings.general.device_name";

class BluetoothObserver : public AAFwk::DataAbilityObserverStub {
public:
    explicit BluetoothObserver(const std::function<void(void)> &onChange)
        : onChange_(onChange) {}
    ~BluetoothObserver() override = default;
    void OnChange() override
    {
        if (onChange_) {
            onChange_();
        }
    }
private:
    std::function<void(void)> onChange_;
};

class BluetoothDataShareHelperUtils final {
public:
    BluetoothDataShareHelperUtils();
    ~BluetoothDataShareHelperUtils() = default;
    int32_t Query(Uri &uri, const std::string &key, std::string &value);
    int32_t Insert(Uri &uri, const std::string &key, const std::string &value);
    int32_t Update(Uri &uri, const std::string &key, const std::string &value);

    static bool CheckDataShareIsReady(void);
    bool RegisterObserver(Uri &uri, const sptr<BluetoothObserver> &dataObserver);
    bool UnregisterObserver(Uri &uri, const sptr<BluetoothObserver> &dataObserver);
private:
    static std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> CreateDataShareHelper();
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper_ = nullptr;
};

bool BluetoothDataShareQuery(const char *uriStr, const char *keyStr, std::string &outValue);
} // namespace bluetooth
} // namespace OHOS
#endif // BLUETOOTH_DATASHARE_UTILS_H