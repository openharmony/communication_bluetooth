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
#include "napi_bluetooth_gatt_server.h"
#include "napi_bluetooth_gatt_server_callback.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

void NGattServerCallback::OnCharacteristicReadRequest(
    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
{
    HILOGI("NGattServerCallback::OnCharacteristicReadRequest called");
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ]) {
        HILOGW("NGattServerCallback::OnCharacteristicReadRequest: This callback is not registered by ability.");
        return;
    }
    HILOGI("NGattServerCallback::OnCharacteristicReadRequest: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ];
    napi_value result = nullptr;
    napi_create_object(callbackInfo->env_, &result);

    ConvertCharacteristicReadReqToJS(callbackInfo->env_, result, device, characteristic, requestId);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(callbackInfo->env_, &undefined);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NGattServerCallback::OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId)
{
    HILOGI("NGattServerCallback::OnCharacteristicWriteRequest called");
    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE]) {
        HILOGW("NGattServerCallback::OnCharacteristicWriteRequest: This callback is not registered by ability.");
        return;
    }
    HILOGI("NGattServerCallback::OnCharacteristicWriteRequest: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE];
    napi_value result = nullptr;
    napi_create_object(callbackInfo->env_, &result);

    ConvertCharacteristicWriteReqToJS(callbackInfo->env_, result, device, characteristic, requestId);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(callbackInfo->env_, &undefined);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}


void NGattServerCallback::OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
{
    HILOGI("NGattServerCallback::OnConnectionStateUpdate called");

    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("NGattServerCallback::OnConnectionStateUpdate connected");
        bool hasAddr = false;
        for (auto it = NGattServer::deviceList.begin(); it != NGattServer::deviceList.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                hasAddr = true;
                break;
            }
        }
        if (!hasAddr) {
            HILOGI("NGattServerCallback::OnConnectionStateUpdate add devices");
            NGattServer::deviceList.push_back(device.GetDeviceAddr());
        }
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("NGattServerCallback::OnConnectionStateUpdate disconnected");
        for (auto it = NGattServer::deviceList.begin(); it != NGattServer::deviceList.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                HILOGI("NGattServerCallback::OnConnectionStateUpdate romove devices start");
                NGattServer::deviceList.erase(it);
                break;
                HILOGI("NGattServerCallback::OnConnectionStateUpdate romove devices end");
            }
        }
    }

    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE]) {
        HILOGW("NGattServerCallback::OnConnectionStateUpdate: This callback is not registered by ability.");
        return;
    }
    HILOGI("NGattServerCallback::OnConnectionStateUpdate: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE];
    napi_value result = nullptr;
    napi_create_object(callbackInfo->env_, &result);

    ConvertBLEConnectStateChangeToJS(callbackInfo->env_, result, device, state);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(callbackInfo->env_, &undefined);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NGattServerCallback::OnDescriptorWriteRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("NGattServerCallback::OnDescriptorWriteRequest called");

    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE]) {
        HILOGW("NGattServerCallback::OnDescriptorWriteRequest: This callback is not registered by ability.");
        return;
    }
    HILOGI("NGattServerCallback::OnDescriptorWriteRequest: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE];
    napi_value result = nullptr;
    napi_create_object(callbackInfo->env_, &result);

    ConvertDescriptorWriteReqToJS(callbackInfo->env_, result, device, descriptor, requestId);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(callbackInfo->env_, &undefined);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NGattServerCallback::OnDescriptorReadRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("NGattServerCallback::OnDescriptorReadRequest called");

    if (!callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ]) {
        HILOGW("NGattServerCallback::OnDescriptorReadRequest: This callback is not registered by ability.");
        return;
    }
    HILOGI("NGattServerCallback::OnDescriptorReadRequest: %{public}s is registered by ability",
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ.c_str());
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ];
    napi_value result = nullptr;
    napi_create_object(callbackInfo->env_, &result);

    ConvertDescriptorReadReqToJS(callbackInfo->env_, result, device, descriptor, requestId);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(callbackInfo->env_, &undefined);
    napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
    napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}
} // namespace Bluetooth
} // namespace OHOS
