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

#include "bluetooth_a2dp_observer_impl.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
void A2dpSourceObserverImpl::OnConnectionStateChanged(const BluetoothRemoteDevice& device, int state, int cause)
{
    HILOGD("enter, remote device address: %{public}s, state: %{public}d, cause: %{public}d", GET_ENCRYPT_ADDR(device),
        state, cause);
    if (!stateChangeFunc) {
        HILOGD("failed to register state change callback");
        return;
    }
    char* deviceId = MallocCString(device.GetDeviceAddr());
    if (deviceId == nullptr) {
        HILOGD("failed to malloc memory");
        return;
    }
    StateChangeParam param = StateChangeParam {
        .deviceId = deviceId, .state = static_cast<int32_t>(state), .cause = static_cast<int32_t>(cause)
    };
    stateChangeFunc(param);
    free(deviceId);
}

void A2dpSourceObserverImpl::RegisterStateChangeFunc(std::function<void(StateChangeParam)> cjCallback)
{
    stateChangeFunc = cjCallback;
}

} // namespace Bluetooth
} // namespace OHOS