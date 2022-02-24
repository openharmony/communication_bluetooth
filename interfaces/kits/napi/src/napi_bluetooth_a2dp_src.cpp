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
#include "bluetooth_a2dp_src.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_a2dp_src.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

NapiA2dpSourceObserver NapiA2dpSource::observer_;

void NapiA2dpSource::DefineA2dpSourceJSClass(napi_env env) {
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),       
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getConnectionDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", GetDeviceState),
        DECLARE_NAPI_FUNCTION("getPlayingState", GetPalyingState),
    };

    napi_define_class(env, "A2dpSource", NAPI_AUTO_LENGTH, A2dpSourceConstructor, nullptr, 
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    
    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(ProfileCode::CODE_BT_PROFILE_A2DP_SOURCE, napiProfile);
    A2dpSource *profile = A2dpSource::GetProfile();
    profile->RegisterObserver(&observer_);
    HILOGI("DefineA2dpSourceJSClass finished");
}

napi_value NapiA2dpSource::A2dpSourceConstructor(napi_env env, napi_callback_info info) {
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiA2dpSource::On(napi_env env, napi_callback_info info) {
    HILOGI("On called");
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != expectedArgsCount) {
        HILOGE("Requires 2 argument.");
        return ret;
    } 
    string type;
    if (!ParseString(env, type, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = std::make_shared<BluetoothCallbackInfo>();
    callbackInfo->env_ = env;

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM1], &valueType);
    if (valueType != napi_function) {
        HILOGE("Wrong argument type. Function expected.");
        return ret;
    }
    napi_create_reference(env, argv[PARAM1], 1, &callbackInfo->callback_);
    observer_.callbackInfos_[type] = callbackInfo;

    HILOGI("%{public}s is registered", type.c_str());
    return ret;
}

napi_value NapiA2dpSource::Off(napi_env env, napi_callback_info info) {
    HILOGI("Off called");
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != expectedArgsCount) {
        HILOGE("Requires 1 argument.");
        return ret;
    }
    string type;
    if (!ParseString(env, type, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }
    observer_.callbackInfos_[type] = nullptr;

    HILOGI("%{public}s is unregistered", type.c_str());

    return ret;
}

napi_value NapiA2dpSource::GetConnectionDevices(napi_env env, napi_callback_info info) {
    HILOGI("GetConnectionDevices called");
    napi_value ret = nullptr;
    napi_create_array(env, &ret);
    A2dpSource *profile = A2dpSource::GetProfile();
    vector<int> states;
    states.push_back(1);
    vector<BluetoothRemoteDevice> devices = profile->GetDevicesByStates(states);
    vector<string> deviceVector;
    for (auto &device: devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }
    ConvertStringVectorToJS(env, ret, deviceVector);
    return ret;
}

napi_value NapiA2dpSource::GetDeviceState(napi_env env, napi_callback_info info) {
    HILOGI("GetDeviceState called");

    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != expectedArgsCount) {
        HILOGE("Requires 1 argument.");
        return ret;
    }
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    A2dpSource *profile = A2dpSource::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    int state = profile->GetDeviceState(device);
    napi_value result = nullptr;
    napi_create_int32(env, state, &result);
    return result;
}

napi_value NapiA2dpSource::GetPalyingState(napi_env env, napi_callback_info info) {
    HILOGI("GetPalyingState called");

    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    napi_value thisVar = nullptr;

    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);

    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    if (argc != expectedArgsCount) {
        HILOGE("Requires 1 argument.");
        return ret;
    }
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    A2dpSource *profile = A2dpSource::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    int res = profile->GetPlayingState(device);

    napi_value result = nullptr;
    napi_create_int32(env, res, &result);
    return result;
}


} // namespace Bluetooth
} // namespace OHOS