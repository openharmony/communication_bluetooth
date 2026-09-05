/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_pbap_pce"
#endif

#include "bluetooth_errorcode.h"
#include "bluetooth_pbap_pce.h"
#include "bluetooth_utils.h"
#include "napi_async_work.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_pbap_pce.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_utils.h"
#include "parser/napi_parser_utils.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

std::shared_ptr<NapiPbapPceObserver> NapiPbapClient::observer_ = std::make_shared<NapiPbapPceObserver>();
thread_local napi_ref g_pbapPceConsRef_ = nullptr;

void NapiPbapClient::DefinePbapClientJSClass(napi_env env, napi_value exports)
{
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("onConnectionStateChange", OnConnectionStateChange),
        DECLARE_NAPI_FUNCTION("offConnectionStateChange", OffConnectionStateChange),
        DECLARE_NAPI_FUNCTION("onSyncStateChange", OnSyncStateChange),
        DECLARE_NAPI_FUNCTION("offSyncStateChange", OffSyncStateChange),
        DECLARE_NAPI_FUNCTION("getConnectedDevices", GetConnectedDevices),
        DECLARE_NAPI_FUNCTION("getConnectionState", GetConnectionState),
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", GetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getSyncState", GetSyncState),
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
    };

    napi_define_class(env, "PbapPce", NAPI_AUTO_LENGTH, PbapClientConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &g_pbapPceConsRef_);
}

napi_value NapiPbapClient::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createPbapClientProfile", CreatePbapClientProfile),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "pbappce:napi_define_properties");
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiPbapClient::CreatePbapClientProfile(napi_env env, napi_callback_info info)
{
    napi_value napiProfile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, g_pbapPceConsRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);

    PbapPce *profile = PbapPce::GetProfile();
    profile->RegisterObserver(observer_);
    return napiProfile;
}

napi_value NapiPbapClient::PbapClientConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiPbapClient::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapClient::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapClient::OnSyncStateChange(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_PBAP_PCE_SYNC_STATE_CHANGE);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapClient::OffSyncStateChange(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_PBAP_PCE_SYNC_STATE_CHANGE);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapClient::OnConnectionStateChange(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.RegisterWithName(env, info,
            STR_BT_PBAP_PCE_CONNECTION_STATE_CHANGE);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapClient::OffConnectionStateChange(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.DeregisterWithName(env, info,
            STR_BT_PBAP_PCE_CONNECTION_STATE_CHANGE);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapClient::GetConnectedDevices(napi_env env, napi_callback_info info)
{
    napi_value ret = nullptr;
    napi_create_array(env, &ret);
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    PbapPce *profile = PbapPce::GetProfile();
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

napi_value NapiPbapClient::GetConnectionState(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    PbapPce *profile = PbapPce::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t state = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    int32_t errorCode = profile->GetDeviceState(device, state);
    HILOGD("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_UNDEF(env, errorCode == BT_NO_ERROR, errorCode);

    napi_value result = nullptr;
    int32_t profileState = GetProfileConnectionState(state);
    napi_create_int32(env, profileState, &result);
    return result;
}

napi_value NapiPbapClient::Connect(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    PbapPce *profile = PbapPce::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Connect(device);
    HILOGD("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_UNDEF(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapClient::Disconnect(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    PbapPce *profile = PbapPce::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Disconnect(device);
    HILOGD("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_UNDEF(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapClient::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    int32_t strategy = 0;
    auto status = CheckSetConnectStrategyParam(env, info, remoteAddr, strategy);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, strategy]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPce *profile = PbapPce::GetProfile();
        int32_t errorCode = profile->SetConnectionStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d", errorCode);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiPbapClient::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPce *profile = PbapPce::GetProfile();
        int32_t errorCode = profile->GetConnectionStrategy(remoteDevice, strategy);
        HILOGI("errorCode: %{public}d, strategy: %{public}d", errorCode, strategy);
        auto object = std::make_shared<NapiNativeInt>(strategy);
        return NapiAsyncWorkRet(errorCode, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiPbapClient::GetSyncState(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    PbapPce *profile = PbapPce::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t state = profile->GetPhoneBookSyncState(device);
    HILOGD("state:%{public}d", state);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, state >= 0, state);

    napi_value result = nullptr;
    int32_t profileState = GetProfilePhoneBookSyncState(state);
    napi_create_int32(env, profileState, &result);
    return result;
}

}  // namespace Bluetooth
}  // namespace OHOS