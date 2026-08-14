/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_hearingaid"
#endif

#include "bluetooth_log.h"
#include "napi_bluetooth_hearingaid.h"
#include "hitrace_meter.h"
#include "bluetooth_hearingaid.h"
#include "bluetooth_override_errorcode.h"
#include "napi_bluetooth_error.h"
#include "napi_parser_utils.h"

namespace OHOS {
namespace Bluetooth {

std::shared_ptr<NapiHearingAidObserver> NapiHearingAid::observer_ = std::make_shared<NapiHearingAidObserver>();
thread_local napi_ref g_consRef_ = nullptr;

void NapiHearingAid::DefineHearingAidJSClass(napi_env env, napi_value exports)
{
    napi_value constructor;
    PropertyValueInit(env, exports);
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("isLocalDeviceSupportHearingAid", IsLocalDeviceSupportHearingAid),
        DECLARE_NAPI_FUNCTION("connect", Connect),
        DECLARE_NAPI_FUNCTION("disconnect", Disconnect),
        DECLARE_NAPI_FUNCTION("setVolume", SetVolume),
        DECLARE_NAPI_FUNCTION("getHearingAidDeviceInfo", GetHearingAidDeviceInfo),
        DECLARE_NAPI_FUNCTION("getConnectedDevices", GetConnectedDevices),
        DECLARE_NAPI_FUNCTION("getConnectionState", GetConnectionState),
        DECLARE_NAPI_FUNCTION("setConnectionStrategy", SetConnectionStrategy),
        DECLARE_NAPI_FUNCTION("getConnectionStrategy", GetConnectionStrategy),
    };

    napi_define_class(env, "NapiHearingAid", NAPI_AUTO_LENGTH, HearingAidConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);

    DefineCreateProfile(env, exports);
    napi_create_reference(env, constructor, 1, &g_consRef_);
}

napi_value NapiHearingAid::HearingAidConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    return thisVar;
}

napi_value NapiHearingAid::DefineCreateProfile(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("createHearingAidSrcProfile", CreateHearingAidSrcProfile),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "hearingAid:napi_define_properties");
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value NapiHearingAid::CreateHearingAidSrcProfile(napi_env env, napi_callback_info info)
{
    napi_value napiProfile;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, g_consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &napiProfile);

    BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
    profile->RegisterObserver(observer_);
    return napiProfile;
}

napi_value DeviceSideInit(napi_env env)
{
    napi_value deviceSide = nullptr;
    napi_create_object(env, &deviceSide);
    SetNamedPropertyByInteger(env, deviceSide, DeviceSide::DEVICE_SIDE_LEFT, "DEVICE_SIDE_LEFT");
    SetNamedPropertyByInteger(env, deviceSide, DeviceSide::DEVICE_SIDE_RIGHT, "DEVICE_SIDE_RIGHT");
    return deviceSide;
}

napi_value DeviceModeInit(napi_env env)
{
    napi_value deviceMode = nullptr;
    napi_create_object(env, &deviceMode);
    SetNamedPropertyByInteger(env, deviceMode, DeviceMode::DEVICE_MODE_MONAURAL, "DEVICE_MODE_MONAURAL");
    SetNamedPropertyByInteger(env, deviceMode, DeviceMode::DEVICE_MODE_BINAURAL, "DEVICE_MODE_BINAURAL");
    return deviceMode;
}

napi_value ConnectionStateInit(napi_env env)
{
    napi_value connectionState = nullptr;
    napi_create_object(env, &connectionState);
    SetNamedPropertyByInteger(env, connectionState,
        HearingAidConnectionState::CONNECTION_STATE_DISCONNECTED, "STATE_DISCONNECTED");
    SetNamedPropertyByInteger(env, connectionState,
        HearingAidConnectionState::CONNECTION_STATE_CONNECTING, "STATE_CONNECTING");
    SetNamedPropertyByInteger(env, connectionState,
        HearingAidConnectionState::CONNECTION_STATE_CONNECTED, "STATE_CONNECTED");
    SetNamedPropertyByInteger(env, connectionState,
        HearingAidConnectionState::CONNECTION_STATE_DISCONNECTING, "STATE_DISCONNECTING");
    return connectionState;
}

