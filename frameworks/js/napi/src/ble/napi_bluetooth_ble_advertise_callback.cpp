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

#include "napi_bluetooth_ble_advertise_callback.h"

#include "bluetooth_log.h"
#include "napi_async_work.h"
#include "napi_native_object.h"
#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_ble_utils.h"

namespace OHOS {
namespace Bluetooth {
NapiBluetoothBleAdvertiseCallback::NapiBluetoothBleAdvertiseCallback()
    : eventSubscribe_(REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE, BT_MODULE_NAME)
{}
    
std::shared_ptr<NapiBluetoothBleAdvertiseCallback> NapiBluetoothBleAdvertiseCallback::GetInstance(void)
{
    static std::shared_ptr<NapiBluetoothBleAdvertiseCallback> instance =
        std::make_shared<NapiBluetoothBleAdvertiseCallback>();
    return instance;
}

void NapiBluetoothBleAdvertiseCallback::OnStartResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    auto nativeObject =
        std::make_shared<NapiNativeAdvertisingStateInfo>(advHandle, static_cast<int>(AdvertisingState::STARTED));
    eventSubscribe_.PublishEvent(REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE, nativeObject);
}

void NapiBluetoothBleAdvertiseCallback::OnEnableResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    auto nativeObject =
        std::make_shared<NapiNativeAdvertisingStateInfo>(advHandle, static_cast<int>(AdvertisingState::ENABLED));
    eventSubscribe_.PublishEvent(REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE, nativeObject);
}

void NapiBluetoothBleAdvertiseCallback::OnDisableResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    auto nativeObject =
        std::make_shared<NapiNativeAdvertisingStateInfo>(advHandle, static_cast<int>(AdvertisingState::DISABLED));
    eventSubscribe_.PublishEvent(REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE, nativeObject);
}

void NapiBluetoothBleAdvertiseCallback::OnStopResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    auto nativeObject =
        std::make_shared<NapiNativeAdvertisingStateInfo>(advHandle, static_cast<int>(AdvertisingState::STOPPED));
    eventSubscribe_.PublishEvent(REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE, nativeObject);
}

void NapiBluetoothBleAdvertiseCallback::OnSetAdvDataEvent(int result)
{}

void NapiBluetoothBleAdvertiseCallback::OnGetAdvHandleEvent(int result, int advHandle)
{
    auto napiAdvHandle = std::make_shared<NapiNativeInt>(advHandle);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GET_ADVERTISING_HANDLE, napiAdvHandle, result);
}
}  // namespace Bluetooth
}  // namespace OHOS