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
#define LOG_TAG "bt_napi_hfp_hf"
#endif

#include "napi_bluetooth_hfp_hf.h"
#include "napi_bluetooth_profile.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hfp_hf.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_event.h"
#include "napi_event_subscribe_module.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

std::shared_ptr<NapiHandsFreeUnitObserver> NapiHandsFreeUnit::observer_ =
    std::make_shared<NapiHandsFreeUnitObserver>();
bool NapiHandsFreeUnit::isRegistered_ = false;

void NapiHandsFreeUnit::DefineHandsFreeUnitJSClass(napi_env env)
{
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getConnectionDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", GetDeviceState),
        DECLARE_NAPI_FUNCTION("getScoState", GetScoState),
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("connectSco", ConnectSco),
        DECLARE_NAPI_FUNCTION("disconnectSco", DisconnectSco),
        DECLARE_NAPI_FUNCTION("sendDTMF", SendDTMF),
    };

    napi_define_class(env, "HandsFreeUnit", NAPI_AUTO_LENGTH, HandsFreeUnitConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_HANDS_FREE_UNIT, napiProfile);
}

napi_value NapiHandsFreeUnit::HandsFreeUnitConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiHandsFreeUnit::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    if (!isRegistered_) {
        HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
        profile->RegisterObserver(observer_);
        isRegistered_ = true;
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiHandsFreeUnit::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiHandsFreeUnit::GetConnectionDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value ret = nullptr;
    napi_create_array(env, &ret);
    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    vector<int> states = { static_cast<int>(BTConnectState::CONNECTED) };
    vector<BluetoothRemoteDevice> devices = profile->GetDevicesByStates(states);
    vector<string> deviceVector;
    for (auto &device: devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }
    ConvertStringVectorToJS(env, ret, deviceVector);
    return ret;
}

napi_value NapiHandsFreeUnit::GetDeviceState(napi_env env, napi_callback_info info)
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
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    int state = profile->GetDeviceState(device);
    int status = GetProfileConnectionState(state);
    napi_value result = nullptr;
    napi_create_int32(env, status, &result);
    HILOGI("status: %{public}d", status);
    return result;
}

napi_value NapiHandsFreeUnit::GetScoState(napi_env env, napi_callback_info info)
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
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    int state = profile->GetScoState(device);
    int status = GetScoConnectionState(state);
    napi_value result = nullptr;
    napi_create_int32(env, status, &result);
    HILOGI("status: %{public}d", status);
    return result;
}

napi_value NapiHandsFreeUnit::Connect(napi_env env, napi_callback_info info)
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
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->Connect(device);
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

napi_value NapiHandsFreeUnit::Disconnect(napi_env env, napi_callback_info info)
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
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->Disconnect(device);
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

napi_value NapiHandsFreeUnit::ConnectSco(napi_env env, napi_callback_info info)
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
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->ConnectSco(device);
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

napi_value NapiHandsFreeUnit::DisconnectSco(napi_env env, napi_callback_info info)
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
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }

    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->DisconnectSco(device);
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

napi_value NapiHandsFreeUnit::SendDTMF(napi_env env, napi_callback_info info)
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
    string deviceId;
    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("string expected.");
        return ret;
    }
    int code;
    if (ParseInt32(env, code, argv[PARAM1])) {
        HILOGE("int32 expected.");
        return ret;
    }

    HandsFreeUnit *profile = HandsFreeUnit::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->SendDTMFTone(device, code);
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

} // namespace Bluetooth
} // namespace OHOS