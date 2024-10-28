/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_cj_access_callback"
#endif

#include "bluetooth_access_impl.h"

#include "bluetooth_access_ffi.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "cj_lambda.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothAccess {
using Bluetooth::BluetoothHost;
using Bluetooth::BT_ERR_INTERNAL_ERROR;
using Bluetooth::BluetoothState;
using Bluetooth::BT_TRANSPORT_BREDR;
using Bluetooth::BT_TRANSPORT_BLE;
using Bluetooth::BTStateID;

CjBluetoothAccessObserver::CjBluetoothAccessObserver()
{}

std::shared_ptr<CjBluetoothAccessObserver> g_bluetoothAccessObserver =
    std::make_shared<CjBluetoothAccessObserver>();
bool g_flag = false;

static void RegisterAccessObserverToHost()
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    host->RegisterObserver(g_bluetoothAccessObserver);
}

void CjBluetoothAccessObserver::OnStateChanged(const int transport, const int status)
{
    BluetoothState state = BluetoothState::STATE_OFF;
    if (!DealStateChange(transport, status, state)) {
        HILOGD("No need callback");
        return;
    }

    HILOGD("state is %{public}d", state);
    if (!stateChangeFunc) {
        HILOGD("failed to register state change callback");
        return;
    }
    stateChangeFunc(static_cast<int32_t>(state));
}

bool CjBluetoothAccessObserver::DealStateChange(const int transport, const int status, BluetoothState &state)
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

void CjBluetoothAccessObserver::GetBrStateByStatus(const int status, BluetoothState &state, bool &isCallback)
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

void CjBluetoothAccessObserver::GetBleStateByStatus(const int status, BluetoothState &state)
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

void CjBluetoothAccessObserver::RegisterStateChangeFunc(std::function<void(int32_t)> cjCallback)
{
    stateChangeFunc = cjCallback;
}

void AccessImpl::RegisterAccessObserver(int32_t callbackType, void (*callback)(), int32_t* errCode)
{
    if (!g_flag) {
        RegisterAccessObserverToHost();
        g_flag = true;
    }

    if (callbackType == REGISTER_STATE_CHANGE_TYPE) {
        auto AccessObserverFunc = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
        if (!AccessObserverFunc) {
            HILOGD("Register state change event failed");
            *errCode = BT_ERR_INTERNAL_ERROR;
            return;
        }
        g_bluetoothAccessObserver->RegisterStateChangeFunc(AccessObserverFunc);
    }
    return;
}

} // namespace BluetoothAccess
} // namespace CJSystemapi
} // namespace OHOS