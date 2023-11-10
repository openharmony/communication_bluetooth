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
#include "napi_bluetooth_ble.h"

#include "napi_bluetooth_ble_advertise_callback.h"
#include "napi_bluetooth_ble_central_manager_callback.h"
#include "napi_bluetooth_error.h"
#include "napi_bluetooth_gatt_client.h"
#include "napi_bluetooth_gatt_server.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_ble_utils.h"

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_utils.h"
#include "../parser/napi_parser_utils.h"

#include <memory>
namespace OHOS {
namespace Bluetooth {
namespace {
struct SysStopBLEContext {
    napi_async_work work = nullptr;
    napi_ref callbackSuccess = nullptr;
    napi_ref callbackFail = nullptr;
    napi_ref callbackComplete = nullptr;
};

namespace {
BleAdvertiser *BleAdvertiserGetInstance(void)
{
    static BleAdvertiser instance;
    return &instance;
}

BleCentralManager *BleCentralManagerGetInstance(void)
{
    static BleCentralManager instance(NapiBluetoothBleCentralManagerCallback::GetInstance());
    return &instance;
}
}  // namespace {}

napi_value GetPropertyValueByNamed(napi_env env, napi_value object, std::string_view propertyName, napi_valuetype type)
{
    napi_value value = nullptr;
    bool hasProperty = false;
    napi_valuetype paraType = napi_undefined;

    NAPI_CALL(env, napi_has_named_property(env, object, propertyName.data(), &hasProperty));
    if (hasProperty) {
        NAPI_CALL(env, napi_get_named_property(env, object, propertyName.data(), &value));
        NAPI_CALL(env, napi_typeof(env, value, &paraType));
        if (paraType != type) {
            return NapiGetNull(env);
        }
    }
    return value;
}

void RegisterBLEObserver(napi_env env, napi_value val, int32_t callbackIndex, const std::string &type)
{
    std::shared_ptr<BluetoothCallbackInfo> pCallbackInfo = std::make_shared<BluetoothCallbackInfo>();
    pCallbackInfo->env_ = env;
    napi_create_reference(env, val, 1, &pCallbackInfo->callback_);
    RegisterSysBLEObserver(pCallbackInfo, callbackIndex, type);
}

bool ParseScanParameters(napi_env env, napi_value arg, ScanOptions &info)
{
    napi_value interval = GetPropertyValueByNamed(env, arg, "interval", napi_number);
    if (interval) {
        napi_get_value_int32(env, interval, &info.interval);
        HILOGI("Scan interval is %{public}d", info.interval);
    } else {
        info.interval = 0;
    }

    std::array<std::string, ARGS_SIZE_THREE> funcArray {"success", "fail", "complete"};

    for (size_t i = 0; i < funcArray.size(); i++) {
        napi_value value = GetPropertyValueByNamed(env, arg, funcArray[i], napi_function);
        if (value) {
            RegisterBLEObserver(env, value, i, REGISTER_SYS_BLE_SCAN_TYPE);
        } else {
            UnregisterSysBLEObserver(REGISTER_SYS_BLE_SCAN_TYPE);
            return false;
        }
    }
    return true;
}

napi_value SysStartBLEScan(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc != 1) {
        return NapiGetNull(env);
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valueType));
    if (valueType != napi_object) {
        return NapiGetNull(env);
    }
    ScanOptions scanOptions;
    if (!ParseScanParameters(env, argv[PARAM0], scanOptions)) {
        HILOGE("SysStartBLEScan Input parameter parsing failed!");
        return NapiGetNull(env);
    }

    BleScanSettings settinngs;
    settinngs.SetReportDelay(scanOptions.interval);
    settinngs.SetScanMode(static_cast<int32_t>(scanOptions.dutyMode));

    BleCentralManagerGetInstance()->StartScan(settinngs);
    return NapiGetNull(env);
}

void SysStopBLEScanExec(napi_env env, void *data)
{
    HILOGI("SysStopBLEScanExec");
    BleCentralManagerGetInstance()->StopScan();
    UnregisterSysBLEObserver(REGISTER_SYS_BLE_SCAN_TYPE);
}

void SysStopBLEScanComplete(napi_env env, napi_status status, void *data)
{
    NAPI_CALL_RETURN_VOID(env, (data == nullptr ? napi_invalid_arg : napi_ok));
    std::unique_ptr<SysStopBLEContext> context(static_cast<SysStopBLEContext *>(data));

    napi_value undefine = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefine));
    napi_value funcComplete = nullptr;
    napi_value funcSuccess = nullptr;
    napi_value callbackResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, context->callbackSuccess, &funcSuccess));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefine, funcSuccess, 0, nullptr, &callbackResult));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, context->callbackComplete, &funcComplete));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefine, funcComplete, 0, nullptr, &callbackResult));
    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, context->callbackSuccess));
    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, context->callbackComplete));
    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, context->callbackFail));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, context->work));
    HILOGI("SysStopBLEScanComplete end");
}

