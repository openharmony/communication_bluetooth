/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_ble_scanner"
#endif

#include "napi_bluetooth_ble_scanner.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_error.h"

namespace OHOS {
namespace Bluetooth {

thread_local napi_ref NapiBleScanner::consRef_ = nullptr;

void NapiBleScanner::DefineBleScannerJSClass(napi_env env)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("startScan", StartScan),
        DECLARE_NAPI_FUNCTION("stopScan", StopScan),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
    };

    napi_value constructor = nullptr;
    napi_define_class(env, "BleScanner", NAPI_AUTO_LENGTH, BleScannerConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    napi_create_reference(env, constructor, 1, &consRef_);
}

napi_value NapiBleScanner::BleScannerConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    NapiBleScanner* bleScanner = new NapiBleScanner();

    auto status = napi_wrap(
        env, thisVar, bleScanner,
        [](napi_env env, void* data, void* hint) {
            NapiBleScanner* scanner = static_cast<NapiBleScanner*>(data);
            if (scanner) {
                delete scanner;
                scanner = nullptr;
            }
        },
        nullptr,
        nullptr);
    if (status != napi_ok) {
        HILOGE("napi_wrap failed");
        delete bleScanner;
        bleScanner = nullptr;
    }

    return thisVar;
}

napi_value NapiBleScanner::CreateBleScanner(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    napi_value result;
    napi_value constructor = nullptr;
    napi_get_reference_value(env, consRef_, &constructor);
    napi_new_instance(env, constructor, 0, nullptr, &result);
    return result;
}

static NapiBleScanner *NapiGetBleScanner(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr) != napi_ok) {
        return nullptr;
    }

    NapiBleScanner *bleScanner = nullptr;
    auto status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&bleScanner));
    if (status != napi_ok) {
        return nullptr;
    }
    return bleScanner;
}

napi_value NapiBleScanner::StartScan(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::vector<BleScanFilter> scanFilters;
    BleScanSettings settings;
    auto status = CheckBleScanParams(env, info, scanFilters, settings);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    NapiBleScanner *napiBleScanner = NapiGetBleScanner(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, napiBleScanner != nullptr, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, napiBleScanner->GetBleCentralManager() != nullptr, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, napiBleScanner->GetCallback() != nullptr, BT_ERR_INVALID_PARAM);

    auto func = [napiBleScanner, settings, scanFilters]() {
        int ret = napiBleScanner->GetBleCentralManager()->StartScan(settings, scanFilters);
        return NapiAsyncWorkRet(ret);
    };

    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success =
        napiBleScanner->GetCallback()->asyncWorkMap_.TryPush(NapiAsyncType::BLE_START_SCAN, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBleScanner::StopScan(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    auto status = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    NapiBleScanner *napiBleScanner = NapiGetBleScanner(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, napiBleScanner != nullptr, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, napiBleScanner->GetBleCentralManager() != nullptr, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, napiBleScanner->GetCallback() != nullptr, BT_ERR_INVALID_PARAM);

    auto func = [napiBleScanner]() {
        int ret = napiBleScanner->GetBleCentralManager()->StopScan();
        return NapiAsyncWorkRet(ret);
    };

    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
    bool success =
        napiBleScanner->GetCallback()->asyncWorkMap_.TryPush(NapiAsyncType::BLE_STOP_SCAN, asyncWork);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_value NapiBleScanner::On(napi_env env, napi_callback_info info)
{
    NapiBleScanner *napiBleScanner = NapiGetBleScanner(env, info);
    if (napiBleScanner && napiBleScanner->GetCallback()) {
        auto status = napiBleScanner->GetCallback()->eventSubscribe_.Register(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}

napi_value NapiBleScanner::Off(napi_env env, napi_callback_info info)
{
    NapiBleScanner *napiBleScanner = NapiGetBleScanner(env, info);
    if (napiBleScanner && napiBleScanner->GetCallback()) {
        auto status = napiBleScanner->GetCallback()->eventSubscribe_.Deregister(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    }
    return NapiGetUndefinedRet(env);
}
} // namespace Bluetooth
} // namespace OHOS