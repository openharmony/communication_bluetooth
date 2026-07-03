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
#include "bluetooth_bas_host.h"
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
const std::string ACCESS_BLUETOOTH = "ohos.permission.ACCESS_BLUETOOTH";
} //namespace

napi_value NapiBas::DefineBasJSFunction(napi_env env, napi_value exports)
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
    BluetoothBasHost::GetProfile()->RegisterBatteryObserver(g_basObserver);
}

napi_value NapiBas::IsBasSupported(napi_env env, napi_callback_info info)
{
    //since 26.0.0
    std::vector<int32_t> validErrCodes = {
        BT_ERR_SYSTEM_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
        BT_ERR_INTERNAL_ERROR
    };
    NAPI_BT_CONTEXT(env, "bas.IsBasSupported", validErrCodes);
    bool isSupported = false;
    int32_t ret = BluetoothBasHost::GetProfile()->IsBasSupported(isSupported);
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, ret == BT_NO_ERROR, ret);
    napi_value result = nullptr;
    napi_get_boolean(env, isSupported, &result);
    return result;
}

napi_value NapiBas::GetRemoteDeviceBatteryInfo(napi_env env, napi_callback_info info)
{
    //since 26.0.0
    std::vector<int32_t> validErrCodes = {
        BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
        BT_ERR_API_NOT_SUPPORT, BT_ERR_SERVICE_DISCONNECTED, BT_ERR_INVALID_STATE,
        BT_ERR_PROFILE_DISABLED, BT_ERR_INTERNAL_ERROR, BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED
    };
    NAPI_BT_CONTEXT(env, "bas.GetRemoteDeviceBatteryInfo", validErrCodes);
    BluetoothAddress deviceId;
    auto status = ParseGetRemoteDeviceBatteryInfoParams(env, info, deviceId);
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int32_t ret = BluetoothBasHost::GetProfile()->GetBatteryLevel(deviceId.address);
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, ret == BT_NO_ERROR, ret);
    auto func = [ret]() {
        return NapiAsyncWorkRet(ret);
    };
    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success = g_basObserver->asyncWorkMap_.TryPush(NapiAsyncType::BAS_GET_BATTERY_LEVEL, asyncWork);
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_status NapiBas::ParseGetRemoteDeviceBatteryInfoParams(napi_env env, napi_callback_info info,
    BluetoothAddress &deviceId)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {0};
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ONE, "Requires 1 argument.", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, argv[PARAM0], {"address", "addressType", "rawAddressType"}));

    bool exist = false;
    NAPI_BT_CALL_RETURN(ParseStringParams(env, argv[PARAM0], "address", exist, deviceId.address));
    if (!exist || !IsValidAddress(deviceId.address)) {
        HILOGE("Invalid address");
        return napi_invalid_arg;
    }

    int32_t addressType = AddressType::UNSET_ADDRESS;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, argv[PARAM0], "addressType", exist, addressType));
    if (!exist) {
        HILOGE("addressType not exist");
        return napi_invalid_arg;
    }
    deviceId.addressType = static_cast<uint8_t>(addressType);

    int32_t rawAddressType = RawAddressType::UNSET_RAW_ADDRESS;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, argv[PARAM0], "rawAddressType", exist, rawAddressType));
    if (!exist) {
        HILOGE("rawAddressType not exist");
        return napi_invalid_arg;
    }
    deviceId.rawAddressType = static_cast<uint8_t>(rawAddressType);
    return napi_ok;
}

napi_value NapiBas::OnBatteryChange(napi_env env, napi_callback_info info)
{
    //since 26.0.0
    std::vector<int32_t> validErrCodes = {
        BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
        BT_ERR_API_NOT_SUPPORT, BT_ERR_INTERNAL_ERROR
    };
    NAPI_BT_CONTEXT(env, "bas.OnBatteryChange", validErrCodes);
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    auto status = g_basObserver->eventSubscribe_.RegisterWithName(env, info,
        STR_BT_BAS_CALLBACK_BATTERY_LEVEL_CHANGE);
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, status == napi_ok, BT_ERR_INTERNAL_ERROR);
    std::map<std::string, int32_t> batteryInfos;
    int32_t ret = BluetoothBasHost::GetProfile()->GetConnectedDeviceBatteryInfos(batteryInfos);
    if (ret != BT_NO_ERROR) {
        HILOGE("GetConnectedDeviceBatteryInfos failed, ret: %{public}d", ret);
    }
    const int32_t maxBatteryLevel = 100;
    for (const auto &[deviceAddr, batteryLevel] : batteryInfos) {
        if (batteryLevel < 0 || batteryLevel > maxBatteryLevel || !IsValidAddress(deviceAddr)) {
            continue;
        }
        BluetoothRemoteDevice remoteDevice(deviceAddr, BTTransport::ADAPTER_BLE);
        g_basObserver->OnBatteryLevelChanged(remoteDevice, batteryLevel);
    }
#else
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, false, BT_ERR_API_NOT_SUPPORT);
#endif
    return NapiGetUndefinedRet(env);
}

napi_value NapiBas::OffBatteryChange(napi_env env, napi_callback_info info)
{
    //since 26.0.0
    std::vector<int32_t> validErrCodes = {
        BT_ERR_PERMISSION_FAILED, BT_ERR_SYSTEM_PERMISSION_FAILED, BT_ERR_INVALID_PARAM,
        BT_ERR_API_NOT_SUPPORT, BT_ERR_INTERNAL_ERROR
    };
    NAPI_BT_CONTEXT(env, "bas.OffBatteryChange", validErrCodes);
#ifdef BLUETOOTH_BAS_FEATURE_ENABLE
    auto status = g_basObserver->eventSubscribe_.DeregisterWithName(env, info,
        STR_BT_BAS_CALLBACK_BATTERY_LEVEL_CHANGE);
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, status == napi_ok, BT_ERR_INTERNAL_ERROR);
#else
    NAPI_BT_ASSERT_ERR_NUM_RETURN_VERIFY(env, false, BT_ERR_API_NOT_SUPPORT);
#endif
    return NapiGetUndefinedRet(env);
}
} // namespace Bluetooth
} // namespace OHOS