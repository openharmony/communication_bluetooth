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
#ifndef LOG_TAG
#define LOG_TAG "bt_cj_socket_spp_client"
#endif

#include "bluetooth_spp_client_impl.h"

#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "cj_lambda.h"
#include "datetime_ex.h"
#include "napi_bluetooth_utils.h"
#include "securec.h"

namespace OHOS {
namespace Bluetooth {
std::map<int, std::shared_ptr<SppClientImpl>> SppClientImpl::clientMap;
int SppClientImpl::count = 0;
const int SOCKET_BUFFER_SIZE = 1024;

int32_t SppClientImpl::SppConnect(std::string deviceId, SppOption sppOption, int32_t* errCode)
{
    HILOGD("enter");
    std::shared_ptr<BluetoothRemoteDevice> device_ = std::make_shared<BluetoothRemoteDevice>(deviceId, 0);
    std::shared_ptr<ClientSocket> client_ =
        std::make_shared<ClientSocket>(*device_, UUID::FromString(sppOption.uuid_), sppOption.type_, sppOption.secure_);
    int errorCode = client_->Connect(SPP_SOCKET_PSM_VALUE);
    if (errorCode != BtStatus::BT_SUCCESS) {
        HILOGE("SppConnect failed");
        *errCode = errorCode;
        return -1;
    }
    std::shared_ptr<SppClientImpl> client = std::make_shared<SppClientImpl>();
    client->device_ = device_;
    client->id_ = SppClientImpl::count++;
    client->client_ = client_;
    clientMap.insert(std::make_pair(client->id_, client));
    HILOGD("SppConnect execute back successfully");
    return client->id_;
}

int32_t SppClientImpl::SppCloseClientSocket(int32_t socket)
{
    HILOGD("enter");
    std::shared_ptr<SppClientImpl> client = nullptr;
    bool isOK = false;

    if (clientMap[socket]) {
        client = clientMap[socket];
    } else {
        HILOGE("invalid socket.");
        return BT_ERR_INVALID_PARAM;
    }

    if (client->client_) {
        client->client_->Close();
        isOK = true;
    }
    clientMap.erase(socket);
    return isOK ? 0 : BT_ERR_INVALID_PARAM;
}

int32_t SppClientImpl::SppWrite(int32_t clientSocket, CArrUI8 data)
{
    HILOGD("enter");
    BluetoothHost* host = &BluetoothHost::GetDefaultHost();
    auto prohibitedTime = host->GetRefusePolicyProhibitedTime();
    if (prohibitedTime < 0 || prohibitedTime > GetSecondsSince1970ToNow()) {
        HILOGE("socket refuse because of Refuse Policy");
        return BT_ERR_INVALID_PARAM;
    }
    auto client = clientMap[clientSocket];
    if (client == nullptr) {
        HILOGE("client is nullptr.");
        return BT_ERR_INVALID_PARAM;
    }
    if (client->client_ == nullptr) {
        HILOGE("client_ is nullptr.");
        return BT_ERR_INVALID_PARAM;
    }
    std::shared_ptr<OutputStream> outputStream = client->client_->GetOutputStream();
    size_t totalSize = static_cast<size_t>(data.size);
    bool isOK = false;
    while (totalSize) {
        int result = outputStream->Write(data.head, totalSize);
        if (result <= 0) {
            HILOGE("outputStream.Write failed, ret = %{public}d", result);
            return BT_ERR_SPP_IO;
        }
        totalSize = totalSize - static_cast<size_t>(result);
        data.head += static_cast<size_t>(result);
        isOK = true;
    }
    return isOK ? 0 : BT_ERR_INVALID_PARAM;
}

int32_t SppClientImpl::On(std::string type, int32_t clientSocket, int64_t cbId)
{
    HILOGD("enter");
    std::shared_ptr<CjBluetoothCallbackInfo> callbackInfo = std::make_shared<BufferCallbackInfo>();
    callbackInfo->callback_ = cbId;
    std::shared_ptr<SppClientImpl> client = SppClientImpl::clientMap[clientSocket];
    if (client == nullptr) {
        HILOGE("client is nullptr.");
        return BT_ERR_INVALID_PARAM;
    }
    if (client->sppReadFlag) {
        HILOGE("client is reading... please off first");
        return BT_ERR_INVALID_PARAM;
    }
    client->sppReadFlag = true;
    client->callbackInfos_[type] = callbackInfo;
    client->thread_ = std::make_shared<std::thread>(SppClientImpl::SppRead, clientSocket);
    client->thread_->detach();
    return 0;
}

int32_t SppClientImpl::Off(std::string type, int32_t clientSocket, int64_t cbId)
{
    HILOGD("enter");
    std::shared_ptr<SppClientImpl> client = SppClientImpl::clientMap[clientSocket];
    if (client == nullptr) {
        HILOGE("client is nullptr.");
        return BT_ERR_INVALID_PARAM;
    }
    client->callbackInfos_[type] = nullptr;
    client->sppReadFlag = false;
    return 0;
}

void SppClientImpl::SppRead(int id)
{
    auto client = clientMap[id];
    if (client == nullptr || !client->sppReadFlag || client->callbackInfos_[REGISTER_SPP_READ_TYPE] == nullptr) {
        HILOGE("thread start failed.");
        return;
    }
    std::shared_ptr<InputStream> inputStream = client->client_->GetInputStream();
    uint8_t buf[SOCKET_BUFFER_SIZE];

    while (true) {
        HILOGD("thread start.");
        (void)memset_s(buf, SOCKET_BUFFER_SIZE, 0, SOCKET_BUFFER_SIZE);
        int ret = inputStream->Read(buf, sizeof(buf));
        if (ret <= 0) {
            HILOGE("inputStream.Read failed, ret = %{public}d", ret);
            return;
        } else {
            if (client == nullptr || !client->sppReadFlag || !client->callbackInfos_[REGISTER_SPP_READ_TYPE]) {
                HILOGE("failed");
                return;
            }
            std::shared_ptr<BufferCallbackInfo> callbackInfo =
                std::static_pointer_cast<BufferCallbackInfo>(client->callbackInfos_[REGISTER_SPP_READ_TYPE]);
            if (callbackInfo == nullptr) {
                HILOGE("callbackInfo nullptr");
                return;
            }

            CArrUI8 buffer {};
            buffer.head = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * ret));
            if (buffer.head == nullptr) {
                HILOGE("malloc failed!");
                return;
            }
            buffer.size = ret;
            if (memcpy_s(buffer.head, ret, buf, ret) != EOK) {
                HILOGE("memcpy_s failed!");
                free(buffer.head);
                return;
            }

            auto cFunc = reinterpret_cast<void (*)(CArrUI8)>(callbackInfo->callback_);
            std::function<void(CArrUI8)> func = CJLambda::Create(cFunc);
            func(buffer);
            free(buffer.head);
        }
    }
    return;
}

std::string SppClientImpl::GetDeviceId(int32_t clientSocket, int32_t* errCode)
{
    HILOGD("enter");
    auto client = clientMap[clientSocket];
    if (client == nullptr) {
        *errCode = BT_ERR_INVALID_PARAM;
        return std::string();
    }
    if (client->client_ == nullptr) {
        *errCode = BT_ERR_INVALID_PARAM;
        return std::string();
    }
    BluetoothRemoteDevice remoteDevice = client->client_->GetRemoteDevice();
    std::string addr = remoteDevice.GetDeviceAddr();
    return addr;
}
} // namespace Bluetooth
} // namespace OHOS