napi_value SysStopBLEScan(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc != 1) {
        return NapiGetNull(env);
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valueType));
    if (valueType != napi_object) {
        return NapiGetNull(env);
    }

    std::unique_ptr<SysStopBLEContext> context = std::make_unique<SysStopBLEContext>();

    std::array<std::string, ARGS_SIZE_THREE> funcArray {"success", "fail", "complete"};
    for (size_t i = 0; i < funcArray.size(); i++) {
        napi_value value = GetPropertyValueByNamed(env, argv[PARAM0], funcArray[i], napi_function);
        if (value) {
            napi_create_reference(env, value, 1,
                &(i == PARAM0 ? context->callbackSuccess :
                                (i == PARAM1 ? context->callbackFail : context->callbackComplete)));
        } else {
            HILOGE("SysStopBLEScan Input parameter parsing failed!");
            return NapiGetNull(env);
        }
    }

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "SysStopBLEScan", NAPI_AUTO_LENGTH, &resourceName));
    SysStopBLEContext *pContext = context.release();
    napi_status status = napi_create_async_work(env, nullptr, resourceName, SysStopBLEScanExec,
        SysStopBLEScanComplete, static_cast<void *>(pContext), &pContext->work);
    if (status != napi_ok) {
        delete pContext;
        return NapiGetNull(env);
    }

    if (napi_queue_async_work(env, pContext->work) != napi_ok) {
        delete pContext;
    }
    return NapiGetNull(env);
}

bool ParseDeviceFoundParameters(napi_env env, napi_value arg)
{
    std::array<std::string, ARGS_SIZE_TWO> funcArray {"success", "fail"};

    for (size_t i = 0; i < funcArray.size(); i++) {
        napi_value value = GetPropertyValueByNamed(env, arg, funcArray[i], napi_function);
        if (value) {
            RegisterBLEObserver(env, value, i, REGISTER_SYS_BLE_FIND_DEVICE_TYPE);
        } else {
            UnregisterSysBLEObserver(REGISTER_SYS_BLE_FIND_DEVICE_TYPE);
            return false;
        }
    }
    return true;
}

napi_value SysSubscribeBLEFound(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc != 1) {
        return NapiGetNull(env);
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valueType));
    if (valueType != napi_object) {
        return NapiGetNull(env);
    }
    if (!ParseDeviceFoundParameters(env, argv[PARAM0])) {
        HILOGE("SysSubscribeBLEFound Input parameter parsing failed!");
    }
    return NapiGetNull(env);
}

napi_value SysUnsubscribeBLEFound(napi_env env, napi_callback_info info)
{
    UnregisterSysBLEObserver(REGISTER_SYS_BLE_FIND_DEVICE_TYPE);
    return NapiGetNull(env);
}
} // namespace

void DefineSystemBLEInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("startBLEScan", SysStartBLEScan),
        DECLARE_NAPI_FUNCTION("stopBLEScan", SysStopBLEScan),
        DECLARE_NAPI_FUNCTION("subscribeBLEFound", SysSubscribeBLEFound),
        DECLARE_NAPI_FUNCTION("unsubscribeBLEFound", SysUnsubscribeBLEFound),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    HILOGI("DefineSystemBLEInterface init");
}

static napi_value On(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    auto CheckBleOnFunc = [env, info]() -> napi_status {
        size_t argc = ARGS_SIZE_TWO;
        napi_value argv[ARGS_SIZE_TWO] = {nullptr};
        napi_value thisVar = nullptr;
        NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
        NAPI_BT_RETURN_IF(argc != ARGS_SIZE_TWO, "Requires 2 arguments", napi_invalid_arg);

        std::string type {};
        NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], type));
        if (type == REGISTER_BLE_FIND_DEVICE_TYPE) {
            NAPI_BT_CALL_RETURN(NapiIsFunction(env, argv[PARAM1]));
            auto napiScanCallback = std::make_shared<NapiCallback>(env, argv[PARAM1]);
            NapiBluetoothBleCentralManagerCallback::GetInstance().SetNapiScanCallback(napiScanCallback);
        } else if (type == REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE) {
            NAPI_BT_CALL_RETURN(NapiIsFunction(env, argv[PARAM1]));
            auto napiAdvertisingStateCallback = std::make_shared<NapiCallback>(env, argv[PARAM1]);
            NapiBluetoothBleAdvertiseCallback::GetInstance().SetNapiAdvertisingStateCallback(
                napiAdvertisingStateCallback);
        } else {
            HILOGE("Invalid type: %{public}s", type.c_str());
            return napi_invalid_arg;
        }

        return napi_ok;
    };

    auto status = CheckBleOnFunc();
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}

