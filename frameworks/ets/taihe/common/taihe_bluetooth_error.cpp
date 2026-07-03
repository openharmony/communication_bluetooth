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
    { BtErrCode::BT_ERR_NULL_HFP_STATE_MACHINE, "Hfp state machine is null." },
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

static std::map<int32_t, int32_t> innerToBusinessErrCodeMap {
    // inner error code ->
    // business error code (ARKTS API, file: bluetooth_errorcode.h, Common error codes + Customized error codes)
    // One inner error code maps to one business error code, business error code can have multiple inner error codes.
    { BtErrCode::BT_ERR_PEERS_MAC_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_MANAGE_ADV_NAME_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_ACCESS_BLUETOOTH_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_DISCOVER_BLUETOOTH_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_MANAGE_BLUETOOTH_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_LOCATION_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_AUDIO_SERVER_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_API_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_MULTI_PERMISSION_FAILED, BtErrCode::BT_ERR_PERMISSION_FAILED },
    { BtErrCode::BT_ERR_INVALID_PARAM_ERROR, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_GATT_CHARACTER_ERROR, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_ASYNCWORK_EXIST, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_ADDRESS_NOT_EXIST, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_BLUETOOTH_TURN_ON, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_BLUETOOTH_TURNING, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_RESTRICT_STATE, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_CLOUD_DEVICE_BONDING, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_DISCOVERY_STATE_ERROR, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_CHARACTER_VALUE_ERROR, BtErrCode::BT_ERR_INVALID_PARAM },
    { BtErrCode::BT_ERR_BLE_SCAN_NO_RESOURCE, BtErrCode::BT_ERR_INTERNAL_ERROR },
    { BtErrCode::BT_ERR_GATT_CONNECT_STATE_ERROR, BtErrCode::BT_ERR_INTERNAL_ERROR },
};

static std::map<int32_t, std::string> innerErrMsgMap {
    { BtErrCode::BT_ERR_PEERS_MAC_PERMISSION_FAILED, "PERSISTENT_BLUETOOTH_PEERS_MAC permission denied." },
    { BtErrCode::BT_ERR_MANAGE_ADV_NAME_PERMISSION_FAILED, "MANAGE_BLUETOOTH_ADVERTISER_NAME permission denied." },
    { BtErrCode::BT_ERR_ACCESS_BLUETOOTH_PERMISSION_FAILED, "ACCESS_BLUETOOTH permission denied." },
    { BtErrCode::BT_ERR_DISCOVER_BLUETOOTH_PERMISSION_FAILED, "DISCOVER_BLUETOOTH permission denied." },
    { BtErrCode::BT_ERR_MANAGE_BLUETOOTH_PERMISSION_FAILED, "MANAGE_BLUETOOTH permission denied." },
    { BtErrCode::BT_ERR_LOCATION_PERMISSION_FAILED, "LOCATION permission denied." },
    { BtErrCode::BT_ERR_AUDIO_SERVER_PERMISSION_FAILED, "Only for audio_server." },
    { BtErrCode::BT_ERR_API_PERMISSION_FAILED, "Api version is unsupported." },
    { BtErrCode::BT_ERR_MULTI_PERMISSION_FAILED, "Multiple permission denied." },
    { BtErrCode::BT_ERR_INVALID_PARAM_ERROR, "Invalid parameter." },
    { BtErrCode::BT_ERR_GATT_CHARACTER_ERROR, "Operation failed. GATT character is nullptr." },
    { BtErrCode::BT_ERR_ASYNCWORK_EXIST,
        "Operation failed. Please call the interface only after the previous callback has been completed." },
    { BtErrCode::BT_ERR_ADDRESS_NOT_EXIST, "Operation failed. Address has not been discovered or recorded." },
    { BtErrCode::BT_ERR_BLUETOOTH_TURN_ON, "Operation failed. Bluetooth switch state is turn on." },
    { BtErrCode::BT_ERR_BLUETOOTH_TURNING, "Operation failed. Bluetooth switch state is turning state." },
    { BtErrCode::BT_ERR_RESTRICT_STATE, "Operation failed. In restrict bluetooth state." },
    { BtErrCode::BT_ERR_CLOUD_DEVICE_BONDING, "Operation failed. Cloud device is bonding." },
    { BtErrCode::BT_ERR_DISCOVERY_STATE_ERROR, "Operation failed. In DISCOVERYING or DISCOVERY_STARTED state." },
    { BtErrCode::BT_ERR_CHARACTER_VALUE_ERROR,
        "Invalid parameter. CharacteristicValue is null or length of characteristicValue is zero." },
    { BtErrCode::BT_ERR_BLE_SCAN_NO_RESOURCE, "Fails to start scan as it is out of hardware resources."},
    { BtErrCode::BT_ERR_GATT_CONNECT_STATE_ERROR, "Operation failed. GATT not in connected state." },
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

bool IsInnerErrorCode(int32_t errCode)
{
    return innerToBusinessErrCodeMap.find(errCode) != innerToBusinessErrCodeMap.end();
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

void HandleSyncErrWithValidCodes(int32_t errCode, const std::vector<int32_t> &validErrCodes)
{
    if (errCode == BtErrCode::BT_NO_ERROR) {
        return;
    }
    auto processResult = ProcessErrCode(errCode, validErrCodes);
    if (!processResult.errMsg.empty()) {
        taihe::set_business_error(processResult.errCode, processResult.errMsg.c_str());
    }
}

void HandleSyncErrAdapter(int32_t errCode, std::vector<int32_t> &validErrCodes)
{
    if (validErrCodes.empty()) {
        HandleSyncErr(errCode);
    } else {
        HandleSyncErrWithValidCodes(errCode, validErrCodes);
    }
}

void ConvertInnerToBusinessErrCode(int32_t innerCode, ErrInfo &info)
{
    info.errCode = innerCode;
    info.errMsg = "Unknown inner error.";
    // find business errCode
    auto mapIter = innerToBusinessErrCodeMap.find(innerCode);
    if (mapIter != innerToBusinessErrCodeMap.end()) {
        info.errCode = mapIter->second;
    }
    // find inner errMsg
    auto innerMsgIter = innerErrMsgMap.find(innerCode);
    if (innerMsgIter != innerErrMsgMap.end()) {
        info.errMsg = innerMsgIter->second;
    }
    HILOGI("innerCode: %{public}d -> errCode: %{public}d, msg: %{public}s",
        innerCode, info.errCode, info.errMsg.c_str());
}

ErrInfo ProcessErrCode(int32_t originalCode, const std::vector<int32_t> &validErrCodes)
{
    ErrInfo result = { originalCode, "" };
    // inner code: originalCode -> business errCode + specific errMsg
    if (IsInnerErrorCode(originalCode)) {
        ConvertInnerToBusinessErrCode(originalCode, result);
        return result;
    }
    bool isValidCode = false;
    for (const auto &code: validErrCodes) {
        if (code == result.errCode) {
            isValidCode = true;
            break;
        }
    }
    if (!isValidCode) {
        // invalid code: BT_ERR_INTERNAL_ERROR + specific errMsg
        result.errCode = BtErrCode::BT_ERR_INTERNAL_ERROR;
        result.errMsg = "Operation failed";
    } else {
        auto detailIter = taiheErrMsgMap.find(result.errCode);
        if (detailIter != taiheErrMsgMap.end()) {
            result.errMsg = detailIter->second;
        }
    }
    return result;
}
} // namespace Bluetooth
} // namespace OHOS