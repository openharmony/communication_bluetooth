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
#define LOG_TAG "bt_napi_access"
#endif

#include "napi_bluetooth_access.h"

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"

#include "napi_bluetooth_error.h"
#include "napi_bluetooth_access_observer.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_spp_client.h"
#include "../parser/napi_parser_utils.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
namespace {
std::shared_ptr<NapiBluetoothAccessObserver> g_bluetoothAccessObserver =
    std::make_shared<NapiBluetoothAccessObserver>();
}  // namespace

napi_value NapiAccess::DefineAccessJSFunction(napi_env env, napi_value exports)
{
    HILOGD("enter");
    RegisterAccessObserverToHost();
    AccessPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getState", GetState),
        DECLARE_NAPI_FUNCTION("enableBluetooth", EnableBluetooth),
        DECLARE_NAPI_FUNCTION("disableBluetooth", DisableBluetooth),
        DECLARE_NAPI_FUNCTION("restrictBluetooth", RestrictBluetooth),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("factoryReset", FactoryReset),
        DECLARE_NAPI_FUNCTION("getLocalAddress", GetLocalAddress),
        DECLARE_NAPI_FUNCTION("on", RegisterAccessObserver),
        DECLARE_NAPI_FUNCTION("off", DeregisterAccessObserver),
#endif
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "access:napi_define_properties");
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

void NapiAccess::RegisterAccessObserverToHost()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    host->RegisterObserver(g_bluetoothAccessObserver);
}

napi_value NapiAccess::EnableBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t ret = host->EnableBle();
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value NapiAccess::RestrictBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto func = []() {
        int32_t ret = BluetoothHost::GetDefaultHost().RestrictBluetooth();
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiAccess::DisableBluetooth(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->DisableBt();
    NAPI_BT_ASSERT_RETURN_FALSE(env, ret == BT_NO_ERROR, ret);
    return NapiGetBooleanTrue(env);
}

napi_value NapiAccess::GetState(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    int32_t state = static_cast<int>(BluetoothHost::GetDefaultHost().GetBluetoothState());
    napi_value result = nullptr;
    napi_create_int32(env, state, &result);
    return result;
}

napi_value NapiAccess::AccessPropertyValueInit(napi_env env, napi_value exports)
{
    HILOGD("enter");
    napi_value stateObj = StateChangeInit(env);
    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("BluetoothState", stateObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "access:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}

napi_value NapiAccess::StateChangeInit(napi_env env)
{
    HILOGD("enter");
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

napi_value NapiAccess::RegisterAccessObserver(napi_env env, napi_callback_info info)
{
    if (g_bluetoothAccessObserver) {
        auto status = g_bluetoothAccessObserver->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiAccess::DeregisterAccessObserver(napi_env env, napi_callback_info info)
{
    if (g_bluetoothAccessObserver) {
        auto status = g_bluetoothAccessObserver->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

#ifdef BLUETOOTH_API_SINCE_10
napi_value NapiAccess::FactoryReset(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    auto func = []() {
        int32_t ret = BluetoothHost::GetDefaultHost().BluetoothFactoryReset();
        HILOGI("factoryReset ret: %{public}d", ret);
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiAccess::GetLocalAddress(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    HILOGI("enter");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    std::string localAddr = INVALID_MAC_ADDRESS;
    int32_t err = host->GetLocalAddress(localAddr);
    napi_create_string_utf8(env, localAddr.c_str(), localAddr.size(), &result);
    NAPI_BT_ASSERT_RETURN(env, err == BT_NO_ERROR, err, result);
    return result;
}
#endif
}  // namespace Bluetooth
}  // namespace OHOS
