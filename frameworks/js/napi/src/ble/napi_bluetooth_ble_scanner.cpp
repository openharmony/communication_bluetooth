/*
 * Copyright (C) 2022-2024 Huawei Device Co., Ltd.
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

static NapiBleScanner *NapiGetBleScanner(napi_env env, napi_value thisVar)
{
    NapiBleScanner *bleScanner = nullptr;
    auto status = napi_unwrap(env, thisVar, (void**)&bleScanner);
    if (status != napi_ok) {
        return nullptr;
    }
    return bleScanner;
}

static NapiBleScanner *NapiGetBleScanner(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr) != napi_ok) {
        return nullptr;
    }
    return NapiGetBleScanner(env, thisVar);
}

static napi_status CheckBleScannerParams(napi_env env, napi_callback_info info,
    std::vector<BleScanFilter> &outScanFilters, BleScanSettings &outSettings,
    std::shared_ptr<BleCentralManager> &outBleCentralManager)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF((argc == 0 || argc > ARGS_SIZE_TWO), "Requires 1 or 2 arguments.", napi_invalid_arg);
    NapiBleScanner *napiBleScanner = NapiGetBleScanner(env, thisVar);
    NAPI_BT_RETURN_IF(napiBleScanner == nullptr, "napiBleScanner is nullptr", napi_invalid_arg);

    outBleCentralManager = napiBleScanner->GetBleCentralManager();

    std::vector<BleScanFilter> scanFilters;
    // Support null param
    napi_valuetype type = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM0], &type));
    if (type == napi_null) {
        BleScanFilter emptyFilter;
        scanFilters.push_back(emptyFilter);
    } else {
        NAPI_BT_CALL_RETURN(ParseScanFilterParameters(env, argv[PARAM0], scanFilters));
    }

    if (argc == ARGS_SIZE_TWO) {
        ScanOptions scanOptions;
        NAPI_BT_CALL_RETURN(ParseScanParameters(env, info, argv[PARAM1], scanOptions));
        outSettings.SetReportDelay(scanOptions.interval);
        outSettings.SetScanMode(static_cast<int32_t>(scanOptions.dutyMode));
        outSettings.SetPhy(static_cast<int32_t>(scanOptions.phyType));
    }

    outScanFilters = std::move(scanFilters);
    return napi_ok;
}

napi_value NapiBleScanner::StartScan(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<BleCentralManager> bleCentralMgr = nullptr;
    std::vector<BleScanFilter> scanFilters;
    BleScanSettings settings;
    auto status = CheckBleScannerParams(env, info, scanFilters, settings, bleCentralMgr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, bleCentralMgr != nullptr, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int ret = bleCentralMgr->StartScan(settings, scanFilters);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == NO_ERROR || ret == BT_ERR_BLE_SCAN_ALREADY_STARTED, ret);

    return NapiGetUndefinedRet(env);
}

static napi_status CheckBleScannerNoArgc(napi_env env, napi_callback_info info,
    std::shared_ptr<BleCentralManager> &outBleCentralManager)
{
    size_t argc = ARGS_SIZE_ZERO;
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr));
    NAPI_BT_RETURN_IF(argc != ARGS_SIZE_ZERO, "Requires 0 argument.", napi_invalid_arg);
    NapiBleScanner *napiBleScanner = NapiGetBleScanner(env, thisVar);
    NAPI_BT_RETURN_IF(napiBleScanner == nullptr, "napiBleScanner is nullptr", napi_invalid_arg);

    outBleCentralManager = napiBleScanner->GetBleCentralManager();
    return napi_ok;
}
napi_value NapiBleScanner::StopScan(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    std::shared_ptr<BleCentralManager> bleCentralMgr = nullptr;
    auto status = CheckBleScannerNoArgc(env, info, bleCentralMgr);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, bleCentralMgr != nullptr, BT_ERR_INVALID_PARAM);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int ret = bleCentralMgr->StopScan();
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == NO_ERROR, ret);

    return NapiGetUndefinedRet(env);
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