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
#include "bluetooth_pbap_pse.h"
#include "bluetooth_utils.h"
#include "napi_async_work.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_pbap_pse.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_utils.h"
#include "../parser/napi_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

enum ShareType {
    SHARE_NAME_AND_PHONE_NUMBER = 0,
    SHARE_ALL = 1,
    SHARE_NOTHING = 2,
};

std::shared_ptr<NapiPbapPseObserver> NapiPbapServer::observer_ = std::make_shared<NapiPbapPseObserver>();
thread_local napi_ref g_consRef_ = nullptr;

void NapiPbapServer::DefinePbapServerJSClass(napi_env env, napi_value exports)
{
    napi_value constructor;
    PbapPropertyValueInit(env, exports);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getConnectedDevices", GetConnectedDevices),
        DECLARE_NAPI_FUNCTION("getConnectionState", GetConnectionState),
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", GetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("setShareType", SetShareType),
        DECLARE_NAPI_FUNCTION("getShareType", GetShareType),
        DECLARE_NAPI_FUNCTION("setPhoneBookAccessAuthorization", SetPhoneBookAccessAuthorization),
        DECLARE_NAPI_FUNCTION("getPhoneBookAccessAuthorization", GetPhoneBookAccessAuthorization),
    };

    napi_define_class(env, "PbapPse", NAPI_AUTO_LENGTH, PbapServerConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &g_consRef_);
}

napi_value NapiPbapServer::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createPbapServerProfile", CreatePbapServerProfile),
    };
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiPbapServer::CreatePbapServerProfile(napi_env env, napi_callback_info info)
{
    napi_value napiProfile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, g_consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);

    PbapPse *profile = PbapPse::GetProfile();
    profile->RegisterObserver(observer_);
    return napiProfile;
}

napi_value NapiPbapServer::PbapServerConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiPbapServer::PbapPropertyValueInit(napi_env env, napi_value exports)
{
    HILOGI("enter");
    napi_value shareTypeObj = ShareTypeInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("ShareType", shareTypeObj),
    };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value NapiPbapServer::ShareTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value shareType = nullptr;
    napi_create_object(env, &shareType);
    SetNamedPropertyByInteger(env, shareType, ShareType::SHARE_NAME_AND_PHONE_NUMBER, "SHARE_NAME_AND_PHONE_NUMBER");
    SetNamedPropertyByInteger(env, shareType, ShareType::SHARE_ALL, "SHARE_ALL");
    SetNamedPropertyByInteger(env, shareType, ShareType::SHARE_NOTHING, "SHARE_NOTHING");
    return shareType;
}

napi_value NapiPbapServer::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapServer::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiPbapServer::GetConnectedDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value ret = nullptr;
    napi_create_array(env, &ret);
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    PbapPse *profile = PbapPse::GetProfile();
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

napi_value NapiPbapServer::GetConnectionState(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    PbapPse *profile = PbapPse::GetProfile();
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

napi_value NapiPbapServer::Disconnect(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_FALSE(env, checkRet, BT_ERR_INVALID_PARAM);

    PbapPse *profile = PbapPse::GetProfile();
    BluetoothRemoteDevice device(remoteAddr, BT_TRANSPORT_BREDR);
    int32_t errorCode = profile->Disconnect(device);
    HILOGD("errorCode:%{public}s", GetErrorCode(errorCode).c_str());
    NAPI_BT_ASSERT_RETURN_FALSE(env, errorCode == BT_NO_ERROR, errorCode);
    return NapiGetBooleanTrue(env);
}

napi_value NapiPbapServer::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    int32_t strategy = 0;
    auto status = CheckSetConnectStrategyParam(env, info, remoteAddr, strategy);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, strategy]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->SetConnectionStrategy(remoteDevice, strategy);
        HILOGI("err: %{public}d", errorCode);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiPbapServer::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t strategy = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
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

bool IsShareTypeValid(int32_t shareType)
{
    return shareType == static_cast<int32_t>(ShareType::SHARE_NAME_AND_PHONE_NUMBER) ||
        shareType == static_cast<int32_t>(ShareType::SHARE_ALL) ||
        shareType == static_cast<int32_t>(ShareType::SHARE_NOTHING);
}

napi_status CheckShareTypeParam(napi_env env, napi_callback_info info, std::string &addr, int32_t &shareType)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE, "Requires 2 or 3 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_RETURN_IF(!ParseInt32(env, shareType, argv[PARAM1]), "ParseInt failed", napi_invalid_arg);
    NAPI_BT_RETURN_IF(!IsShareTypeValid(shareType), "Invalid shareType", napi_invalid_arg);
    return napi_ok;
}

napi_value NapiPbapServer::SetShareType(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    int32_t shareType = 0;
    auto status = CheckShareTypeParam(env, info, remoteAddr, shareType);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, shareType]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->SetShareType(remoteDevice, shareType);
        HILOGI("errorCode: %{public}d", errorCode);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiPbapServer::GetShareType(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t shareType = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->GetShareType(remoteDevice, shareType);
        HILOGI("errorCode: %{public}d, shareType: %{public}d", errorCode, shareType);
        auto object = std::make_shared<NapiNativeInt>(shareType);
        return NapiAsyncWorkRet(errorCode, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiPbapServer::SetPhoneBookAccessAuthorization(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    int32_t accessAuthorization = 0;
    auto status = CheckAccessAuthorizationParam(env, info, remoteAddr, accessAuthorization);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr, accessAuthorization]() {
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->SetPhoneBookAccessAuthorization(remoteDevice, accessAuthorization);
        HILOGI("errorCode: %{public}d", errorCode);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiPbapServer::GetPhoneBookAccessAuthorization(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    auto status = CheckDeviceAddressParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t accessAuthorization = 0;
        BluetoothRemoteDevice remoteDevice(remoteAddr, BT_TRANSPORT_BREDR);
        PbapPse *profile = PbapPse::GetProfile();
        int32_t errorCode = profile->GetPhoneBookAccessAuthorization(remoteDevice, accessAuthorization);
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