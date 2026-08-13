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
#define LOG_TAG "bt_ipc_a2dp_src_stub"
#endif

#include "bluetooth_a2dp_src_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "parcel_bt_uuid.h"
#include "permission_manager.h"
#include "raw_address.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothA2dpSrcInterfaceCode::code, {&BluetoothA2dpSrcStub::func, perm}
namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
const int32_t A2DP_MAX_SRC_CONNECTION_NUMS = 0x07;
const int32_t A2DP_MAX_PLAYING_STREAMS_NUMS = 32;

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothA2dpSrcStub::A2dpSrcStubFuncPerm> BluetoothA2dpSrcStub::memberFuncMap_ = {
    {STUB_FUNC(BT_A2DP_SRC_CONNECT, ConnectInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_A2DP_SRC_DISCONNECT, DisconnectInner, CHECK_PERM(false, {DISCOVER_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_A2DP_SRC_REGISTER_OBSERVER, RegisterObserverInner, nullptr)},
    {STUB_FUNC(BT_A2DP_SRC_DEREGISTER_OBSERVER, DeregisterObserverInner, nullptr)},
    {STUB_FUNC(BT_A2DP_SRC_GET_DEVICE_BY_STATES, GetDevicesByStatesInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_A2DP_SRC_GET_DEVICE_STATE, GetDeviceStateInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_A2DP_SRC_GET_PLAYING_STATE, GetPlayingStateInner,
        CHECK_PERM(false, {USE_BLUETOOTH}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_A2DP_SRC_SET_CONNECT_STRATEGY, SetConnectStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_GET_CONNECT_STRATEGY, GetConnectStrategyInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_SET_ACTIVE_SINK_DEVICE, SetActiveSinkDeviceInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_GET_ACTIVE_SINK_DEVICE, GetActiveSinkDeviceInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_GET_CODEC_STATUS, GetCodecStatusInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_GET_CODEC_PREFERENCE, GetCodecPreferenceInner, CHECK_PERM(true, {}, {ACCESS_BLUETOOTH}))},
    {STUB_FUNC(BT_A2DP_SRC_SET_CODEC_PREFERENCE, SetCodecPreferenceInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_SWITCH_OPTIONAL_CODECS, SwitchOptionalCodecsInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_GET_OPTIONAL_CODECS_SUPPORT_STATE, GetOptionalCodecsSupportStateInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_START_PLAYING, StartPlayingInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_SUSPEND_PLAYING, SuspendPlayingInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_STOP_PLAYING, StopPlayingInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_WRITE_FRAME, WriteFrameInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_GET_RENDER_POSITION, GetRenderPositionInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_OFFLOAD_START_PLAYING, OffloadStartPlayingInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_OFFLOAD_STOP_PLAYING, OffloadStopPlayingInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_OFFLOAD_SESSION_REQUEST, A2dpOffloadSessionPathRequestInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_OFFLOAD_GET_CODEC_STATUS, GetOffloadCodecStatusInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_ENABLE_AUTO_PLAY, EnableAutoPlayInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_DISABLE_AUTO_PLAY, DisableAutoPlayInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_GET_AUTO_PLAY_DISABLED_DURATION, GetAutoPlayDisabledDurationInner,
        CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SRC_GET_VIRTUALDEVICE_LIST, GetVirtualDeviceListInner,
        CHECK_PERM(false, {}, {ACCESS_BLUETOOTH}))},
};

BluetoothA2dpSrcStub::BluetoothA2dpSrcStub()
{}

BluetoothA2dpSrcStub::~BluetoothA2dpSrcStub()
{}

int BluetoothA2dpSrcStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothA2dpSrc ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothA2dpSrcStub);
}

int32_t BluetoothA2dpSrcStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }

    int32_t result = Connect(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

int32_t BluetoothA2dpSrcStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }

    int32_t result = Disconnect(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothA2dpSourceObserver> observer = OHOS::iface_cast<IBluetoothA2dpSourceObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterObserver(observer);

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::DeregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothA2dpSourceObserver> observer = OHOS::iface_cast<IBluetoothA2dpSourceObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterObserver(observer);

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int32_t> states = {};
    int32_t stateSize = data.ReadInt32();
    if (stateSize > A2DP_MAX_SRC_CONNECTION_NUMS) {
        return BT_ERR_INVALID_PARAM;
    }

    for (int i = 0; i < stateSize; i++) {
        int32_t state = data.ReadInt32();
        states.push_back(state);
    }

    std::vector<RawAddress> rawAddrs;
    int ret = GetDevicesByStates(states, rawAddrs);
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply WriteInt32 failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (ret == NO_ERROR) {
        if (!reply.WriteInt32(rawAddrs.size())) {
            HILOGE("reply WriteInt32 failed");
            return BT_ERR_IPC_TRANS_FAILED;
        }

        for (auto rawAddr : rawAddrs) {
            if (!reply.WriteString(rawAddr.GetAddress())) {
                HILOGE("reply WriteString failed");
                return BT_ERR_IPC_TRANS_FAILED;
            }
        }
    }
    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetDeviceStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }

    int state = 0;
    int result = GetDeviceState(RawAddress(addr), state);
    if (!reply.WriteInt32(result)) {
        HILOGE("reply WriteInt32 failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (result == NO_ERROR) {
        if (!reply.WriteInt32(state)) {
            HILOGE("reply WriteInt32 failed");
            return BT_ERR_IPC_TRANS_FAILED;
        }
    }

    return NO_ERROR;
}

