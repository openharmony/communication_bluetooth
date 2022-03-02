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
#include <uv.h>
#include "bluetooth_log.h"
#include "napi_bluetooth_gatt_server.h"
#include "napi_bluetooth_gatt_server_callback.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

void NapiGattServerCallback::OnCharacteristicReadRequest(
    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
{
    HILOGI("NapiGattServerCallback::OnCharacteristicReadRequest called");
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ]) {
        HILOGW("NapiGattServerCallback::OnCharacteristicReadRequest: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiGattServerCallback::OnCharacteristicReadRequest: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ.c_str());
    std::shared_ptr<GattCharacteristicCallbackInfo> callbackInfo = 
        std::static_pointer_cast<GattCharacteristicCallbackInfo>(callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ]);
    
    callbackInfo->info_ = requestId;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
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
            ConvertCharacteristicReadReqToJS(callbackInfo->env_, result, callbackInfo->deviceId_,
                callbackInfo->characteristic_, callbackInfo->info_);

            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value callResult = nullptr;
            napi_get_undefined(callbackInfo->env_, &undefined);
            napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
            napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
        }
    );
}

void NapiGattServerCallback::OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId)
{
    HILOGI("NapiGattServerCallback::OnCharacteristicWriteRequest called");
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE]) {
        HILOGW("NapiGattServerCallback::OnCharacteristicWriteRequest: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiGattServerCallback::OnCharacteristicWriteRequest: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE.c_str());

    std::shared_ptr<GattCharacteristicCallbackInfo> callbackInfo = 
        std::static_pointer_cast<GattCharacteristicCallbackInfo>(callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE]);
    
    callbackInfo->info_ = requestId;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
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
            ConvertCharacteristicWriteReqToJS(callbackInfo->env_, result, callbackInfo->deviceId_, 
                callbackInfo->characteristic_, callbackInfo->info_);

            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value callResult = nullptr;
            napi_get_undefined(callbackInfo->env_, &undefined);
            napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
            napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
        }
    );
}


void NapiGattServerCallback::OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
{
    HILOGI("NapiGattServerCallback::OnConnectionStateUpdate called");

    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("NapiGattServerCallback::OnConnectionStateUpdate connected");
        bool hasAddr = false;
        for (auto it = NapiGattServer::deviceList.begin(); it != NapiGattServer::deviceList.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                hasAddr = true;
                break;
            }
        }
        if (!hasAddr) {
            HILOGI("NapiGattServerCallback::OnConnectionStateUpdate add devices");
            NapiGattServer::deviceList.push_back(device.GetDeviceAddr());
        }
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("NapiGattServerCallback::OnConnectionStateUpdate disconnected");
        for (auto it = NapiGattServer::deviceList.begin(); it != NapiGattServer::deviceList.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                HILOGI("NapiGattServerCallback::OnConnectionStateUpdate romove devices start");
                NapiGattServer::deviceList.erase(it);
                break;
                HILOGI("NapiGattServerCallback::OnConnectionStateUpdate romove devices end");
            }
        }
    }
    
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE]) {
        HILOGW("NapiGattServerCallback::OnConnectionStateUpdate: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiGattServerCallback::OnConnectionStateUpdate: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE];

    callbackInfo->state_ = state;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
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
        }
    );
}

void NapiGattServerCallback::OnDescriptorWriteRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("NapiGattServerCallback::OnDescriptorWriteRequest called");
    
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE]) {
        HILOGW("NapiGattServerCallback::OnDescriptorWriteRequest: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiGattServerCallback::OnDescriptorWriteRequest: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE.c_str());
    std::shared_ptr<GattDescriptorCallbackInfo> callbackInfo =
        std::static_pointer_cast<GattDescriptorCallbackInfo>(callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE]);
    
    callbackInfo->info_ = requestId;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
    callbackInfo->descriptor_ = descriptor;
    
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = (void*)callbackInfo.get();

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            GattDescriptorCallbackInfo *callbackInfo = (GattDescriptorCallbackInfo *)work->data;
            napi_value result = nullptr;
            napi_create_object(callbackInfo->env_, &result);
            ConvertDescriptorWriteReqToJS(callbackInfo->env_, result, callbackInfo->deviceId_, 
                callbackInfo->descriptor_, callbackInfo->info_);

            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value callResult = nullptr;
            napi_get_undefined(callbackInfo->env_, &undefined);
            napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
            napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
        }
    );
}

void NapiGattServerCallback::OnDescriptorReadRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("NapiGattServerCallback::OnDescriptorReadRequest called");
    
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ]) {
        HILOGW("NapiGattServerCallback::OnDescriptorReadRequest: This callback is not registered by ability.");
        return;
    }
    HILOGI("NapiGattServerCallback::OnDescriptorReadRequest: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ.c_str());
   std::shared_ptr<GattDescriptorCallbackInfo> callbackInfo =
        std::static_pointer_cast<GattDescriptorCallbackInfo>(callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ]);
    
    callbackInfo->info_ = requestId;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
    callbackInfo->descriptor_ = descriptor;
    
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = (void*)callbackInfo.get();

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            GattDescriptorCallbackInfo *callbackInfo = (GattDescriptorCallbackInfo *)work->data;
            napi_value result = nullptr;
            napi_create_object(callbackInfo->env_, &result);
            ConvertDescriptorReadReqToJS(callbackInfo->env_, result, callbackInfo->deviceId_, callbackInfo->descriptor_,
                callbackInfo->info_);

            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value callResult = nullptr;
            napi_get_undefined(callbackInfo->env_, &undefined);
            napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
            napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
        }
    );
}
} // namespace Bluetooth
} // namespace OHOS
