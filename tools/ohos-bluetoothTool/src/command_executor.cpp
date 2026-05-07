/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "command_executor.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace OHOS {
namespace Bluetooth {
namespace Tool {

constexpr int BT_NO_ERROR = 0;

CommandExecutor::CommandExecutor()
{
}

CommandExecutor::~CommandExecutor()
{
}

std::string EscapeJson(const std::string& str)
{
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"':
                oss << "\\\"";
                break;
            case '\\':
                oss << "\\\\";
                break;
            case '\n':
                oss << "\\n";
                break;
            case '\r':
                oss << "\\r";
                break;
            case '\t':
                oss << "\\t";
                break;
            default:
                oss << c;
                break;
        }
    }
    return oss.str();
}

std::string CreateSuccessJson(const std::string& dataJson)
{
    return "{\"type\":\"result\",\"status\":\"success\",\"data\":" + dataJson + "}";
}

std::string CreateErrorJson(const std::string& errCode, const std::string & errMsg, const std::string& suggestion)
{
    return "{\"type\":\"result\",\"status\":\"failed\",\"errCode\":\"" + EscapeJson(errCode) +
        "\",\"errMsg\":\"" + EscapeJson(errMsg) +
        "\",\"suggestion\":\"" + EscapeJson(suggestion) + "\"}";
}

int CommandExecutor::ExecuteEnableBt()
{
    int result = btTool_.EnableBluetooth();
    if (result == BT_NO_ERROR) {
        std::cout << CreateSuccessJson("{\"message\":\"Bluetooth enabled successfully\"}") << std::endl;
        return 0;
    } else {
        std::cout << CreateErrorJson("ERR_BT_ENABLE_FAILED",
            "Failed to enable Bluetooth",
            "Please check if Bluetooth service is running and you have permission") << std::endl;
        return 1;
    }
}

int CommandExecutor::ExecuteDisableBt()
{
    int result = btTool_.DisableBluetooth();
    if (result == BT_NO_ERROR) {
        std::cout << CreateSuccessJson("{\"message\":\"Bluetooth disabled successfully\"}") << std::endl;
        return 0;
    } else {
        std::cout << CreateErrorJson("ERR_BT_DISABLE_FAILED",
            "Failed to disable Bluetooth",
            "Please check if Bluetooth service is running properly") << std::endl;
        return 1;
    }
}

int CommandExecutor::ExecuteGetState()
{
    int state = btTool_.GetBtState();
    bool brEnabled = btTool_.IsBrEnabled();
    bool bleEnabled = btTool_.IsBleEnabled();

    std::ostringstream data;
    data << "{\"state\":\"" << Utils::BtStateToString(state) << "\","
        << "\"stateCode\":" << Utils::GetStateCode(state) << ","
        <<"\"description\":\"" << Utils::BtStateToDescription(state) << "\","
        <<"\"classicBluetooth\":\"" << (brEnabled ? "Enabled" : "Disabled") << "\","
        <<"\"ble\":\"" << (bleEnabled ? "Enabled" : "Disabled") << "\"}";

    std::cout << CreateSuccessJson(data.str()) << std::endl;
    return 0;
}

}
}
}