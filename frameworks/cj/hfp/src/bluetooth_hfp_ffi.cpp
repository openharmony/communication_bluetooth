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
#define LOG_TAG "bt_cj_hfp_ffi"
#endif

#include "bluetooth_hfp_ffi.h"

#include "bluetooth_hfp_ag_impl.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace Bluetooth {
extern "C" {
CArrString FfiBluetoothHfpGetConnectedDevices(int32_t* errCode)
{
    return HandsFreeAudioGatewayImpl::GetConnectionDevices(errCode);
}

int32_t FfiBluetoothHfpGetConnectionState(char* device, int32_t* errCode)
{
    return HandsFreeAudioGatewayImpl::GetConnectionState(device, errCode);
}

void FfiBluetoothHfpOn(int32_t type, int64_t id, int32_t* errCode)
{
    HandsFreeAudioGatewayImpl::On(type, id, errCode);
}
}
} // namespace Bluetooth
} // namespace OHOS