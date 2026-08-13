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

#ifndef CONTROL_INTERCEPT_PLUGIN_H
#define CONTROL_INTERCEPT_PLUGIN_H

#include <string>
#include <vector>

enum class ControlInterceptType {
    GATT_FAST_CONN = 1,
    GATT_CONN = 2,
    AVRCP_CONN = 3,
    PAIR_CONN = 4,
    ACL_CONN = 5,
    A2DP_CONN = 6,
    HFP_CONN = 7,
    HID_CONN = 8,
    SOCKET_CONN = 9,
    GATT_WRITE = 10,
    OPP_WRITE = 11,
    GATT_ADD_SERVICE = 12,
    SOCKET_LISTEN = 13,
    OPP_READ = 14,
    GATT_RECEIVE = 15,
};

struct ControlInterceptMessage {
    ControlInterceptType type;
    std::string addr;
    bool isSystemHap = false;
    bool isNativeCaller = false;
    std::vector<std::string> filePaths = {};
    int pid = 0;
    int uid = 0;
};

/**
 * @brief 判断是否允许Gatt快速连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 如果允许连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedFastestGattConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断是否允许Gatt连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 如果允许连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedGattConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断Avrcp是否允许连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 如果允许连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedAvrcpConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断Pair是否允许连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 如果允许连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedPairConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断Acl是否允许连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 如果允许连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedAclConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断A2dp是否允许连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 如果允许连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedA2dpConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断Hfp是否允许连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 如果允许连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedHfpConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断Hid是否允许连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedHidConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断Socket是否允许连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的允许连接，返回值，如果true，否则返回false
 */
bool ControlInterceptIsAllowedSocketConn(const ControlInterceptMessage &msg);

/**
 * @brief 判断Gatt是否允许写
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许写，返回true，否则返回false
 */
bool ControlInterceptIsAllowedGattWrite(const ControlInterceptMessage &msg);

/**
 * @brief 判断Opp是否允许写
 * @param msg 输入参数，包含地址和文件路径列表
 * @return 返回bool类型的值，如果允许写返回true否则返回false
 */
bool ControlInterceptIsAllowedOppWrite(const ControlInterceptMessage &msg);

/**
 * @brief GATT添加服务管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许添加服务，返回true，否则返回false
 */
bool ControlInterceptIsAllowedGattAddService(const ControlInterceptMessage &msg);

/**
 * @brief Socket监听管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许监听，返回true，否则返回false
 */
bool ControlInterceptIsAllowedSocketListen(const ControlInterceptMessage &msg);

/**
 * @brief 判断Opp是否允许读
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许读，返回true，否则返回false
 */
bool ControlInterceptIsAllowedOppRead(const ControlInterceptMessage &msg);

/**
 * @brief 判断GATT是否被连接
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许被连接，返回true，否则返回false
 */
bool ControlInterceptIsAllowedGattReceive(const ControlInterceptMessage &msg);

/**
 * @brief GATT快速连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*GattFastConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief GATT连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*GattConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief AVRCP连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*AvrcpConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief Pair连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*PairConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief ACL连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*AclConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief A2DP连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*A2dpConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief HFP连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*HfpConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief HID连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*HidConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief SOCKET连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许连接，返回true，否则返回false
 */
typedef bool (*SocketConnControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief GATT写管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许写，返回true，否则返回false
 */
typedef bool (*GattWriteControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief OPP写管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许写，返回true，否则返回false
 */
typedef bool (*OppWriteControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief GATT添加服务管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许添加服务，返回true，否则返回false
 */
typedef bool (*GattAddServiceControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief Socket监听管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许监听，返回true，否则返回false
 */
typedef bool (*SocketListenControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief OPP读管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许读，返回true，否则返回false
 */
typedef bool (*OppReadControlIntercept)(const ControlInterceptMessage &msg);

/**
 * @brief GATT被连接管控函数
 * @param msg 输入参数，包含地址和调用者信息
 * @return 返回bool类型的值，如果允许被连接，返回true，否则返回false
 */
typedef bool (*GattReceiveControlIntercept)(const ControlInterceptMessage &msg);

struct ControlInterceptPlugin {
    GattFastConnControlIntercept fastestGattConn = nullptr;
    GattConnControlIntercept gattConn = nullptr;
    AvrcpConnControlIntercept avrcpConn = nullptr;
    PairConnControlIntercept pairConn = nullptr;
    AclConnControlIntercept aclConn = nullptr;
    A2dpConnControlIntercept a2dpConn = nullptr;
    HfpConnControlIntercept hfpConn = nullptr;
    HidConnControlIntercept hidConn = nullptr;
    SocketConnControlIntercept socketConn = nullptr;
    GattWriteControlIntercept gattWrite = nullptr;
    OppWriteControlIntercept oppWrite = nullptr;
    GattAddServiceControlIntercept gattAddService = nullptr;
    SocketListenControlIntercept socketListen = nullptr;
    OppReadControlIntercept oppRead = nullptr;
    GattReceiveControlIntercept gattReceive = nullptr;
};

void ControlInterceptRegisterPlugin(ControlInterceptPlugin &plugin);

class ControlInterceptPluginCreator {
public:
    explicit ControlInterceptPluginCreator(ControlInterceptPlugin &plugin)
    {
        ControlInterceptRegisterPlugin(plugin);
    }
    ~ControlInterceptPluginCreator() = default;
};

#define REGISTER_CONTROL_INTERCEPT_PLUGIN(plugin) \
    static const ControlInterceptPluginCreator pluginCreator(plugin)

/**
 * @brief 销毁管控拦截模块，仅用于DT用例测试
 */
void ControlInterceptDestroy(void);

#endif  // CONTROL_INTERCEPT_PLUGIN_H
