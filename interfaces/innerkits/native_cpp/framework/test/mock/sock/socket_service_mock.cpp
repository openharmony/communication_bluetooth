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

#include "socket_service.h"
#include "bluetooth_log.h"

namespace bluetooth {

SocketService::SocketService()
{

}

SocketService::~SocketService()
{

}

int SocketService::Connect(const std::string &addr, const Uuid &uuid, int securityFlag, int type)
{
    HILOGI("mock------------[SocketService Connect]");
    int socketFd = 12;

    return socketFd;
}

int SocketService::Listen(const std::string &name, const Uuid &uuid, int securityFlag, int type)
{
    //LOG_INFO("mock------------[SocketService]%{public}s", __func__);
    HILOGI("mock------------[SocketService Listen ]");
    int socketFd = 12;
    return socketFd;
}

}  // namespace bluetooth