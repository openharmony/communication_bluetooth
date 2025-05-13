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
#define LOG_TAG "bt_cj_socket_spp_ffi"
#endif

#include "bluetooth_spp_ffi.h"

#include "bluetooth_spp_server_impl.h"
#include "bluetooth_spp_utils.h"

namespace OHOS {
namespace Bluetooth {
extern "C" {
static SppOption parseCSppOptions(CSppOption options)
{
    SppOption ret = { .uuid_ = options.uuid, .secure_ = options.secure, .type_ = BtSocketType(options.type) };
    return ret;
}

int32_t FfiBluetoothSocketSppListen(char* name, CSppOption options, int32_t* errCode)
{
    return SppServerImpl::SppListen(name, parseCSppOptions(options), errCode);
}

int32_t FfiBluetoothSocketSppAccept(int32_t serverSocket, int32_t* errCode)
{
    return SppServerImpl::SppAccept(serverSocket, errCode);
}

int32_t FfiBluetoothSocketSppConnect(char* deviceId, CSppOption options, int32_t* errCode)
{
    return SppClientImpl::SppConnect(deviceId, parseCSppOptions(options), errCode);
}

char* FfiBluetoothSocketGetDeviceId(int32_t clientSocket, int32_t* errCode)
{
    std::string deviceId = SppClientImpl::GetDeviceId(clientSocket, errCode);
    if (*errCode != SUCCESS_CODE) {
        return nullptr;
    }
    return MallocCString(deviceId);
}

void FfiBluetoothSocketSppCloseServerSocket(int32_t socket, int32_t* errCode)
{
    *errCode = SppServerImpl::SppCloseServerSocket(socket);
}

void FfiBluetoothSocketSppCloseClientSocket(int32_t socket, int32_t* errCode)
{
    *errCode = SppClientImpl::SppCloseClientSocket(socket);
}

void FfiBluetoothSocketSppWrite(int32_t clientSocket, CArrUI8 data, int32_t* errCode)
{
    *errCode = SppClientImpl::SppWrite(clientSocket, data);
}

void FfiBluetoothSocketOn(char* type, int32_t clientSocket, int64_t cbId, int32_t* errCode)
{
    *errCode = SppServerImpl::RegisterSocketObserver(type, clientSocket, cbId);
}

void FfiBluetoothSocketOff(char* type, int32_t clientSocket, int64_t cbId, int32_t* errCode)
{
    *errCode = SppServerImpl::DeRegisterSocketObserver(type, clientSocket, cbId);
}
}
} // namespace Bluetooth
} // namespace OHOS