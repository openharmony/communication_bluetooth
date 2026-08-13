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
#define LOG_TAG "bt_service_obex_server_transport"
#endif

#ifndef OSI_NO_EINTR_AND_EAGAIN
#define OSI_NO_EINTR_AND_EAGAIN(fn) \
do {                                \
} while((fn) == -1 && (errno == EINTR || errno == EAGAIN))
#endif

#include "obex_def.h"
#include "obex_server_transport.h"
#include <sys/socket.h>
#include <unistd.h>
#include "log.h"
#include "common_util.h"
#include "securec.h"
#include <poll.h>

namespace OHOS {
namespace bluetooth {
ObexServerTransport::ObexServerTransport(std::shared_ptr<ObexSocketDevice> &socketDevice) : socketDevice_(socketDevice)
{
    HILOGI("ObexServerTransport Create");
    if (socketDevice_->mSocketType == SocketType::TYPE_L2CAP ||
        socketDevice_->mSocketType == SocketType::TYPE_L2CAP_LE) {
        l2capBuffer_ = std::vector<uint8_t>(socketDevice_->mMaxRxPacketSize);
        l2capBufferReadIndex_ = 0;
        l2capBufferSize_ = 0;
    }
}

ObexServerTransport::~ObexServerTransport()
{
    socketDevice_ = nullptr;
    HILOGI("ObexServerTransport destroy");
}

ssize_t ObexServerTransport::Read(uint8_t *buf, size_t length, int timeoutMs)
{
    if (socketDevice_ == nullptr) {
        HILOGE("socketDevice_ closed");
        return -1;
    }

    if (buf == nullptr || length == 0) {
        HILOGE("buf is nullptr or length error");
        return -1;
    }

    if (socketDevice_->mSocketType == SocketType::TYPE_L2CAP ||
        socketDevice_->mSocketType == SocketType::TYPE_L2CAP_LE) {
        return ReadL2capData(buf, length, timeoutMs);
    } else {
        return ReadRfcommData(buf, length, timeoutMs);
    }
}

bool ObexServerTransport::SetRecvTimeoutMs(int socketFd, int timeoutMs)
{
    if (timeoutMs <= 0) {
        return true;
    }
    struct pollfd fds[1];
    fds[0].fd = socketFd;
    fds[0].events = POLLIN;  // 监听读事件

    // 设置超时时间 activity = 0 表示超时未收到数据
    int timeout_ms = timeoutMs;
    int activity = -1;
    OSI_NO_EINTR_AND_EAGAIN(activity = poll(fds, 1, timeout_ms));
    if (activity <= 0) {
        HILOGE("socket exception! activity is %{public}d", activity);
        return false;
    } else if (fds[0].revents & POLLIN) {
        return true;
    } else {
        HILOGE("socket recv exception!");
        return false;
    }
}

ssize_t ObexServerTransport::ReadL2capData(uint8_t *buf, size_t length, int timeoutMs)
{
    if (socketDevice_ == nullptr || socketDevice_->mSocketFd == -1) {
        HILOGE("socket closed");
        return -1;
    }
    if (l2capBufferReadIndex_ == l2capBufferSize_) {
        if (SetRecvTimeoutMs(socketDevice_->mSocketFd, timeoutMs)) {
            ssize_t ret = recv(socketDevice_->mSocketFd,
                l2capBuffer_.data(), socketDevice_->mMaxRxPacketSize, MSG_NOSIGNAL);
            if (ret <= 0) {
                HILOGE("socket exception! errno:%{public}d", errno);
                return -1;
            }
            l2capBufferSize_ = static_cast<uint32_t>(ret);
            l2capBufferReadIndex_ = 0;
        } else {
            HILOGE("setRecvTimeoutMs failed");
            return -1;
        }
    }
    if (l2capBufferSize_ < l2capBufferReadIndex_) {
        HILOGE("buffer size or buffer read index error");
        return -1;
    }
    if (length > (l2capBufferSize_ - l2capBufferReadIndex_)) {
        length = l2capBufferSize_ - l2capBufferReadIndex_;
    }
    (void)memcpy_s(buf, length, l2capBuffer_.data() + l2capBufferReadIndex_, length);
    l2capBufferReadIndex_ += length;
    return length;
}

ssize_t ObexServerTransport::ReadRfcommData(uint8_t *buf, size_t length, int timeoutMs)
{
    ssize_t ret = -1;
    if (socketDevice_ == nullptr || socketDevice_->mSocketFd == -1) {
        HILOGE("socket closed");
        return ret;
    }
    if (SetRecvTimeoutMs(socketDevice_->mSocketFd, timeoutMs)) {
        ret = recv(socketDevice_->mSocketFd, buf, length, MSG_NOSIGNAL);
        HILOGI("ret:%{public}d length:%{public}d", ret, length);
        if (ret <= 0) {
            HILOGE("socket exception! errno:%{public}d", errno);
        }
    } else {
        HILOGE("socket recv exception!");
    }
    return ret;
}

ssize_t ObexServerTransport::Write(const uint8_t *buf, size_t length)
{
    if (socketDevice_ == nullptr) {
        HILOGE("socketDevice_ closed");
        return -1;
    }
    if (socketDevice_->mSocketFd == -1) {
        HILOGE("socket closed.");
        return -1;
    }
    auto ret = send(socketDevice_->mSocketFd, buf, length, MSG_NOSIGNAL);
    HILOGI("Write buf length is %{public}d", length);
    if (ret == -1) {
        HILOGE("Error.");
    }
    return ret;
}

void ObexServerTransport::Close()
{
    if (socketDevice_ == nullptr) {
        HILOGE("socketDevice_ closed");
        return;
    }
    socketDevice_->Close();
}
}  // namespace bluetooth
}  // namespace OHOS