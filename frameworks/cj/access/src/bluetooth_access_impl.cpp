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

#include "bluetooth_access_impl.h"
#include "bluetooth_log.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothAccess {
using Bluetooth::BluetoothHost;
using Bluetooth::BTStateID;
using Bluetooth::BluetoothState;

void AccessImpl::EnableBluetooth(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    *errCode = host->EnableBle();
    return;
}

void AccessImpl::DisableBluetooth(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    *errCode = host->DisableBt();
    return;
}

int32_t AccessImpl::GetState(int32_t* errCode)
{
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t state = BTStateID::STATE_TURN_OFF;
    *errCode = host->GetBtState(state);
    int32_t status = static_cast<int32_t>(BluetoothState::STATE_OFF);
    switch (state) {
        case BTStateID::STATE_TURNING_ON:
            HILOGD("STATE_TURNING_ON(1)");
            status = static_cast<int32_t>(BluetoothState::STATE_TURNING_ON);
            break;
        case BTStateID::STATE_TURN_ON:
            HILOGD("STATE_ON(2)");
            status = static_cast<int32_t>(BluetoothState::STATE_ON);
            break;
        case BTStateID::STATE_TURNING_OFF:
            HILOGD("STATE_TURNING_OFF(3)");
            status = static_cast<int32_t>(BluetoothState::STATE_TURNING_OFF);
            break;
        case BTStateID::STATE_TURN_OFF:
            HILOGD("STATE_OFF(0)");
            status = static_cast<int32_t>(BluetoothState::STATE_OFF);
            break;
        default:
            HILOGE("get state failed");
            break;
    }

    bool enableBle = host->IsBleEnabled();
    if (enableBle && (state == BTStateID::STATE_TURN_OFF)) {
        HILOGD("BR off and BLE on, STATE_BLE_ON(5)");
        status = static_cast<int32_t>(BluetoothState::STATE_BLE_ON);
    } else if (!enableBle && (state == BTStateID::STATE_TURN_OFF)) {
        status = static_cast<int32_t>(BluetoothState::STATE_OFF);
    }
    return status;
}
} // namespace CJBluetoothAccess
} // namespace CJSystemapi
} // namespace OHOS