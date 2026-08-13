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

#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <map>
#include "raw_address.h"
#include "common_util.h"
#include "preferences.h"
#include "preferences_errno.h"
#include "preferences_helper.h"
#include "log.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::NativePreferences;

enum PreferencesManagerType {
    ABS_VOLUME_SWITCH = 0,
    ABS_VOLUME = 1,
    NON_ABS_VOLUME = 2,
    HFP_VOLUME = 3,
    CALL_LOG = 4,
    VIRTUAL_AUTO_CONN_SWITCH = 5,
    SATELLITE_CONTROL = 6,
    CAR_KEY_DFX_DATA = 7,
    CAR_KEY_CARD = 8,
    CONNECT_ALL_PROFILE_APP_TYPE = 9,
    WATCH_SAIS_DEVICE = 10,
};

/**
 * @brief This class implements the preferences manager.
 */
class PreferencesManager {
public:
    PreferencesManager() = default;
    ~PreferencesManager() = default;

    static void Delete(const std::string& key, PreferencesManagerType preferencesType);

    template <typename T>
    static void Save(const std::string& key, const T& value, PreferencesManagerType preferencesType)
    {
        HITRACE_METER(BT_TRACE_TAG);
        auto preferences = GetPreferences(preferencesType);
        if (!preferences) {
            return;
        }
        int32_t errCode = NativePreferences::E_ERROR;
        if (std::is_same<T, bool>::value) {
            errCode = preferences->PutBool(key, value);
            HILOGD("%{public}s, preferencesType: %{public}d, value: %{public}d", GET_ENCRYPT_STR_ADDR(key),
                preferencesType, value);
        } else if (std::is_same<T, int>::value) {
            errCode = preferences->PutInt(key, value);
            HILOGD("%{public}s, preferencesType: %{public}d, value: %{public}d", GET_ENCRYPT_STR_ADDR(key),
                preferencesType, value);
        } else if (std::is_same<T, int64_t>::value) {
            errCode = preferences->PutLong(key, value);
            HILOGD("%{public}s, preferencesType: %{public}d, value: %{public}ld", GET_ENCRYPT_STR_ADDR(key),
                preferencesType, value);
        } else {
            HILOGE("invalid value type, preferencesType: %{public}d", preferencesType);
            return;
        }
        if (errCode != NativePreferences::E_OK) {
            HILOGE("save error, preferencesType: %{public}d, code=%{public}d", preferencesType, errCode);
            return;
        }
        preferences->Flush();
    }

    static void SaveString(const std::string& key, const std::string& value, PreferencesManagerType preferencesType)
    {
        HITRACE_METER(BT_TRACE_TAG);
        auto preferences = GetPreferences(preferencesType);
        if (!preferences) {
            return;
        }
        int32_t errCode = preferences->PutString(key, value);
        HILOGD("%{public}s, preferencesType: %{public}d, value: %{public}s", GET_ENCRYPT_STR_ADDR(key),
               preferencesType, value.c_str());
        if (errCode != NativePreferences::E_OK) {
            HILOGE("save error, preferencesType: %{public}d, code=%{public}d", preferencesType, errCode);
            return;
        }
        preferences->Flush();
    }

    static std::string GetString(const std::string& key, std::string defaultValue,
        PreferencesManagerType preferencesType)
    {
        HITRACE_METER(BT_TRACE_TAG);
        std::string result = defaultValue;
        auto preferences = GetPreferences(preferencesType);
        if (!preferences) {
            return result;
        }
        result = preferences->GetString(key, defaultValue);
        HILOGD("%{public}s, preferencesType: %{public}d, result: %{public}s", GET_ENCRYPT_STR_ADDR(key),
                preferencesType, result.c_str());
        return result;
    }

    template <typename T>
    static T Get(const std::string& key, T defaultValue, PreferencesManagerType preferencesType)
    {
        HITRACE_METER(BT_TRACE_TAG);
        T result = defaultValue;
        auto preferences = GetPreferences(preferencesType);
        if (!preferences) {
            return result;
        }
        if (std::is_same<T, bool>::value) {
            result = preferences->GetBool(key, defaultValue);
            HILOGD("%{public}s, preferencesType: %{public}d, result: %{public}d", GET_ENCRYPT_STR_ADDR(key),
                preferencesType, result);
        } else if (std::is_same<T, int>::value) {
            result = preferences->GetInt(key, defaultValue);
            HILOGD("%{public}s, preferencesType: %{public}d, result: %{public}d", GET_ENCRYPT_STR_ADDR(key),
                preferencesType, result);
        } else if (std::is_same<T, int64_t>::value) {
            result = preferences->GetLong(key, defaultValue);
        } else {
            HILOGE("invalid value type, preferencesType: %{public}d", preferencesType);
        }
        return result;
    }

    static std::map<std::string, PreferencesValue> GetAll(PreferencesManagerType preferencesType)
    {
        HITRACE_METER(BT_TRACE_TAG);
        auto preferences = GetPreferences(preferencesType);
        if (!preferences) {
            return {};
        }
        return preferences->GetAll();
    }

private:
    static std::shared_ptr<Preferences> GetPreferences(PreferencesManagerType preferencesType);

private:
    static std::map<PreferencesManagerType, std::string> preferencesMgrPathMap_;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
