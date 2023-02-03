/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_pan.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_pan.h"
#include "napi_bluetooth_pan_observer.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;
NapiBluetoothPanObserver NapiBluetoothPan::observer_;

void NapiBluetoothPan::DefinePanJSClass(napi_env env)
{
    napi_value constructor;
    napi_property_descriptor properties [] = {
        DECLARE_NAPI_FUNCTION("on", NapiBluetoothPan::On),
        DECLARE_NAPI_FUNCTION("off", NapiBluetoothPan::Off),
        DECLARE_NAPI_FUNCTION("getConnectionDevices", NapiBluetoothPan::GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", NapiBluetoothPan::GetDeviceState),
        DECLARE_NAPI_FUNCTION("disconnect", NapiBluetoothPan::Disconnect),
        DECLARE_NAPI_FUNCTION("setTethering", NapiBluetoothPan::SetTethering),
        DECLARE_NAPI_FUNCTION("isTetheringOn", NapiBluetoothPan::IsTetheringOn),
    };

    napi_define_class(env, "NapiBluetoothPan", NAPI_AUTO_LENGTH, NapiBluetoothPan::PanConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_PAN_NETWORK, napiProfile);
    Pan *profile = Pan::GetProfile();
    profile->RegisterObserver(&NapiBluetoothPan::observer_);
    HILOGI("finished");
}

napi_value NapiBluetoothPan::PanConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiBluetoothPan::On(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
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

napi_value NapiBluetoothPan::Off(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
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
    if (observer_.callbackInfos_[type] != nullptr) {
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observer_.callbackInfos_[type];
    napi_delete_reference(env, callbackInfo->callback_);
    }
    observer_.callbackInfos_[type] = nullptr;

    HILOGI("%{public}s is unregistered", type.c_str());

    return ret;
}

napi_value NapiBluetoothPan::GetConnectionDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value ret = nullptr;
    if (napi_create_array(env, &ret) != napi_ok) {
        HILOGE("napi_create_array failed.");
    }

    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    Pan *profile = Pan::GetProfile();
    vector<int> states = { static_cast<int>(BTConnectState::CONNECTED) };
    vector<BluetoothRemoteDevice> devices;
    int errorCode = profile->GetDevicesByStates(states, devices);
    HILOGI("errorCode:%{public}s, devices size:%{public}zu", GetErrorCode(errorCode).c_str(), devices.size());
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_SUCCESS, errorCode, ret);

    vector<string> deviceVector;
    for (auto &device : devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }
    ConvertStringVectorToJS(env, ret, deviceVector);
    return ret;
}

napi_value NapiBluetoothPan::GetDeviceState(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value result = nullptr;
    int32_t profileState = ProfileConnectionState::STATE_DISCONNECTED;
    if (napi_create_int32(env, profileState, &result) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }

    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN(env, checkRet, BT_ERR_INVALID_PARAM, result);

    Pan *profile = Pan::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int32_t errorCode = profile->GetDeviceState(device, state);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_SUCCESS, errorCode, result);

    profileState = GetProfileConnectionState(state);
    if (napi_create_int32(env, profileState, &result) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }
    HILOGI("profileState: %{public}d", profileState);
    return result;
}

napi_value NapiBluetoothPan::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    Pan *profile = Pan::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Disconnect(device);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_SUCCESS, errorCode);

    return NapiGetBooleanTrue(env);
}

static bool CheckSetTetheringParam(napi_env env, napi_callback_info info, bool &out)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_BT_RETURN_IF(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok, "call failed.", false);
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 argument.", false);
    NAPI_BT_RETURN_IF(!ParseBool(env, out, argv[PARAM0]), "Bool expected.", false);
    return true;
}

napi_value NapiBluetoothPan::SetTethering(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    bool value = false;
    bool checkRet = CheckSetTetheringParam(env, info, value);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    Pan *profile = Pan::GetProfile();
    int32_t errorCode = profile->SetTethering(value);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_UNDEF(env, errorCode == BT_SUCCESS, errorCode);

    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothPan::IsTetheringOn(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM);

    Pan *profile = Pan::GetProfile();
    bool result = false;
    int32_t errorCode = profile->IsTetheringOn(result);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_SUCCESS, errorCode);

    HILOGI("IsTetheringOn: %{public}d", result);
    return NapiGetBooleanRet(env, result);
}
}  // namespace Bluetooth
}  // namespace OHOS