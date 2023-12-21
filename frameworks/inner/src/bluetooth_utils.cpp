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
#include <chrono>
#include <random>
#include "securec.h"
#include "__config"
#include "bluetooth_def.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "iosfwd"
#include "iservice_registry.h"
#include "string"
#include "system_ability_definition.h"

using namespace std;

namespace OHOS {
namespace Bluetooth {
constexpr int startPos = 6;
constexpr int endPos = 13;
constexpr int RANDOM_ADDR_ARRAY_SIZE = 4;
constexpr int RANDOM_ADDR_MAC_BIT_SIZE = 12;
constexpr int RANDOM_ADDR_FIRST_BIT = 1;
constexpr int RANDOM_ADDR_LAST_BIT = 11;
constexpr int RANDOM_ADDR_SPLIT_SIZE = 2;
constexpr int HEX_BASE = 16;
constexpr int OCT_BASE = 8;

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

std::string GetUpdateOutputStackActionName(int action)
{
    switch (action) {
        case static_cast<int>(UpdateOutputStackAction::ACTION_WEAR):
            return "WEAR(0)";
        case static_cast<int>(UpdateOutputStackAction::ACTION_UNWEAR):
            return "UNWEAR(1)";
        case static_cast<int>(UpdateOutputStackAction::ACTION_ENABLE_FROM_REMOTE):
            return "ENABLE_FROM_REMOTE(2)";
        case static_cast<int>(UpdateOutputStackAction::ACTION_DISABLE_FROM_REMOTE):
            return "DISABLE_FROM_REMOTE(3)";
        case static_cast<int>(UpdateOutputStackAction::ACTION_ENABLE_WEAR_DETECTION):
            return "ENABLE_WEAR_DETECTION(4)";
        case static_cast<int>(UpdateOutputStackAction::ACTION_DISABLE_WEAR_DETECTION):
            return "DISABLE_WEAR_DETECTION(5)";
        case static_cast<int>(UpdateOutputStackAction::ACTION_USER_OPERATION):
            return "USER_OPERATION(6)";
        default:
            return "Unknown";
    }
}

static std::map<int32_t, std::string> BtErrCodeMap {
    { BtErrCode::BT_NO_ERROR, "BT_NO_ERROR" },
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
    { BtErrCode::BT_ERR_NO_ACTIVE_HFP_DEVICE, "Active hfp device is not exist." },
    { BtErrCode::BT_ERR_NULL_HFP_STATE_MACHINE, "Hfp state machine is not null." },
    { BtErrCode::BT_ERR_HFP_NOT_CONNECT, "Hfp is not connected." },
    { BtErrCode::BT_ERR_SCO_HAS_BEEN_CONNECTED, "Sco has been connected." },
    { BtErrCode::BT_ERR_VR_HAS_BEEN_STARTED, "Voice recognition has been started." },
    { BtErrCode::BT_ERR_AUDIO_NOT_IDLE, "Audio is not idle." },
    { BtErrCode::BT_ERR_VIRTUAL_CALL_NOT_STARTED, "Virtual call is not started." },
    { BtErrCode::BT_ERR_DISCONNECT_SCO_FAILED, "Disconnect sco failed." },
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

void ToUpper(char* arr)
{
    for (size_t i = 0; i < strlen(arr); ++i) {
        if (arr[i] >= 'a' && arr[i] <= 'z') {
            arr[i] = toupper(arr[i]);
        }
    }
}

std::string GenerateRandomMacAddress()
{
    std::string randomMac = "";
    char strMacTmp[RANDOM_ADDR_ARRAY_SIZE] = {0};
    std::mt19937_64 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    for (int i = 0; i < RANDOM_ADDR_MAC_BIT_SIZE; i++) {
        int ret = -1;
        if (i != RANDOM_ADDR_FIRST_BIT) {
            std::uniform_int_distribution<> distribution(0, HEX_BASE - 1);
            ret = sprintf_s(strMacTmp, RANDOM_ADDR_ARRAY_SIZE, "%x", distribution(gen));
        } else {
            std::uniform_int_distribution<> distribution(0, OCT_BASE - 1);
            ret = sprintf_s(strMacTmp, RANDOM_ADDR_ARRAY_SIZE, "%x", RANDOM_ADDR_SPLIT_SIZE * distribution(gen));
        }
        if (ret == -1) {
            HILOGE("GenerateRandomMacAddress failed, sprintf_s return -1!");
        }
        ToUpper(strMacTmp);
        randomMac += strMacTmp;
        if ((i % RANDOM_ADDR_SPLIT_SIZE != 0 && (i != RANDOM_ADDR_LAST_BIT))) {
            randomMac.append(":");
        }
    }
    return randomMac;
}

bool CheckConnectionStrategyInvalid(int32_t strategy)
{
    if (strategy == static_cast<int32_t>(BTStrategyType::CONNECTION_ALLOWED) ||
        strategy == static_cast<int32_t>(BTStrategyType::CONNECTION_FORBIDDEN)) {
        return true;
    }
    return false;
}

bool CheckAccessAuthorizationInvalid(int32_t accessAuthorization)
{
    if (accessAuthorization == static_cast<int32_t>(BTPermissionType::ACCESS_UNKNOWN) ||
        accessAuthorization == static_cast<int32_t>(BTPermissionType::ACCESS_ALLOWED) ||
        accessAuthorization == static_cast<int32_t>(BTPermissionType::ACCESS_FORBIDDEN)) {
        return true;
    }
    return false;
}

}  // namespace Bluetooth
}  // namespace OHOS