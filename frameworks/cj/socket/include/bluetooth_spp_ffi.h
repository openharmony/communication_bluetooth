/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_SPP_FFI_H
#define BLUETOOTH_SPP_FFI_H

#include "cj_common_ffi.h"

extern "C" {
struct CSppOption {
    char* uuid;
    bool secure;
    int32_t type;
};

FFI_EXPORT int32_t FfiBluetoothSocketSppListen(char* name, CSppOption options, int32_t* errCode);
FFI_EXPORT int32_t FfiBluetoothSocketSppAccept(int32_t serverSocket, int32_t* errCode);
FFI_EXPORT int32_t FfiBluetoothSocketSppConnect(char* deviceId, CSppOption options, int32_t* errCode);
FFI_EXPORT char* FfiBluetoothSocketGetDeviceId(int32_t clientSocket, int32_t* errCode);
FFI_EXPORT void FfiBluetoothSocketSppCloseServerSocket(int32_t socket, int32_t* errCode);
FFI_EXPORT void FfiBluetoothSocketSppCloseClientSocket(int32_t socket, int32_t* errCode);
FFI_EXPORT void FfiBluetoothSocketSppWrite(int32_t clientSocket, CArrUI8 data, int32_t* errCode);
FFI_EXPORT void FfiBluetoothSocketOn(char* type, int32_t clientSocket, int64_t cbId, int32_t* errCode);
FFI_EXPORT void FfiBluetoothSocketOff(char* type, int32_t clientSocket, int64_t cbId, int32_t* errCode);
}

#endif
