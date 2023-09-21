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
#include "napi_bluetooth_access_observer.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <uv.h>

namespace OHOS {
namespace Bluetooth {
void NapiBluetoothAccessObserver::UvQueueWorkOnStateChanged(uv_work_t *work, BluetoothState &state)
{
    HILOGD("start");

    if (work == nullptr) {
        HILOGE("work is null");
        return;
    }
    auto callbackData = (AfterWorkCallbackData<NapiBluetoothAccessObserver,
        decltype(&NapiBluetoothAccessObserver::UvQueueWorkOnStateChanged),
        BluetoothState> *)work->data;
    if (callbackData == nullptr) {
        HILOGE("callbackData is null");
        return;
    }

    napi_value result = 0;
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_value callResult = 0;

    napi_get_undefined(callbackData->env, &undefined);
    HILOGD("state is %{public}d", state);
    napi_create_int32(callbackData->env, static_cast<int32_t>(state), &result);
    napi_get_reference_value(callbackData->env, callbackData->callback, &callback);
    napi_call_function(callbackData->env, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
}

void NapiBluetoothAccessObserver::OnStateChanged(const int transport, const int status)
{
    HILOGD("start");
    BluetoothState state = BluetoothState::STATE_OFF;
    if (!DealStateChange(transport, status, state)) {
        return;
    }
    if (stateChangeCallback == nullptr) {
        HILOGD("This callback is not registered by ability.");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(stateChangeCallback->env_, &loop);
    if (loop == nullptr) {
        HILOGE("loop instance is nullptr");
        return;
    }

    auto callbackData = new (std::nothrow) AfterWorkCallbackData<NapiBluetoothAccessObserver,
        decltype(&NapiBluetoothAccessObserver::UvQueueWorkOnStateChanged),
        BluetoothState>();
    if (callbackData == nullptr) {
        HILOGE("new callbackData failed");
        return;
    }

    callbackData->object = this;
    callbackData->function = &NapiBluetoothAccessObserver::UvQueueWorkOnStateChanged;
    callbackData->env = stateChangeCallback->env_;
    callbackData->callback = stateChangeCallback->callback_;
    callbackData->data = state;

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOGE("new work failed");
        delete callbackData;
        callbackData = nullptr;
        return;
    }
    work->data = static_cast<void *>(callbackData);
    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, AfterWorkCallback<decltype(callbackData)>);
    if (ret != 0) {
        delete callbackData;
        callbackData = nullptr;
        delete work;
        work = nullptr;
    }
}

void NapiBluetoothAccessObserver::EnableBt()
{
    HILOGD("start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->EnableBt();
    if (ret != BT_NO_ERROR) {
        HILOGE("failed");
    }
}

void NapiBluetoothAccessObserver::DisableBle()
{
    HILOGD("start");
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int ret = host->DisableBle();
    if (ret != BT_NO_ERROR) {
        HILOGE("failed");
    }
}

bool NapiBluetoothAccessObserver::DealStateChange(const int transport, const int status, BluetoothState &state)
{
    HILOGD("transport is %{public}d, status is %{public}d", transport, status);
    bool isCallback = true;
    if (transport == BT_TRANSPORT_BREDR) {
        GetBrStateByStatus(status, state, isCallback);
    } else if (transport == BT_TRANSPORT_BLE) {
        GetBleStateByStatus(status, state);
    }
    return isCallback;
}

void NapiBluetoothAccessObserver::GetBrStateByStatus(const int status, BluetoothState &state, bool &isCallback)
{
    switch (status) {
        case BTStateID::STATE_TURNING_ON:
            HILOGD("STATE_TURNING_ON(1)");
            state = BluetoothState::STATE_TURNING_ON;
            break;
        case BTStateID::STATE_TURN_ON:
            HILOGD("STATE_ON(2)");
            state = BluetoothState::STATE_ON;
            break;
        case BTStateID::STATE_TURNING_OFF:
            HILOGD("STATE_TURNING_OFF(3)");
            state = BluetoothState::STATE_TURNING_OFF;
            break;
        case BTStateID::STATE_TURN_OFF: {
            HILOGD("STATE_OFF(0)");
            isCallback = false;
            break;
        }
        default:
            break;
    }
}

void NapiBluetoothAccessObserver::GetBleStateByStatus(const int status, BluetoothState &state)
{
    switch (status) {
        case BTStateID::STATE_TURNING_ON:
            HILOGD("STATE_BLE_TURNING_ON(4)");
            state = BluetoothState::STATE_BLE_TURNING_ON;
            break;
        case BTStateID::STATE_TURN_ON:
            HILOGD("STATE_BLE_ON(5)");
            state = BluetoothState::STATE_BLE_ON;
            break;
        case BTStateID::STATE_TURNING_OFF:
            HILOGD("STATE_BLE_TURNING_OFF(6)");
            state = BluetoothState::STATE_BLE_TURNING_OFF;
            break;
        case BTStateID::STATE_TURN_OFF:
            HILOGD("STATE_OFF(0)");
            state = BluetoothState::STATE_OFF;
            break;
        default:
            break;
    }
}
}  // namespace Bluetooth
}  // namespace OHOS
