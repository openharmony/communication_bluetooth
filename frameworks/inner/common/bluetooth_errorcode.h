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

#ifndef BLUETOOTH_ERRORCODE_H
#define BLUETOOTH_ERRORCODE_H

namespace OHOS {
namespace Bluetooth {

/* Bluetooth errcode defines */
enum BtErrCode {
    // Common error codes
    BT_ERR_PERMISSION_FAILED = 201,
    BT_ERR_SYSTEM_PERMISSION_FAILED = 202,
    BT_ERR_INVALID_PARAM = 401,
    BT_ERR_API_NOT_SUPPORT = 801,

    // Customized error codes
    BT_SUCCESS = 0,

    BT_ERR_BASE_SYSCAP = 2900000,
    BT_ERR_SERVICE_DISCONNECTED     = BT_ERR_BASE_SYSCAP + 1,
    BT_ERR_UNBONDED_DEVICE          = BT_ERR_BASE_SYSCAP + 2,
    BT_ERR_INVALID_STATE            = BT_ERR_BASE_SYSCAP + 3,
    BT_ERR_PROFILE_DISABLED         = BT_ERR_BASE_SYSCAP + 4,
    BT_ERR_DEVICE_DISCONNECTED      = BT_ERR_BASE_SYSCAP + 5,
    BT_ERR_MAX_CONNECTION           = BT_ERR_BASE_SYSCAP + 6,

    BT_ERR_INTERNAL_ERROR           = BT_ERR_BASE_SYSCAP + 99,
    BT_ERR_IPC_TRANS_FAILED         = BT_ERR_BASE_SYSCAP + 100,

    BT_ERR_GATT_READ_NOT_PERMITTED  = BT_ERR_BASE_SYSCAP + 1000,
    BT_ERR_GATT_WRITE_NOT_PERMITTED = BT_ERR_BASE_SYSCAP + 1001,
    BT_ERR_GATT_MAX_SERVER          = BT_ERR_BASE_SYSCAP + 1002,

    BT_ERR_SPP_SERVER_STATE         = BT_ERR_BASE_SYSCAP + 1050,
    BT_ERR_SPP_BUSY                 = BT_ERR_BASE_SYSCAP + 1051,
    BT_ERR_SPP_DEVICE_NOT_FOUND     = BT_ERR_BASE_SYSCAP + 1052,
    BT_ERR_SPP_IO                   = BT_ERR_BASE_SYSCAP + 1054,

    // will deprected
    NO_ERROR = 0,
    ERR_INVALID_STATE = 1,
    ERR_INVALID_VALUE = 2,
    ERROR = 3,
};

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_ERRORCODE_H