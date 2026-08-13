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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_obex_server_socket"
#endif

#include <sys/socket.h>
#include "bt_sock.h"

#include "interface_profile_socket.h"
#include "interface_profile_manager.h"
#include "log.h"
#include "obex_server_socket_thread.h"
#include "obex_server_socket.h"

namespace OHOS {
namespace bluetooth {
static ObexServerSocketThread g_socketThread;
constexpr int8_t DYNAMIC_PSM = -2;

ObexServerSocket::ObexServerSocket(const std::string &socketListenName, int32_t rfcommChannel, int32_t l2capPsm,
    const std::string &serviceUuid, std::shared_ptr<IObexSocketObserver> observer)
    : socketListenName_(socketListenName),
      rfcommChannel_(rfcommChannel),
      l2capPsm_(l2capPsm),
      serviceUuid_(serviceUuid),
      observer_(observer)
{
    HILOGI("ObexServerSocket Create, socketListenName: %{public}s, \
        rfcommChannel: %{public}d, l2capPsm_: %{public}d",
        socketListenName_.c_str(),
        rfcommChannel_,
        l2capPsm_);
}

ObexServerSocket::~ObexServerSocket()
{
    HILOGI("~ObexServerSocket()");
}

void ObexServerSocket::Startup(int securityFlags)
{
    HILOGI("ObexServerSocket Startup");

    g_socketThread.InitThread();

    if (!CreateSocket(securityFlags)) {
        return;
    }

    StartAccept();
}

int32_t ObexServerSocket::GetL2capPsm()
{
    return l2capPsm_;
}

void ObexServerSocket::SetL2capPsm(int32_t psm)
{
    l2capPsm_ = psm;
}

void ObexServerSocket::Shutdown()
{
    HILOGI("intro");

    if (rfcommSocketFd_ != -1) {
        g_socketThread.ThreadRemoveFdAndClose(rfcommSocketFd_);
        rfcommSocketFd_ = -1;
    }

    if (l2capSocketFd_ != -1) {
        g_socketThread.ThreadRemoveFdAndClose(l2capSocketFd_);
        l2capSocketFd_ = -1;
    }

    g_socketThread.DeInitThread();
    HILOGI("ObexServerSocket Shutdown finish");
}

bool ObexServerSocket::CreateSocket(int securityFlags)
{
    HILOGI("intro");

    IProfileSocket *socketService =
        (IProfileSocket *)IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_SPP);
    if (socketService == nullptr) {
        HILOGE("ObexServerSocket socketService is null");
        return false;
    }

    Uuid serverUuid = Uuid::ConvertFromString(serviceUuid_);
    if (rfcommChannel_ > 0) {
        rfcommSocketFd_ = socketService->Listen(socketListenName_, serverUuid, securityFlags, 0, rfcommChannel_);
        if (rfcommSocketFd_ == -1) {
            HILOGE("ObexServerSocket rfcommSocketFd Listen fail");
            return false;
        }
        setsockopt(rfcommSocketFd_, SOL_SOCKET, SO_SNDTIMEO, static_cast<void *>(&time_), sizeof(time_));
        setsockopt(rfcommSocketFd_, SOL_SOCKET, SO_RCVTIMEO, static_cast<void *>(&time_), sizeof(time_));
        HILOGD("ObexServerSocket CreateSocket, rfcommSocketFd = %{public}d", rfcommSocketFd_);
    }

    if (l2capPsm_ > 0 || l2capPsm_ == DYNAMIC_PSM) {
        l2capSocketFd_ = socketService->Listen(socketListenName_, serverUuid, securityFlags, 1, l2capPsm_);
        if (l2capSocketFd_ == -1) {
            HILOGE("ObexServerSocket l2capSocketFd Listen fail");
            return false;
        }
        if (l2capPsm_ == DYNAMIC_PSM) {
            int32_t channel = 0;
            auto ret = recv(l2capSocketFd_, &channel, sizeof(channel), MSG_NOSIGNAL);
            if (ret != static_cast<int>(sizeof(channel))) {
                HILOGE("recv cmd errno: %{public}d", errno);
                return false;
            }
            SetL2capPsm(channel);
        }
        setsockopt(l2capSocketFd_, SOL_SOCKET, SO_SNDTIMEO, static_cast<void *>(&time_), sizeof(time_));
        setsockopt(l2capSocketFd_, SOL_SOCKET, SO_RCVTIMEO, static_cast<void *>(&time_), sizeof(time_));
        HILOGD("ObexServerSocket CreateSocket, l2capSocketFd = %{public}d", l2capSocketFd_);
    }

    return true;
}

void ObexServerSocket::StartAccept()
{
    HILOGI("ObexServerSocket StartAccept");

    if (rfcommSocketFd_ != -1) {
        g_socketThread.ThreadAddFd(rfcommSocketFd_, static_cast<int>(SocketType::TYPE_RFCOMM),
            OBEX_SOCK_THREAD_FD_RD, observer_);
    }
    if (l2capSocketFd_ != -1) {
        g_socketThread.ThreadAddFd(l2capSocketFd_, static_cast<int>(SocketType::TYPE_L2CAP),
            OBEX_SOCK_THREAD_FD_RD, observer_);
    }
}
}  // namespace bluetooth
}  // namespace OHOS