static napi_value Off(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    auto CheckBleOffFunc = [env, info]() -> napi_status {
        size_t argc = ARGS_SIZE_TWO;
        napi_value argv[ARGS_SIZE_TWO] = {nullptr};
        napi_value thisVar = nullptr;
        NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
        NAPI_BT_RETURN_IF(
            argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO, "Requires 1 or 2 arguments", napi_invalid_arg);

        std::string type {};
        NAPI_BT_CALL_RETURN(NapiParseString(env, argv[PARAM0], type));
        if (type == REGISTER_BLE_FIND_DEVICE_TYPE) {
            NapiBluetoothBleCentralManagerCallback::GetInstance().SetNapiScanCallback(nullptr);
        } else if (type == REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE) {
            NapiBluetoothBleAdvertiseCallback::GetInstance().SetNapiAdvertisingStateCallback(nullptr);
        } else {
            HILOGE("Invalid type: %{public}s", type.c_str());
            return napi_invalid_arg;
        }
        return napi_ok;
    };

    auto status = CheckBleOffFunc();
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    return NapiGetUndefinedRet(env);
}

void DefineBLEJSObject(napi_env env, napi_value exports)
{
    HILOGD("enter");
    PropertyInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createGattServer", NapiGattServer::CreateGattServer),
        DECLARE_NAPI_FUNCTION("createGattClientDevice", NapiGattClient::CreateGattClientDevice),
        DECLARE_NAPI_FUNCTION("startBLEScan", StartBLEScan),
        DECLARE_NAPI_FUNCTION("stopBLEScan", StopBLEScan),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getConnectedBLEDevices", GetConnectedBLEDevices),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_FUNCTION("startAdvertising", StartAdvertising),
        DECLARE_NAPI_FUNCTION("stopAdvertising", StopAdvertising),
        DECLARE_NAPI_FUNCTION("enableAdvertising", EnableAdvertising),
        DECLARE_NAPI_FUNCTION("disableAdvertising", DisableAdvertising),
#endif
    };

#ifdef BLUETOOTH_API_SINCE_10
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
#else
    napi_value BLEObject = nullptr;
    napi_create_object(env, &BLEObject);
    napi_define_properties(env, BLEObject, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, "BLE", BLEObject);
#endif
}

static void ConvertMatchMode(ScanOptions &params, int32_t matchMode)
{
    switch (matchMode) {
        case MatchMode::MATCH_MODE_AGGRESSIVE:
            params.MatchMode = MatchMode::MATCH_MODE_AGGRESSIVE;
            break;
        case MatchMode::MATCH_MODE_STICKY:
            params.MatchMode = MatchMode::MATCH_MODE_STICKY;
            break;
        default:
            break;
    }
}

static void ConvertDutyMode(ScanOptions &params, int32_t dutyMode)
{
    switch (dutyMode) {
        case static_cast<int32_t>(ScanDuty::SCAN_MODE_LOW_POWER):
            params.dutyMode = ScanDuty::SCAN_MODE_LOW_POWER;
            break;
        case static_cast<int32_t>(ScanDuty::SCAN_MODE_BALANCED):
            params.dutyMode = ScanDuty::SCAN_MODE_BALANCED;
            break;
        case static_cast<int32_t>(ScanDuty::SCAN_MODE_LOW_LATENCY):
            params.dutyMode = ScanDuty::SCAN_MODE_LOW_LATENCY;
            break;
        default:
            break;
    }
}

static napi_status ParseScanParameters(
    const napi_env &env, const napi_callback_info &info, const napi_value &scanArg, ScanOptions &params)
{
    (void)info;
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, scanArg, {"interval", "dutyMode", "matchMode"}));

    bool exist = false;
    int32_t interval = 0;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, scanArg, "interval", exist, interval));
    if (exist) {
        HILOGI("Scan interval is %{public}d", interval);
        params.interval = interval;
    }

    int32_t dutyMode = 0;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, scanArg, "dutyMode", exist, dutyMode));
    if (exist) {
        HILOGI("Scan dutyMode is %{public}d", dutyMode);
        ConvertDutyMode(params, dutyMode);
    }

    int32_t matchMode = 0;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, scanArg, "matchMode", exist, matchMode));
    if (exist) {
        HILOGI("Scan matchMode is %{public}d", matchMode);
        ConvertMatchMode(params, matchMode);
    }
    return napi_ok;
}

static napi_status ParseScanFilterDeviceIdParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    std::string deviceId {};
    NAPI_BT_CALL_RETURN(ParseStringParams(env, scanFilter, "deviceId", exist, deviceId));
    if (exist) {
        if (!IsValidAddress(deviceId)) {
            HILOGE("Invalid deviceId: %{public}s", deviceId.c_str());
            return napi_invalid_arg;
        }

        HILOGI("Scan filter device id is %{public}s", GetEncryptAddr(deviceId).c_str());
        bleScanFilter.SetDeviceId(deviceId);
    }
    return napi_ok;
}

static napi_status ParseScanFilterLocalNameParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    std::string name {};
    NAPI_BT_CALL_RETURN(ParseStringParams(env, scanFilter, "name", exist, name));
    if (exist) {
        if (name.empty()) {
            HILOGE("name is empty");
            return napi_invalid_arg;
        }
        HILOGI("Scan filter name is %{public}s", name.c_str());
        bleScanFilter.SetName(name);
    }
    return napi_ok;
}

static napi_status ParseScanFilterServiceUuidParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    UUID uuid {};
    NAPI_BT_CALL_RETURN(ParseUuidParams(env, scanFilter, "serviceUuid", exist, uuid));
    if (exist) {
        HILOGI("Scan filter serviceUuid is %{public}s", uuid.ToString().c_str());
        bleScanFilter.SetServiceUuid(uuid);
    }

    UUID uuidMask {};
    NAPI_BT_CALL_RETURN(ParseUuidParams(env, scanFilter, "serviceUuidMask", exist, uuidMask));
    if (exist) {
        HILOGI("Scan filter serviceUuidMask is %{public}s", uuidMask.ToString().c_str());
        bleScanFilter.SetServiceUuidMask(uuidMask);
    }
    return napi_ok;
}

static napi_status ParseScanFilterSolicitationUuidParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    UUID uuid {};
    NAPI_BT_CALL_RETURN(ParseUuidParams(env, scanFilter, "serviceSolicitationUuid", exist, uuid));
    if (exist) {
        HILOGI("Scan filter serviceSolicitationUuid is %{public}s", uuid.ToString().c_str());
        bleScanFilter.SetServiceSolicitationUuid(uuid);
    }

    UUID uuidMask {};
    NAPI_BT_CALL_RETURN(ParseUuidParams(env, scanFilter, "serviceSolicitationUuidMask", exist, uuidMask));
    if (exist) {
        HILOGI("Scan filter serviceSolicitationUuidMask is %{public}s", uuidMask.ToString().c_str());
        bleScanFilter.SetServiceSolicitationUuidMask(uuidMask);
    }
    return napi_ok;
}

static napi_status ParseScanFilterServiceDataParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    std::vector<uint8_t> data {};
    NAPI_BT_CALL_RETURN(ParseArrayBufferParams(env, scanFilter, "serviceData", exist, data));
    if (exist) {
        bleScanFilter.SetServiceData(std::move(data));
    }

    std::vector<uint8_t> dataMask {};
    NAPI_BT_CALL_RETURN(ParseArrayBufferParams(env, scanFilter, "serviceDataMask", exist, dataMask));
    if (exist) {
        bleScanFilter.SetServiceDataMask(std::move(dataMask));
    }
    return napi_ok;
}

static napi_status ParseScanFilterManufactureDataParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    napi_value result;
    NAPI_BT_CALL_RETURN(ParseNumberParams(env, scanFilter, "manufactureId", exist, result));
    if (exist) {
        uint32_t manufacturerId = 0;
        NAPI_BT_CALL_RETURN(napi_get_value_uint32(env, result, &manufacturerId));
        bleScanFilter.SetManufacturerId(manufacturerId);
        HILOGI("Scan filter manufacturerId is %{public}#x", manufacturerId);
    }

    exist = false;
    std::vector<uint8_t> data {};
    NAPI_BT_CALL_RETURN(ParseArrayBufferParams(env, scanFilter, "manufactureData", exist, data));
    if (exist) {
        bleScanFilter.SetManufactureData(std::move(data));
    }

    std::vector<uint8_t> dataMask {};
    NAPI_BT_CALL_RETURN(ParseArrayBufferParams(env, scanFilter, "manufactureDataMask", exist, dataMask));
    if (exist) {
        bleScanFilter.SetManufactureDataMask(std::move(dataMask));
    }
    return napi_ok;
}

static napi_status ParseScanFilter(const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    HILOGD("enter");
    std::vector<std::string> expectedNames {"deviceId", "name", "serviceUuid", "serviceUuidMask",
        "serviceSolicitationUuid", "serviceSolicitationUuidMask", "serviceData", "serviceDataMask", "manufactureId",
        "manufactureData", "manufactureDataMask"};
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, scanFilter, expectedNames));

    NAPI_BT_CALL_RETURN(ParseScanFilterDeviceIdParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterLocalNameParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterServiceUuidParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterSolicitationUuidParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterServiceDataParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterManufactureDataParameters(env, scanFilter, bleScanFilter));
    return napi_ok;
}

static napi_status ParseScanFilterParameters(const napi_env &env, napi_value &args, std::vector<BleScanFilter> &params)
{
    HILOGD("enter");
    NAPI_BT_CALL_RETURN(NapiIsArray(env, args));

    uint32_t length = 0;
    NAPI_BT_CALL_RETURN(napi_get_array_length(env, args, &length));
    NAPI_BT_RETURN_IF(length == 0, "Requires array length > 0", napi_invalid_arg);
    for (uint32_t i = 0; i < length; i++) {
        napi_value scanFilter;
        NAPI_BT_CALL_RETURN(napi_get_element(env, args, i, &scanFilter));
        NAPI_BT_CALL_RETURN(NapiIsObject(env, scanFilter));
        BleScanFilter bleScanFilter;
        NAPI_BT_CALL_RETURN(ParseScanFilter(env, scanFilter, bleScanFilter));
        params.push_back(bleScanFilter);
    }
    return napi_ok;
}

static napi_status CheckBleScanParams(napi_env env, napi_callback_info info, std::vector<BleScanFilter> &outScanfilters,
    BleScanSettings &outSettinngs)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF((argc == 0 || argc > ARGS_SIZE_TWO), "Requires 1 or 2 arguments.", napi_invalid_arg);

    std::vector<BleScanFilter> scanfilters;
    // Support null param
    napi_valuetype type = napi_undefined;
    NAPI_BT_CALL_RETURN(napi_typeof(env, argv[PARAM0], &type));
    if (type == napi_null) {
        BleScanFilter emptyFilter;
        scanfilters.push_back(emptyFilter);
    } else {
        NAPI_BT_CALL_RETURN(ParseScanFilterParameters(env, argv[PARAM0], scanfilters));
    }

    if (argc == ARGS_SIZE_TWO) {
        ScanOptions scanOptions;
        NAPI_BT_CALL_RETURN(ParseScanParameters(env, info, argv[PARAM1], scanOptions));
        outSettinngs.SetReportDelay(scanOptions.interval);
        outSettinngs.SetScanMode(static_cast<int32_t>(scanOptions.dutyMode));
    }

    outScanfilters = std::move(scanfilters);
    return napi_ok;
}

napi_value StartBLEScan(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    std::vector<BleScanFilter> scanfilters;
    BleScanSettings settinngs;
    auto status = CheckBleScanParams(env, info, scanfilters, settinngs);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int ret = BleCentralManagerGetInstance()->ConfigScanFilter(scanfilters);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == NO_ERROR, ret);
    ret = BleCentralManagerGetInstance()->StartScan(settinngs);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == NO_ERROR, ret);

    return NapiGetUndefinedRet(env);
}

napi_value StopBLEScan(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    auto status = CheckEmptyParam(env, info);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

    int ret = BleCentralManagerGetInstance()->StopScan();
    NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == NO_ERROR, ret);
    return NapiGetUndefinedRet(env);
}

static napi_status ParseAdvertisingSettingsParameters(
    const napi_env &env, const napi_value &object, BleAdvertiserSettings &outSettings)
{
    HILOGD("enter");
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"interval", "txPower", "connectable"}));

    bool exist = false;
    uint32_t interval = 0;
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32Optional(env, object, "interval", interval, exist));
    if (exist) {
        const uint32_t minInterval = 32;
        const uint32_t maxInterval = 16384;
        if (interval < minInterval || interval > maxInterval) {
            HILOGE("Invalid interval: %{public}d", interval);
            return napi_invalid_arg;
        }
        HILOGI("interval: %{public}u", interval);
        outSettings.SetInterval(interval);
    }

    int32_t txPower = 0;
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32Optional(env, object, "txPower", txPower, exist));
    if (exist) {
        const int32_t minTxPower = -127;
        const int32_t maxTxPower = 1;
        if (txPower < minTxPower || txPower > maxTxPower) {
            HILOGE("Invalid tx power: %{public}d", txPower);
            return napi_invalid_arg;
        }
        HILOGI("txPower is %{public}d", txPower);
        outSettings.SetTxPower(txPower);
    }

    bool connectable = false;
    NAPI_BT_CALL_RETURN(NapiParseObjectBooleanOptional(env, object, "connectable", connectable, exist));
    if (exist) {
        HILOGI("connectable: %{public}d", connectable);
        outSettings.SetConnectable(connectable);
    }
    return napi_ok;
}

static napi_status ParseServiceUuidParameters(napi_env env, napi_value object, BleAdvertiserData &outData)
{
    HILOGD("enter");
    std::vector<UUID> vec {};
    NAPI_BT_CALL_RETURN(NapiParseObjectArray(env, object, "serviceUuids", vec));
    for (size_t i = 0; i < vec.size(); ++i) {
        outData.AddServiceUuid(vec[i]);
        HILOGI("Service Uuid = %{public}s", vec[i].ToString().c_str());
    }
    return napi_ok;
}

static napi_status ParseManufactureDataParameters(napi_env env, napi_value object, BleAdvertiserData &outData)
{
    HILOGD("enter");
    std::vector<NapiAdvManufactureData> vec {};
    NAPI_BT_CALL_RETURN(NapiParseObjectArray(env, object, "manufactureData", vec));
    for (size_t i = 0; i < vec.size(); ++i) {
        outData.AddManufacturerData(vec[i].id, vec[i].value);
    }
    return napi_ok;
}

static napi_status ParseServiceDataParameters(napi_env env, napi_value object, BleAdvertiserData &outData)
{
    HILOGD("enter");
    std::vector<NapiAdvServiceData> vec {};
    NAPI_BT_CALL_RETURN(NapiParseObjectArray(env, object, "serviceData", vec));
    for (size_t i = 0; i < vec.size(); ++i) {
        outData.AddServiceData(
            ParcelUuid::FromString(vec[i].uuid),
            std::string(vec[i].value.begin(), vec[i].value.end()));
    }
    return napi_ok;
}

static napi_status ParseAdvertisDataParameters(const napi_env &env,
    const napi_value &object, BleAdvertiserData &outData)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(
        env, object, {"serviceUuids", "manufactureData", "serviceData", "includeDeviceName", "includeTxPower"}));

    NAPI_BT_CALL_RETURN(ParseServiceUuidParameters(env, object, outData));
    NAPI_BT_CALL_RETURN(ParseManufactureDataParameters(env, object, outData));
    NAPI_BT_CALL_RETURN(ParseServiceDataParameters(env, object, outData));
    bool exist = false;
    bool includeDeviceName = false;
    NAPI_BT_CALL_RETURN(NapiParseObjectBooleanOptional(env, object, "includeDeviceName", includeDeviceName, exist));
    HILOGI("includeDeviceName: %{public}d", includeDeviceName);
    outData.SetIncludeDeviceName(includeDeviceName);

    bool includeTxPower = false;
    NAPI_BT_CALL_RETURN(NapiParseObjectBooleanOptional(env, object, "includeTxPower", includeTxPower, exist));
    HILOGI("includeTxPower: %{public}d", includeTxPower);
    outData.SetIncludeTxPower(includeTxPower);

    return napi_ok;
}

napi_status CheckAdvertisingDataWithDuration(napi_env env, napi_value object, BleAdvertiserSettings &outSettings,
    BleAdvertiserData &outAdvData, BleAdvertiserData &outRspData, uint16_t &outDuration)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"advertisingSettings", "advertisingData",
        "advertisingResponse", "duration"}));

    napi_value advSettingsObject;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, "advertisingSettings", advSettingsObject));
    BleAdvertiserSettings settings;
    NAPI_BT_CALL_RETURN(ParseAdvertisingSettingsParameters(env, advSettingsObject, settings));

    napi_value advDataObject;
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, "advertisingData", advDataObject));
    BleAdvertiserData advData;
    NAPI_BT_CALL_RETURN(ParseAdvertisDataParameters(env, advDataObject, advData));

    BleAdvertiserData advRsp;
    if (NapiIsObjectPropertyExist(env, object, "advertisingResponse")) {
        napi_value advRspObject;
        NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, "advertisingResponse", advRspObject));
        NAPI_BT_CALL_RETURN(ParseAdvertisDataParameters(env, advRspObject, advRsp));
    }

    bool exist = false;
    uint32_t duration = 0;
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32Optional(env, object, "duration", duration, exist));
    if (exist) {
        const uint32_t minDuration = 0;
        const uint32_t maxDuration = 65535;
        if (duration < minDuration || duration > maxDuration) {
            HILOGE("Invalid duration: %{public}d", duration);
            return napi_invalid_arg;
        }
        HILOGI("duration: %{public}u", duration);
    }
    outDuration = duration;
    outSettings = std::move(settings);
    outAdvData = std::move(advData);
    outRspData = std::move(advRsp);
    return napi_ok;
}

napi_status CheckAdvertisingData(napi_env env, napi_callback_info info, BleAdvertiserSettings &outSettings,
    BleAdvertiserData &outAdvData, BleAdvertiserData &outRspData)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_TWO && argc != ARGS_SIZE_THREE), "need 2 or 3 arguments.", napi_invalid_arg);

    BleAdvertiserSettings settings;
    NAPI_BT_CALL_RETURN(ParseAdvertisingSettingsParameters(env, argv[PARAM0], settings));
    BleAdvertiserData advData;
    NAPI_BT_CALL_RETURN(ParseAdvertisDataParameters(env, argv[PARAM1], advData));
    BleAdvertiserData rspData;
    if (argc == ARGS_SIZE_THREE) {
        NAPI_BT_CALL_RETURN(ParseAdvertisDataParameters(env, argv[PARAM2], rspData));
    }

    outSettings = std::move(settings);
    outAdvData = std::move(advData);
    outRspData = std::move(rspData);
    return napi_ok;
}

napi_value StartAdvertising(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisVar = nullptr;
    auto checkRes = napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRes == napi_ok, BT_ERR_INVALID_PARAM);

    BleAdvertiserSettings settings;
    BleAdvertiserData advData;
    BleAdvertiserData rspData;
    if (argc != 0 && NapiIsObjectPropertyExist(env, argv[PARAM0], "advertisingSettings")) {
        uint16_t duration = 0;
        auto status = CheckAdvertisingDataWithDuration(env, argv[PARAM0], settings, advData, rspData, duration);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
        auto func = [settings, advData, rspData, duration]() {
            BleAdvertiser *bleAdvertiser = BleAdvertiserGetInstance();
            if (bleAdvertiser == nullptr) {
                HILOGE("bleAdvertiser is nullptr");
                return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
            }
            int ret = bleAdvertiser->StartAdvertising(
                settings, advData, rspData, duration, NapiBluetoothBleAdvertiseCallback::GetInstance());
            return NapiAsyncWorkRet(ret);
        };

        auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NEED_CALLBACK);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);
        bool success = NapiBluetoothBleAdvertiseCallback::GetInstance().asyncWorkMap_.TryPush(
            NapiAsyncType::GET_ADVERTISING_HANDLE, asyncWork);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, success, BT_ERR_INTERNAL_ERROR);

        asyncWork->Run();
        return asyncWork->GetRet();
    } else {
        auto status = CheckAdvertisingData(env, info, settings, advData, rspData);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
        int ret = BleAdvertiserGetInstance()->StartAdvertising(
            settings, advData, rspData, 0, NapiBluetoothBleAdvertiseCallback::GetInstance());
        NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_NO_ERROR, ret);
        return NapiGetUndefinedRet(env);
    }
}

#ifdef BLUETOOTH_API_SINCE_10
napi_status CheckAdvertisingEnableParams(napi_env env, napi_callback_info info,
    uint32_t &outAdvHandle, uint16_t &outDuration)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO), "need 1 or 2 arguments.", napi_invalid_arg);

    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, argv[PARAM0], {"advertisingId", "duration"}));
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32(env, argv[PARAM0], "advertisingId", outAdvHandle));
    if (outAdvHandle != BleAdvertiserGetInstance()->GetAdvHandle(NapiBluetoothBleAdvertiseCallback::GetInstance())) {
        HILOGE("Invalid outAdvHandle: %{public}d", outAdvHandle);
        return napi_invalid_arg;
    }

    bool exist = false;
    uint32_t duration = 0;
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32Optional(env, argv[PARAM0], "duration", duration, exist));
    if (exist) {
        const uint32_t minDuration = 0;
        const uint32_t maxDuration = 65535;
        if (duration < minDuration || duration > maxDuration) {
            HILOGE("Invalid duration: %{public}d", duration);
            return napi_invalid_arg;
        }
        HILOGI("duration: %{public}u", duration);
    }
    outDuration = duration;

    return napi_ok;
}

napi_value EnableAdvertising(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    uint32_t advHandle = 0xFF;
    uint16_t duration = 0;
    auto status = CheckAdvertisingEnableParams(env, info, advHandle, duration);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    auto func = [advHandle, duration]() {
        BleAdvertiser *bleAdvertiser = BleAdvertiserGetInstance();
        if (bleAdvertiser == nullptr) {
            HILOGE("bleAdvertiser is nullptr");
            return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = bleAdvertiser->EnableAdvertising(
            advHandle, duration, NapiBluetoothBleAdvertiseCallback::GetInstance());
        return NapiAsyncWorkRet(ret);
    };

    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}

napi_status CheckAdvertisingDisableParams(napi_env env, napi_callback_info info, uint32_t &outAdvHandle)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_BT_RETURN_IF((argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO), "need 1 or 2 arguments.", napi_invalid_arg);

    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, argv[PARAM0], {"advertisingId"}));
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32(env, argv[PARAM0], "advertisingId", outAdvHandle));
    if (outAdvHandle != BleAdvertiserGetInstance()->GetAdvHandle(NapiBluetoothBleAdvertiseCallback::GetInstance())) {
        HILOGE("Invalid outAdvHandle: %{public}d", outAdvHandle);
        return napi_invalid_arg;
    }

    return napi_ok;
}

napi_value DisableAdvertising(napi_env env, napi_callback_info info)
{
    HILOGI("enter");
    uint32_t advHandle = 0xFF;
    auto status = CheckAdvertisingDisableParams(env, info, advHandle);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
    auto func = [advHandle]() {
        BleAdvertiser *bleAdvertiser = BleAdvertiserGetInstance();
        if (bleAdvertiser == nullptr) {
            HILOGE("bleAdvertiser is nullptr");
            return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = bleAdvertiser->DisableAdvertising(
            advHandle, NapiBluetoothBleAdvertiseCallback::GetInstance());
        return NapiAsyncWorkRet(ret);
    };

    auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);

    asyncWork->Run();
    return asyncWork->GetRet();
}
#endif

napi_status CheckStopAdvWithAdvId(napi_env env, napi_value object, uint32_t &outAdvHandle)
{
    NAPI_BT_CALL_RETURN(NapiParseUint32(env, object, outAdvHandle));
    if (outAdvHandle != BleAdvertiserGetInstance()->GetAdvHandle(NapiBluetoothBleAdvertiseCallback::GetInstance())) {
        HILOGE("Invalid outAdvHandle: %{public}d", outAdvHandle);
        return napi_invalid_arg;
    }
    return napi_ok;
}

