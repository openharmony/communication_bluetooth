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
#define LOG_TAG "bt_taihe_error"
#endif

#include <map>

#include "bluetooth_errorcode.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_utils.h"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {

static std::map<int32_t, std::string> taiheErrMsgMap {
    { BtErrCode::BT_ERR_SERVICE_DISCONNECTED, "Service stoped." },
    { BtErrCode::BT_ERR_UNBONDED_DEVICE, "Device is not bonded." },
    { BtErrCode::BT_ERR_INVALID_STATE, "Bluetooth disabled." },
    { BtErrCode::BT_ERR_PROFILE_DISABLED, "Profile not supported." },
    { BtErrCode::BT_ERR_DEVICE_DISCONNECTED, "Device not connected." },
    { BtErrCode::BT_ERR_MAX_CONNECTION, "The maximum number of connections has been reached." },
    { BtErrCode::BT_ERR_INTERNAL_ERROR, "Operation failed" },
    { BtErrCode::BT_ERR_IPC_TRANS_FAILED, "IPC failed." },
    { BtErrCode::BT_ERR_UNAVAILABLE_PROXY, "The value of proxy is a null pointer." },
    { BtErrCode::BT_ERR_PERMISSION_FAILED, "Permission denied." },
    { BtErrCode::BT_ERR_SYSTEM_PERMISSION_FAILED, "Non-system applications are not allowed to use system APIs."},
    { BtErrCode::BT_ERR_PROHIBITED_BY_EDM, "Bluetooth is prohibited by EDM."},
    { BtErrCode::BT_ERR_INVALID_PARAM, "Invalid parameter." },
    { BtErrCode::BT_ERR_API_NOT_SUPPORT, "Capability is not supported." },
    { BtErrCode::BT_ERR_GATT_READ_NOT_PERMITTED, "Gatt read forbiden." },
    { BtErrCode::BT_ERR_GATT_WRITE_NOT_PERMITTED, "Gatt write forbiden." },
    { BtErrCode::BT_ERR_GATT_MAX_SERVER, "Max gatt server." },
    { BtErrCode::BT_ERR_SPP_SERVER_STATE, "SPP server not running." },
    { BtErrCode::BT_ERR_SPP_BUSY, "SPP translate busy." },
    { BtErrCode::BT_ERR_SPP_DEVICE_NOT_FOUND, "Device is not inquired." },
    { BtErrCode::BT_ERR_SPP_IO, "SPP IO error." },
    { BtErrCode::BT_ERR_NO_ACTIVE_HFP_DEVICE, "Active hfp device is not exist." },
    { BtErrCode::BT_ERR_NULL_HFP_STATE_MACHINE, "Hfp state machine is not null." },
    { BtErrCode::BT_ERR_HFP_NOT_CONNECT, "Hfp is not connected." },
    { BtErrCode::BT_ERR_SCO_HAS_BEEN_CONNECTED, "Sco has been connected." },
    { BtErrCode::BT_ERR_VR_HAS_BEEN_STARTED, "Voice recognition has been started." },
    { BtErrCode::BT_ERR_AUDIO_NOT_IDLE, "Audio is not idle." },
    { BtErrCode::BT_ERR_VIRTUAL_CALL_NOT_STARTED, "Virtual call is not started." },
    { BtErrCode::BT_ERR_DISCONNECT_SCO_FAILED, "Disconnect sco failed." },
    { BtErrCode::BT_ERR_MAX_RESOURCES, "The number of resources reaches the upper limit." },
    { BtErrCode::BT_ERR_BLE_ADV_DATA_EXCEED_LIMIT, "The length of the advertising data exceeds the upper limit." },
    { BtErrCode::BT_ERR_BLE_INVALID_ADV_ID, "Invalid advertising id." },
    { BtErrCode::BT_ERR_OPERATION_BUSY, "The operation is busy. The last operation is not complete." },
    { BtErrCode::BT_ERR_GATT_CONNECTION_NOT_ESTABILISHED, "The connection is not established." },
    { BtErrCode::BT_ERR_GATT_CONNECTION_CONGESTED, "The connection is congested." },
    { BtErrCode::BT_ERR_GATT_CONNECTION_NOT_ENCRYPTED, "The connection is not encrypted." },
    { BtErrCode::BT_ERR_GATT_CONNECTION_NOT_AUTHENTICATED, "The connection is not authenticated." },
    { BtErrCode::BT_ERR_GATT_CONNECTION_NOT_AUTHORIZED, "The connection is not authorized." },
    { BtErrCode::BT_ERR_BLE_SCAN_NO_RESOURCE, "Fails to start scan as it is out of hardware resources."},
    { BtErrCode::BT_ERR_BLE_SCAN_ALREADY_STARTED, "Failed to start scan as Ble scan is already started by the app."},
    { BtErrCode::BT_ERR_DIALOG_FOR_USER_NOT_RESPOND, "The user does not respond."},
    { BtErrCode::BT_ERR_DIALOG_FOR_USER_REFUSE, "User refuse the action."},
};
std::string GetTaiheErrMsg(const int32_t errCode)
{
    auto iter = taiheErrMsgMap.find(errCode);
    if (iter != taiheErrMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        errMessage.append(std::to_string(errCode)).append(": ").append(iter->second);
        return errMessage;
    }
    return "Inner error.";
}

void HandleSyncErr(int32_t errCode)
{
    if (errCode == BtErrCode::BT_NO_ERROR) {
        return;
    }
    std::string errMsg = GetTaiheErrMsg(errCode);
    if (errMsg != "") {
        taihe::set_business_error(errCode, errMsg.c_str());
    }
}
} // namespace Bluetooth
} // namespace OHOS