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
#ifndef BLUETOOTH_SPP_CLIENT_IMPL_H
#define BLUETOOTH_SPP_CLIENT_IMPL_H

#include <map>
#include <queue>
#include <thread>

#include "bluetooth_socket.h"
#include "bluetooth_spp_utils.h"
#include "cj_common_ffi.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
struct CjBluetoothCallbackInfo {
    int64_t callback_ = 0;
    std::string deviceId_;
};

struct SppCallbackBuffer {
    ssize_t len_;
    char data_[1024];
};

struct BufferCallbackInfo : public CjBluetoothCallbackInfo {
    void PushData(const std::shared_ptr<SppCallbackBuffer>& data)
    {
        std::lock_guard<std::mutex> lock(bufferLock_);
        buffer_.push(data);
    }

    std::shared_ptr<SppCallbackBuffer> PopData()
    {
        std::lock_guard<std::mutex> lock(bufferLock_);
        if (buffer_.empty()) {
            return nullptr;
        }
        std::shared_ptr<SppCallbackBuffer> ret = buffer_.front();
        buffer_.pop();
        return ret;
    }

private:
    std::mutex bufferLock_;
    std::queue<std::shared_ptr<SppCallbackBuffer>> buffer_;
};

constexpr const char* REGISTER_SPP_READ_TYPE = "sppRead";

struct SppClientImpl {
    static int32_t SppConnect(std::string deviceId, SppOption sppOption, int32_t* errCode);
    static int32_t SppCloseClientSocket(int32_t socket);
    static int32_t SppWrite(int32_t clientSocket, CArrUI8 data);
    static int32_t On(std::string type, int32_t clientSocket, int64_t cbId);
    static int32_t Off(std::string type, int32_t clientSocket, int64_t cbId);
    static void SppRead(int id);
    static std::string GetDeviceId(int32_t clientSocket, int32_t* errCode);

    static int count;
    static std::map<int, std::shared_ptr<SppClientImpl>> clientMap;

    int id_ = -1;
    bool sppReadFlag = false;
    std::map<std::string, std::shared_ptr<CjBluetoothCallbackInfo>> callbackInfos_ = {};

    std::shared_ptr<ClientSocket> client_ = nullptr;
    std::shared_ptr<BluetoothRemoteDevice> device_ = nullptr;
    std::shared_ptr<std::thread> thread_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif /* BLUETOOTH_SPP_CLIENT_IMPL_H */