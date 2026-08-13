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

#ifndef BLUETOOTH_OVERRIDE_ERRORCODE_H
#define BLUETOOTH_OVERRIDE_ERRORCODE_H

namespace OHOS {
namespace Bluetooth {

/* Bluetooth errcode defines */
enum BtOvErrCode {
    // Common error codes
    BT_OV_ERR_PERMISSION_FAILED = 201,
    BT_OV_ERR_SYSTEM_PERMISSION_FAILED = 202,
    BT_OV_ERR_PROHIBITED_BY_EDM = 203,
    BT_OV_ERR_INVALID_PARAM = 401,
    BT_OV_ERR_API_NOT_SUPPORT = 801,

    BT_OV_NO_ERROR = 0,

    BT_OV_ERR_BASE_SYSCAP = 1020700000,
    BT_OV_ERR_SERVICE_STOPPED        = BT_OV_ERR_BASE_SYSCAP + 1,
    BT_OV_ERR_SERVICE_DISABLED       = BT_OV_ERR_BASE_SYSCAP + 3,
    BT_OV_ERR_PROFILE_NOT_SUPPORTED  = BT_OV_ERR_BASE_SYSCAP + 4,
    BT_OV_ERR_INTERNAL_ERROR         = BT_OV_ERR_BASE_SYSCAP + 99,
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_OVERRIDE_ERRORCODE_H