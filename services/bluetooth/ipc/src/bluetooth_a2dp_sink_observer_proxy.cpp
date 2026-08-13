/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_a2dp_sink_observer_proxy"
#endif

#include "bluetooth_a2dp_sink_observer_proxy.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
void BluetoothA2dpSinkObserverProxy::OnConnectionStateChanged(const RawAddress &device, int state, int cause)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSinkObserverProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return;
    }
    if (!data.WriteInt32(state)) {
        HILOGE("state error");
        return;
    }
    if (!data.WriteInt32(cause)) {
        HILOGE("cause error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    auto remote = Remote();
    CHECK_AND_RETURN_LOG(remote, "remote nullptr");
    int ret = remote->SendRequest(
        BluetoothA2dpSinkObserverInterfaceCode::BT_A2DP_SINK_OBSERVER_CONNECTION_STATE_CHANGED, data, reply, option);
    if (ret != NO_ERROR) {
        HILOGE("OnPlayingStateChanged done fail");
        return;
    }
}

}  // namespace Bluetooth
}  // namespace OHOS