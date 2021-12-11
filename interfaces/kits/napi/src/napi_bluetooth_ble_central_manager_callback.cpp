/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "napi_bluetooth_ble_central_manager_callback.h"

#include <cstring>
#include <map>
#include <memory>
#include <string>

#include "bluetooth_log.h"
#include "napi_bluetooth_utils.h"
#include "securec.h"

namespace OHOS {
namespace Bluetooth {
void NapiBluetoothBleCentralManagerCallback::OnScanCallback(const BleScanResult &result)
{
    HILOGI("NapiBluetoothBleCentralManagerCallback::OnScanCallback called");
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[REGISTER_BLE_FIND_DEVICE_TYPE]) {
        HILOGW("NapiBluetoothHostObserver::OnStateChanged: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiBluetoothHostObserver::OnStateChanged: %{public}s is registered by ability",
        REGISTER_BLE_FIND_DEVICE_TYPE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observers[REGISTER_BLE_FIND_DEVICE_TYPE];
    napi_value napiResult = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackInfo->env_, &undefined);
    std::vector<BleScanResult> results;
    results.push_back(result);
    ConvertScanResult(results, callbackInfo->env_, napiResult);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &napiResult, &callResult);
}

void NapiBluetoothBleCentralManagerCallback::OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results)
{
    HILOGI("NapiBluetoothBleCentralManagerCallback::OnBleBatchScanResultsEvent called");
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[REGISTER_BLE_FIND_DEVICE_TYPE]) {
        HILOGW("NapiBluetoothHostObserver::OnStateChanged: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiBluetoothHostObserver::OnStateChanged: %{public}s is registered by ability",
        REGISTER_BLE_FIND_DEVICE_TYPE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observers[REGISTER_BLE_FIND_DEVICE_TYPE];
    napi_value result = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackInfo->env_, &undefined);
    ConvertScanResult(results, callbackInfo->env_, result);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NapiBluetoothBleCentralManagerCallback::OnStartScanFailed(int resultCode)
{
    HILOGI("NapiBluetoothBleCentralManagerCallback::OnStartScanFailed called, resultCode is %{public}d", resultCode);
}

void NapiBluetoothBleCentralManagerCallback::ConvertScanResult(
    const std::vector<BleScanResult> &results, const napi_env &env, napi_value &scanResultArray)
{
    HILOGI("NapiBluetoothBleCentralManagerCallback::ConvertScanResult called");
    napi_create_array(env, &scanResultArray);
    size_t count = 0;
    for (auto bleScanResult : results) {
        napi_value result = nullptr;
        napi_value value = nullptr;
        napi_create_object(env, &result);
        // deviceId?: string
        napi_create_string_utf8(
            env, bleScanResult.GetPeripheralDevice().GetDeviceAddr().c_str(), NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, result, "deviceId", value);
        // rssi?: number
        napi_create_int32(env, bleScanResult.GetRssi(), &value);
        napi_set_named_property(env, result, "rssi", value);
        // data?: ArrayBuffer
        uint8_t *native = nullptr;
        napi_value buffer = nullptr;
        napi_create_arraybuffer(env, bleScanResult.GetPayload().size(), reinterpret_cast<void **>(&native), &buffer);
        memcpy_s(native,
            bleScanResult.GetPayload().size(),
            bleScanResult.GetPayload().data(),
            bleScanResult.GetPayload().size());
        napi_create_typedarray(env, napi_uint8_array, bleScanResult.GetPayload().size(), buffer, 0, &value);
        napi_set_named_property(env, result, "data", value);
        napi_set_element(env, scanResultArray, count, result);
        ++count;
    }
    HILOGI("NapiBluetoothBleCentralManagerCallback::ConvertScanResult called end");
}
}  // namespace Bluetooth
}  // namespace OHOS