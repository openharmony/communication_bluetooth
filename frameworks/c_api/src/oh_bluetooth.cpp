/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "oh_bluetooth.h"

#include "bluetooth_log.h"
#include "bluetooth_host.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace OHOS {
namespace Bluetooth {

Bluetooth_ResultCode OH_Bluetooth_GetBluetoothSwitchState(Bluetooth_SwitchState *state)
{
    if (!state) {
        HILOGE("invalid param");
        return Bluetooth_ResultCode::BLUETOOTH_INVALID_PARAM;
    }
    BluetoothHost *host = &BluetoothHost::GetDefaultHost();
    int32_t btState = BTStateID::STATE_TURN_OFF;
    host->GetBtState(btState);

    Bluetooth_SwitchState status = Bluetooth_SwitchState::BLUETOOTH_STATE_OFF;
    switch (btState) {
        case BTStateID::STATE_TURNING_ON:
            status = Bluetooth_SwitchState::BLUETOOTH_STATE_TURNING_ON;
            break;
        case BTStateID::STATE_TURN_ON:
            status = Bluetooth_SwitchState::BLUETOOTH_STATE_ON;
            break;
        case BTStateID::STATE_TURNING_OFF:
            status = Bluetooth_SwitchState::BLUETOOTH_STATE_TURNING_OFF;
            break;
        case BTStateID::STATE_TURN_OFF:
            status = Bluetooth_SwitchState::BLUETOOTH_STATE_OFF;
            break;
        default:
            HILOGE("get state failed");
            break;
    }

    bool enableBle = host->IsBleEnabled();
    if (enableBle && (btState == BTStateID::STATE_TURN_OFF)) {
        status = Bluetooth_SwitchState::BLUETOOTH_STATE_BLE_ON;
    } else if (!enableBle && (btState == BTStateID::STATE_TURN_OFF)) {
        status = Bluetooth_SwitchState::BLUETOOTH_STATE_OFF;
    }
    *state = status;
    return Bluetooth_ResultCode::BLUETOOTH_SUCCESS;
}

} // namespace Bluetooth
} // namespace OHOS

#ifdef __cplusplus
}
#endif
