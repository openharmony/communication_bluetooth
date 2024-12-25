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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_pan"
#endif

#include "bluetooth_log.h"
#include "bluetooth_pan.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_pan.h"
#include "napi_bluetooth_pan_observer.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_utils.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;
std::shared_ptr<NapiBluetoothPanObserver> NapiBluetoothPan::observer_ = std::make_shared<NapiBluetoothPanObserver>();
thread_local napi_ref NapiBluetoothPan::consRef_ = nullptr;

napi_value NapiBluetoothPan::CreatePanProfile(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value napiProfile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_PAN_NETWORK, napiProfile);
    Pan *profile = Pan::GetProfile();
    profile->RegisterObserver(NapiBluetoothPan::observer_);
    HILOGI("finished");

    return napiProfile;
}

void NapiBluetoothPan::DefinePanJSClass(napi_env env, napi_value exports)
{
    napi_value constructor;
    napi_property_descriptor properties [] = {
        DECLARE_NAPI_FUNCTION("on", NapiBluetoothPan::On),
        DECLARE_NAPI_FUNCTION("off", NapiBluetoothPan::Off),
        DECLARE_NAPI_FUNCTION("disconnect", NapiBluetoothPan::Disconnect),
        DECLARE_NAPI_FUNCTION("setTethering", NapiBluetoothPan::SetTethering),
        DECLARE_NAPI_FUNCTION("isTetheringOn", NapiBluetoothPan::IsTetheringOn),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("getConnectedDevices", NapiBluetoothPan::GetConnectedDevices),
        DECLARE_NAPI_FUNCTION("getConnectionState", NapiBluetoothPan::GetConnectionState),
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", NapiBluetoothPan::SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", NapiBluetoothPan::GetConnectionStrategy),
#else
        DECLARE_NAPI_FUNCTION("getConnectionDevices", NapiBluetoothPan::GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", NapiBluetoothPan::GetDeviceState),
#endif
    };

    napi_define_class(env, "NapiBluetoothPan", NAPI_AUTO_LENGTH, NapiBluetoothPan::PanConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

#ifdef BLUETOOTH_API_SINCE_10
    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &consRef_);
#else
    napi_define_class(env, "NapiBluetoothPan", NAPI_AUTO_LENGTH, PanConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_PAN_NETWORK, napiProfile);
    Pan *profile = Pan::GetProfile();
    profile->RegisterObserver(NapiBluetoothPan::observer_);
#endif
}

napi_value NapiBluetoothPan::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties [] = {
        DECLARE_NAPI_FUNCTION("createPanProfile", NapiBluetoothPan::CreatePanProfile),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "pan:napi_define_properties");
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiBluetoothPan::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NAPI_BT_ASSERT_RETURN_UNDEF(env, false, BT_ERR_API_NOT_SUPPORT);
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothPan::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    NAPI_BT_ASSERT_RETURN_UNDEF(env, false, BT_ERR_API_NOT_SUPPORT);
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothPan::PanConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiBluetoothPan::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothPan::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
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
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_NO_ERROR, errorCode, ret);

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
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_NO_ERROR, errorCode, result);

    profileState = GetProfileConnectionState(state);
    if (napi_create_int32(env, profileState, &result) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }
    HILOGI("profileState: %{public}d", profileState);
    return result;
}

napi_value NapiBluetoothPan::GetConnectedDevices(napi_env env, napi_callback_info info)
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
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_NO_ERROR, errorCode, ret);

    vector<string> deviceVector;
    for (auto &device : devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }
    ConvertStringVectorToJS(env, ret, deviceVector);
    return ret;
}

napi_value NapiBluetoothPan::GetConnectionState(napi_env env, napi_callback_info info)
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
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_NO_ERROR, errorCode, result);

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
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_NO_ERROR, errorCode);

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
    NAPI_BT_ASSERT_RETURN_UNDEF(env, errorCode == BT_NO_ERROR, errorCode);

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
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_NO_ERROR, errorCode);

    HILOGI("IsTetheringOn: %{public}d", result);
    return NapiGetBooleanRet(env, result);
}
}  // namespace Bluetooth
}  // namespace OHOS