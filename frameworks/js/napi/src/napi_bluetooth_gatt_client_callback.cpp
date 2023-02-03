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
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_gatt_client.h"
#include "napi_bluetooth_gatt_client_callback.h"

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
    ReadCharacteristicValueCallbackInfo *callbackInfo = client_->readCharacteristicValueCallbackInfo_;
    if (!callbackInfo) {
        HILOGE("CallbackInfo does not exist");
        return;
    }
    HILOGI("asyncState: %{public}d", callbackInfo->asyncState_);
    if (callbackInfo->asyncState_ == ASYNC_START) {
        callbackInfo->ret = ret;
        callbackInfo->outputCharacteristic_ = &characteristic;
        callbackInfo->asyncState_ = ASYNC_DONE;
    }
}

void NapiGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    ReadDescriptorValueCallbackInfo *callbackInfo = client_->readDescriptorValueCallbackInfo_;
    if (!callbackInfo) {
        HILOGE("CallbackInfo does not exist");
        return;
    }
    HILOGI("asyncState: %{public}d", callbackInfo->asyncState_);
    if (callbackInfo->asyncState_ == ASYNC_START) {
        callbackInfo->ret = ret;
        callbackInfo->outputDescriptor_ = &descriptor;
        callbackInfo->asyncState_ = ASYNC_DONE;
    }
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
} // namespace Bluetooth
} // namespace OHOS
