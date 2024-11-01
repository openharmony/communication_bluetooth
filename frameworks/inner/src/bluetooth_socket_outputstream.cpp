/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_fwk_socket_outputstream"
#endif

#include <unistd.h>
#include <cerrno>
#include <cinttypes>
#include "bluetooth_socket_outputstream.h"
#include "bluetooth_log.h"
#include "sys/socket.h"

namespace OHOS {
namespace Bluetooth {

static constexpr int32_t SOCKET_SEND_TIME_THRESHOLD = 1000; // 1000ms
static int64_t GetNowTimestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t timestamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return timestamp;
}

OutputStream::OutputStream(int socketFd) : socketFd_(socketFd)
{}

OutputStream::~OutputStream()
{}

int OutputStream::Write(const uint8_t *buf, size_t length)
{
    if (socketFd_ == -1) {
        HILOGE("socket closed.");
        return -1;
    }

    int64_t beginTimestamp = GetNowTimestamp();
    auto ret = send(socketFd_, buf, length, MSG_NOSIGNAL);
    int64_t endTimestamp = GetNowTimestamp();
    if (endTimestamp - beginTimestamp > SOCKET_SEND_TIME_THRESHOLD) {
        HILOGE("socket send time %{public}" PRId64, endTimestamp - beginTimestamp);
    }

    HILOGD("ret: %{public}zd", ret);

    if (ret <= 0) {
        HILOGE("socket write exception! ret:%{public}zd errno:%{public}d", ret, errno);
    }
    return ret;
}
}  // namespace Bluetooth
}  // namespace OHOS