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

#ifndef BLUETOOTH_A2DP_FFI_H
#define BLUETOOTH_A2DP_FFI_H

#include "cj_common_ffi.h"
#include "ffi_remote_data.h"

extern "C" {
FFI_EXPORT int32_t FfiBluetoothA2dpGetPlayingState(char* deviceId, int32_t* errCode);
FFI_EXPORT CArrString FfiBluetoothA2dpGetConnectedDevices(int32_t* errCode);
FFI_EXPORT int32_t FfiBluetoothA2dpGetConnectionState(char* device, int32_t* errCode);
FFI_EXPORT void FfiBluetoothA2dpOn(int32_t type, int64_t id, int32_t* errCode);
}

#endif
