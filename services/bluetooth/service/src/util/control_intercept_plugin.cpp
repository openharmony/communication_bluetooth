/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_control_intercept"
#endif

#include "control_intercept_plugin.h"

#include <shared_mutex>
#include <vector>
#include "log.h"
#define CHECK_PLUGIN_IS_ALLOWED(msg, checkType, fun, fmt, ...)      \
    do {                                                            \
        if (msg.type == checkType && fun) {                         \
            CHECK_AND_RETURN_LOG_RET(fun(msg), false, fmt);         \
        }                                                           \
    } while (0)

static std::vector<ControlInterceptPlugin> g_controlInterceptPlugins {};
static std::shared_mutex g_controlInterceptPluginsMutex {};

static bool PluginIsAllowed(const ControlInterceptMessage &msg, const ControlInterceptPlugin &plugin);

void ControlInterceptRegisterPlugin(ControlInterceptPlugin &plugin)
{
    std::unique_lock<std::shared_mutex> lock(g_controlInterceptPluginsMutex);
    g_controlInterceptPlugins.push_back(plugin);
}

static bool ControlInterceptIsAllowedInner(const ControlInterceptMessage &msg)
{
    std::shared_lock<std::shared_mutex> lock(g_controlInterceptPluginsMutex);
    for (const auto &plugin : g_controlInterceptPlugins) {
        if (!PluginIsAllowed(msg, plugin)) {
            return false;
        }
    }
    return true;
}

void ControlInterceptDestroy(void)
{
    std::unique_lock<std::shared_mutex> lock(g_controlInterceptPluginsMutex);
    g_controlInterceptPlugins.clear();
}

static bool PluginIsAllowed(const ControlInterceptMessage &msg, const ControlInterceptPlugin &plugin)
{
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::GATT_CONN, plugin.gattConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::AVRCP_CONN, plugin.avrcpConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::PAIR_CONN, plugin.pairConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::ACL_CONN, plugin.aclConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::A2DP_CONN, plugin.a2dpConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::HFP_CONN, plugin.hfpConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::HID_CONN, plugin.hidConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::SOCKET_CONN, plugin.socketConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::GATT_FAST_CONN, plugin.fastestGattConn, "can not connect");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::GATT_WRITE, plugin.gattWrite, "can not write");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::OPP_WRITE, plugin.oppWrite, "can not write");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::GATT_ADD_SERVICE, plugin.gattAddService,
        "can not add service");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::SOCKET_LISTEN, plugin.socketListen, "can not listen");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::OPP_READ, plugin.oppRead, "can not read");
    CHECK_PLUGIN_IS_ALLOWED(msg, ControlInterceptType::GATT_RECEIVE, plugin.gattReceive, "can not receive");
    return true;
}

bool ControlInterceptIsAllowedFastestGattConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::GATT_FAST_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedGattConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::GATT_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedAvrcpConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::AVRCP_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedPairConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::PAIR_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedAclConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::ACL_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedA2dpConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::A2DP_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedHfpConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::HFP_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedHidConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::HID_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedSocketConn(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::SOCKET_CONN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedGattWrite(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::GATT_WRITE;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedOppWrite(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::OPP_WRITE;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedGattAddService(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::GATT_ADD_SERVICE;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedSocketListen(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::SOCKET_LISTEN;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedOppRead(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::OPP_READ;
    return ControlInterceptIsAllowedInner(innerMsg);
}

bool ControlInterceptIsAllowedGattReceive(const ControlInterceptMessage &msg)
{
    ControlInterceptMessage innerMsg = msg;
    innerMsg.type = ControlInterceptType::GATT_RECEIVE;
    return ControlInterceptIsAllowedInner(innerMsg);
}