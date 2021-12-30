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

#include "bluetooth_log.h"
#include "napi_bluetooth_gatt_client.h"
#include "napi_bluetooth_gatt_client_callback.h"

namespace OHOS {
namespace Bluetooth {
void NGattClientCallback::OnCharacteristicChanged(const GattCharacteristic &characteristic)
{
    HILOGI("NGattClientCallback::OnCharacteristicChanged called");

    if (!callbackInfos_[STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE]) {
        HILOGW("NGattClientCallback::OnCharacteristicChanged: This callback is not registered by ability.");
        return;
    }
    HILOGI("NGattClientCallback::OnCharacteristicChanged: %{public}s is registered by ability",
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE];

    napi_value result = nullptr;
    napi_create_object(callbackInfo->env_, &result);

    ConvertBLECharacteristicToJS(callbackInfo->env_, result, const_cast<GattCharacteristic &>(characteristic));
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(callbackInfo->env_, &undefined);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NGattClientCallback::OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("NGattClientCallback::OnCharacteristicChanged called");

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

void NGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("NGattClientCallback::OnDescriptorReadResult called");

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

void NGattClientCallback::OnConnectionStateChanged(int connectionState, int ret)
{
    HILOGI("NGattClientCallback::OnConnectionStateChanged called");

    if (!callbackInfos_[STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE]) {
        HILOGW("NGattClientCallback::OnConnectionStateChanged: This callback is not registered by ability.");
        return;
    }
    HILOGI("NGattClientCallback::OnConnectionStateChanged: %{public}s is registered by ability",
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE];

    napi_value result = nullptr;
    napi_create_object(callbackInfo->env_, &result);
    ConvertBLEConnectStateChangeToJS(callbackInfo->env_, result, connectionState, ret);

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(callbackInfo->env_, &undefined);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NGattClientCallback::OnServicesDiscovered(int status)
{
    HILOGI("NGattClientCallback::OnServicesDiscovered called");
    DiscoverServicesCallbackInfo *callbackInfo = client_->discoverServicesCallbackInfo_;
    if (!callbackInfo) {
        return;
    }
    if (callbackInfo->asyncState_ == ASYNC_START) {
        callbackInfo->status_ = status;
        callbackInfo->asyncState_ = ASYNC_DONE;
    }
}
} // namespace Bluetooth
} // namespace OHOS
