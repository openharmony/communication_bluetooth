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
#include "bluetooth_opp.h"
#include "bluetooth_utils.h"
#include "napi_async_work.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_opp.h"
#include "napi_bluetooth_profile.h"
#include "napi_bluetooth_utils.h"
#include "../parser/napi_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

std::shared_ptr<NapiBluetoothOppObserver> NapiBluetoothOpp::observer_ = std::make_shared<NapiBluetoothOppObserver>();
thread_local napi_ref g_consRef_ = nullptr;

void NapiBluetoothOpp::DefineOppJSClass(napi_env env, napi_value exports)
{
    napi_value constructor;
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("onEvent", On),
        DECLARE_NAPI_FUNCTION("offEvent", Off),
        DECLARE_NAPI_FUNCTION("sendFile", SendFile),
        DECLARE_NAPI_FUNCTION("setIncomingFileConfirmation", SetIncomingFileConfirmation),
        DECLARE_NAPI_FUNCTION("getCurrentTransferInformation", GetCurrentTransferInformation),
        DECLARE_NAPI_FUNCTION("cancelTransfer", CancelTransfer),
        DECLARE_NAPI_FUNCTION("getConnectionDevices", GetConnectionDevices),
        DECLARE_NAPI_FUNCTION("getDeviceState", GetDeviceState),
    };

    napi_define_class(env, "NapiBluetoothOpp", NAPI_AUTO_LENGTH, OppConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &g_consRef_);
}

napi_value NapiBluetoothOpp::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createOppServerProfile", CreateOppServerProfile),
    };
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiBluetoothOpp::CreateOppServerProfile(napi_env env, napi_callback_info info)
{
    napi_value napiProfile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, g_consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);

    Opp *profile = Opp::GetProfile();
    profile->RegisterObserver(observer_);
    return napiProfile;
}

napi_value NapiBluetoothOpp::OppConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiBluetoothOpp::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBluetoothOpp::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_status CheckSetIncomingFileConfirmation(napi_env env, napi_callback_info info, bool &accept)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Require 1 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBoolean(env, argv[PARAM0], accept));
    return napi_ok;
}

napi_status CheckSendFileParam(napi_env env, napi_callback_info info, std::string &addr,
    std::vector<std::string> &filePaths, std::vector<std::string> &mimeTypes)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_THREE, "Require 3 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_CALL_RETURN(NapiParseStringArray(env, argv[PARAM1], filePaths));
    NAPI_BT_CALL_RETURN(NapiParseStringArray(env, argv[PARAM2], mimeTypes));
    return napi_ok;
}

napi_value NapiBluetoothOpp::SendFile(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    std::string device {};
    std::vector<std::string> filePaths;
    std::vector<std::string> mimeTypes;

    auto status = CheckSendFileParam(env, info, device, filePaths, mimeTypes);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [device, filePaths, mimeTypes]() {
        Opp *profile = Opp::GetProfile();
        bool result = false;
        int32_t errorCode = profile->SendFile(device, filePaths, mimeTypes, result);
        HILOGI("err: %{public}d result: %{public}d", errorCode, result);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBluetoothOpp::SetIncomingFileConfirmation(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    bool accept = false;
    auto status = CheckSetIncomingFileConfirmation(env, info, accept);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [accept]() {
        Opp *profile = Opp::GetProfile();
        int32_t errorCode = profile->SetIncomingFileConfirmation(accept);
        HILOGI("err: %{public}d", errorCode);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBluetoothOpp::GetCurrentTransferInformation(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    napi_value ret = nullptr;
    napi_create_object(env, &ret);
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    Opp *profile = Opp::GetProfile();
    BluetoothOppTransferInformation information;
    int32_t errorCode = profile->GetCurrentTransferInformation(information);
    HILOGI("GetCurrentTransferInformation errorCode is %{public}d", errorCode);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, errorCode == BT_NO_ERROR, errorCode);

    auto status = ConvertOppTransferInformationToJS(env, ret, information);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INTERNAL_ERROR);
    return ret;
}

napi_value NapiBluetoothOpp::CancelTransfer(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    napi_value ret = nullptr;
    bool isOk = false;
    napi_get_boolean(env, isOk, &ret);
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    Opp *profile = Opp::GetProfile();
    profile->CancelTransfer(isOk);
    napi_get_boolean(env, isOk, &ret);
    return ret;
}

napi_value NapiBluetoothOpp::GetConnectionDevices(napi_env env, napi_callback_info info)
{
    HILOGI("enter");

    napi_value ret = nullptr;
    napi_create_array(env, &ret);
    napi_status checkRet = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN(env, checkRet == napi_ok, BT_ERR_INVALID_PARAM, ret);

    Opp *profile = Opp::GetProfile();
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

napi_value NapiBluetoothOpp::GetDeviceState(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_ERR_INVALID_PARAM);

    Opp *profile = Opp::GetProfile();
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
}  // namespace Bluetooth
}  // namespace OHOS