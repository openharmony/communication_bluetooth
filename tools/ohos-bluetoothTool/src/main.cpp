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

#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <iomanip>
#include "command_executor.h"

namespace OHOS {
namespace Bluetooth {
namespace Tool {

typedef std::function<int(int, char**)> CommandHandler;

struct Command {
    const char* name;
    const char* description;
    const char* usage;
    const char* parameters;
    const char* examples;
    CommandHandler handler;
};

static std::unordered_map<std::string, Command> g_commands;
static const char* PROGRAM_NAME = "ohos-bluetoothTool";
static bool g_hasSubCommand = true;
static const char* TOOL_DESCRIPTION = "Bluetooth device management CLI tool. Used for enabling, disabling,"
    "and querying Bluetooth state. Not suitable for Bluetooth device pairing or connection management.";

CommandExecutor g_executor;

void OutputJson(const std::string& jsonStr)
{
    std::cout << jsonStr << std::endl;
}

void PrintHelp(const std::string& targetCmd = "")
{
    if (targetCmd.empty()) {
        std::cout << PROGRAM_NAME << " - " << TOOL_DESCRIPTION << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  " << PROGRAM_NAME << " [options]" << std::endl;
        std::cout << "  " << PROGRAM_NAME << " <command> [options]" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Parameters:" << std::endl;
        std::cout << "  --help             Display this help message" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "SubCommands:" << std::endl;
        for (const auto& pair : g_commands) {
            std::cout << "  " << std::left << std::setw(18) << pair.first << " " ; // 间隔为18
            std::cout << (pair.second.description ? pair.second.description : "") << std::endl;
        }
        std::cout << "" << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  " << PROGRAM_NAME << " --help" << std::endl;
        for (const auto& pair : g_commands) {
            if (strcmp(pair.first.c_str(), "help") != 0) {
                std::cout<< "  " << PROGRAM_NAME << " " << pair.first << " --help" << std::endl;
                break;
            }
        }
        return;
    }
    auto it = g_commands.find(targetCmd);
    if (it == g_commands.end()) {
        std::cout << "{\"type\":\"result\",\"status\":\"failed\",\"errCode\":\"ERR_INVALID_COMMAND\",\"errMsg\":";
        std::cout << "\"Unknown command: " << targetCmd << "\",\"suggestion\":\"Run 'ohos-bluetoothTool --help' to";
        std::cout << "see available commands\"}" << std::endl;
        return;
    }
    const Command& cmd = it->second;
    std::cout << PROGRAM_NAME << " " << cmd.name << " - " << (cmd.description ? cmd.description : "N/A") << std::endl;
    if (cmd.usage) {
        std::cout << "" << std::endl << "Usage:" << std::endl << "  " << cmd.usage << std::endl;
    }
    if (cmd.parameters) {
        std::cout << "" << std::endl << "Parameters" << std::endl << cmd.parameters << std::endl;
    }
    std::cout << "    " << std::left << std::setw(18) << "--help" << "Display this help message" << std::endl; // 间隔为18
    if (cmd.examples) {
        std::cout << "" << std::endl << "Examples:" << std::endl << cmd.examples << std::endl;
    }
}

int CmdHelp(int argc, char** argv)
{
    std::string targetCmd;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            targetCmd = argv[i];
            break;
        }
    }

    PrintHelp(targetCmd);
    return 0;
}

int CmdEnableBt(int argc, char** argv)
{
    return g_executor.ExecuteEnableBt();
}

int CmdDisableBt(int argc, char** argv)
{
    return g_executor.ExecuteDisableBt();
}

int CmdGetState(int argc, char** argv)
{
    return g_executor.ExecuteGetState();
}

void InitCommands()
{
    g_commands["help"] = {
        "help", "Show this help message", "ohos-bluetoothTool help [command]",
        "    command          Optional. Show detailed help for a specific command.",
        "    # Show all commands\n"
        "    ohos-bluetoothTool help\n"
        "    # Show specific command help\n"
        "    ohos-bluetoothTool help enable-bt",
        CmdHelp
    };
    
    g_commands["enable-bt"] = {
        "enable-bt", "Enable Bluetooth", "ohos-bluetoothTool enable-bt",
        "    None             No parameters required.",
        "    # Enable Bluetooth\n"
        "    ohos-bluetoothTool enable-bt",
        CmdEnableBt
    };

    g_commands["disable-bt"] = {
        "disable-bt", "Disable Bluetooth", "ohos-bluetoothTool disable-bt",
        "    None             No parameters required.",
        "    # Disable Bluetooth\n"
        "    ohos-bluetoothTool disable-bt",
        CmdDisableBt
    };

    g_commands["get-state"] = {
        "get-state", "Get Bluetooth adapter state", "ohos-bluetoothTool get-state",
        "    None             No parameters required.",
        "    # Get current Bluetooth state\n"
        "    ohos-bluetoothTool get-state",
        CmdGetState
    };

    g_hasSubCommand = (g_commands.size() > 1); // 1代表超过help之外还有别的命令
}

void PrintUsage(const char* prog)
{
    std::cout << "Usage: " << prog << " <command> [options...]" << std::endl;
    std::cout << "Run '" << prog << " --help' for more information" << std::endl;
}

}
}
}

int main(int argc, char** argv)
{
    using namespace OHOS::Bluetooth::Tool;

    if (argv == nullptr || argc <= 0 || argv[0] == nullptr) {
        return 1;
    }

    const char* progName = argv[0];

    if (argc >= 2 && strcmp(argv[1], "--help") == 0) { // 2 代表2个参数
        InitCommands();
        char* helpArgv[1] = {argv[0]};
        CmdHelp(1, helpArgv); // 1 代表1个参数
        return 0;
    }

    if (argc < 2) { // 2 代表2个参数
        PrintUsage(progName);
        return 1;
    }

    InitCommands();

    std::string cmdName = argv[1];

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            char* helpArgv[2] = {argv[0], const_cast<char*>(cmdName.c_str())};
            CmdHelp(2, helpArgv);  // 2 代表2个参数
            return 0;
        }
    }

    auto it = g_commands.find(cmdName);
    if (it == g_commands.end()) {
        std::cout << "{\"type\":\"result\",\"status\":\"failed\",\"errCode\":\"ERR_INVALID_COMMAND\",\"errMsg\":";
        std::cout << "\"Unknown command: " << cmdName << "\",\"suggestion\":\"Run 'ohos-bluetoothTool --help' ";
        std::cout << "to see available commands \"}" << std::endl;
        return 1;
    }

    return it->second.handler(argc - 2, argv + 2); // 2 代表2个参数
}