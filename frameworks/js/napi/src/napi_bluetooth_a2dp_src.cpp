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
#include "bluetooth_a2dp_src.h"
#include "bluetooth_errorcode.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_a2dp_src.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_host.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

NapiA2dpSourceObserver NapiA2dpSource::observer_;
bool NapiA2dpSource::isRegistered_ = false;

void NapiA2dpSource::DefineA2dpSourceJSClass(napi_env env)
{
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getConnectionDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", GetDeviceState),
        DECLARE_NAPI_FUNCTION("getPlayingState", GetPlayingState),
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
    };

    napi_define_class(env, "A2dpSource", NAPI_AUTO_LENGTH, A2dpSourceConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_A2DP_SOURCE, napiProfile);
    HILOGI("finished");
}

napi_value NapiA2dpSource::A2dpSourceConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiA2dpSource::On(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::unique_lock<std::shared_mutex> guard(NapiA2dpSourceObserver::g_a2dpSrcCallbackInfosMutex);

    napi_value ret = nullptr;
    ret = NapiEvent::OnEvent(env, info, observer_.callbackInfos_);
    if (!isRegistered_) {
        A2dpSource *profile = A2dpSource::GetProfile();
        profile->RegisterObserver(&observer_);
        isRegistered_ = true;
    }
    HILOGI("napi A2dpSource is registered");
    return ret;
}

napi_value NapiA2dpSource::Off(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::unique_lock<std::shared_mutex> guard(NapiA2dpSourceObserver::g_a2dpSrcCallbackInfosMutex);

    napi_value ret = nullptr;
    ret = NapiEvent::OffEvent(env, info, observer_.callbackInfos_);
    HILOGI("Napi A2dpSource is unregistered");
    return ret;
}

napi_value NapiA2dpSource::GetConnectionDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
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

napi_value NapiA2dpSource::GetDeviceState(napi_env env, napi_callback_info info)
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

    A2dpSource *profile = A2dpSource::GetProfile();
    BluetoothRemoteDevice device(deviceId, 1);
    int state = profile->GetDeviceState(device);
    napi_value result = nullptr;
    int status = GetProfileConnectionState(state);
    napi_create_int32(env, status, &result);
    HILOGI("status: %{public}d", status);
    return result;
}

napi_value NapiA2dpSource::GetPlayingState(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    int state = PlayingState::STATE_NOT_PLAYING;
    napi_value ret = nullptr;
    napi_create_int32(env, state, &ret);

    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN(env, checkRet, BT_ERR_INVALID_PARAM, ret);

    int transport = GetDeviceTransport(remoteAddr);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t res = profile->GetPlayingState(remoteDevice, state);
    HILOGI("res: %{public}d", res);
    NAPI_BT_ASSERT_RETURN(env, (res == BT_SUCCESS), res, ret);

    return NapiGetInt32Ret(env, state);
}

napi_value NapiA2dpSource::Connect(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    int transport = GetDeviceTransport(remoteAddr);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t ret = profile->Connect(remoteDevice);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);

    return NapiGetBooleanTrue(env);
}

napi_value NapiA2dpSource::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    int transport = GetDeviceTransport(remoteAddr);
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t ret = profile->Disconnect(remoteDevice);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_SUCCESS, ret);

    return NapiGetBooleanTrue(env);
}
} // namespace Bluetooth
} // namespace OHOS