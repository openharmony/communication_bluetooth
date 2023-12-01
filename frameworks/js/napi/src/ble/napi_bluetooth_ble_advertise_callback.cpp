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
std::shared_ptr<NapiBluetoothBleAdvertiseCallback> NapiBluetoothBleAdvertiseCallback::GetInstance(void)
{
    static std::shared_ptr<NapiBluetoothBleAdvertiseCallback> instance = 
        std::make_shared<NapiBluetoothBleAdvertiseCallback>();
    return instance;
}

void NapiBluetoothBleAdvertiseCallback::SetNapiAdvertisingStateCallback(const std::shared_ptr<NapiCallback> &callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    napiAdvertisingStateCallback_ = callback;
}

void NapiBluetoothBleAdvertiseCallback::OnStartResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    std::lock_guard<std::mutex> lock(callbackMutex_);
    CHECK_AND_RETURN_LOG(napiAdvertisingStateCallback_ != nullptr, "AdvCallback is nullptr!");

    auto func = [advHandle, callback = napiAdvertisingStateCallback_]() {
        if (callback == nullptr) {
            HILOGE("napiAdvertisingStateCallback_ is nullptr");
            return;
        }
        auto napiNative = std::make_shared<NapiNativeAdvertisingStateInfo>(advHandle,
            static_cast<int>(AdvertisingState::STARTED));
        callback->CallFunction(napiNative);
    };
    DoInJsMainThread(napiAdvertisingStateCallback_->GetNapiEnv(), func);
}

void NapiBluetoothBleAdvertiseCallback::OnEnableResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    std::lock_guard<std::mutex> lock(callbackMutex_);
    CHECK_AND_RETURN_LOG(napiAdvertisingStateCallback_ != nullptr, "AdvCallback is nullptr!");

    auto func = [advHandle, callback = napiAdvertisingStateCallback_]() {
        if (callback == nullptr) {
            HILOGE("napiAdvertisingStateCallback_ is nullptr");
            return;
        }
        auto napiNative = std::make_shared<NapiNativeAdvertisingStateInfo>(advHandle,
            static_cast<int>(AdvertisingState::ENABLED));
        callback->CallFunction(napiNative);
    };
    DoInJsMainThread(napiAdvertisingStateCallback_->GetNapiEnv(), func);
}

void NapiBluetoothBleAdvertiseCallback::OnDisableResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    std::lock_guard<std::mutex> lock(callbackMutex_);
    CHECK_AND_RETURN_LOG(napiAdvertisingStateCallback_ != nullptr, "AdvCallback is nullptr!");

    auto func = [advHandle, callback = napiAdvertisingStateCallback_]() {
        if (callback == nullptr) {
            HILOGE("napiAdvertisingStateCallback_ is nullptr");
            return;
        }
        auto napiNative = std::make_shared<NapiNativeAdvertisingStateInfo>(advHandle,
            static_cast<int>(AdvertisingState::DISABLED));
        callback->CallFunction(napiNative);
    };
    DoInJsMainThread(napiAdvertisingStateCallback_->GetNapiEnv(), func);
}

void NapiBluetoothBleAdvertiseCallback::OnStopResultEvent(int result, int advHandle)
{
    HILOGI("enter, result: %{public}d advHandle: %{public}d", result, advHandle);
    std::lock_guard<std::mutex> lock(callbackMutex_);
    CHECK_AND_RETURN_LOG(napiAdvertisingStateCallback_ != nullptr, "AdvCallback is nullptr!");

    auto func = [advHandle, callback = napiAdvertisingStateCallback_]() {
        if (callback == nullptr) {
            HILOGE("napiAdvertisingStateCallback_ is nullptr");
            return;
        }
        auto napiNative = std::make_shared<NapiNativeAdvertisingStateInfo>(advHandle,
            static_cast<int>(AdvertisingState::STOPPED));
        callback->CallFunction(napiNative);
    };
    DoInJsMainThread(napiAdvertisingStateCallback_->GetNapiEnv(), func);
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