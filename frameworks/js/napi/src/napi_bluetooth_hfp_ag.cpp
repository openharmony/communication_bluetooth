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
#include "bluetooth_hfp_ag.h"

#include "bluetooth_errorcode.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_hfp_ag.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_event.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

NapiHandsFreeAudioGatewayObserver NapiHandsFreeAudioGateway::observer_;
bool NapiHandsFreeAudioGateway::isRegistered_ = false;

void NapiHandsFreeAudioGateway::DefineHandsFreeAudioGatewayJSClass(napi_env env)
{
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getConnectionDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", GetDeviceState),
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("getScoState", GetScoState),
        DECLARE_NAPI_FUNCTION("connectSco", ConnectSco),
        DECLARE_NAPI_FUNCTION("disconnectSco", DisconnectSco),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("openVoiceRecognition", OpenVoiceRecognition),
        DECLARE_NAPI_FUNCTION("closeVoiceRecognition", CloseVoiceRecognition),
    };

    napi_define_class(env, "HandsFreeAudioGateway", NAPI_AUTO_LENGTH, HandsFreeAudioGatewayConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_HANDS_FREE_AUDIO_GATEWAY, napiProfile);
    HILOGI("finished");
}

napi_value NapiHandsFreeAudioGateway::HandsFreeAudioGatewayConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiHandsFreeAudioGateway::On(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::unique_lock<std::shared_mutex> guard(NapiHandsFreeAudioGatewayObserver::g_handsFreeAudioGatewayCallbackMutex);

    napi_value ret = nullptr;
    ret = NapiEvent::OnEvent(env, info, observer_.callbackInfos_);
    if (!isRegistered_) {
        HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
        profile->RegisterObserver(&observer_);
        isRegistered_ = true;
    }

    HILOGI("Hands Free Audio Gateway is registered");
    return ret;
}

napi_value NapiHandsFreeAudioGateway::Off(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::unique_lock<std::shared_mutex> guard(NapiHandsFreeAudioGatewayObserver::g_handsFreeAudioGatewayCallbackMutex);

    napi_value ret = nullptr;
    ret = NapiEvent::OffEvent(env, info, observer_.callbackInfos_);
    HILOGI("Hands Free Audio Gateway is unregistered");
    return ret;
}

napi_value NapiHandsFreeAudioGateway::GetConnectionDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value ret = nullptr;
    if (napi_create_array(env, &ret) != napi_ok) {
        HILOGE("napi_create_array failed.");
    }
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    vector<BluetoothRemoteDevice> devices;
    int errorCode = profile->GetConnectedDevices(devices);
    HILOGI("errorCode:%{public}s, devices size:%{public}zu", GetErrorCode(errorCode).c_str(), devices.size());
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_SUCCESS, errorCode, ret);

    vector<string> deviceVector;
    for (auto &device: devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }
    ConvertStringVectorToJS(env, ret, deviceVector);
    return ret;
}

napi_value NapiHandsFreeAudioGateway::GetDeviceState(napi_env env, napi_callback_info info)
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

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int32_t errorCode = profile->GetDeviceState(device, state);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_SUCCESS, errorCode, result);

    profileState = GetProfileConnectionState(state);
    if (napi_create_int32(env, profileState, &result) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }
    return result;
}

napi_value NapiHandsFreeAudioGateway::GetScoState(napi_env env, napi_callback_info info)
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

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    int state = profile->GetScoState(device);
    int status = GetScoConnectionState(state);
    napi_value result = nullptr;
    napi_create_int32(env, status, &result);
    HILOGI("status: %{public}d", status);
    return result;
}

napi_value NapiHandsFreeAudioGateway::ConnectSco(napi_env env, napi_callback_info info)
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

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->SetActiveDevice(device);
    if (isOK) {
        isOK = profile->ConnectSco();
    }
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

napi_value NapiHandsFreeAudioGateway::DisconnectSco(napi_env env, napi_callback_info info)
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

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->SetActiveDevice(device);
    if (isOK) {
        isOK = profile->DisconnectSco();
    }
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

napi_value NapiHandsFreeAudioGateway::OpenVoiceRecognition(napi_env env, napi_callback_info info)
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

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->OpenVoiceRecognition(device);
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

napi_value NapiHandsFreeAudioGateway::CloseVoiceRecognition(napi_env env, napi_callback_info info)
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

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    bool isOK = profile->CloseVoiceRecognition(device);
    napi_value result = nullptr;
    napi_get_boolean(env, isOK, &result);
    HILOGI("res: %{public}d", isOK);
    return result;
}

napi_value NapiHandsFreeAudioGateway::Connect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Connect(device);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_SUCCESS, errorCode);
    return NapiGetBooleanTrue(env);
}

napi_value NapiHandsFreeAudioGateway::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    HandsFreeAudioGateway *profile = HandsFreeAudioGateway::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Disconnect(device);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_SUCCESS, errorCode);
    return NapiGetBooleanTrue(env);
}
} // namespace Bluetooth
} // namespace OHOS