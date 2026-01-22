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
#define LOG_TAG "bt_cj_hid_host_observer"
#endif

#include "bluetooth_hid_host_observer_impl.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
void BluetoothHidHostObserverImpl::RegisterStateChangeFunc(std::function<void(StateChangeParam)> cjCallback)
{
    [[maybe_unused]] std::function<void(StateChangeParam)> stateChangeFuncCopy;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        stateChangeFuncCopy = stateChangeFunc;
        stateChangeFunc = cjCallback;
    }
}

void BluetoothHidHostObserverImpl::OnConnectionStateChanged(const BluetoothRemoteDevice& device, int state, int cause)
{
    HILOGD("enter, remote device address: %{public}s, state: %{public}d, cause: %{public}d", GET_ENCRYPT_ADDR(device),
        state, cause);
    if (!stateChangeFunc) {
        HILOGE("failed to register state change callback");
        return;
    }
    char* deviceId = MallocCString(device.GetDeviceAddr());
    if (deviceId == nullptr) {
        HILOGE("failed to malloc memory");
        return;
    }
    StateChangeParam param = StateChangeParam {
        .deviceId = deviceId, .state = static_cast<int32_t>(state), .cause = static_cast<int32_t>(cause)
    };
    std::function<void(StateChangeParam)> stateChangeFuncCopy;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        stateChangeFuncCopy = stateChangeFunc;
    }
    stateChangeFuncCopy(param);
    free(deviceId);
    return;
}
} // namespace Bluetooth
} // namespace OHOS