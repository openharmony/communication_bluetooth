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

#include "bluetooth_a2dp_src_proxy.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int32_t BluetoothA2dpSrcProxy::Connect(const RawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_CONNECT, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return reply.ReadInt32();
}

int32_t BluetoothA2dpSrcProxy::Disconnect(const RawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_DISCONNECT, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return reply.ReadInt32();
}

void BluetoothA2dpSrcProxy::RegisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_REGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return;
    }
}

void BluetoothA2dpSrcProxy::DeregisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_DEREGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return;
    }
}

int BluetoothA2dpSrcProxy::GetDevicesByStates(const std::vector<int32_t> &states, std::vector<RawAddress> &rawAddrs)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("[GetDevicesByStates] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!WriteParcelableInt32Vector(states, data)) {
        HILOGE("[GetDevicesByStates] fail: write result failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_DEVICE_BY_STATES, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return BT_ERR_INTERNAL_ERROR;
    }

    int errorCode = reply.ReadInt32();
    if (errorCode == NO_ERROR) {
        int32_t rawAddsSize = reply.ReadInt32();
        for (int i = 0; i < rawAddsSize; i++) {
            rawAddrs.push_back(RawAddress(reply.ReadString()));
        }
    }

    return errorCode;
}

int BluetoothA2dpSrcProxy::GetDeviceState(const RawAddress &device, int &state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_DEVICE_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int errorCode = reply.ReadInt32();
    if (errorCode == NO_ERROR) {
        state = reply.ReadInt32();
    }

    return errorCode;
}

int32_t BluetoothA2dpSrcProxy::GetPlayingState(const RawAddress &device, int &state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_PLAYING_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t exception = reply.ReadInt32();
    if (exception == NO_ERROR) {
        state = reply.ReadInt32();
    }
    return exception;
}

int BluetoothA2dpSrcProxy::SetConnectStrategy(const RawAddress &device, int32_t strategy)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(strategy)) {
        HILOGE("write strategy error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SET_CONNECT_STRATEGY, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::GetConnectStrategy(const RawAddress &device, int &strategy)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_CONNECT_STRATEGY, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t res = reply.ReadInt32();
    if (res == NO_ERROR) {
        strategy = reply.ReadInt32();
    }

    return res;
}

int BluetoothA2dpSrcProxy::SetActiveSinkDevice(const RawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return ERROR;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SET_ACTIVE_SINK_DEVICE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return ERROR;
    }

    return reply.ReadInt32();
}

RawAddress BluetoothA2dpSrcProxy::GetActiveSinkDevice()
{
    MessageParcel data;
    std::string address = "";
    RawAddress rawAddress = RawAddress(address);
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return rawAddress;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_ACTIVE_SINK_DEVICE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return rawAddress;
    }

    rawAddress = RawAddress(reply.ReadString());
    return rawAddress;
}

BluetoothA2dpCodecStatus BluetoothA2dpSrcProxy::GetCodecStatus(const RawAddress &device)
{
    MessageParcel data;
    BluetoothA2dpCodecStatus codecStatus;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return codecStatus;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return codecStatus;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_CODEC_STATUS, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return codecStatus;
    }

    std::shared_ptr<BluetoothA2dpCodecStatus> statusPtr(reply.ReadParcelable<BluetoothA2dpCodecStatus>());
    if (!statusPtr) {
        return codecStatus;
    }

    return *statusPtr;
}

int BluetoothA2dpSrcProxy::GetCodecPreference(const RawAddress &device, BluetoothA2dpCodecInfo &info)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_CODEC_PREFERENCE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return ERROR;
    }
    int32_t exception = reply.ReadInt32();
    std::shared_ptr<BluetoothA2dpCodecInfo> bluetoothA2dpCodecInfo(reply.ReadParcelable<BluetoothA2dpCodecInfo>());
    if (bluetoothA2dpCodecInfo == nullptr) {
        HILOGE("transport error");
        return ERROR;
    }
    info = *bluetoothA2dpCodecInfo;
    return exception;
}

int BluetoothA2dpSrcProxy::SetCodecPreference(const RawAddress &device, const BluetoothA2dpCodecInfo &info)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return ERROR;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return ERROR;
    }
    if (!data.WriteParcelable(&info)) {
        HILOGE("transport error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SET_CODEC_PREFERENCE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return ERROR;
    }

    return reply.ReadInt32();
}

void BluetoothA2dpSrcProxy::SwitchOptionalCodecs(const RawAddress &device, bool isEnable)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return;
    }
    if (!data.WriteBool(isEnable)) {
        HILOGW("write isEnable failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SWITCH_OPTIONAL_CODECS, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return;
    }
}

int BluetoothA2dpSrcProxy::GetOptionalCodecsSupportState(const RawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return ERROR;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_OPTIONAL_CODECS_SUPPORT_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return ERROR;
    }

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::StartPlaying(const RawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return ERROR;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_START_PLAYING, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return ERROR;
    }

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::SuspendPlaying(const RawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return ERROR;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SUSPEND_PLAYING, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return ERROR;
    }

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::StopPlaying(const RawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return ERROR;
    }
    if (!data.WriteString(device.GetAddress())) {
        HILOGE("write device error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_STOP_PLAYING, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return ERROR;
    }

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::WriteFrame(const uint8_t *data, uint32_t size)
{
    MessageParcel messageData;
    if (!messageData.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return ERROR;
    }
    std::vector<uint8_t> dataVector;
    dataVector.assign(data, data + size);
    messageData.WriteUInt8Vector(dataVector);

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_WRITE_FRAME, messageData, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return ERROR;
    }
    return reply.ReadInt32();
}

void BluetoothA2dpSrcProxy::GetRenderPosition(uint16_t &delayValue, uint16_t &sendDataSize, uint32_t &timeStamp)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return;
    }
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_RENDER_POSITION, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("error: %{public}d", error);
        return;
    }
    delayValue = reply.ReadUint16();
    sendDataSize = reply.ReadUint16();
    timeStamp = reply.ReadUint32();
}

bool BluetoothA2dpSrcProxy::WriteParcelableInt32Vector(const std::vector<int32_t> &parcelableVector, Parcel &reply)
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