static napi_status CheckEmptyArgs(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value thisVar = nullptr;
    NAPI_BT_CALL_RETURN(napi_get_cb_info(env, info, &argc, nullptr, &thisVar, NULL));
    NAPI_BT_RETURN_IF(argc > 0, "no needed arguments.", napi_invalid_arg);
    return napi_ok;
}

napi_value StopAdvertising(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    auto checkRes = napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL);
    NAPI_BT_ASSERT_RETURN_UNDEF(env, checkRes == napi_ok, BT_ERR_INVALID_PARAM);

    if (argc != ARGS_SIZE_ZERO) {
        auto status = napi_ok;
        if (argc != ARGS_SIZE_ONE && argc != ARGS_SIZE_TWO) {
            status = napi_invalid_arg;
        }
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
        uint32_t advHandle = 0xFF;
        status = CheckStopAdvWithAdvId(env, argv[PARAM0], advHandle);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);
        auto func = []() {
            BleAdvertiser *bleAdvertiser = BleAdvertiserGetInstance();
            if (bleAdvertiser == nullptr) {
                HILOGE("bleAdvertiser is nullptr");
                return NapiAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
            }
            int ret = bleAdvertiser->StopAdvertising(
                NapiBluetoothBleAdvertiseCallback::GetInstance());
            return NapiAsyncWorkRet(ret);
        };

        auto asyncWork = NapiAsyncWorkFactory::CreateAsyncWork(env, info, func, ASYNC_WORK_NO_NEED_CALLBACK);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, asyncWork, BT_ERR_INTERNAL_ERROR);

        asyncWork->Run();
        return asyncWork->GetRet();
    } else {
        auto status = CheckEmptyArgs(env, info);
        NAPI_BT_ASSERT_RETURN_UNDEF(env, status == napi_ok, BT_ERR_INVALID_PARAM);

        int ret = BleAdvertiserGetInstance()->StopAdvertising(NapiBluetoothBleAdvertiseCallback::GetInstance());
        NAPI_BT_ASSERT_RETURN_UNDEF(env, ret == BT_NO_ERROR, ret);
        return NapiGetUndefinedRet(env);
    }
}

napi_value GetConnectedBLEDevices(napi_env env, napi_callback_info info)
{
    HILOGD("enter");
    napi_value result = nullptr;
    napi_create_array(env, &result);

    auto status = ConvertStringVectorToJS(env, result, NapiGattServer::deviceList);
    NAPI_BT_ASSERT_RETURN(env, status == napi_ok, BT_ERR_INTERNAL_ERROR, result);
    return result;
}

napi_value PropertyInit(napi_env env, napi_value exports)
{
    HILOGD("enter");

    napi_value matchModeObj = nullptr;
    napi_value scanDutyObj = nullptr;
    napi_create_object(env, &matchModeObj);
    napi_create_object(env, &scanDutyObj);

    SetNamedPropertyByInteger(env, matchModeObj, MatchMode::MATCH_MODE_STICKY, "MATCH_MODE_STICKY");
    SetNamedPropertyByInteger(env, matchModeObj, MatchMode::MATCH_MODE_AGGRESSIVE, "MATCH_MODE_AGGRESSIVE");
    SetNamedPropertyByInteger(
        env, scanDutyObj, static_cast<int32_t>(ScanDuty::SCAN_MODE_BALANCED), "SCAN_MODE_BALANCED");
    SetNamedPropertyByInteger(
        env, scanDutyObj, static_cast<int32_t>(ScanDuty::SCAN_MODE_LOW_LATENCY), "SCAN_MODE_LOW_LATENCY");
    SetNamedPropertyByInteger(
        env, scanDutyObj, static_cast<int32_t>(ScanDuty::SCAN_MODE_LOW_POWER), "SCAN_MODE_LOW_POWER");

#ifdef BLUETOOTH_API_SINCE_10
    napi_value gattWriteTypeObj = nullptr;
    napi_create_object(env, &gattWriteTypeObj);
    SetNamedPropertyByInteger(env, gattWriteTypeObj, static_cast<int32_t>(NapiGattWriteType::WRITE), "WRITE");
    SetNamedPropertyByInteger(
        env, gattWriteTypeObj, static_cast<int32_t>(NapiGattWriteType::WRITE_NO_RESPONSE), "WRITE_NO_RESPONSE");
#endif

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("MatchMode", matchModeObj),
        DECLARE_NAPI_PROPERTY("ScanDuty", scanDutyObj),
#ifdef BLUETOOTH_API_SINCE_10
        DECLARE_NAPI_PROPERTY("GattWriteType", gattWriteTypeObj),
#endif
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);

    return exports;
}
}  // namespace Bluetooth
}  // namespace OHOS