napi_value ConnectionStrategyInit(napi_env env)
{
    napi_value connectionStrategy = nullptr;
    napi_create_object(env, &connectionStrategy);
    SetNamedPropertyByInteger(env, connectionStrategy,
        HearingAidConnectionStrategy::CONNECTION_STRATEGY_UNSUPPORTED, "CONNECTION_STRATEGY_UNSUPPORTED");
    SetNamedPropertyByInteger(env, connectionStrategy,
        HearingAidConnectionStrategy::CONNECTION_STRATEGY_ALLOWED, "CONNECTION_STRATEGY_ALLOWED");
    SetNamedPropertyByInteger(env, connectionStrategy,
        HearingAidConnectionStrategy::CONNECTION_STRATEGY_FORBIDDEN, "CONNECTION_STRATEGY_FORBIDDEN");
    return connectionStrategy;
}

napi_value NapiHearingAid::PropertyValueInit(napi_env env, napi_value exports)
{
    napi_value deviceSideObj = DeviceSideInit(env);
    napi_value deviceModeObj = DeviceModeInit(env);
    napi_value connectionStateObj = ConnectionStateInit(env);
    napi_value connectionStrategyObj = ConnectionStrategyInit(env);

    napi_property_descriptor exportProps[] = {
        DECLARE_NAPI_PROPERTY("DeviceSide", deviceSideObj),
        DECLARE_NAPI_PROPERTY("DeviceMode", deviceModeObj),
        DECLARE_NAPI_PROPERTY("HearingAidConnectionState", connectionStateObj),
        DECLARE_NAPI_PROPERTY("HearingAidConnectionStrategy", connectionStrategyObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "hearingaid:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportProps) / sizeof(*exportProps), exportProps);
    return exports;
}

napi_value NapiHearingAid::On(napi_env env, napi_callback_info info)
{
    if (observer_) {
        HILOGI("NapiHearingAid::On");
        auto status = observer_->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_OV_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiHearingAid::Off(napi_env env, napi_callback_info info)
{
    if (observer_) {
        HILOGI("NapiHearingAid::Off");
        auto status = observer_->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_OV_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiHearingAid::IsLocalDeviceSupportHearingAid(napi_env env, napi_callback_info info)
{
    bool support = false;
    BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
    int32_t ret = profile->IsLocalDeviceSupportHearingAid(support);
    HILOGI("ret: %{public}d, support: %{public}d", ret, support);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_OV_NO_ERROR, ret);

    NapiNativeBool object(support);
    return object.ToNapiValue(env);
}

napi_value NapiHearingAid::Connect(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_OV_ERR_INVALID_PARAM);

    BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
    int32_t ret = profile->Connect(remoteAddr);
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_OV_NO_ERROR, ret);

    return NapiGetUndefinedRet(env);
}

napi_value NapiHearingAid::Disconnect(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRet, BT_OV_ERR_INVALID_PARAM);

    BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
    int32_t ret = profile->Disconnect(remoteAddr);
    HILOGI("ret: %{public}d", ret);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_OV_NO_ERROR, ret);

    return NapiGetUndefinedRet(env);
}

napi_status NapiHearingAid::CheckVolume(napi_env env, napi_callback_info info, std::string& addr, int64_t& volume)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Require 2 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_CALL_RETURN(NapiParseInt64(env, argv[PARAM1], volume));
    return napi_ok;
}

napi_value NapiHearingAid::SetVolume(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    int64_t volume = -1;
    auto status = CheckVolume(env, info, remoteAddr, volume);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_OV_ERR_INVALID_PARAM);

    BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
    int32_t ret = profile->SetVolume(remoteAddr, volume);
    HILOGI("ret: %{public}d, volume: %{public}lld", ret, volume);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_OV_NO_ERROR, ret);

    return NapiGetUndefinedRet(env);
}

void NapiHearingAid::ConvertDeviceInfoToJs(napi_env env, napi_value &object, int32_t side, int32_t mode, int64_t id)
{
    napi_value value = nullptr;

    napi_create_int32(env, side, &value);
    napi_set_named_property(env, object, "deviceSide", value);

    napi_create_int32(env, mode, &value);
    napi_set_named_property(env, object, "deviceMode", value);

    napi_create_int64(env, id, &value);
    napi_set_named_property(env, object, "hearingAidId", value);
}

