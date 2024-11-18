/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_a2dp_sink_proxy"
#endif

#include "bluetooth_a2dp_sink_proxy.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int BluetoothA2dpSinkProxy::Connect(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSinkProxy::Disconnect(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_DISCONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

void BluetoothA2dpSinkProxy::RegisterObserver(const sptr<IBluetoothA2dpSinkObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(
        data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_REGISTER_OBSERVER, data, reply, option);
}

void BluetoothA2dpSinkProxy::DeregisterObserver(const sptr<IBluetoothA2dpSinkObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(
        data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_DEREGISTER_OBSERVER, data, reply, option);
}

std::vector<RawAddress> BluetoothA2dpSinkProxy::GetDevicesByStates(const std::vector<int32_t> &states)
{
    MessageParcel data;
    std::vector<RawAddress> rawAdds = {};
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()),
        rawAdds, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(WriteParcelableInt32Vector(states, data), rawAdds, "write rawAdds error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_GET_DEVICE_BY_STATES,
        data, reply, option, rawAdds);

    int32_t rawAddsSize = reply.ReadInt32();
    const int32_t maxSize = 100;
    if (rawAddsSize > maxSize) {
        return rawAdds;
    }
    for (int i = 0; i < rawAddsSize; i++) {
        rawAdds.push_back(RawAddress(reply.ReadString()));
    }
    return rawAdds;
}

int BluetoothA2dpSinkProxy::GetDeviceState(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_GET_DEVICE_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSinkProxy::GetPlayingState(const RawAddress &device, int &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_GET_PLAYING_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t exception = reply.ReadInt32();
    if (exception == NO_ERROR) {
        state = reply.ReadInt32();
    }
    return exception;
}

int BluetoothA2dpSinkProxy::SetConnectStrategy(const RawAddress &device, int32_t strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(strategy), BT_ERR_IPC_TRANS_FAILED, "write strategy error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_SET_CONNECT_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSinkProxy::GetConnectStrategy(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_GET_CONNECT_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSinkProxy::SendDelay(const RawAddress &device, int32_t delayValue)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSinkProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(delayValue), BT_ERR_IPC_TRANS_FAILED, "write delayValue error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSinkInterfaceCode::BT_A2DP_SINK_SEND_DELAY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

bool BluetoothA2dpSinkProxy::WriteParcelableInt32Vector(
    const std::vector<int32_t> &parcelableVector, Parcel &reply)
{
    if (!reply.WriteInt32(parcelableVector.size())) {
        HILOGE("write ParcelableVector failed");
        return false;
    }

    for (auto parcelable : parcelableVector) {
        if (!reply.WriteInt32(parcelable)) {
            HILOGE("write ParcelableVector failed");
            return false;
        }
    }
    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS