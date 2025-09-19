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
#define LOG_TAG "bt_taihe_access_observer"
#endif
 
#include "ohos.bluetooth.access.proj.hpp"
#include "ohos.bluetooth.access.impl.hpp"

#include "bluetooth_log.h"
#include "taihe_bluetooth_access_observer.h"

namespace OHOS {
namespace Bluetooth {

TaiheBluetoothAccessObserver::TaiheBluetoothAccessObserver()
{}

std::shared_ptr<TaiheBluetoothAccessObserver> TaiheBluetoothAccessObserver::GetInstance(void)
{
    static std::shared_ptr<TaiheBluetoothAccessObserver> instance =
        std::make_shared<TaiheBluetoothAccessObserver>();
    return instance;
}

void TaiheBluetoothAccessObserver::OnStateChanged(const int transport, const int status)
{
    std::shared_lock<std::shared_mutex> guard(stateChangedObserverLock);
    HILOGI("transport is %{public}d, status is %{public}d", transport, status);
    BluetoothState state = BluetoothState::STATE_OFF;
    DealStateChange(transport, status, state);
    auto taiheResult = static_cast<::ohos::bluetooth::access::BluetoothState::key_t>(state);
    for (auto callback : stateChangedObserverVec) {
        if (callback) {
            (*callback)(taiheResult);
        }
    }
}

void TaiheBluetoothAccessObserver::DealStateChange(const int transport, const int status, BluetoothState &state)
{
    HILOGD("transport is %{public}d, status is %{public}d", transport, status);
    if (transport == BT_TRANSPORT_BREDR) {
        GetBrStateByStatus(status, state);
    } else if (transport == BT_TRANSPORT_BLE) {
        GetBleStateByStatus(status, state);
    }
}

void TaiheBluetoothAccessObserver::GetBrStateByStatus(const int status, BluetoothState &state)
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
        case BTStateID::STATE_TURN_OFF:
            HILOGD("STATE_OFF(0)");
            state = BluetoothState::STATE_OFF;
            break;
        default:
            break;
    }
}

void TaiheBluetoothAccessObserver::GetBleStateByStatus(const int status, BluetoothState &state)
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

void TaiheBluetoothAccessInit::RegisterAccessObserverToHost()
{
    HILOGD("enter");
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    host->RegisterObserver(TaiheBluetoothAccessObserver::GetInstance());
}

} // namespace Bluetooth
} // namespace OHOS
