/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include "napi_async_work.h"
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
thread_local napi_ref g_napiProfile = nullptr;

napi_value NapiA2dpSource::DefineA2dpSourceJSClass(napi_env env, napi_value exports)
{
    A2dpPropertyValueInit(env, exports);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("getPlayingState", GetPlayingState),
#ifndef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("getConnectionDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", GetDeviceState),
#endif
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", GetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionState", GetConnectionState),
        DECLARE_NAPI_FUNCTION("getConnectedDevices", getConnectedDevices),
#endif
    };

    napi_value constructor;
    napi_define_class(env,
        "A2dpSource",
        NAPI_AUTO_LENGTH,
        A2dpSourceConstructor,
        nullptr,
        sizeof(properties) / sizeof(properties[0]),
        properties,
        &constructor);
#ifdef BLUETOOTH_API_SINCE_10
    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &g_napiProfile);
#else
    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_A2DP_SOURCE, napiProfile);
#endif
    HILOGI("finished");
    return exports;
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

napi_value NapiA2dpSource::GetPlayingState(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    int state = PlayingState::STATE_NOT_PLAYING;
    napi_value ret = nullptr;
    napi_create_int32(env, state, &ret);

    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN(env, checkRet, BT_ERR_INVALID_PARAM, ret);

    int transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t errorCode = profile->GetPlayingState(remoteDevice, state);
    HILOGI("errorCode: %{public}d", errorCode);
    NAPI_BT_ASSERT_RETURN(env, (errorCode == BT_NO_ERROR), errorCode, ret);

    return NapiGetInt32Ret(env, state);
}

napi_value NapiA2dpSource::Connect(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    int transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t ret = profile->Connect(remoteDevice);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);

    return NapiGetBooleanTrue(env);
}

napi_value NapiA2dpSource::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    int transport = BT_TRANSPORT_BREDR;
    BluetoothRemoteDevice remoteDevice = BluetoothRemoteDevice(remoteAddr, transport);
    A2dpSource *profile = A2dpSource::GetProfile();
    int32_t ret = profile->Disconnect(remoteDevice);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);

    return NapiGetBooleanTrue(env);
}

napi_value PlayingStateInit(napi_env env)
{
    HILOGI("enter");
    napi_value playingState = nullptr;
    napi_create_object(env, &playingState);
    SetNamedPropertyByInteger(env, playingState, PlayingState::STATE_NOT_PLAYING, "STATE_NOT_PLAYING");
    SetNamedPropertyByInteger(env, playingState, PlayingState::STATE_PLAYING, "STATE_PLAYING");
    return playingState;
}

napi_value NapiA2dpSource::A2dpPropertyValueInit(napi_env env, napi_value exports)
{
    napi_value playingStateObj = PlayingStateInit(env);
    napi_property_descriptor exportProps[] = {
        DECLARE_NAPI_PROPERTY("PlayingState", playingStateObj),
    };
    napi_define_properties(env, exports, sizeof(exportProps) / sizeof(*exportProps), exportProps);
    HILOGI("end");
    return exports;
}

napi_value NapiA2dpSource::GetConnectionDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value ret = nullptr;
    napi_create_array(env, &ret);
    A2dpSource *profile = A2dpSource::GetProfile();
    vector<int> states;
    states.push_back(1);
    vector<BluetoothRemoteDevice> devices;
    int errorCode = profile->GetDevicesByStates(states, devices);
    NAPI_BT_ASSERT_RETURN(env, (errorCode == BT_NO_ERROR), errorCode, ret);

    vector<string> deviceVector;
    for (auto &device : devices) {
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

    int32_t profileState = ProfileConnectionState::STATE_DISCONNECTED;
    if (napi_create_int32(env, profileState, &ret) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }

    int btConnectState = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int errorCode = profile->GetDeviceState(device, btConnectState);
    NAPI_BT_ASSERT_RETURN(env, (errorCode == BT_NO_ERROR), errorCode, ret);

    napi_value result = nullptr;
    int status = GetProfileConnectionState(btConnectState);
    napi_create_int32(env, status, &result);
    HILOGI("status: %{public}d", status);
    return result;
}

#ifdef BLUETOOTH_API_SINCE_10
napi_value NapiA2dpSource::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createA2dpSrcProfile", CreateA2dpSrcProfile),
    };
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiA2dpSource::CreateA2dpSrcProfile(napi_env env, napi_callback_info info)
{
    napi_value profile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, g_napiProfile, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &profile);
    return profile;
}

napi_value NapiA2dpSource::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    std::string remoteAddr{};
    int32_t strategy = 0;
    auto status = CheckSetConnectStrategyParam(env, info, remoteAddr, strategy);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int transport = BT_TRANSPORT_BREDR;
    auto func = [remoteAddr, transport, strategy]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, transport);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiA2dpSource::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int transport = BT_TRANSPORT_BREDR;
    auto func = [remoteAddr, transport]() {
        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, transport);
        A2dpSource *profile = A2dpSource::GetProfile();
        int32_t err = profile->GetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d, deviceName: %{public}d", err, strategy);
        auto object = std::make_shared<NapiNativeInt>(strategy);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiA2dpSource::GetConnectionState(napi_env env, napi_callback_info info)
{
    return GetDeviceState(env, info);
}
napi_value NapiA2dpSource::getConnectedDevices(napi_env env, napi_callback_info info)
{
    return GetConnectionDevices(env, info);
}

#endif
}  // namespace Bluetooth
}  // namespace OHOS