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

#include "napi_bluetooth_access.h"

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"

#include "napi_bluetooth_error.h"
#include "napi_bluetooth_access_observer.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_spp_client.h"
#include "../parser/napi_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
namespace {
NapiBluetoothAccessObserver g_bluetoothAccessObserver;
}  // namespace

napi_value NapiAccess::DefineAccessJSFunction(napi_env env, napi_value exports)
{
    HILOGI("start");
    RegisterAccessObserverToHost();
    AccessPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getState", GetState),
        DECLARE_NAPI_FUNCTION("enableBluetooth", EnableBluetooth),
        DECLARE_NAPI_FUNCTION("disableBluetooth", DisableBluetooth),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("on", RegisterAccessObserver),
        DECLARE_NAPI_FUNCTION("off", DeregisterAccessObserver),
#endif
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    HILOGI("end");
    return exports;
}

void NapiAccess::RegisterAccessObserverToHost()
{
    HILOGI("start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    host->RegisterObserver(g_bluetoothAccessObserver);
}

napi_value NapiAccess::EnableBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->EnableBle();
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    if (ret == BT_NO_ERROR) {
        SetCurrentAppOperate(true);
    }
    return NapiGetBooleanTrue(env);
}

napi_value NapiAccess::DisableBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    // only ble
    int state = BTStateID::STATE_TURN_OFF;
    int ret = host->GetBtState(state);
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    if (state == BTStateID::STATE_TURN_OFF) {
        ret = host->DisableBle();
        NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    } else {
        ret = host->DisableBt();
        NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    }

    if (ret == BT_NO_ERROR) {
        SetCurrentAppOperate(true);
    }
    return NapiGetBooleanTrue(env);
}

napi_value NapiAccess::GetState(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t state = BTStateID::STATE_TURN_OFF;
    int32_t err = host->GetBtState(state);
    NAPI_BT_ASSERT_RETURN_FALSE(env, err == NO_ERROR, err);
    int32_t status = static_cast<int32_t>(BluetoothState::STATE_OFF);
    switch (state) {
        case BTStateID::STATE_TURNING_ON:
            HILOGI("STATE_TURNING_ON(1)");
            status = static_cast<int32_t>(BluetoothState::STATE_TURNING_ON);
            break;
        case BTStateID::STATE_TURN_ON:
            HILOGI("STATE_ON(2)");
            status = static_cast<int32_t>(BluetoothState::STATE_ON);
            break;
        case BTStateID::STATE_TURNING_OFF:
            HILOGI("STATE_TURNING_OFF(3)");
            status = static_cast<int32_t>(BluetoothState::STATE_TURNING_OFF);
            break;
        case BTStateID::STATE_TURN_OFF:
            HILOGI("STATE_OFF(0)");
            status = static_cast<int32_t>(BluetoothState::STATE_OFF);
            break;
        default:
            HILOGE("get state failed");
            break;
    }

    bool enableBle = host->IsBleEnabled();
    if (enableBle && (state == BTStateID::STATE_TURN_OFF)) {
        HILOGI("BR off and BLE on, STATE_BLE_ON(5)");
        status = static_cast<int32_t>(BluetoothState::STATE_BLE_ON);
    } else if (!enableBle && (state == BTStateID::STATE_TURN_OFF)) {
        status = static_cast<int32_t>(BluetoothState::STATE_OFF);
    }

    napi_value result = nullptr;
    napi_create_int32(env, status, &result);
    return result;
}

napi_value NapiAccess::AccessPropertyValueInit(napi_env env, napi_value exports)
{
    HILOGI("start");
    napi_value stateObj = StateChangeInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("BluetoothState", stateObj),
    };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    HILOGI("end");
    return exports;
}

