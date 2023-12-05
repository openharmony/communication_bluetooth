/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#include <uv.h>
#include "bluetooth_utils.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_map_mse_observer.h"

namespace OHOS {
namespace Bluetooth {
std::shared_mutex NapiMapMseObserver::g_mapMseCallbackInfosMutex;
void NapiMapMseObserver::OnConnectionStateChanged(const BluetoothRemoteDevice &device, int32_t state)
{
    HILOGD("enter, remote device address: %{public}s, state: %{public}d", GET_ENCRYPT_ADDR(device), state);
    std::unique_lock<std::shared_mutex> guard(g_mapMseCallbackInfosMutex);

    std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>>::iterator it =
        callbackInfos_.find(STR_BT_MAP_MSE_CONNECTION_STATE_CHANGE);
    if (it == callbackInfos_.end() || it->second == nullptr) {
        HILOGW("This callback is not registered by ability.");
        return;
    }
    std::shared_ptr<BluetoothCallbackInfo> callbackInfo = it->second;

    callbackInfo->state_ = state;
    callbackInfo->deviceId_ = device.GetDeviceAddr();
    NapiEvent::CheckAndNotify(callbackInfo, state);
}

} // namespace Bluetooth
} // namespace OHOS