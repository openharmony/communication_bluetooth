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
#define LOG_TAG "bt_server_svc_cmd"
#endif

#include "file_ex.h"
#include "bluetooth_host_server.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "interface_adapter_manager.h"
#include "string_ex.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
namespace {
constexpr int32_t MIN_ARGS_SIZE = 1;
constexpr int32_t SVC_RET_SUCCESS = 0;
constexpr int32_t SVC_RET_FAILED = -1;
const std::string ARGS_HELP = "help";
const std::string ARGS_ENABLE = "enable";
const std::string ARGS_DISABLE = "disable";
}

static void ShowSvcHelp(std::string& result)
{
    result.append("svc bluetooth help:\n")
        .append("svc bluetooth enable: enable bluetooth device\n")
        .append("svc bluetooth disable: disable bluetooth device\n");
}

int32_t BluetoothHostServer::OnSvcCmd(int32_t fd, const std::vector<std::u16string>& args)
{
    std::string info = "";
    sptr<BluetoothHostServer> hostServer = BluetoothHostServer::GetInstance();
    int32_t svcResult = SVC_RET_FAILED;
    CHECK_AND_RETURN_LOG_RET(hostServer != nullptr, SVC_RET_FAILED, "hostServer is nullptr");
    if (args.size() != MIN_ARGS_SIZE) {
        ShowSvcHelp(info);
        std::string error = "wrong parameter size\n" + info;
        if (!SaveStringToFd(fd, error)) {
            HILOGE("Bluetooth device save string to fd failed.");
        }
        return svcResult;
    }
    std::string cmd = Str16ToStr8(args[0]);
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    if (cmd == ARGS_HELP) {
        ShowSvcHelp(info);
        svcResult = SVC_RET_SUCCESS;
    } else if (cmd == ARGS_ENABLE) {
        bool noAutoConnect = false;
        int32_t ret = hostServer->EnableBle(noAutoConnect);
        if (ret == BT_NO_ERROR || ret == BT_ERR_SWITCH_OP_TRANSFERRED) {
            info = "bluetooth enable success\n";
            svcResult = SVC_RET_SUCCESS;
        } else {
            info = "bluetooth enable failed\n";
        }
    } else if (cmd == ARGS_DISABLE) {
        int32_t ret = hostServer->DisableBt();
        info = "bluetooth disable success\n";
        svcResult = SVC_RET_SUCCESS;
    } else {
        ShowSvcHelp(info);
        std::string error = "wrong parameter name\n" + info;
    }
    if (!SaveStringToFd(fd, info)) {
        HILOGE("Bluetooth device save string to fd failed.");
    }
    return svcResult;
}
}  // namespace Bluetooth
}  // namespace OHOS
