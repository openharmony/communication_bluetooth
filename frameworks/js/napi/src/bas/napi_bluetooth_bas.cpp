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
#define LOG_TAG "bt_napi_bas"
#endif

#include "napi_bluetooth_bas.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "napi_bluetooth_bas_callback.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_utils.h"
#include "napi_native_object.h"
#include "napi_async_callback.h"

namespace OHOS {
namespace Bluetooth {
namespace {
const std::shared_ptr<NapiBasCallback> g_basObserver =
    std::make_shared<NapiBasCallback>();
int32_t g_registerStatus = BT_NO_ERROR;
} //namespace

napi_value NapiBas::DefineBasJsFunction(napi_env env, napi_value exports)
{
    HILOGI("start");
    RegisterAccessObserverToHost();
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("isBasSupported", IsBasSupported),
        DECLARE_NAPI_FUNCTION("getRemoteDeviceBatteryInfo", GetRemoteDeviceBatteryInfo),
        DECLARE_NAPI_FUNCTION("onBatteryChange", OnBatteryChange),
        DECLARE_NAPI_FUNCTION("offBatteryChange", OffBatteryChange),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

void NapiBas::RegisterAccessObserverToHost()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    g_registerStatus = host->RegisterBatteryObserver(g_basObserver);
}

napi_value NapiBas::IsBasSupported(napi_env env, napi_callback_info info)
{
    bool isSupported = BluetoothHost::GetDefaultHost().IsBasSupported();
    napi_value result = nullptr;
    napi_get_boolean(env, isSupported, &result);
    return result;
}

napi_value NapiBas::GetRemoteDeviceBatteryInfo(napi_env env, napi_callback_info info)
{
    std::string deviceId;
    auto status = ParseGetRemoteDeviceBatteryInfoParams(env, info, deviceId);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status = napi_ok, BT_ERR_INVALID_PARAM);

    auto func = [deviceId]() {
        int32_t ret = BluetoothHost::GetDefaultHost().GetBatteryLevel(deviceId);
        return NapiAsyncWorkRet(ret);
    }

    auto aysncWork = NapiAsyncWorkFactory::CreateAysncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, aysncWork, BT_ERR_INTERNAL_ERROR);
    bool success = g_basObserver->asyncWorkMap.TryPush(NapiAsyncType::BAS_GET_BATTERY_LEVEL, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    aysncWork->Run();
    return aysncWork->GetRet();
}

napi_value NapiBas::ParseGetRemoteDeviceBatteryInfoParams(napi_env env, napi_callback_info info, std::string &deviceId)
{
    size_t expectedArgsCount = ARGS_SIZE_ONE;
    size argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_ONE] = {0};

    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != expectedArgsCount, "Requires 1 argumnent.", napi_invalid_arg);

    if (!ParseString(env, deviceId, argv[PARAM0])) {
        HILOGE("Parse deviceId failed");
        return napi_invalid_arg;
    }
    if (!IsValidAddress(deviceId)) {
        HILOGE("Invalid deviceId");
        return napi_invalid_arg;
    }
    return napi_ok;
}

napi_value NapiBas::OnBatteryChange(napi_env env, napi_callback_info info)
{
    NAPI_BT_ASSERT_RETURN_UNDEF(env, g_registerStatus == BT_NO_ERROR, g_registerStatus);
    auto status->g_basObserver->enventSubscribe_.RegisterWithName(env, info,
        STR_BT_BAS_CALLBACK_BATTERY_LEVLE_CHANGE);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INTERNAL_ERROR);
    auto batteryInfos = BluetoothHost::GetDefaultHost().GetConnectedDeviceBatteryInfos();
    const int32_t maxBatteryLevel = 100;
    for (const auto &[deviceAddr, batteryLevel] : batteryInfos) {
        if (batteryLevel < 0 || batteryLevel > maxBatteryLevel || !IsValidAddress(deviceAddr)) {
            continue;
        }
        BluetoothRemoteDevice remoteDevice(deviceAddr, BTTransport::ADAPTER_BLE);
        g_basObserver->OnBatteryLevelChanged(remoteDevice, batteryLevel);
    }
    return NapiGetUndefineRet(env);
}

napi_value NapiBas::OffBatteryChange(napi_env env, napi_callback_info info)
{
    NAPI_BT_ASSERT_RETURN_UNDEF(env, g_registerStatus == BT_NO_ERROR, g_registerStatus);
    auto status->g_basObserver->enventSubscribe_.DeregisterWithName(env, info,
        STR_BT_BAS_CALLBACK_BATTERY_LEVLE_CHANGE);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INTERNAL_ERROR);
    return NapiGetUndefineRet(env);
}
} // namespace Bluetooth
} // namespace OHOS