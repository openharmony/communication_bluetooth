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

#include "bluetooth_a2dp_src_proxy.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
#define MAX_A2DP_VIRTUAL_DEVICE 10
int32_t BluetoothA2dpSrcProxy::Connect(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothA2dpSrcProxy::Disconnect(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_DISCONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

void BluetoothA2dpSrcProxy::RegisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(
        data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_REGISTER_OBSERVER, data, reply, option);
}

void BluetoothA2dpSrcProxy::DeregisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_DEREGISTER_OBSERVER, data, reply, option);
}

int BluetoothA2dpSrcProxy::GetDevicesByStates(const std::vector<int32_t> &states, std::vector<RawAddress> &rawAddrs)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(WriteParcelableInt32Vector(states, data), BT_ERR_IPC_TRANS_FAILED, "write states error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_DEVICE_BY_STATES,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int errorCode = reply.ReadInt32();
    if (errorCode == NO_ERROR) {
        int32_t rawAddsSize = reply.ReadInt32();
        const int32_t maxSize = 100;
        if (rawAddsSize > maxSize) {
            return BT_ERR_INVALID_PARAM;
        }
        for (int i = 0; i < rawAddsSize; i++) {
            rawAddrs.push_back(RawAddress(reply.ReadString()));
        }
    }

    return errorCode;
}

int BluetoothA2dpSrcProxy::GetDeviceState(const RawAddress &device, int &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_DEVICE_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int errorCode = reply.ReadInt32();
    if (errorCode == NO_ERROR) {
        state = reply.ReadInt32();
    }

    return errorCode;
}

int32_t BluetoothA2dpSrcProxy::GetPlayingState(const RawAddress &device, int &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_PLAYING_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t exception = reply.ReadInt32();
    if (exception == NO_ERROR) {
        state = reply.ReadInt32();
    }
    return exception;
}

int BluetoothA2dpSrcProxy::SetConnectStrategy(const RawAddress &device, int32_t strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(strategy), BT_ERR_IPC_TRANS_FAILED, "write strategy error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SET_CONNECT_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::GetConnectStrategy(const RawAddress &device, int &strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_CONNECT_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t res = reply.ReadInt32();
    if (res == NO_ERROR) {
        strategy = reply.ReadInt32();
    }
    return res;
}

int BluetoothA2dpSrcProxy::SetActiveSinkDevice(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SET_ACTIVE_SINK_DEVICE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

RawAddress BluetoothA2dpSrcProxy::GetActiveSinkDevice()
{
    MessageParcel data;
    std::string address = "";
    RawAddress rawAddress = RawAddress(address);
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        rawAddress, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_ACTIVE_SINK_DEVICE,
        data, reply, option, rawAddress);

    rawAddress = RawAddress(reply.ReadString());
    return rawAddress;
}

BluetoothA2dpCodecStatus BluetoothA2dpSrcProxy::GetCodecStatus(const RawAddress &device)
{
    MessageParcel data;
    BluetoothA2dpCodecStatus codecStatus;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        codecStatus, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), codecStatus, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_CODEC_STATUS,
        data, reply, option, codecStatus);

    std::shared_ptr<BluetoothA2dpCodecStatus> statusPtr(reply.ReadParcelable<BluetoothA2dpCodecStatus>());
    if (!statusPtr) {
        return codecStatus;
    }

    return *statusPtr;
}

int BluetoothA2dpSrcProxy::GetCodecPreference(const RawAddress &device, BluetoothA2dpCodecInfo &info)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_CODEC_PREFERENCE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t exception = reply.ReadInt32();
    if (exception != BT_NO_ERROR) {
        HILOGE("error: %{public}d", exception);
        return exception;
    }
    std::shared_ptr<BluetoothA2dpCodecInfo> bluetoothA2dpCodecInfo(reply.ReadParcelable<BluetoothA2dpCodecInfo>());
    if (bluetoothA2dpCodecInfo == nullptr) {
        HILOGE("bluetoothA2dpCodecInfo is nullptr");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    info = *bluetoothA2dpCodecInfo;
    return exception;
}

int BluetoothA2dpSrcProxy::SetCodecPreference(const RawAddress &device, const BluetoothA2dpCodecInfo &info)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&info), BT_ERR_IPC_TRANS_FAILED, "write info error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SET_CODEC_PREFERENCE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

void BluetoothA2dpSrcProxy::SwitchOptionalCodecs(const RawAddress &device, bool isEnable)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteString(device.GetAddress()), "write device error");
    CHECK_AND_RETURN_LOG(data.WriteBool(isEnable), "write isEnable error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SWITCH_OPTIONAL_CODECS, data, reply, option);
}

