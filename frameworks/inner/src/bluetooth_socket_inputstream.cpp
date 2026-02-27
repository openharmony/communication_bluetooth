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
#ifndef LOG_TAG
#define LOG_TAG "bt_fwk_socket_inutputstream"
#endif

#include <unistd.h>
#include <cstddef>
#include "bluetooth_log.h"
#include "bluetooth_socket_inputstream.h"
#include <cerrno>
#include "sys/socket.h"

namespace OHOS {
namespace Bluetooth {
InputStream::InputStream(int socketFd) : socketFd_(socketFd)
{}

InputStream::~InputStream()
{}

ssize_t InputStream::Read(uint8_t *buf, size_t length)
{
    if (socketFd_ == -1) {
        HILOGE("socket closed");
        return -1;
    }

    auto ret = recv(socketFd_, buf, length, MSG_NOSIGNAL);

    HILOGD("ret:%{public}zd", ret);

    if (ret <= 0) {
        HILOGE("socket read exception! ret:%{public}zd errno:%{public}d", ret, errno);
    }
    return ret;
}
}  // namespace Bluetooth
}  // namespace OHOS