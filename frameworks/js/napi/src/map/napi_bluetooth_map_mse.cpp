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
#include "bluetooth_errorcode.h"
#include "bluetooth_map_mse.h"
#include "bluetooth_utils.h"
#include "napi_async_work.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_map_mse.h"
#include "napi_bluetooth_utils.h"
#include "../parser/napi_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

std::shared_ptr<NapiMapMseObserver> NapiMapMse::observer_ = std::make_shared<NapiMapMseObserver>();
thread_local napi_ref g_consRef_ = nullptr;

void NapiMapMse::DefineMapMseJSClass(napi_env env, napi_value exports)
{
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getConnectedDevices", GetConnectedDevices),
        DECLARE_NAPI_FUNCTION("getConnectionState", GetConnectionState),
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", GetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("setMessageAccessAuthorization", SetMessageAccessAuthorization),
        DECLARE_NAPI_FUNCTION("getMessageAccessAuthorization", GetMessageAccessAuthorization),
    };

    napi_define_class(env, "MapMse", NAPI_AUTO_LENGTH, MapMseConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &g_consRef_);
}

napi_value NapiMapMse::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createMapMseProfile", CreateMapMseProfile),
    };
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiMapMse::CreateMapMseProfile(napi_env env, napi_callback_info info)
{
    napi_value napiProfile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, g_consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);

    MapMse *profile = MapMse::GetProfile();
    profile->RegisterObserver(observer_);
    return napiProfile;
}

napi_value NapiMapMse::MapMseConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiMapMse::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiMapMse::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiMapMse::GetConnectedDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value ret = nullptr;
    napi_create_array(env, &ret);
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    MapMse *profile = MapMse::GetProfile();
    vector<int32_t> states = { static_cast<int32_t>(BTConnectState::CONNECTED) };
    vector<BluetoothRemoteDevice> devices {};
    int32_t errorCode = profile->GetDevicesByStates(states, devices);
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_NO_ERROR, errorCode, ret);

    vector<string> deviceVector;
    for (auto &device : devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }

    auto status = ConvertStringVectorToJS(env, ret, deviceVector);
    NAPI_BT_ASSERT_RETURN(env, status == napi_ok, BT_ERR_INTERNAL_ERROR, ret);
    return ret;
}

napi_value NapiMapMse::GetConnectionState(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    MapMse *profile = MapMse::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int32_t errorCode = profile->GetDeviceState(device, state);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_UNDEF(env, errorCode == BT_NO_ERROR, errorCode);

    napi_value result = nullptr;
    int32_t profileState = GetProfileConnectionState(state);
    napi_create_int32(env, profileState, &result);
    return result;
}

napi_value NapiMapMse::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    MapMse *profile = MapMse::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Disconnect(device);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetBooleanTrue(env);
}

napi_value NapiMapMse::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr{};
    int32_t strategy = 0;
    auto status = CheckSetConnectStrategyParam(env, info, remoteAddr, strategy);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, strategy]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        MapMse *profile = MapMse::GetProfile();
        int32_t errorCode = profile->SetConnectionStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d", errorCode);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiMapMse::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        MapMse *profile = MapMse::GetProfile();
        int32_t errorCode = profile->GetConnectionStrategy(remoteDevice, strategy);
        HILOGI("errorCode: %{public}d, deviceName: %{public}d", errorCode, strategy);
        auto object = std::make_shared<NapiNativeInt>(strategy);
        return NapiAsyncWorkRet(errorCode, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiMapMse::SetMessageAccessAuthorization(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    int32_t accessAuthorization = 0;
    auto status = CheckAccessAuthorizationParam(env, info, remoteAddr, accessAuthorization);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, accessAuthorization]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        MapMse *profile = MapMse::GetProfile();
        int32_t errorCode = profile->SetMessageAccessAuthorization(remoteDevice, accessAuthorization);
        HILOGI("errorCode: %{public}d", errorCode);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiMapMse::GetMessageAccessAuthorization(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t accessAuthorization = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        MapMse *profile = MapMse::GetProfile();
        int32_t errorCode = profile->GetMessageAccessAuthorization(remoteDevice, accessAuthorization);
        HILOGI("errorCode: %{public}d, accessAuthorization: %{public}d", errorCode, accessAuthorization);
        auto object = std::make_shared<NapiNativeInt>(accessAuthorization);
        return NapiAsyncWorkRet(errorCode, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

}  // namespace Bluetooth
}  // namespace OHOS