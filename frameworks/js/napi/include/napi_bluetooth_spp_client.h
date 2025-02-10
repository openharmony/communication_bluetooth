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
#ifndef NAPI_BLUETOOTH_SPP_CLIENT_H_
#define NAPI_BLUETOOTH_SPP_CLIENT_H_

#include <thread>
#include <map>
#include <queue>
#include "bluetooth_socket.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
struct SppConnectCallbackInfo : public AsyncCallbackInfo {
    std::shared_ptr<ClientSocket> client_ = nullptr;
    std::string deviceId_ = "";
    std::shared_ptr<BluetoothRemoteDevice> device_ = nullptr;
    std::shared_ptr<SppOption> sppOption_ = nullptr;
};

struct SppCallbackBuffer {
    ssize_t len_;
    char data_[1024];
};

struct BufferCallbackInfo : public BluetoothCallbackInfo {
    void PushData(const std::shared_ptr<SppCallbackBuffer> &data)
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

constexpr const char *REGISTER_SPP_READ_TYPE = "sppRead";
std::shared_ptr<SppOption> GetSppOptionFromJS(napi_env env, napi_value object);

struct NapiSppClient {
    static napi_value SppConnect(napi_env env, napi_callback_info info);
    static napi_value SppCloseClientSocket(napi_env env, napi_callback_info info);
    static napi_value SppWrite(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static void SppRead(int id);
    static napi_value SppWriteAsync(napi_env env, napi_callback_info info);
    static napi_value SppReadAsync(napi_env env, napi_callback_info info);

    static int count;
    static std::map<int, std::shared_ptr<NapiSppClient>> clientMap;

    int id_ = -1;
    bool sppReadFlag = false;
    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> callbackInfos_ = {};

    std::shared_ptr<ClientSocket> client_ = nullptr;
    std::shared_ptr<BluetoothRemoteDevice> device_ = nullptr;
    std::shared_ptr<std::thread> thread_;
    napi_threadsafe_function sppReadThreadSafeFunc_ = nullptr;
};
} // namespace Bluetooth
} // namespace OHOS
#endif /* NAPI_BLUETOOTH_SPP_CLIENT_H_ */