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
#define LOG_TAG "bt_cj_socket_spp_server"
#endif

#include "bluetooth_spp_server_impl.h"

#include "bluetooth_errorcode.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
const int NUM_20 = 20;
int SppServerImpl::count = 0;
std::map<int, std::shared_ptr<SppServerImpl>> SppServerImpl::serverMap;

int32_t SppServerImpl::SppListen(std::string name, SppOption sppOption, int32_t* errCode)
{
    HILOGD("enter");
    std::shared_ptr<ServerSocket> server_ =
        std::make_shared<ServerSocket>(name, UUID::FromString(sppOption.uuid_), sppOption.type_, sppOption.secure_);
    int errorCode = server_->Listen();
    if (server_->GetStringTag() != "") {
        HILOGD("SppListen execute listen success");
    } else {
        HILOGE("SppListen execute listen failed");
        *errCode = static_cast<int32_t>(errorCode);
        return -1;
    }
    std::shared_ptr<SppServerImpl> server = std::make_shared<SppServerImpl>();
    server->id_ = SppServerImpl::count++;
    server->server_ = server_;
    serverMap.insert(std::make_pair(server->id_, server));
    return static_cast<int32_t>(server->id_);
}

int32_t SppServerImpl::SppAccept(int32_t serverSocket, int32_t* errCode)
{
    HILOGD("enter");
    std::shared_ptr<SppServerImpl> server = SppServerImpl::serverMap[serverSocket];
    if (!server) {
        HILOGE("server is null");
        *errCode = BT_ERR_INVALID_PARAM;
        return -1;
    }
    std::shared_ptr<ServerSocket> server_ = server->server_;
    std::shared_ptr<ClientSocket> client_ = server_->Accept(NUM_20);
    if (client_ == nullptr) {
        HILOGE("client_ is null");
        *errCode = BT_ERR_INVALID_PARAM;
        return -1;
    }
    std::shared_ptr<SppClientImpl> client = std::make_shared<SppClientImpl>();
    client->id_ = SppClientImpl::count++;
    client->client_ = client_;
    SppClientImpl::clientMap.insert(std::make_pair(client->id_, client));
    return static_cast<int32_t>(client->id_);
}

int32_t SppServerImpl::RegisterSocketObserver(std::string type, int32_t clientSocket, int64_t cbId)
{
    return SppClientImpl::On(type, clientSocket, cbId);
}

int32_t SppServerImpl::DeRegisterSocketObserver(std::string type, int32_t clientSocket, int64_t cbId)
{
    return SppClientImpl::Off(type, clientSocket, cbId);
}

int32_t SppServerImpl::SppCloseServerSocket(int32_t socket)
{
    HILOGD("enter");
    bool isOK = false;

    std::shared_ptr<SppServerImpl> server = nullptr;
    std::shared_ptr<SppClientImpl> client = nullptr;
    if (SppClientImpl::clientMap[socket] != nullptr) {
        client = SppClientImpl::clientMap[socket];
        if (client->client_) {
            client->client_->Close();
            SppClientImpl::clientMap.erase(socket);
        }
    } else {
        HILOGE("invalid socket.");
    }
    if (serverMap[socket] != nullptr) {
        server = serverMap[socket];
        if (server->server_) {
            server->server_->Close();
            serverMap.erase(socket);
            isOK = true;
        }
    } else {
        HILOGE("invalid socket.");
    }
    return isOK ? 0 : BT_ERR_INVALID_PARAM;
}
} // namespace Bluetooth
} // namespace OHOS