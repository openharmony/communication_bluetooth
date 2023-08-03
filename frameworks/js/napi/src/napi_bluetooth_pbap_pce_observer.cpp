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
#include "bluetooth_utils.h"
#include "napi_async_callback.h"
#include "napi_bluetooth_pbap_pce_observer.h"

namespace OHOS {
namespace Bluetooth {
void NapiPbapPceObserver::OnServiceConnectionStateChanged(const BluetoothRemoteDevice &device, int state)
{
    HILOGI("enter, remote device address: %{public}s, state: %{public}d", GET_ENCRYPT_ADDR(device), state);
    if (!callbackInfos_[STR_BT_PBAP_PCE_CONNECTION_STATE_CHANGE]) {
        HILOGW("This callback is not registered by ability.");
        return;
    }

    std::shared_ptr<BluetoothCallbackInfo> callbackInfo =
        callbackInfos_[STR_BT_PBAP_PCE_CONNECTION_STATE_CHANGE];
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
            NapiHandleScope scope(callbackInfo->env_);
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

} // namespace Bluetooth
} // namespace OHOS