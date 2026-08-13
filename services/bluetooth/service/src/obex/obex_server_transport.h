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

#ifndef OBEX_SERVER_TRANSPORT_H
#define OBEX_SERVER_TRANSPORT_H

#include <cstdint>
#include "obex_def.h"
#include "obex_socket_device.h"

namespace OHOS {
namespace bluetooth {
class ObexServerTransport {
public:
    explicit ObexServerTransport(std::shared_ptr<ObexSocketDevice> &socketDevice);
    ~ObexServerTransport();
    ssize_t Read(uint8_t *buf, size_t length, int timeoutMs = 0);
    ssize_t Write(const uint8_t *buf, size_t length);
    void Close();
    inline uint32_t GetMaxTxPacketSize() { return socketDevice_->mMaxTxPacketSize; }
    inline uint32_t GetMaxRxPacketSize() { return socketDevice_->mMaxRxPacketSize; }
    inline std::string GetRemoteAddress() { return socketDevice_->mDeviceAddress; }
    inline bool IsSrmSupported() { return socketDevice_->mSocketType != SocketType::TYPE_RFCOMM; }

private:
    bool SetRecvTimeoutMs(int socketFd, int timeoutMs);
    ssize_t ReadL2capData(uint8_t *buf, size_t length, int timeoutMs = 0);
    ssize_t ReadRfcommData(uint8_t *buf, size_t length, int timeoutMs = 0);

private:
    std::shared_ptr<ObexSocketDevice> socketDevice_ = nullptr;
    std::vector<uint8_t> l2capBuffer_{};
    uint32_t l2capBufferReadIndex_ = 0;
    uint32_t l2capBufferSize_ = 0;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif  // OBEX_SERVER_TRANSPORT_H