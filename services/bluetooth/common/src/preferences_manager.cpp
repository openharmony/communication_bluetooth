/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_service_preferences"
#endif

#include "preferences_manager.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::NativePreferences;

std::map<PreferencesManagerType, std::string> PreferencesManager::preferencesMgrPathMap_ = {
    { PreferencesManagerType::ABS_VOLUME_SWITCH, "/data/service/el1/public/bluetooth/bt_abs_support.xml" },
    { PreferencesManagerType::ABS_VOLUME, "/data/service/el1/public/bluetooth/bt_abs_volume.xml" },
    { PreferencesManagerType::NON_ABS_VOLUME, "/data/service/el1/public/bluetooth/bt_non_abs_volume.xml" },
    { PreferencesManagerType::HFP_VOLUME, "/data/service/el1/public/bluetooth/bt_hfp_volume.xml" },
    { PreferencesManagerType::CALL_LOG, "/data/service/el1/public/bluetooth/bt_hfp_call_log.xml" },
    { PreferencesManagerType::VIRTUAL_AUTO_CONN_SWITCH,
      "/data/service/el1/public/bluetooth/bt_virtual_auto_conn_switch.xml" },
    { PreferencesManagerType::SATELLITE_CONTROL, "/data/service/el1/public/bluetooth/bt_satellite_control.xml"},
    { PreferencesManagerType::CAR_KEY_DFX_DATA, "/data/service/el1/public/bluetooth/car_key_dfx.xml"},
    { PreferencesManagerType::CAR_KEY_CARD, "/data/service/el1/public/bluetooth/car_key_card.xml"},
    { PreferencesManagerType::CONNECT_ALL_PROFILE_APP_TYPE,
        "/data/service/el1/public/bluetooth/connect_all_profile_app_type.xml"},
    { PreferencesManagerType::WATCH_SAIS_DEVICE, "/data/service/el1/public/bluetooth/watch_sais_device.xml"},
};

std::shared_ptr<Preferences> PreferencesManager::GetPreferences(PreferencesManagerType preferencesType)
{
    auto it = preferencesMgrPathMap_.find(preferencesType);
    if (it == preferencesMgrPathMap_.end()) {
        HILOGE("not found path, preferencesType: %{public}d", preferencesType);
        return nullptr;
    }
    int32_t errCode = E_ERROR;
    std::shared_ptr<Preferences> preferences = PreferencesHelper::GetPreferences(it->second, errCode);
    if (preferences == nullptr || errCode != E_OK) {
        HILOGE("get preferences error, preferencesType: %{public}d, code=%{public}d", preferencesType, errCode);
        return nullptr;
    }
    return preferences;
}

void PreferencesManager::Delete(const std::string& key, PreferencesManagerType preferencesType)
{
    HITRACE_METER(BT_TRACE_TAG);
    auto preferences = GetPreferences(preferencesType);
    if (!preferences) {
        return;
    }
    preferences->Delete(key);
    preferences->Flush();
}
}  // namespace bluetooth
}  // namespace OHOS
