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

#ifndef OBEX_SOCKET_DEVICE_H
#define OBEX_SOCKET_DEVICE_H

#include <cstdint>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "log.h"

namespace OHOS {
namespace bluetooth {
enum class SocketType : uint8_t { TYPE_RFCOMM = 0, TYPE_L2CAP = 1, TYPE_L2CAP_LE = 2 };
const uint32_t MAX_PACKET_SIZE = 0xFFFE;
struct ObexSocketDevice {
public:
    ObexSocketDevice(int32_t socketFd, uint32_t maxTxPacketSize, uint32_t maxRxPacketSize, SocketType transportType,
        const std::string &deviceAddress)
        : mSocketFd(socketFd),
          mMaxTxPacketSize(maxTxPacketSize),
          mMaxRxPacketSize(maxRxPacketSize),
          mSocketType(transportType),
          mDeviceAddress(deviceAddress)
    {}
    ~ObexSocketDevice() { Close(); }
    void Close()
    {
        std::lock_guard<std::mutex> lock(mSocketFdLock);
        HILOGI("mSocketFd_ is %{public}d", mSocketFd);
        if (mSocketFd > 0) {
            shutdown(mSocketFd, SHUT_RD);
            shutdown(mSocketFd, SHUT_WR);
            close(mSocketFd);
            mSocketFd = -1;
        }
    }
    std::mutex mSocketFdLock;
    int32_t mSocketFd;
    uint32_t mMaxTxPacketSize = 0;
    uint32_t mMaxRxPacketSize = 0;
    SocketType mSocketType = SocketType::TYPE_RFCOMM;
    std::string mDeviceAddress = "";
};
}  // namespace bluetooth
}  // namespace OHOS
#endif