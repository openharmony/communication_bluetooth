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
#include "interface_profile_socket.h"
#include "raw_address.h"
#include "context.h"

namespace bluetooth {
/**
 * @brief This Socket class provides a set of methods that is called by Framework API, and manage
 *        the socket object and the message of other thread.
 *
 */
class SocketService : public IProfileSocket {
public:

    SocketService();
    ~SocketService();

    int Connect(const std::string &addr, const Uuid &uuid, int securityFlag, int type) override;
    int Listen(const std::string &name, const Uuid &uuid, int securityFlag, int type) override;

    int Connect(const RawAddress &device) override {return 0;};
    int Disconnect(const RawAddress &device) override {return 0;};
    std::list<RawAddress> GetConnectDevices() override {std::list<RawAddress> addr; return addr;};
    int GetConnectState() override {return 0;};
    int GetMaxConnectNum() override {return 0;};
    utility::Context *GetContext() override {return nullptr;};

};
}
#endif  // SOCKET_SERVICE_H