napi_value NapiAccess::StateChangeInit(napi_env env)
{
    HILOGI("enter");
    napi_value state = nullptr;
    napi_create_object(env, &state);
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_OFF), "STATE_OFF");
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_TURNING_ON), "STATE_TURNING_ON");
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_ON), "STATE_ON");
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_TURNING_OFF), "STATE_TURNING_OFF");
    SetNamedPropertyByInteger(
        env, state, static_cast<int>(BluetoothState::STATE_BLE_TURNING_ON), "STATE_BLE_TURNING_ON");
    SetNamedPropertyByInteger(env, state, static_cast<int>(BluetoothState::STATE_BLE_ON), "STATE_BLE_ON");
    SetNamedPropertyByInteger(
        env, state, static_cast<int>(BluetoothState::STATE_BLE_TURNING_OFF), "STATE_BLE_TURNING_OFF");
    return state;
}

static bool IsValidObserverType(std::string &outType)
{
    return outType == REGISTER_STATE_CHANGE_TYPE;
}

static napi_status NapiParseObserverType(napi_env env, napi_value value, std::string &outType)
{
    std::string type{};
    NAPI_BT_CALL_RETURN(NapiParseString(env, value, type));
    NAPI_BT_RETURN_IF(!IsValidObserverType(type), "Invalid type", napi_invalid_arg);
    outType = std::move(type);
    return napi_ok;
}

static napi_status CheckAccessObserverParams(napi_env env, size_t argc, napi_value *argv, std::string &outType,
    std::shared_ptr<BluetoothCallbackInfo> &outpCallbackInfo)
{
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);
    std::string type;
    NAPI_BT_CALL_RETURN(NapiParseObserverType(env, argv[PARAM0], type));
    napi_valuetype valueType = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM1], &valueType));
    NAPI_BT_RETURN_IF(valueType != napi_function, "Requires a function argument", napi_function_expected);
    NAPI_BT_CALL_RETURN(napi_create_reference(env, argv[PARAM1], 1, &outpCallbackInfo->callback_));

    outType = type;
    outpCallbackInfo->env_ = env;
    return napi_ok;
}

napi_value NapiAccess::RegisterAccessObserver(napi_env env, napi_callback_info info)
{
    // fsafasfasf
    HILOGI("enter");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    HILOGI("argc: %{public}zu", argc);
    std::string type;
    std::shared_ptr<BluetoothCallbackInfo> pCallbackInfo = std::make_shared<BluetoothCallbackInfo>();
    auto status = CheckAccessObserverParams(env, argc, argv, type, pCallbackInfo);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    g_bluetoothAccessObserver.stateChangeCallback = pCallbackInfo;
    HILOGI("%{public}s is registered", type.c_str());
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

static napi_status CheckAccessDeregisterObserver(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    if (argc == ARGS_SIZE_ONE) {
        std::string type;
        NAPI_BT_CALL_RETURN(NapiParseObserverType(env, argv[PARAM0], type));
        g_bluetoothAccessObserver.stateChangeCallback = nullptr;
    } else {
        NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments.", napi_invalid_arg);
        std::string type;
        NAPI_BT_CALL_RETURN(NapiParseObserverType(env, argv[PARAM0], type));
        std::shared_ptr<BluetoothCallbackInfo> pCallbackInfo = std::make_shared<BluetoothCallbackInfo>();
        NAPI_BT_RETURN_IF(pCallbackInfo == nullptr, "pCallbackInfo is nullptr.", napi_invalid_arg);
        pCallbackInfo->env_ = env;
        NAPI_BT_CALL_RETURN(NapiIsFunction(env, argv[PARAM1]));
        NAPI_BT_CALL_RETURN(napi_create_reference(env, argv[PARAM1], 1, &pCallbackInfo->callback_));
        if (pCallbackInfo != nullptr) {
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(pCallbackInfo->env_, &undefined);
            napi_value result = nullptr;
            napi_create_int32(pCallbackInfo->env_, static_cast<int32_t>(BluetoothState::STATE_OFF), &result);
            napi_get_reference_value(pCallbackInfo->env_, pCallbackInfo->callback_, &callback);
            napi_call_function(pCallbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
        }
        g_bluetoothAccessObserver.stateChangeCallback = nullptr;
    }
    return napi_ok;
}

napi_value NapiAccess::DeregisterAccessObserver(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto status = CheckAccessDeregisterObserver(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}
}  // namespace Bluetooth
}  // namespace OHOS
