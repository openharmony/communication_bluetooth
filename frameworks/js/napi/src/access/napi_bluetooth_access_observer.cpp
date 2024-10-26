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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_access_observer"
#endif

#include "napi_bluetooth_access_observer.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_event_subscribe_module.h"

#include <uv.h>

namespace OHOS {
namespace Bluetooth {
NapiBluetoothAccessObserver::NapiBluetoothAccessObserver()
    : eventSubscribe_(REGISTER_STATE_CHANGE_TYPE, BT_MODULE_NAME)
{}

void NapiBluetoothAccessObserver::OnStateChanged(const int transport, const int status)
{
    BluetoothState state = BluetoothState::STATE_OFF;
    if (!DealStateChange(transport, status, state)) {
        HILOGD("No need callback");
        return;
    }

    HILOGD("state is %{public}d", state);
    auto nativeObject = std::make_shared<NapiNativeInt>(static_cast<int>(state));
    eventSubscribe_.PublishEvent(REGISTER_STATE_CHANGE_TYPE, nativeObject);
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
