/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "napi_bluetooth_gatt_client.h"
#include "napi_bluetooth_gatt_server.h"
#include "napi_bluetooth_utils.h"

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"

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

NapiBluetoothBleCentralManagerCallback bleCentralMangerCallback;
NapiBluetoothBleAdvertiseCallback bleAdvertiseCallback;
BleAdvertiser bleAdvertiser;
std::unique_ptr<BleCentralManager> bleCentralManager = std::make_unique<BleCentralManager>(bleCentralMangerCallback);

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

    bleCentralManager->StartScan(settinngs);
    return NapiGetNull(env);
}
void SysStopBLEScanExec(napi_env env, void *data)
{
    HILOGI("SysStopBLEScanExec");
    bleCentralManager->StopScan();
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
void DefineBLEJSObject(napi_env env, napi_value exports)
{
    HILOGI("DefineBLEJSObject start");
    napi_value BLEObject = nullptr;
    PropertyInit(env, exports);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createGattServer", NapiGattServer::CreateGattServer),
        DECLARE_NAPI_FUNCTION("createGattClientDevice", NapiGattClient::CreateGattClientDevice),
        DECLARE_NAPI_FUNCTION("startBLEScan", StartBLEScan),
        DECLARE_NAPI_FUNCTION("stopBLEScan", StopBLEScan),
        DECLARE_NAPI_FUNCTION("on", RegisterObserver),
        DECLARE_NAPI_FUNCTION("off", DeregisterObserver),
        DECLARE_NAPI_FUNCTION("getConnectedBLEDevices", GetConnectedBLEDevices),
    };

    napi_create_object(env, &BLEObject);
    napi_define_properties(env, BLEObject, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, "BLE", BLEObject);
    HILOGI("DefineBLEJSObject end");
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

static napi_value ParseScanParameters(
    const napi_env &env, const napi_callback_info &info, const napi_value &scanArg, ScanOptions &params)
{
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc == 0) {
        return NapiGetNull(env);
    }

    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;

    NAPI_CALL(env, napi_has_named_property(env, scanArg, "interval", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, scanArg, "interval", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, result, &params.interval);
        HILOGD("StartBLEScan interval is %{public}d", params.interval);
    }

    NAPI_CALL(env, napi_has_named_property(env, scanArg, "dutyMode", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, scanArg, "dutyMode", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        int32_t dutyMode = 0;
        napi_get_value_int32(env, result, &dutyMode);
        HILOGD("StartBLEScan dutyMode is %{public}d", dutyMode);
        ConvertDutyMode(params, dutyMode);
    }

    NAPI_CALL(env, napi_has_named_property(env, scanArg, "matchMode", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, scanArg, "matchMode", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        int32_t matchMode = -1;
        napi_get_value_int32(env, result, &matchMode);
        HILOGD("StartBLEScan matchMode is %{public}d", matchMode);
        ConvertMatchMode(params, matchMode);
    }
    return NapiGetNull(env);
}

static napi_value ParseScanFilterParameters(const napi_env &env, napi_value &args)
{
    if (args == nullptr) {
        return NapiGetNull(env);
    }

    bool isArray = false;
    napi_is_array(env, args, &isArray);
    if (isArray) {
        uint32_t length = 0;
        napi_get_array_length(env, args, &length);
        for (size_t i = 0; i < length; ++i) {
            napi_value scanFilter;
            napi_value result;
            napi_valuetype valuetype = napi_undefined;
            napi_get_element(env, args, i, &scanFilter);
            NAPI_CALL(env, napi_typeof(env, scanFilter, &valuetype));
            NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
            bool hasProperty = false;
            NAPI_CALL(env, napi_has_named_property(env, scanFilter, "deviceId", &hasProperty));
            if (hasProperty) {
                napi_get_named_property(env, scanFilter, "deviceId", &result);
                std::string deviceId;
                ParseString(env, deviceId, result);
                HILOGD("ParseScanFilterParameters::deviceId = %{public}s", deviceId.c_str());
            }

            NAPI_CALL(env, napi_has_named_property(env, scanFilter, "name", &hasProperty));
            if (hasProperty) {
                napi_get_named_property(env, scanFilter, "name", &result);
                std::string name;
                ParseString(env, name, result);
                HILOGD("ParseScanFilterParameters::name = %{public}s", name.c_str());
            }

            NAPI_CALL(env, napi_has_named_property(env, scanFilter, "serviceUuid", &hasProperty));
            if (hasProperty) {
                napi_get_named_property(env, scanFilter, "serviceUuid", &result);
                std::string serviceUuid;
                ParseString(env, serviceUuid, result);
                HILOGD("ParseScanFilterParameters::serviceUuid = %{public}s", serviceUuid.c_str());
            }
        }
    }

    return NapiGetNull(env);
}

napi_value StartBLEScan(napi_env env, napi_callback_info info)
{
    HILOGI("StartBLEScan start");
    size_t argc = ARGS_SIZE_TWO;
    napi_value argv[ARGS_SIZE_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc == 0) {
        return NapiGetNull(env);
    }

    ParseScanFilterParameters(env, argv[PARAM0]);

    BleScanSettings settinngs;
    if (argv[PARAM1] != nullptr) {
        ScanOptions scanOptions;
        ParseScanParameters(env, info, argv[PARAM1], scanOptions);
        settinngs.SetReportDelay(scanOptions.interval);
        settinngs.SetScanMode(static_cast<int32_t>(scanOptions.dutyMode));
    }

    bleCentralManager->StartScan(settinngs);
    HILOGI("StartBLEScan end");
    return NapiGetNull(env);
}

napi_value StopBLEScan(napi_env env, napi_callback_info info)
{
    HILOGI("StopBLEScan start");
    bleCentralManager->StopScan();
    HILOGI("StopBLEScan end");
    return NapiGetNull(env);
}

static napi_value ParseAdvertisingSettingsParameters(
    const napi_env &env, const napi_callback_info &info, const napi_value &args, BleAdvertiserSettings &settings)
{
    HILOGI("ParseAdvertisingSettingsParameters start");
    bool hasProperty = false;
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;

    NAPI_CALL(env, napi_has_named_property(env, args, "interval", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, args, "interval", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        int32_t interval = 0;
        napi_get_value_int32(env, result, &interval);
        HILOGD("ParseAdvertisingSettingsParameters interval is %{public}d", interval);
        settings.SetInterval(interval);
    }
    NAPI_CALL(env, napi_has_named_property(env, args, "txPower", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, args, "txPower", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        int32_t txPower = 0;
        napi_get_value_int32(env, result, &txPower);
        HILOGD("ParseAdvertisingSettingsParameters txPower is %{public}d", txPower);
        settings.SetTxPower(txPower);
    }

    NAPI_CALL(env, napi_has_named_property(env, args, "connectable", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, args, "connectable", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_boolean, "Wrong argument type. Boolean expected.");
        bool connectable = true;
        napi_get_value_bool(env, result, &connectable);
        HILOGD("ParseAdvertisingSettingsParameters connectable is %{public}d", connectable);
        settings.SetConnectable(connectable);
    }
    HILOGI("ParseAdvertisingSettingsParameters end");
    return NapiGetNull(env);
}

static napi_value ParseServiceUuidParameters(const napi_env &env, const napi_value &args, BleAdvertiserData &data)
{
    HILOGI("ParseServiceUuidParameters start");
    napi_value result = nullptr;

    napi_get_named_property(env, args, "serviceUuids", &result);
    bool isArray = false;
    napi_is_array(env, result, &isArray);
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");
    uint32_t length = 0;
    napi_get_array_length(env, result, &length);
    NAPI_ASSERT(env, length > 0, "The array is empty.");
    for (size_t serviceUuidIter = 0; serviceUuidIter < length; ++serviceUuidIter) {
        napi_value uuid = nullptr;
        napi_get_element(env, result, serviceUuidIter, &uuid);
        std::string serviceUuid;
        ParseString(env, serviceUuid, uuid);
        data.AddServiceUuid(ParcelUuid::FromString(serviceUuid));
        HILOGD("Service Uuid = %{public}s", serviceUuid.c_str());
    }
    HILOGI("ParseServiceUuidParameters end");
    return NapiGetNull(env);
}

static napi_value ParseManufactureDataParameters(const napi_env &env, const napi_value &args, BleAdvertiserData &data)
{
    HILOGI("ParseManufactureDataParameters start");
    napi_value result = nullptr;

    napi_get_named_property(env, args, "manufactureData", &result);
    bool isArray = false;
    napi_is_array(env, result, &isArray);
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");
    uint32_t length = 0;
    napi_get_array_length(env, result, &length);
    if (length == 0) {
        HILOGE("ParseManufactureDataParameters,The array is empty.");
        return NapiGetNull(env);
    }
    for (size_t manufactureDataIter = 0; manufactureDataIter < length; ++manufactureDataIter) {
        napi_value manufactureData = nullptr;
        napi_valuetype valuetype = napi_undefined;
        napi_get_element(env, result, manufactureDataIter, &manufactureData);
        NAPI_CALL(env, napi_typeof(env, manufactureData, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        bool hasProperty = false;
        NAPI_CALL(env, napi_has_named_property(env, manufactureData, "manufactureId", &hasProperty));
        NAPI_ASSERT(env, hasProperty, "manufactureId expected.");
        napi_get_named_property(env, manufactureData, "manufactureId", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");
        int32_t manufactureId = 0;
        napi_get_value_int32(env, result, &manufactureId);
        HILOGD("ParseManufactureDataParameters::manufactureId = %{public}d", manufactureId);
        NAPI_CALL(env, napi_has_named_property(env, manufactureData, "manufactureValue", &hasProperty));
        NAPI_ASSERT(env, hasProperty, "manufactureValue expected.");
        napi_get_named_property(env, manufactureData, "manufactureValue", &result);
        bool isArrayBuffer = false;
        napi_is_arraybuffer(env, result, &isArrayBuffer);
        if (isArrayBuffer) {
            uint8_t *arrayBufferData = nullptr;
            size_t arrayBufferTotal = 0;
            ParseArrayBuffer(env, &arrayBufferData, arrayBufferTotal, result);
            std::string manuData(arrayBufferData, arrayBufferData + arrayBufferTotal);
            data.AddManufacturerData(manufactureId, manuData);
        }
    }
    HILOGI("ParseManufactureDataParameters end");
    return NapiGetNull(env);
}

static napi_value ParseServiceDataParameters(const napi_env &env, const napi_value &args, BleAdvertiserData &data)
{
    HILOGI("ParseServiceDataParameters start");
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    napi_get_named_property(env, args, "serviceData", &result);
    bool isArray = false;
    napi_is_array(env, result, &isArray);
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");
    uint32_t length = 0;
    napi_get_array_length(env, result, &length);
    if (length == 0) {
        HILOGE("ParseServiceDataParameters,The array is empty.");
        return NapiGetNull(env);
    }
    for (size_t serviceDataIter = 0; serviceDataIter < length; ++serviceDataIter) {
        napi_value serviceData = nullptr;
        napi_get_element(env, result, serviceDataIter, &serviceData);
        NAPI_CALL(env, napi_typeof(env, serviceData, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        bool hasProperty = false;
        NAPI_CALL(env, napi_has_named_property(env, serviceData, "serviceUuid", &hasProperty));
        NAPI_ASSERT(env, hasProperty, "serviceUuid expected.");
        napi_get_named_property(env, serviceData, "serviceUuid", &result);
        std::string serviceBuffer;
        ParseString(env, serviceBuffer, result);
        ParcelUuid serviceUuid(ParcelUuid::FromString(serviceBuffer));
        HILOGD("ParseManufactureDataParameters::serviceUuid = %{public}s", serviceUuid.ToString().c_str());

        NAPI_CALL(env, napi_has_named_property(env, serviceData, "serviceValue", &hasProperty));
        NAPI_ASSERT(env, hasProperty, "serviceValue expected.");
        napi_get_named_property(env, serviceData, "serviceValue", &result);
        bool isArrayBuffer = false;
        napi_is_arraybuffer(env, result, &isArrayBuffer);
        NAPI_ASSERT(env, isArrayBuffer, "Array buffer expected.");
        uint8_t *arrayBufferData;
        size_t arrayBufferTotal;
        napi_get_arraybuffer_info(env, result, reinterpret_cast<void **>(&arrayBufferData), &arrayBufferTotal);
        std::string serviceDataStr(arrayBufferData, arrayBufferData + arrayBufferTotal);
        data.AddServiceData(serviceUuid, serviceDataStr);
    }
    HILOGI("ParseServiceDataParameters end");
    return NapiGetNull(env);
}

static napi_value ParseAdvertisDataParameters(
    const napi_env &env, const napi_callback_info &info, const napi_value &args, BleAdvertiserData &data)
{
    HILOGI("ParseAdvertisDataParameters start");
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, args, "serviceUuids", &hasProperty));
    if (hasProperty) {
        ParseServiceUuidParameters(env, args, data);
    }
    NAPI_CALL(env, napi_has_named_property(env, args, "manufactureData", &hasProperty));
    if (hasProperty) {
        ParseManufactureDataParameters(env, args, data);
    }
    NAPI_CALL(env, napi_has_named_property(env, args, "serviceData", &hasProperty));
    if (hasProperty) {
        ParseServiceDataParameters(env, args, data);
    }
    HILOGI("ParseAdvertisDataParameters end");
    return NapiGetNull(env);
}

napi_value StartAdvertising(napi_env env, napi_callback_info info)
{
    HILOGI("StartAdvertising start");
    size_t argc = ARGS_SIZE_THREE;
    napi_value argv[ARGS_SIZE_THREE] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc == 0) {
        return NapiGetNull(env);
    }
    BleAdvertiserSettings settings;
    ParseAdvertisingSettingsParameters(env, info, argv[PARAM0], settings);
    BleAdvertiserData AdvData;
    ParseAdvertisDataParameters(env, info, argv[PARAM1], AdvData);
    BleAdvertiserData ScanRespData;
    if (argv[PARAM2] != nullptr) {
        ParseAdvertisDataParameters(env, info, argv[PARAM2], ScanRespData);
    }

    bleAdvertiser.StartAdvertising(settings, AdvData, ScanRespData, bleAdvertiseCallback);
    HILOGI("StartAdvertising end");
    return NapiGetNull(env);
}

napi_value StopAdvertising(napi_env env, napi_callback_info info)
{
    HILOGI("StopAdvertising start");
    bleAdvertiser.StopAdvertising(bleAdvertiseCallback);
    HILOGI("StopAdvertising end");
    return NapiGetNull(env);
}

napi_value GetConnectedBLEDevices(napi_env env, napi_callback_info info)
{
    HILOGI("GetConnectedBLEDevices start");
    napi_value result = nullptr;
    napi_create_array(env, &result);

    ConvertStringVectorToJS(env, result, NapiGattServer::deviceList);
    HILOGI("GetConnectedBLEDevices end");
    return result;
}

napi_value PropertyInit(napi_env env, napi_value exports)
{
    HILOGI("%{public}s, enter", __func__);

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

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("MatchMode", matchModeObj),
        DECLARE_NAPI_PROPERTY("ScanDuty", scanDutyObj),
    };

    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);
    return exports;
}
}  // namespace Bluetooth
}  // namespace OHOS