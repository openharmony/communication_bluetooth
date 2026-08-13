/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef OBEX_SERVER_SOCKET_H
#define OBEX_SERVER_SOCKET_H

#include <atomic>
#include <future>
#include <mutex>
#include <thread>
#include "sys/time.h"
#include "obex_socket_observer.h"

namespace OHOS {
namespace bluetooth {
class ObexServerSocket {
public:
    explicit ObexServerSocket(const std::string &socketListenName, int32_t rfcommChannel, int32_t l2capPsm,
        const std::string &serviceUuid, std::shared_ptr<IObexSocketObserver> observer);
    ~ObexServerSocket();
    void Startup(int securityFlags);
    void Shutdown();
    int32_t GetL2capPsm();
    void SetL2capPsm(int32_t psm);

private:
    bool CreateSocket(int securityFlags);
    void StartAccept();

private:
    std::string socketListenName_ = "";
    int32_t rfcommChannel_ = 0;
    int32_t l2capPsm_ = 0;
    std::string serviceUuid_ = "";
    int32_t rfcommSocketFd_ = -1;
    int32_t l2capSocketFd_ = -1;
    struct timeval time_ = {0, 0};
    std::shared_ptr<IObexSocketObserver> observer_ = nullptr;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_SERVER_SOCKET_H