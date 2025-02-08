/*
 * Copyright (C) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_ERRORCODE_H
#define BLUETOOTH_ERRORCODE_H

namespace OHOS {
namespace Bluetooth {

/* Bluetooth errcode defines */
enum BtErrCode {
    // Common error codes
    BT_ERR_PERMISSION_FAILED = 201,
    BT_ERR_SYSTEM_PERMISSION_FAILED = 202,
    BT_ERR_PROHIBITED_BY_EDM = 203,
    BT_ERR_INVALID_PARAM = 401,
    BT_ERR_API_NOT_SUPPORT = 801,

    // Customized error codes
    BT_NO_ERROR = 0,

    BT_ERR_BASE_SYSCAP = 2900000,
    BT_ERR_SERVICE_DISCONNECTED     = BT_ERR_BASE_SYSCAP + 1,
    BT_ERR_UNBONDED_DEVICE          = BT_ERR_BASE_SYSCAP + 2,
    BT_ERR_INVALID_STATE            = BT_ERR_BASE_SYSCAP + 3,
    BT_ERR_PROFILE_DISABLED         = BT_ERR_BASE_SYSCAP + 4,
    BT_ERR_DEVICE_DISCONNECTED      = BT_ERR_BASE_SYSCAP + 5,
    BT_ERR_MAX_CONNECTION           = BT_ERR_BASE_SYSCAP + 6,
    BT_ERR_TIMEOUT                  = BT_ERR_BASE_SYSCAP + 7,
    BT_ERR_UNAVAILABLE_PROXY        = BT_ERR_BASE_SYSCAP + 8,
    BT_ERR_DIALOG_FOR_USER_CONFIRM  = BT_ERR_BASE_SYSCAP + 9,

    BT_ERR_INTERNAL_ERROR           = BT_ERR_BASE_SYSCAP + 99,
    BT_ERR_IPC_TRANS_FAILED         = BT_ERR_BASE_SYSCAP + 100,

    BT_ERR_GATT_READ_NOT_PERMITTED  = BT_ERR_BASE_SYSCAP + 1000,
    BT_ERR_GATT_WRITE_NOT_PERMITTED = BT_ERR_BASE_SYSCAP + 1001,
    BT_ERR_GATT_MAX_SERVER          = BT_ERR_BASE_SYSCAP + 1002,

    BT_ERR_SPP_SERVER_STATE         = BT_ERR_BASE_SYSCAP + 1050,
    BT_ERR_SPP_BUSY                 = BT_ERR_BASE_SYSCAP + 1051,
    BT_ERR_SPP_DEVICE_NOT_FOUND     = BT_ERR_BASE_SYSCAP + 1052,
    BT_ERR_SPP_IO                   = BT_ERR_BASE_SYSCAP + 1054,

    BT_ERR_NO_ACTIVE_HFP_DEVICE     = BT_ERR_BASE_SYSCAP + 2000,
    BT_ERR_NULL_HFP_STATE_MACHINE   = BT_ERR_BASE_SYSCAP + 2001,
    BT_ERR_HFP_NOT_CONNECT          = BT_ERR_BASE_SYSCAP + 2002,
    BT_ERR_SCO_HAS_BEEN_CONNECTED   = BT_ERR_BASE_SYSCAP + 2003,
    BT_ERR_VR_HAS_BEEN_STARTED      = BT_ERR_BASE_SYSCAP + 2004,
    BT_ERR_AUDIO_NOT_IDLE           = BT_ERR_BASE_SYSCAP + 2005,
    BT_ERR_VIRTUAL_CALL_NOT_STARTED = BT_ERR_BASE_SYSCAP + 2006,
    BT_ERR_DISCONNECT_SCO_FAILED    = BT_ERR_BASE_SYSCAP + 2007,

    BT_ERR_BLE_SCAN_ALREADY_STARTED = BT_ERR_BASE_SYSCAP + 2050,
    BT_ERR_BLE_SCAN_MAX_FILTER      = BT_ERR_BASE_SYSCAP + 2051,
    BT_ERR_BLE_CHANGE_SCAN_FILTER_FAIL = BT_ERR_BASE_SYSCAP + 2052,
    BT_ERR_BLE_CHANGE_SCAN_WRONG_STATE = BT_ERR_BASE_SYSCAP + 2053,

    // will deprected
    NO_ERROR = BT_NO_ERROR,
    ERROR = BT_ERR_INTERNAL_ERROR,
    ERR_INVALID_STATE = BT_ERR_INVALID_STATE,
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_ERRORCODE_H