napi_value NapiHearingAid::GetHearingAidDeviceInfo(napi_env env, napi_callback_info info)
{
    napi_value ret = nullptr;
    napi_create_object(env, &ret);

    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN(env, checkRet, BT_OV_ERR_INVALID_PARAM, ret);

    int32_t side;
    int32_t mode;
    int64_t id;
    BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
    int32_t errorCode = profile->GetHearingAidDeviceInfo(remoteAddr, side, mode, id);
    HILOGI("errorCode: %{public}d, side: %{public}d, mode: %{public}d, id: %{public}lld", errorCode, side, mode, id);
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_OV_NO_ERROR, errorCode, ret);

    ConvertDeviceInfoToJs(env, ret, side, mode, id);
    return ret;
}

napi_value NapiHearingAid::GetConnectedDevices(napi_env env, napi_callback_info info)
{
    napi_value ret = nullptr;
    napi_create_array(env, &ret);

    std::vector<std::string> connectedDevices;
    BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
    int32_t errorCode = profile->GetConnectedDevices(connectedDevices);
    HILOGI("errorCode: %{public}d, connectedDevices number: %{public}u", errorCode, connectedDevices.size());
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_OV_NO_ERROR, errorCode, ret);

    auto status = ConvertStringVectorToJS(env, ret, connectedDevices);
    NAPI_BT_ASSERT_RETURN(env, status == napi_ok, status, ret);
    return ret;
}

napi_value NapiHearingAid::GetConnectionState(napi_env env, napi_callback_info info)
{
    int32_t state = HearingAidConnectionState::CONNECTION_STATE_DISCONNECTED;
    napi_value ret = nullptr;
    napi_create_int32(env, state, &ret);

    std::string remoteAddr{};
    bool checkRet = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN(env, checkRet, BT_OV_ERR_INVALID_PARAM, ret);

    BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
    int32_t errorCode = profile->GetConnectionState(remoteAddr, state);
    HILOGI("errorCode: %{public}d, state: %{public}d", errorCode, state);
    NAPI_BT_ASSERT_RETURN(env, errorCode == BT_OV_NO_ERROR, errorCode, ret);

    return NapiGetInt32Ret(env, state);
}

napi_status NapiHearingAid::CheckSetConnectionStrategy(napi_env env, napi_callback_info info,
    std::string& addr, int32_t& strategy)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Require 2 arguments.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, argv[PARAM0], addr));
    NAPI_BT_CALL_RETURN(NapiParseInt32(env, argv[PARAM1], strategy));
    return napi_ok;
}

napi_value NapiHearingAid::SetConnectionStrategy(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    int32_t strategy = HearingAidConnectionStrategy::CONNECTION_STRATEGY_UNSUPPORTED;
    auto status = CheckSetConnectionStrategy(env, info, remoteAddr, strategy);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_OV_ERR_INVALID_PARAM);

    auto func = [remoteAddr, strategy]() {
        BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
        int32_t errorCode = profile->SetConnectionStrategy(remoteAddr, strategy);
        HILOGI("errorCode: %{public}d, strategy: %{public}d", errorCode, strategy);
        return NapiAsyncWorkRet(errorCode);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_OV_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiHearingAid::GetConnectionStrategy(napi_env env, napi_callback_info info)
{
    std::string remoteAddr{};
    auto status = CheckDeivceIdParam(env, info, remoteAddr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status, BT_OV_ERR_INVALID_PARAM);

    auto func = [remoteAddr]() {
        int32_t strategy = HearingAidConnectionStrategy::CONNECTION_STRATEGY_UNSUPPORTED;
        BluetoothHearingAid *profile = BluetoothHearingAid::GetProfile();
        int32_t err = profile->GetConnectionStrategy(remoteAddr, strategy);
        HILOGI("err: %{public}d, strategy: %{public}d", err, strategy);
        auto object = std::make_shared<NapiNativeInt>(strategy);
        return NapiAsyncWorkRet(err, object);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_OV_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

}  // namespace Bluetooth
}  // namespace OHOS
