/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_ble_advertise_callback"
#endif

#include "taihe_bluetooth_ble_advertise_callback.h"
#include "ohos.bluetooth.ble.impl.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
TaiheBluetoothBleAdvertiseCallback::TaiheBluetoothBleAdvertiseCallback()
{}

std::shared_ptr<TaiheBluetoothBleAdvertiseCallback> TaiheBluetoothBleAdvertiseCallback::GetInstance(void)
{
    static std::shared_ptr<TaiheBluetoothBleAdvertiseCallback> instance =
        std::make_shared<TaiheBluetoothBleAdvertiseCallback>();
    return instance;
}

void TaiheBluetoothBleAdvertiseCallback::OnStartResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        HILOGI("OnStartResultEvent, advHandle is same, advHandle: %{public}d", advHandle_);
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        auto taiheAdvHandle = std::make_shared<TaiheNativeInt>(advHandle);
        AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GET_ADVERTISING_HANDLE, taiheAdvHandle, result);
    }
}

void TaiheBluetoothBleAdvertiseCallback::OnEnableResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        HILOGI("OnEnableResultEvent, advHandle is same, advHandle: %{public}d", advHandle_);
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        auto taiheAdvResult = std::make_shared<TaiheNativeInt>(result);
        AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::BLE_ENABLE_ADVERTISING, taiheAdvResult, result);
    }
}

void TaiheBluetoothBleAdvertiseCallback::OnDisableResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    if (advHandle_ == advHandle) {
        HILOGI("OnDisableResultEvent, advHandle is same, advHandle: %{public}d", advHandle_);
        result = GetSDKAdaptedStatusCode(result); // Adaptation for old sdk
        auto taiheAdvResult = std::make_shared<TaiheNativeInt>(result);
        AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::BLE_DISABLE_ADVERTISING, taiheAdvResult, result);
    }
}

void TaiheBluetoothBleAdvertiseCallback::OnStopResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
}

void TaiheBluetoothBleAdvertiseCallback::OnSetAdvDataEvent(int result)
{}

void TaiheBluetoothBleAdvertiseCallback::OnGetAdvHandleEvent(int result, int advHandle)
{
    HILOGI("OnGetAdvHandleEvent enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    advHandle_ = advHandle;
    HILOGI("OnGetAdvHandleEvent leave");
}

void TaiheBluetoothBleAdvertiseCallback::OnChangeAdvResultEvent(int result, int advHandle)
{}

}  // namespace Bluetooth
}  // namespace OHOS