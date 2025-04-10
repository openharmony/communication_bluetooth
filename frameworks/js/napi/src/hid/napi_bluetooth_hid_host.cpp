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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_hid_host"
#endif

#include "bluetooth_hid_host.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi_async_work.h"
#include "napi_bluetooth_hid_host_observer.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_hid_host.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;
std::shared_ptr<NapiBluetoothHidHostObserver> NapiBluetoothHidHost::observer_ =
    std::make_shared<NapiBluetoothHidHostObserver>();
thread_local napi_ref NapiBluetoothHidHost::consRef_ = nullptr;

void NapiBluetoothHidHost::DefineHidHostJSClass(napi_env env, napi_value exports)
{
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("getConnectedDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getConnectionState", GetDeviceState),
#else
        DECLARE_NAPI_FUNCTION("getConnectionDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", GetDeviceState),
#endif
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", GetConnectionStrategy),
    };

    napi_define_class(env, "NapiBluetoothHidHost", NAPI_AUTO_LENGTH, HidHostConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
#ifdef BLUETOOTH_API_SINCE_10
    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &consRef_);
#else
    napi_value napiProfile;
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_HID_HOST, napiProfile);
    HidHost *profile = HidHost::GetProfile();
    profile->RegisterObserver(observer_);
#endif
}

napi_value NapiBluetoothHidHost::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createHidHostProfile", CreateHidHostProfile),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "hid:napi_define_properties");
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiBluetoothHidHost::CreateHidHostProfile(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value napiProfile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);
    NapiProfile::SetProfile(env, ProfileId::PROFILE_HID_HOST, napiProfile);

    HidHost *profile = HidHost::GetProfile();
    profile->RegisterObserver(observer_);

    return napiProfile;
}


napi_value NapiBluetoothHidHost::HidHostConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiBluetoothHidHost::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidHost::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothHidHost::GetConnectionDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    napi_value ret = nullptr;
    if (napi_create_array(env, &ret) != napi_ok) {
        HILOGE("napi_create_array failed.");
    }
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    HidHost *profile = HidHost::GetProfile();
    vector<int> states = { static_cast<int>(BTConnectState::CONNECTED) };
    vector<BluetoothRemoteDevice> devices;
    int errorCode = profile->GetDevicesByStates(states, devices);
    HILOGI("errorCode:%{public}s, devices size:%{public}zu", GetErrorCode(errorCode).c_str(), devices.size());
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_NO_ERROR, errorCode, ret);

    vector<string> deviceVector;
    for (auto &device: devices) {
        deviceVector.push_back(device.GetDeviceAddr());
    }
    ConvertStringVectorToJS(env, ret, deviceVector);
    return ret;
}

napi_value NapiBluetoothHidHost::GetDeviceState(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    int32_t profileState = ProfileConnectionState::STATE_DISCONNECTED;
    if (napi_create_int32(env, profileState, &result) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }

    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN(env, checkRet, BT_ERR_INVALID_PARAM, result);

    HidHost *profile = HidHost::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int32_t errorCode = profile->GetDeviceState(device, state);
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_NO_ERROR, errorCode, result);

    profileState = GetProfileConnectionState(state);
    if (napi_create_int32(env, profileState, &result) != napi_ok) {
        HILOGE("napi_create_int32 failed.");
    }
    return result;
}

napi_value NapiBluetoothHidHost::Connect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    HidHost *profile = HidHost::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Connect(device);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetBooleanTrue(env);
}

napi_value NapiBluetoothHidHost::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGI("Disconnect called");
    std::string remoteAddr {};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    HidHost *profile = HidHost::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Disconnect(device);
    HILOGI("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetBooleanTrue(env);
}

napi_value NapiBluetoothHidHost::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr {};
    int32_t strategy = 0;
    auto status = CheckSetConnectStrategyParam(env, info, remoteAddr, strategy);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, strategy]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HidHost *profile = HidHost::GetProfile();
        int32_t err = profile->SetConnectStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d", err);
        return NapiAsyncWorkRet(err);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBluetoothHidHost::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGD("start");
    std::string remoteAddr {};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        HidHost *profile = HidHost::GetProfile();
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
}  // namespace Bluetooth
}  // namespace OHOS