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
#include <uv.h>
#include "bluetooth_log.h"
#include "napi_bluetooth_gatt_client.h"
#include "napi_bluetooth_gatt_client_callback.h"

namespace OHOS {
namespace Bluetooth {
void NapiGattClientCallback::OnCharacteristicChanged(const GattCharacteristic &characteristic)
{
    HILOGI("NapiGattClientCallback::OnCharacteristicChanged called");
    if (!callbackInfos_[STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE]) {
        HILOGW("NapiGattClientCallback::OnCharacteristicChanged: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiGattClientCallback::OnCharacteristicChanged: %{public}s is registered by ability",
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE.c_str());
    std::shared_ptr<GattCharacteristicCallbackInfo> callbackInfo =
        std::static_pointer_cast<GattCharacteristicCallbackInfo>(
            callbackInfos_[STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE]);
    HILOGI("uuid is %{public}s", characteristic.GetUuid().ToString().c_str());
    callbackInfo->characteristic_ = characteristic;
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = (void*)callbackInfo.get();
    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            GattCharacteristicCallbackInfo *callbackInfo = (GattCharacteristicCallbackInfo *)work->data;
            napi_value result = nullptr;
            napi_create_object(callbackInfo->env_, &result);
            ConvertBLECharacteristicToJS(callbackInfo->env_, result, callbackInfo->characteristic_);
            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value callResult = nullptr;
            napi_get_undefined(callbackInfo->env_, &undefined);
            napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
            napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
            delete work;
            work = nullptr;
        }
    );
}

void NapiGattClientCallback::OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("NapiGattClientCallback::OnCharacteristicChanged called");
    ReadCharacteristicValueCallbackInfo *callbackInfo = client_->readCharacteristicValueCallbackInfo_;
    if (!callbackInfo) {
        return;
    }
    if (callbackInfo->asyncState_ == ASYNC_START) {
        callbackInfo->ret = ret;
        callbackInfo->outputCharacteristic_ = &characteristic;
        callbackInfo->asyncState_ = ASYNC_DONE;
    }
}

void NapiGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("NapiGattClientCallback::OnDescriptorReadResult called");
    ReadDescriptorValueCallbackInfo *callbackInfo = client_->readDescriptorValueCallbackInfo_;
    if (!callbackInfo) {
        return;
    }
    if (callbackInfo->asyncState_ == ASYNC_START) {
        callbackInfo->ret = ret;
        callbackInfo->outputDescriptor_ = &descriptor;
        callbackInfo->asyncState_ = ASYNC_DONE;
    }
}

void NapiGattClientCallback::OnConnectionStateChanged(int connectionState, int ret)
{
    HILOGI("NapiGattClientCallback::OnConnectionStateChanged called");
    if (!callbackInfos_[STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE]) {
        HILOGW("NapiGattClientCallback::OnConnectionStateChanged: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiGattClientCallback::OnConnectionStateChanged: %{public}s is registered by ability",
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE];

    callbackInfo->state_ = connectionState;
    callbackInfo->deviceId_ = client_->GetDevice()->GetDeviceAddr();
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = (void*)callbackInfo.get();

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            BluetoothCallbackInfo *callbackInfo = (BluetoothCallbackInfo *)work->data;
            napi_value result = nullptr;
            napi_create_object(callbackInfo->env_, &result);
            ConvertStateChangeParamToJS(callbackInfo->env_, result, callbackInfo->deviceId_, callbackInfo->state_);      
            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value callResult = nullptr;
            napi_get_undefined(callbackInfo->env_, &undefined);
            napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
            napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
            delete work;
            work = nullptr;
        }
    );
}

void NapiGattClientCallback::OnServicesDiscovered(int status)
{
    HILOGI("NapiGattClientCallback::OnServicesDiscovered called");
}
} // namespace Bluetooth
} // namespace OHOS
