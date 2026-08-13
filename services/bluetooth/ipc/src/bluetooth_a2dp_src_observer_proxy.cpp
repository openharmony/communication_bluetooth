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
#define LOG_TAG "bt_ipc_a2dp_src_observer_proxy"
#endif

#include "bluetooth_a2dp_src_observer_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
void BluetoothA2dpSrcObserverProxy::OnConnectionStateChanged(const RawAddress &device, int state, int cause)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcObserverProxy::GetDescriptor())) {
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

    ErrCode result = InnerTransact(BT_A2DP_SRC_OBSERVER_CONNECTION_STATE_CHANGED, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("error: %{public}d", result);
        return;
    }
}

void BluetoothA2dpSrcObserverProxy::OnCaptureConnectionStateChanged(const RawAddress &device, int state,
    const BluetoothA2dpCodecInfo &info)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcObserverProxy::GetDescriptor())) {
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
    if (!data.WriteParcelable(&info)) {
        HILOGE("transport error");
        return;
    }

    MessageParcel reply;
    MessageOption option{ MessageOption::TF_ASYNC };

    ErrCode result = InnerTransact(BT_A2DP_SRC_OBSERVER_CAPTURE_CONNECTION_STATE_CHANGED, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("error: %{public}d", result);
        return;
    }
}

void BluetoothA2dpSrcObserverProxy::OnPlayingStatusChanged(const RawAddress &device, int playingState, int error)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcObserverProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return;
    }
    if (!data.WriteInt32(playingState)) {
        HILOGE("playingState error");
        return;
    }

    if (!data.WriteInt32(error)) {
        HILOGE("error error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    ErrCode result = InnerTransact(BT_A2DP_SRC_OBSERVER_PLAYING_STATUS_CHANGED, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("error: %{public}d", result);
        return;
    }
}

void BluetoothA2dpSrcObserverProxy::OnConfigurationChanged(
    const RawAddress &device, const BluetoothA2dpCodecInfo &info, int error)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcObserverProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return;
    }
    if (!data.WriteParcelable(&info)) {
        HILOGE("transport error");
        return;
    }
    if (!data.WriteInt32(error)) {
        HILOGE("error error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    ErrCode result = InnerTransact(BT_A2DP_SRC_OBSERVER_CONFIGURATION_CHANGED, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("error: %{public}d", result);
        return;
    }
}

void BluetoothA2dpSrcObserverProxy::OnMediaStackChanged(const RawAddress &device, int action)
{
    MessageParcel data;

    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothA2dpSrcObserverProxy::GetDescriptor()),
        "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteString(device.GetAddress()), "Write device error.");
    CHECK_AND_RETURN_LOG(data.WriteInt32(action), "Write action error.");

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };
    int32_t result = InnerTransact(BT_A2DP_SRC_OBSERVER_MEDIASTACK_CHANGED, option, data, reply);
    if (result != BT_NO_ERROR) {
        HILOGE("error: %{public}d", result);
        return;
    }
}

void BluetoothA2dpSrcObserverProxy::OnVirtualDeviceChanged(int32_t action, std::string address)
{
    MessageParcel data;

    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothA2dpSrcObserverProxy::GetDescriptor()),
        "WriteInterfaceToken error");
    
    CHECK_AND_RETURN_LOG(data.WriteInt32(action), "Write action error.");
    CHECK_AND_RETURN_LOG(data.WriteString(address), "Write address error.");

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };
    int32_t result = InnerTransact(BT_A2DP_SRC_OBSERVER_VIRTUALDEVICE_CHANGED, option, data, reply);
    if (result != BT_NO_ERROR) {
        HILOGE("error: %{public}d", result);
        return;
    }
}

ErrCode BluetoothA2dpSrcObserverProxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return ERR_DEAD_OBJECT;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return ERR_OK;
        }
        case DEAD_OBJECT: {
            HILOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return ERR_DEAD_OBJECT;
        }
        default: {
            HILOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return TRANSACTION_ERR;
        }
    }
}
}  // namespace Bluetooth
}  // namespace OHOS