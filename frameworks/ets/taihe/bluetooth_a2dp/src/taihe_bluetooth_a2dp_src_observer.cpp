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
#define LOG_TAG "bt_taihe_a2dp_src_observer"
#endif

#include "ohos.bluetooth.a2dp.proj.hpp"
#include "ohos.bluetooth.a2dp.impl.hpp"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_a2dp_src_observer.h"

namespace OHOS {
namespace Bluetooth {

TaiheA2dpSourceObserver::TaiheA2dpSourceObserver()
{}

void TaiheA2dpSourceObserver::OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause)
{
    std::shared_lock<std::shared_mutex> guard(eventSubscribe_.lock_);
    HILOGD("enter, remote device address: %{public}s, state: %{public}d, cause: %{public}d",
        GET_ENCRYPT_ADDR(device), state, cause);
    ::taihe::string deviceId_(device.GetDeviceAddr());
    int32_t profileConnectionState = TaiheUtils::GetProfileConnectionState(state);
    auto state_ = ::ohos::bluetooth::constant::ProfileConnectionState::from_value(profileConnectionState);
    auto cause_ = ::ohos::bluetooth::baseProfile::DisconnectCause::from_value(cause);
    auto taiheResult = ::ohos::bluetooth::baseProfile::StateChangeParam {
        .deviceId = deviceId_,
        .state = state_,
        .cause = cause_,
    };
    for (auto callback : eventSubscribe_.callbackVec_) {
        if (callback) {
            (*callback)(taiheResult);
        }
    }
}

}  // namespace Bluetooth
}  // namespace OHOS