int32_t BluetoothA2dpSrcStub::GetPlayingStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    int32_t state = 0;
    int32_t result = GetPlayingState(RawAddress(addr), state);

    (void)reply.WriteInt32(result);
    (void)reply.WriteInt32(state);
    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::SetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    int strategy = data.ReadInt32();

    int result = SetConnectStrategy(RawAddress(addr), strategy);

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    int strategy = 0;
    int result = GetConnectStrategy(RawAddress(addr), strategy);
    if (!reply.WriteInt32(result)) {
        HILOGE("reply writing res failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!reply.WriteInt32(strategy)) {
        HILOGE("reply writing strategy failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::SetActiveSinkDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    int result = SetActiveSinkDevice(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetActiveSinkDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    RawAddress adds = GetActiveSinkDevice();
    if (!reply.WriteString(adds.GetAddress())) {
        HILOGE("write WriteString failed");
        return ERR_INVALID_STATE;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetCodecStatusInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    BluetoothA2dpCodecStatus result = GetCodecStatus(RawAddress(addr));

    bool ret = reply.WriteParcelable(&result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

int32_t BluetoothA2dpSrcStub::GetCodecPreferenceInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothA2dpCodecInfo info;
    int result = GetCodecPreference(RawAddress(addr), info);
    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (result != BT_NO_ERROR) {
        HILOGE("GetCodecPreferenceInner error.");
        return result;
    }
    if (!reply.WriteParcelable(&info)) {
        HILOGE("transport error");
        return BT_ERR_INTERNAL_ERROR;
    }

    return BT_NO_ERROR;
}

int32_t BluetoothA2dpSrcStub::SetCodecPreferenceInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::shared_ptr<BluetoothA2dpCodecInfo> info(data.ReadParcelable<BluetoothA2dpCodecInfo>());
    if (!info) {
        return BT_ERR_IPC_TRANS_FAILED;
    }
    int result = SetCodecPreference(RawAddress(addr), *info);

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return BT_NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::SwitchOptionalCodecsInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    bool isEnable = data.ReadBool();
    SwitchOptionalCodecs(RawAddress(addr), isEnable);
    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetOptionalCodecsSupportStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    int result = GetOptionalCodecsSupportState(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::StartPlayingInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    int result = StartPlaying(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::SuspendPlayingInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    int result = SuspendPlaying(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::StopPlayingInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    int result = StopPlaying(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::WriteFrameInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint8_t> dataVector;
    if (!data.ReadUInt8Vector(&dataVector)) {
        HILOGE("data reading failed.");
        return TRANSACTION_ERR;
    }

    int ret = WriteFrame(dataVector.data(), dataVector.size());
    if (!reply.WriteInt32(ret)) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetRenderPositionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    if (!IsValidAddress(addr)) {
        HILOGE("addr is invalid.");
        return TRANSACTION_ERR;
    }
    uint32_t delayValue;
    uint64_t sendDataSize;
    uint32_t timeStamp;
    int result = GetRenderPosition(RawAddress(addr), delayValue, sendDataSize, timeStamp);
    if (!reply.WriteInt32(result)) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    if (!reply.WriteUint32(delayValue)) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    if (!reply.WriteUint64(sendDataSize)) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    if (!reply.WriteUint32(timeStamp)) {
        HILOGE("reply writing failed.");
        return TRANSACTION_ERR;
    }
    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::OffloadStartPlayingInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int32_t sessionSize = data.ReadInt32();
    CHECK_AND_RETURN_LOG_RET(sessionSize != 0 && sessionSize < A2DP_MAX_PLAYING_STREAMS_NUMS,
        BT_ERR_INVALID_PARAM, "session size error:%{public}d", sessionSize);

    std::vector<int32_t> sessionsId = {};
    for (int i = 0; i < sessionSize; i++) {
        int32_t sessionId = data.ReadInt32();
        sessionsId.push_back(sessionId);
    }
    int result = OffloadStartPlaying(RawAddress(addr), sessionsId);
    bool ret = reply.WriteInt32(result);
    CHECK_AND_RETURN_LOG_RET(ret, BT_ERR_IPC_TRANS_FAILED, "reply write failed.");
    return BT_NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::OffloadStopPlayingInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int32_t sessionSize = data.ReadInt32();
    CHECK_AND_RETURN_LOG_RET(sessionSize != 0 && sessionSize < A2DP_MAX_PLAYING_STREAMS_NUMS,
        BT_ERR_INVALID_PARAM, "session size error:%{public}d", sessionSize);

    std::vector<int32_t> sessionsId = {};
    for (int i = 0; i < sessionSize; i++) {
        int32_t sessionId = data.ReadInt32();
        sessionsId.push_back(sessionId);
    }
    int result = OffloadStopPlaying(RawAddress(addr), sessionsId);
    bool ret = reply.WriteInt32(result);
    CHECK_AND_RETURN_LOG_RET(ret, BT_ERR_IPC_TRANS_FAILED, "reply write failed.");
    return BT_NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::A2dpOffloadSessionPathRequestInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int32_t infoSize = data.ReadInt32();
    CHECK_AND_RETURN_LOG_RET(infoSize < A2DP_MAX_PLAYING_STREAMS_NUMS, BT_ERR_INVALID_PARAM,
        "error size:%{public}d", infoSize);
    std::vector<BluetoothA2dpStreamInfo> streamsInfo = {};
    BluetoothA2dpStreamInfo stream;
    for (int32_t i = 0; i < infoSize; i++) {
        stream.sessionId = data.ReadInt32();
        stream.streamType = data.ReadInt32();
        stream.sampleRate = data.ReadInt32();
        stream.isSpatialAudio = data.ReadInt32();
        streamsInfo.push_back(stream);
    }
    int result = A2dpOffloadSessionPathRequest(RawAddress(addr), streamsInfo);
    bool ret = reply.WriteInt32(result);
    CHECK_AND_RETURN_LOG_RET(ret, BT_ERR_IPC_TRANS_FAILED, "reply write failed");
    return BT_NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetOffloadCodecStatusInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    BluetoothA2dpOffloadCodecStatus result = GetOffloadCodecStatus(RawAddress(addr));
    bool ret = reply.WriteParcelable(&result);
    CHECK_AND_RETURN_LOG_RET(ret, BT_ERR_IPC_TRANS_FAILED, "reply write failed");
    return BT_NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetVirtualDeviceListInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> devices;
    GetVirtualDeviceList(devices);

    if (!reply.WriteUint32(devices.size())) {
        HILOGE("BluetoothA2dpSrcStub: GetVirtualDeviceListInner reply writing failed.");
        return TRANSACTION_ERR;
    }
    
    for (const auto& address : devices) {
        if (!reply.WriteString(address)) {
            HILOGE("BluetoothA2dpSrcStub: GetVirtualDeviceListInner reply writing failed.");
            return TRANSACTION_ERR;
        }
    }

    return NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::EnableAutoPlayInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = EnableAutoPlay(RawAddress(addr));
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply write failed");
    return BT_NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::DisableAutoPlayInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int32_t duration = data.ReadInt32();
    int result = DisableAutoPlay(RawAddress(addr), duration);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply write failed");
    return BT_NO_ERROR;
}

ErrCode BluetoothA2dpSrcStub::GetAutoPlayDisabledDurationInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddress(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int duration = 0;
    int result = GetAutoPlayDisabledDuration(RawAddress(addr), duration);
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(result), BT_ERR_IPC_TRANS_FAILED, "reply write failed");
    CHECK_AND_RETURN_LOG_RET(reply.WriteInt32(duration), BT_ERR_IPC_TRANS_FAILED, "reply write failed");
    return BT_NO_ERROR;
}

bool BluetoothA2dpSrcStub::IsValidAddress(std::string addr)
{
    if (addr.empty()) {
        HILOGE("addr is empty.");
        return false;
    }
    const std::regex deviceIdRegex("^[0-9a-fA-F]{2}(:[0-9a-fA-F]{2}){5}$");
    return regex_match(addr, deviceIdRegex);
}
}  // namespace Bluetooth
}  // namespace OHOS