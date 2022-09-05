/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief Defines a bluetooth system that provides basic blurtooth connection and profile functions,
 *        including A2DP, AVRCP, BLE, GATT, HFP, MAP, PBAP, and SPP, etc.
 *
 */

/**
 * @file socket_service.h
 *
 * @brief Declares SPP service functions, including basic and observer functions.
 *
 */

#ifndef SOCKET_SERVICE_H
#define SOCKET_SERVICE_H

#include <mutex>
#include <string>
#include <vector>
#include "bt_uuid.h"
#include "context.h"
#include "interface_profile_socket.h"
#include "profile_service_manager.h"
#include "socket.h"

namespace OHOS {
namespace bluetooth {
/**
 * @brief This Socket class provides a set of methods that is called by Framework API, and manage
 *        the socket object and the message of other thread.
 *
 */
class SocketService : public IProfileSocket, public utility::Context {
public:
    /**
     * @brief Constructor.
     *
     */
    SocketService();

    /**
     * @brief Destructor.
     *
     */
    ~SocketService();

    utility::Context *GetContext() override;

    /**
     * @brief Starts a socket service.
     *
     * @details Switch to the thread of the Socket service in this function.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     */
    void Enable() override;

    /**
     * @brief Shut down the Socket target service.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     */
    void Disable() override;

    /**
     * @brief Connects to a peer bluetooth device.
     *
     * @param device Remote device address.operation
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     */
    int Connect(const RawAddress &device) override;

    /**
     * @brief Disconnects from a peer bluetooth device.
     *
     * @param device Remote device address.
     * @return Returns <b>true</b> if the operation is successful; returns <b>false</b> if the operation fails.
     */
    int Disconnect(const RawAddress &device) override;

    /**
     * @brief Get the connected devices list.
     * @return @c std::list<RawAddress> : the connected devices list.
     */
    std::list<RawAddress> GetConnectDevices() override;

    /**
     * @brief Get the Connect State object.
     * @return int
     */
    int GetConnectState() override;

    /**
     * @brief Get the maximum number of connected devices.
     * @return @c int : connected devices number.
     */
    int GetMaxConnectNum() override;

    /**
     * @brief process the message of the socket .
     * @param @msg: the message of the socket.
     */
    void ProcessMessage(const utility::Message &msg);

    /**
     * @brief The client initiates the connection.
     * @details The client queries the SDP and finds the channel to be connected through UUID.
     *          Client sets security level to GAP.
     * @param addr addr
     * @param uuid server record uuid to search scn.
     * @param securityFlag require the connection to be encrypted and authenticated.
     * @param type socket type.
     * @return the upper socket fd that generated by the socketpair .
     * @since 6
     */
    int Connect(const std::string &addr, const Uuid &uuid, int securityFlag, int type) override;

    /**
     * @brief The server listen and accept the connection.
     * @details The server registers service records to SDP with service name, uuid and server channel
     *          number that assigned by rfcomm. Server sets security level to GAP.
     * @param name server service name.
     * @param uuid server uuid.
     * @param securityFlag require the connection to be encrypted and authenticated.
     * @param type socket type.
     * @return the upper socket fd that generated by the socketpair.
     * @since 6
     */
    int Listen(const std::string &name, const Uuid &uuid, int securityFlag, int type) override;

    /**
     * @brief shutdown for adapter manager
     * @return int
     */
    void ShutDownInternal();

private:
    // service status
    bool isShutdown_ {false};
    // client push status
    bool isClientPush_ {false};
    // all socket object
    std::vector<std::unique_ptr<Socket>> clientSockets_ {};
    std::vector<std::unique_ptr<Socket>> serverSockets_ {};
    std::vector<Socket *> acceptSockets_ {};
    std::mutex mutex_ {};

    /**
     * @brief Enables the Socket service.
     *
     */
    void EnableNative(void);

    /**
     * @brief Disable the Socket service.
     *
     */
    void DisableNative(void);

    /**
     * @brief cleanup server socket object.
     *
     */
    void CleanupServerSocket();

    /**
     * @brief delete socket object.
     *
     */
    void CleanupClientSocket(Socket *socket);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // SOCKET_SERVICE_H
