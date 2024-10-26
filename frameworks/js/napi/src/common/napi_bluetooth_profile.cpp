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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_profile"
#endif

#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_utils.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
thread_local std::map<ProfileId, napi_ref> NapiProfile::profiles_;

void NapiProfile::DefineProfileFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getProfile", GetProfile),
        DECLARE_NAPI_FUNCTION("getProfileInst", GetProfile),
        DECLARE_NAPI_FUNCTION("getProfileInstance", GetProfile),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "profile:napi_define_properties");
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    ProfileEnumInit(env, exports);
}

void NapiProfile::SetProfile(napi_env env, ProfileId code, napi_value profile)
{
    napi_ref profileRef;
    if (napi_create_reference(env, profile, 1, &profileRef) != napi_ok) {
        HILOGE("napi create reference failed, profileId:%{public}d", code);
        return;
    }
    profiles_[code] = profileRef;
}

napi_value NapiProfile::GetProfile(napi_env env, napi_callback_info info)
{
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;
    napi_value ret = NapiGetNull(env);
    napi_status funcRet = napi_generic_failure;

    funcRet =  napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    NAPI_BT_RETURN_IF(funcRet != napi_ok, "call napi_get_cb_info failed.", ret);
    NAPI_BT_RETURN_IF(argc != expectedArgsCount, "Requires 1 argument", ret);

    int32_t profileId;
    NAPI_BT_RETURN_IF(!ParseInt32(env, profileId, argv[PARAM0]), "False type! Int32 required.", ret);

    napi_ref profileRef = profiles_[static_cast<ProfileId>(profileId)];
    napi_value profile = nullptr;
    funcRet = napi_get_reference_value(env, profileRef, &profile);
    NAPI_BT_RETURN_IF(funcRet != napi_ok, "call napi_get_reference_value failed", ret);
    NAPI_BT_RETURN_IF(profile == nullptr, "napi get reference failed.", ret);

    HILOGI("profileId:%{public}d", profileId);
    return profile;
}

void NapiProfile::ProfileEnumInit(napi_env env, napi_value exports)
{
    HILOGD("enter");
    napi_value sppTypeObj = SppTypeInit(env);
    napi_value playingStateObj = PlayingStateInit(env);
    napi_value profileIdObj = ProfileIdInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("SppType", sppTypeObj),
        DECLARE_NAPI_PROPERTY("PlayingState", playingStateObj),
        DECLARE_NAPI_PROPERTY("ProfileId", profileIdObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "profile:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
}

napi_value NapiProfile::SppTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value sppType = nullptr;
    napi_create_object(env, &sppType);
    SetNamedPropertyByInteger(env, sppType, SppType::SPP_RFCOMM, "SPP_RFCOMM");
    return sppType;
}

napi_value NapiProfile::PlayingStateInit(napi_env env)
{
    HILOGD("enter");
    napi_value playingState = nullptr;
    napi_create_object(env, &playingState);
    SetNamedPropertyByInteger(env, playingState, PlayingState::STATE_NOT_PLAYING, "STATE_NOT_PLAYING");
    SetNamedPropertyByInteger(env, playingState, PlayingState::STATE_PLAYING, "STATE_PLAYING");
    return playingState;
}

napi_value NapiProfile::ProfileIdInit(napi_env env)
{
    HILOGD("enter");
    napi_value profileId = nullptr;
    napi_create_object(env, &profileId);
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_A2DP_SINK, "PROFILE_A2DP_SINK");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_A2DP_SOURCE, "PROFILE_A2DP_SOURCE");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_AVRCP_CT, "PROFILE_AVRCP_CT");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_AVRCP_TG, "PROFILE_AVRCP_TG");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_HANDS_FREE_AUDIO_GATEWAY,
        "PROFILE_HANDS_FREE_AUDIO_GATEWAY");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_HANDS_FREE_UNIT, "PROFILE_HANDS_FREE_UNIT");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_HID_HOST, "PROFILE_HID_HOST");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_PAN_NETWORK, "PROFILE_PAN_NETWORK");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_PBAP_CLIENT, "PROFILE_PBAP_CLIENT");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_PBAP_SERVER, "PROFILE_PBAP_SERVER");
    SetNamedPropertyByInteger(env, profileId, ProfileId::PROFILE_OPP, "PROFILE_OPP");
    return profileId;
}
} // namespace Bluetooth
} // namespace OHOS
