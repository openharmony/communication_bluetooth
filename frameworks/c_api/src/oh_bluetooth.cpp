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
#ifndef LOG_TAG
#define LOG_TAG "bt_c_api_ohbluetooth"
#endif

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
    *state = static_cast<Bluetooth_SwitchState>(BluetoothHost::GetDefaultHost().GetBluetoothState());
    return Bluetooth_ResultCode::BLUETOOTH_SUCCESS;
}

} // namespace Bluetooth
} // namespace OHOS

#ifdef __cplusplus
}
#endif
