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

#include "bluetooth_a2dp_ffi.h"

#include "bluetooth_a2dp_impl.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace Bluetooth {
extern "C" {
int32_t FfiBluetoothA2dpGetPlayingState(char* deviceId, int32_t* errCode)
{
    return A2dpSourceProfileImpl::GetPlayingState(deviceId, errCode);
}

CArrString FfiBluetoothA2dpGetConnectedDevices(int32_t* errCode)
{
    return A2dpSourceProfileImpl::GetConnectionDevices(errCode);
}

int32_t FfiBluetoothA2dpGetConnectionState(char* device, int32_t* errCode)
{
    return A2dpSourceProfileImpl::GetConnectionState(device, errCode);
}

void FfiBluetoothA2dpOn(int32_t type, int64_t id, int32_t* errCode)
{
    A2dpSourceProfileImpl::On(type, id, errCode);
}
}
} // namespace Bluetooth
} // namespace OHOS