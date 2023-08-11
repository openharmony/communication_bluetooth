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
#include <uv.h>
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_gatt_server.h"
#include "napi_bluetooth_gatt_server_callback.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

void NapiGattServerCallback::OnCharacteristicReadRequest(
    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ]) {
        HILOGI("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<GattCharacteristicCallbackInfo> callbackInfo =
        std::static_pointer_cast<GattCharacteristicCallbackInfo>(
            callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ]);
    callbackInfo->info_ = requestId;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
    callbackInfo->characteristic_ = characteristic;

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = static_cast<void *>(callbackInfo.get());

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            GattCharacteristicCallbackInfo *callbackInfo = static_cast<GattCharacteristicCallbackInfo *>(work->data);
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
            delete work;
            work = nullptr;
        }
    );
}

void NapiGattServerCallback::OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE]) {
        HILOGI("This callback is not registered by ability.");
        return;
    }

    std::shared_ptr<GattCharacteristicCallbackInfo> callbackInfo =
        std::static_pointer_cast<GattCharacteristicCallbackInfo>(
            callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE]);
    callbackInfo->info_ = requestId;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
    callbackInfo->characteristic_ = characteristic;

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = static_cast<void *>(callbackInfo.get());

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            GattCharacteristicCallbackInfo *callbackInfo = static_cast<GattCharacteristicCallbackInfo *>(work->data);
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
            delete work;
            work = nullptr;
        }
    );
}

void NapiGattServerCallback::OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
{
    HILOGI("enter, state: %{public}d, remote device address: %{public}s", state, GET_ENCRYPT_ADDR(device));
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("connected");
        bool hasAddr = false;
        for (auto it = NapiGattServer::deviceList.begin(); it != NapiGattServer::deviceList.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                hasAddr = true;
                break;
            }
        }
        if (!hasAddr) {
            HILOGI("add devices");
            NapiGattServer::deviceList.push_back(device.GetDeviceAddr());
        }
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("disconnected");
        for (auto it = NapiGattServer::deviceList.begin(); it != NapiGattServer::deviceList.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                HILOGI("romove device");
                NapiGattServer::deviceList.erase(it);
                break;
            }
        }
    }

    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE]) {
        HILOGI("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE];

    callbackInfo->state_ = state;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = static_cast<void *>(callbackInfo.get());

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            BluetoothCallbackInfo *callbackInfo = static_cast<BluetoothCallbackInfo *>(work->data);
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

void NapiGattServerCallback::OnDescriptorWriteRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE]) {
        HILOGI("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<GattDescriptorCallbackInfo> callbackInfo =
        std::static_pointer_cast<GattDescriptorCallbackInfo>(
            callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE]);
    callbackInfo->info_ = requestId;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
    callbackInfo->descriptor_ = descriptor;

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = static_cast<void *>(callbackInfo.get());

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            GattDescriptorCallbackInfo *callbackInfo = static_cast<GattDescriptorCallbackInfo *>(work->data);
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
            delete work;
            work = nullptr;
        }
    );
}

void NapiGattServerCallback::OnDescriptorReadRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ]) {
        HILOGI("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<GattDescriptorCallbackInfo> callbackInfo =
        std::static_pointer_cast<GattDescriptorCallbackInfo>(
            callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ]);
    callbackInfo->info_ = requestId;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
    callbackInfo->descriptor_ = descriptor;

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = static_cast<void *>(callbackInfo.get());

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            GattDescriptorCallbackInfo *callbackInfo = static_cast<GattDescriptorCallbackInfo *>(work->data);
            napi_value result = nullptr;
            napi_create_object(callbackInfo->env_, &result);
            ConvertDescriptorReadReqToJS(callbackInfo->env_, result, callbackInfo->deviceId_,
                callbackInfo->descriptor_, callbackInfo->info_);

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

void NapiGattServerCallback::OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu)
{
    HILOGI("enter, remote device address: %{public}s, mtu: %{public}d",
        GET_ENCRYPT_ADDR(device), mtu);
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_MTU_CHANGE]) {
        HILOGI("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_MTU_CHANGE];
    callbackInfo->info_ = mtu;
    callbackInfo->deviceId_ = device.GetDeviceAddr();

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callbackInfo->env_, &loop);
    uv_work_t *work = new uv_work_t;
    work->data = static_cast<void *>(callbackInfo.get());

    uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            BluetoothCallbackInfo *callbackInfo = static_cast<BluetoothCallbackInfo *>(work->data);
            if (callbackInfo == nullptr) {
                HILOGE("callbackInfo is null!");
                return;
            }
            NapiHandleScope scope(callbackInfo->env_);
            napi_value result = 0;
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(callbackInfo->env_, &undefined);
            napi_create_int32(callbackInfo->env_, static_cast<int32_t>(callbackInfo->info_), &result);
            napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
            napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
            delete work;
            work = nullptr;
        }
    );
}

void NapiGattServerCallback::OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int ret)
{
    HILOGI("ret: %{public}d", ret);
    auto napiRet = std::make_shared<NapiNativeInt>(ret);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_SERVER_NOTIFY_CHARACTERISTIC, napiRet, ret);
}
} // namespace Bluetooth
} // namespace OHOS