int BluetoothA2dpSrcProxy::GetOptionalCodecsSupportState(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_OPTIONAL_CODECS_SUPPORT_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::StartPlaying(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_START_PLAYING,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::SuspendPlaying(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SUSPEND_PLAYING,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::StopPlaying(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_STOP_PLAYING,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::WriteFrame(const uint8_t *data, uint32_t size)
{
    MessageParcel messageData;
    CHECK_AND_RETURN_LOG_RET(messageData.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    std::vector<uint8_t> dataVector;
    dataVector.assign(data, data + size);
    messageData.WriteUInt8Vector(dataVector);

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_WRITE_FRAME,
        messageData, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::GetRenderPosition(const RawAddress &device, uint32_t &delayValue, uint64_t &sendDataSize,
                                             uint32_t &timeStamp)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()), BT_ERR_IPC_TRANS_FAILED,
                             "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_RENDER_POSITION, data, reply, option,
                                   BT_ERR_IPC_TRANS_FAILED);
    int ret = reply.ReadInt32();
    delayValue = reply.ReadUint32();
    sendDataSize = reply.ReadUint64();
    timeStamp = reply.ReadUint32();
    return ret;
}

int BluetoothA2dpSrcProxy::OffloadStartPlaying(const RawAddress &device, const std::vector<int32_t> &sessionsId)
{
    return OffloadPlayingControl(device, sessionsId, BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_OFFLOAD_START_PLAYING);
}

int BluetoothA2dpSrcProxy::OffloadStopPlaying(const RawAddress &device, const std::vector<int32_t> &sessionsId)
{
    return OffloadPlayingControl(device, sessionsId, BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_OFFLOAD_STOP_PLAYING);
}

int BluetoothA2dpSrcProxy::A2dpOffloadSessionPathRequest(const RawAddress &device,
    const std::vector<BluetoothA2dpStreamInfo> &info)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(info.size()), BT_ERR_IPC_TRANS_FAILED, "write info size error");
    for (auto streamInfo : info) {
        CHECK_AND_RETURN_LOG_RET(
            data.WriteInt32(streamInfo.sessionId), BT_ERR_IPC_TRANS_FAILED, "write sessionId error");
        CHECK_AND_RETURN_LOG_RET(
            data.WriteInt32(streamInfo.streamType), BT_ERR_IPC_TRANS_FAILED, "write streamType error");
        CHECK_AND_RETURN_LOG_RET(
            data.WriteInt32(streamInfo.sampleRate), BT_ERR_IPC_TRANS_FAILED, "write sampleRate error");
        CHECK_AND_RETURN_LOG_RET(
            data.WriteInt32(streamInfo.isSpatialAudio), BT_ERR_IPC_TRANS_FAILED, "write isSpatialAudio error");
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_OFFLOAD_SESSION_REQUEST,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

BluetoothA2dpOffloadCodecStatus BluetoothA2dpSrcProxy::GetOffloadCodecStatus(const RawAddress &device)
{
    MessageParcel data;
    BluetoothA2dpOffloadCodecStatus offloadStatus;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()), offloadStatus,
        "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), offloadStatus, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_OFFLOAD_GET_CODEC_STATUS,
        data, reply, option, offloadStatus);

    std::shared_ptr<BluetoothA2dpOffloadCodecStatus> statusPtr(reply.ReadParcelable<BluetoothA2dpOffloadCodecStatus>());
    CHECK_AND_RETURN_LOG_RET(statusPtr, offloadStatus, "statusPtr is nullptr.");
    return *statusPtr;
}

bool BluetoothA2dpSrcProxy::WriteParcelableInt32Vector(const std::vector<int32_t> &parcelableVector, Parcel &reply)
{
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(parcelableVector.size()), false, "write ParcelableVector error");
    for (auto parcelable : parcelableVector) {
        CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(parcelable), false, "write parcelable error");
    }
    return true;
}

int BluetoothA2dpSrcProxy::OffloadPlayingControl(const RawAddress &device, const std::vector<int32_t> &sessionsId,
    int32_t control)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()), BT_ERR_IPC_TRANS_FAILED,
        "WriteInterfaceToken error control:%{public}d", control);
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED,
        "write device error control:%{public}d", control);
    CHECK_AND_RETURN_LOG_RET(WriteParcelableInt32Vector(sessionsId, data), BT_ERR_IPC_TRANS_FAILED,
        "write sessionId error control:%{public}d, sessions size:%{public}d", control, (int32_t)sessionsId.size());

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(control, data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::EnableAutoPlay(const RawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_ENABLE_AUTO_PLAY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);
    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::DisableAutoPlay(const RawAddress &device, const int duration)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(duration), BT_ERR_IPC_TRANS_FAILED, "write duration error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_DISABLE_AUTO_PLAY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);
    return reply.ReadInt32();
}

int BluetoothA2dpSrcProxy::GetAutoPlayDisabledDuration(const RawAddress &device, int &duration)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device.GetAddress()), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_AUTO_PLAY_DISABLED_DURATION,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);
    int32_t res = reply.ReadInt32();
    if (res == BT_NO_ERROR) {
        duration = reply.ReadInt32();
    }
    return res;
}

void BluetoothA2dpSrcProxy::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()), "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_GET_VIRTUALDEVICE_LIST, data, reply, option);

    int32_t rawAddsSize = reply.ReadInt32();

    CHECK_AND_RETURN_LOG(rawAddsSize <= MAX_A2DP_VIRTUAL_DEVICE, "virtual device size error.");

    for (int i = 0; i < rawAddsSize; i++) {
        devices.push_back(reply.ReadString());
    }

    return;
}

void BluetoothA2dpSrcProxy::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothA2dpSrcProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(action), "write action error");
    CHECK_AND_RETURN_LOG(data.WriteString(address), "write address error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN(BluetoothA2dpSrcInterfaceCode::BT_A2DP_SRC_SET_VIRTUAL_DEVICE, data, reply, option);
    return;
}
}  // namespace Bluetooth
}  // namespace OHOS