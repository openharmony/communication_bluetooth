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
#include "napi_bluetooth_ble_central_manager_callback.h"

#include "bluetooth_log.h"
#include "napi_bluetooth_utils.h"
#include "securec.h"

namespace OHOS {
namespace Bluetooth {
void NapiBluetoothBleCentralManagerCallback::UvQueueWorkOnScanCallback(
    uv_work_t *work, std::shared_ptr<BleScanResult> &result)
{
    HILOGI("OnScanCallback uv_work_t start");

    if (work == nullptr) {
        HILOGE("work is null");
        return;
    }
    auto callbackData = (AfterWorkCallbackData<NapiBluetoothBleCentralManagerCallback,
        decltype(&NapiBluetoothBleCentralManagerCallback::UvQueueWorkOnScanCallback),
        std::shared_ptr<BleScanResult>> *)work->data;
    if (callbackData == nullptr) {
        HILOGE("callbackData is null");
        return;
    }
    napi_value napiResult = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackData->env, &undefined);
    std::vector<BleScanResult> results;
    results.push_back(*result);
    ConvertScanResult(results, callbackData->env, napiResult);
    napi_get_reference_value(callbackData->env, callbackData->callback, &callback);
    napi_call_function(callbackData->env, undefined, callback, ARGS_SIZE_ONE, &napiResult, &callResult);
}

void NapiBluetoothBleCentralManagerCallback::OnScanCallback(const BleScanResult &result)
{
    HILOGI("NapiBluetoothBleCentralManagerCallback::OnScanCallback called");
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[REGISTER_BLE_FIND_DEVICE_TYPE]) {
        HILOGE("NapiBluetoothHostObserver::OnStateChanged: This callback is not registered by ability.");
        return;
    }
    HILOGD("NapiBluetoothHostObserver::OnStateChanged: %{public}s is registered by ability",
        REGISTER_BLE_FIND_DEVICE_TYPE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observers[REGISTER_BLE_FIND_DEVICE_TYPE];
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    if (loop == nullptr) {
        HILOGE("loop instance is nullptr");
        return;
    }

    auto callbackData = new (std::nothrow) AfterWorkCallbackData<NapiBluetoothBleCentralManagerCallback,
        decltype(&NapiBluetoothBleCentralManagerCallback::UvQueueWorkOnScanCallback),
        std::shared_ptr<BleScanResult>>();
    if (callbackData == nullptr) {
        HILOGE("new callbackData failed");
        return;
    }

    callbackData->object = this;
    callbackData->function = &NapiBluetoothBleCentralManagerCallback::UvQueueWorkOnScanCallback;
    callbackData->env = callbackInfo->env_;
    callbackData->callback = callbackInfo->callback_;
    callbackData->data = std::make_shared<BleScanResult>(result);

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("new work failed");
        delete callbackData;
        callbackData = nullptr;
        return;
    }

    work->data = (void *)callbackData;

    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, AfterWorkCallback<decltype(callbackData)>);
    if (ret != 0) {
        delete callbackData;
        callbackData = nullptr;
        delete work;
        work = nullptr;
    }
}

void NapiBluetoothBleCentralManagerCallback::UvQueueWorkOnBleBatchScanResultsEvent(
    uv_work_t *work, const std::vector<BleScanResult> &results)
{
    HILOGI("OnBleBatchScanResultsEvent uv_work_t start");

    if (work == nullptr) {
        HILOGE("work is null");
        return;
    }
    auto callbackData = (AfterWorkCallbackData<NapiBluetoothBleCentralManagerCallback,
        decltype(&NapiBluetoothBleCentralManagerCallback::UvQueueWorkOnBleBatchScanResultsEvent),
        std::vector<BleScanResult>> *)work->data;
    if (callbackData == nullptr) {
        HILOGE("callbackData is null");
        return;
    }

    napi_value result = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;
    napi_get_undefined(callbackData->env, &undefined);
    ConvertScanResult(results, callbackData->env, result);
    napi_get_reference_value(callbackData->env, callbackData->callback, &callback);
    napi_call_function(callbackData->env, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NapiBluetoothBleCentralManagerCallback::OnBleBatchScanResultsEvent(const std::vector<BleScanResult> &results)
{
    HILOGI("NapiBluetoothBleCentralManagerCallback::OnBleBatchScanResultsEvent called");
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> observers = GetObserver();
    if (!observers[REGISTER_BLE_FIND_DEVICE_TYPE]) {
        HILOGE("NapiBluetoothHostObserver::OnStateChanged: This callback is not registered by ability.");
        return;
    }
    HILOGD("NapiBluetoothHostObserver::OnStateChanged: %{public}s is registered by ability",
        REGISTER_BLE_FIND_DEVICE_TYPE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = observers[REGISTER_BLE_FIND_DEVICE_TYPE];
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    if (loop == nullptr) {
        HILOGE("loop instance is nullptr");
        return;
    }

    auto callbackData = new (std::nothrow) AfterWorkCallbackData<NapiBluetoothBleCentralManagerCallback,
        decltype(&NapiBluetoothBleCentralManagerCallback::UvQueueWorkOnBleBatchScanResultsEvent),
        std::vector<BleScanResult>>();
    if (callbackData == nullptr) {
        HILOGE("new callbackData failed");
        return;
    }

    callbackData->object = this;
    callbackData->function = &NapiBluetoothBleCentralManagerCallback::UvQueueWorkOnBleBatchScanResultsEvent;
    callbackData->env = callbackInfo->env_;
    callbackData->callback = callbackInfo->callback_;
    callbackData->data = results;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("new work failed");
        delete callbackData;
        callbackData = nullptr;
        return;
    }

    work->data = (void *)callbackData;

    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, AfterWorkCallback<decltype(callbackData)>);
    if (ret != 0) {
        delete callbackData;
        callbackData = nullptr;
        delete work;
        work = nullptr;
    }
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
        napi_create_string_utf8(
            env, bleScanResult.GetPeripheralDevice().GetDeviceAddr().c_str(), NAPI_AUTO_LENGTH, &value);
        napi_set_named_property(env, result, "deviceId", value);
        napi_create_int32(env, bleScanResult.GetRssi(), &value);
        napi_set_named_property(env, result, "rssi", value);
        uint8_t *native = nullptr;
        napi_value buffer = nullptr;
        napi_create_arraybuffer(env, bleScanResult.GetPayload().size(), reinterpret_cast<void **>(&native), &buffer);
        if (memcpy_s(native,
            bleScanResult.GetPayload().size(),
            bleScanResult.GetPayload().data(),
            bleScanResult.GetPayload().size()) != EOK) {
                HILOGE("NapiBluetoothBleCentralManagerCallback::ConvertScanResult memcpy_s fail");
                return;
            }
        napi_create_typedarray(env, napi_uint8_array, bleScanResult.GetPayload().size(), buffer, 0, &value);
        napi_set_named_property(env, result, "data", value);
        napi_set_element(env, scanResultArray, count, result);
        ++count;
    }
    HILOGI("NapiBluetoothBleCentralManagerCallback::ConvertScanResult called end");
}
}  // namespace Bluetooth
}  // namespace OHOS