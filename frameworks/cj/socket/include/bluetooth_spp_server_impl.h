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
#ifndef BLUETOOTH_SPP_SERVER_IMPL_H
#define BLUETOOTH_SPP_SERVER_IMPL_H

#include <map>

#include "bluetooth_socket.h"
#include "bluetooth_spp_client_impl.h"

namespace OHOS {
namespace Bluetooth {
struct SppServerImpl {
    static int32_t SppListen(std::string name, SppOption sppOption, int32_t* errCode);
    static int32_t SppAccept(int32_t serverSocket, int32_t* errCode);
    static int32_t SppCloseServerSocket(int32_t socket);
    static int32_t RegisterSocketObserver(std::string type, int32_t clientSocket, int64_t cbId);
    static int32_t DeRegisterSocketObserver(std::string type, int32_t clientSocket, int64_t cbId);

    static std::map<int, std::shared_ptr<SppServerImpl>> serverMap;
    static int count;

    std::shared_ptr<ServerSocket> server_ = nullptr;
    int id_ = -1;
};
} // namespace Bluetooth
} // namespace OHOS
#endif /* BLUETOOTH_SPP_SERVER_IMPL_H */