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
#include "napi_async_work.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_gatt_client.h"
#include "napi_bluetooth_gatt_client_callback.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
std::shared_mutex NapiGattClientCallback::g_gattClientCallbackInfosMutex;

void NapiGattClientCallback::OnCharacteristicChanged(const GattCharacteristic &characteristic)
{
    std::unique_lock<std::shared_mutex> guard(g_gattClientCallbackInfosMutex);
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>>::iterator it =
        callbackInfos_.find(STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE);
    if (it == callbackInfos_.end() || it->second == nullptr) {
        HILOGI("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = it->second;
    HILOGI("uuid is %{public}s", characteristic.GetUuid().ToString().c_str());

    NapiEvent::CheckAndNotify(callbackInfo, characteristic);
}

void NapiGattClientCallback::OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    auto napiCharacter = std::make_shared<NapiNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_CHARACTER, napiCharacter, ret);
}

void NapiGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    auto napiDescriptor = std::make_shared<NapiNativeBleDescriptor>(descriptor);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_DESCRIPTOR, napiDescriptor, ret);
}

void NapiGattClientCallback::OnConnectionStateChanged(int connectionState, int ret)
{
    std::unique_lock<std::shared_mutex> guard(g_gattClientCallbackInfosMutex);
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>>::iterator it =
        callbackInfos_.find(STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE);
    if (it == callbackInfos_.end() || it->second == nullptr) {
        HILOGI("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = it->second;
    HILOGI("connectionState:%{public}d, ret:%{public}d", connectionState, ret);
    callbackInfo->state_ = connectionState;
    callbackInfo->deviceId_ = client_->GetDevice()->GetDeviceAddr();
    NapiEvent::CheckAndNotify(callbackInfo, callbackInfo->state_);
}

void NapiGattClientCallback::OnServicesDiscovered(int status)
{
    HILOGI("status:%{public}d", status);
}

void NapiGattClientCallback::OnReadRemoteRssiValueResult(int rssi, int ret)
{
    HILOGI("rssi: %{public}d, ret: %{public}d", rssi, ret);
    auto napiRssi = std::make_shared<NapiNativeInt>(rssi);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_REMOTE_RSSI_VALUE, napiRssi, ret);
}

void NapiGattClientCallback::OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    auto napiCharacter = std::make_shared<NapiNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_WRITE_CHARACTER, napiCharacter, ret);
#endif
}

void NapiGattClientCallback::OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    auto napiDescriptor = std::make_shared<NapiNativeBleDescriptor>(descriptor);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_WRITE_DESCRIPTOR, napiDescriptor, ret);
#endif
}

void NapiGattClientCallback::OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, status: %{public}d", characteristic.GetUuid().ToString().c_str(), status);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_ENABLE_CHARACTER_CHANGED, nullptr, status);
#endif
}

void NapiGattClientCallback::OnMtuUpdate(int mtu, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("ret: %{public}d, mtu: %{public}d", ret, mtu);
    std::unique_lock<std::shared_mutex> guard(g_gattClientCallbackInfosMutex);
    auto it = callbackInfos_.find(STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE);
    if (it == callbackInfos_.end() || it->second == nullptr) {
        HILOGI("BLEMtuChange callback is not registered.");
        return;
    }
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = it->second;

    auto func = [callbackInfo, mtu]() {
        napi_value result = nullptr;
        napi_create_int32(callbackInfo->env_, mtu, &result);

        napi_value callback = nullptr;
        napi_value undefined = nullptr;
        napi_value callResult = nullptr;
        napi_get_undefined(callbackInfo->env_, &undefined);
        napi_get_reference_value(callbackInfo->env_, callbackInfo->callback_, &callback);
        napi_call_function(callbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
    };
    DoInJsMainThread(callbackInfo->env_, func);
#endif
}

} // namespace Bluetooth
} // namespace OHOS
