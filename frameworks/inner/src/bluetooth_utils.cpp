/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "bluetooth_utils.h"
#include <map>
#include "__config"
#include "bluetooth_errorcode.h"
#include "bluetooth_def.h"
#include "bluetooth_log.h"
#include "iosfwd"
#include "string"

using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int startPos = 6;
constexpr int endPos = 13;
std::string GetEncryptAddr(std::string addr)
{
    if (addr.empty() || addr.length() != ADDRESS_LENGTH) {
        HILOGE("addr is invalid.");
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    // 00:01:**:**:**:05
    for (int i = startPos; i <= endPos; i++) {
        out[i] = tmp[i];
    }
    return out;
}

std::string GetBtStateName(int state)
{
    switch (state) {
        case BTStateID::STATE_TURNING_ON:
            return "STATE_TURNING_ON(0)";
        case BTStateID::STATE_TURN_ON:
            return "STATE_TURN_ON(1)";
        case BTStateID::STATE_TURNING_OFF:
            return "STATE_TURNING_OFF(2)";
        case BTStateID::STATE_TURN_OFF:
            return "STATE_TURN_OFF(3)";
        default:
            return "Unknown";
    }
}

std::string GetBtTransportName(int transport)
{
    switch (transport) {
        case BTTransport::ADAPTER_BREDR:
            return "ADAPTER_BREDR(0)";
        case BTTransport::ADAPTER_BLE:
            return "ADAPTER_BLE(1)";
        default:
            return "Unknown";
    }
}

std::string GetProfileConnStateName(int state)
{
    switch (state) {
        case static_cast<int>(BTConnectState::CONNECTING):
            return "CONNECTING(0)";
        case static_cast<int>(BTConnectState::CONNECTED):
            return "CONNECTED(1)";
        case static_cast<int>(BTConnectState::DISCONNECTING):
            return "DISCONNECTING(2)";
        case static_cast<int>(BTConnectState::DISCONNECTED):
            return "DISCONNECTED(3)";
        default:
            return "Unknown";
    }
}

static std::map<int32_t, std::string> BtErrCodeMap {
    { BtErrCode::BT_SUCCESS, "BT_SUCCESS" },
    { BtErrCode::BT_ERR_PERMISSION_FAILED, "BT_ERR_PERMISSION_FAILED" },
    { BtErrCode::BT_ERR_SYSTEM_PERMISSION_FAILED, "BT_ERR_SYSTEM_PERMISSION_FAILED" },
    { BtErrCode::BT_ERR_INVALID_PARAM, "BT_ERR_INVALID_PARAM" },
    { BtErrCode::BT_ERR_API_NOT_SUPPORT, "BT_ERR_API_NOT_SUPPORT" },
    { BtErrCode::BT_ERR_SERVICE_DISCONNECTED, "BT_ERR_SERVICE_DISCONNECTED" },
    { BtErrCode::BT_ERR_UNBONDED_DEVICE, "BT_ERR_UNBONDED_DEVICE" },
    { BtErrCode::BT_ERR_INVALID_STATE, "BT_ERR_INVALID_STATE" },
    { BtErrCode::BT_ERR_PROFILE_DISABLED, "BT_ERR_PROFILE_DISABLED" },
    { BtErrCode::BT_ERR_DEVICE_DISCONNECTED, "BT_ERR_DEVICE_DISCONNECTED" },
    { BtErrCode::BT_ERR_MAX_CONNECTION, "BT_ERR_MAX_CONNECTION" },
    { BtErrCode::BT_ERR_INTERNAL_ERROR, "BT_ERR_INTERNAL_ERROR" },
    { BtErrCode::BT_ERR_IPC_TRANS_FAILED, "BT_ERR_IPC_TRANS_FAILED" },
    { BtErrCode::BT_ERR_GATT_READ_NOT_PERMITTED, "BT_ERR_GATT_READ_NOT_PERMITTED" },
    { BtErrCode::BT_ERR_GATT_WRITE_NOT_PERMITTED, "BT_ERR_GATT_WRITE_NOT_PERMITTED" },
    { BtErrCode::BT_ERR_GATT_MAX_SERVER, "BT_ERR_GATT_MAX_SERVER" },
    { BtErrCode::BT_ERR_SPP_SERVER_STATE, "BT_ERR_SPP_SERVER_STATE" },
    { BtErrCode::BT_ERR_SPP_BUSY, "BT_ERR_SPP_BUSY" },
    { BtErrCode::BT_ERR_SPP_DEVICE_NOT_FOUND, "BT_ERR_SPP_DEVICE_NOT_FOUND" },
    { BtErrCode::BT_ERR_SPP_IO, "BT_ERR_SPP_IO" },
};

std::string GetErrorCode(int32_t errCode)
{
    std::string errlog = "unknown error code: ";
    auto iter = BtErrCodeMap.find(errCode);
    if (iter != BtErrCodeMap.end()) {
        errlog = iter->second;
    }
    errlog.append("(").append(std::to_string(errCode)).append(")");
    return errlog;
}

}  // namespace Bluetooth
}  // namespace OHOS