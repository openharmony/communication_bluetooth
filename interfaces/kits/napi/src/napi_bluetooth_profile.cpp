/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
std::map<ProfileCode, napi_value> NapiProfile::profiles_;

void DefineProfileFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {  
        DECLARE_NAPI_FUNCTION("getProfile", NapiProfile::GetProfile),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);

}

void NapiProfile::SetProfile(ProfileCode code, napi_value profile)
{
    profiles_[code] = profile;
}

napi_value NapiProfile::GetProfile(napi_env env, napi_callback_info info)
{
    HILOGI("GetProfile called");
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;
    bool isOK = false;
    napi_value ret = nullptr;
    napi_get_boolean(env, isOK, &ret);

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != expectedArgsCount) {
        HILOGE("Requires 1 argument.");
        return ret;
    }
    int profile_code;
    if (!ParseInt32(env, profile_code, argv[PARAM0])) {
        HILOGE("False type! Int32 required.");
        return ret;
    }

    napi_value profile = profiles_[static_cast<ProfileCode>(profile_code)];
    if (!profile) {
        return ret;
    }
    return profile;
}

} // namespace Bluetooth
} // namespace OHOS
