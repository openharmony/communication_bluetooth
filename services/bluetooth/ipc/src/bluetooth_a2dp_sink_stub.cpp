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
#define LOG_TAG "bt_ipc_a2dp_sink_stub"
#endif

#include "bluetooth_a2dp_sink_stub.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "common_util.h"
#include "ipc_types.h"
#include "parcel_bt_uuid.h"
#include "permission_manager.h"
#include "raw_address.h"
#include "bt_hicollie_adapter.h"

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, func, perm) BluetoothA2dpSinkInterfaceCode::code, {&BluetoothA2dpSinkStub::func, perm}

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
const int32_t A2DP_MAX_SNK_CONNECTION_NUMS = 0x07;

// Note: Permissions need to be configured when the itf to be used. "nullptr" means no permission needed.
const std::map<uint32_t, BluetoothA2dpSinkStub::A2dpSinkStubFuncPerm> BluetoothA2dpSinkStub::memberFuncMap_ = {
    {STUB_FUNC(BT_A2DP_SINK_CONNECT, ConnectInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SINK_DISCONNECT, DisconnectInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SINK_REGISTER_OBSERVER, RegisterObserverInner, nullptr)},
    {STUB_FUNC(BT_A2DP_SINK_DEREGISTER_OBSERVER, DeregisterObserverInner, nullptr)},
    {STUB_FUNC(BT_A2DP_SINK_GET_DEVICE_BY_STATES, GetDevicesByStatesInner, nullptr)},
    {STUB_FUNC(BT_A2DP_SINK_GET_DEVICE_STATE, GetDeviceStateInner, nullptr)},
    {STUB_FUNC(BT_A2DP_SINK_GET_PLAYING_STATE, GetPlayingStateInner, nullptr)},
    {STUB_FUNC(BT_A2DP_SINK_SET_CONNECT_STRATEGY, SetConnectStrategyInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SINK_GET_CONNECT_STRATEGY, GetConnectStrategyInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
    {STUB_FUNC(BT_A2DP_SINK_SEND_DELAY, SendDelayInner,
        CHECK_PERM(false, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH)))},
};

BluetoothA2dpSinkStub::BluetoothA2dpSinkStub()
{}

BluetoothA2dpSinkStub::~BluetoothA2dpSinkStub()
{}

int BluetoothA2dpSinkStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::string name = "BluetoothA2dpSink ipcCode: " + std::to_string(code);
    BtHicollieAdapter hicollie(name);
    CHECK_PERMISSION_AND_EXECUTE_FUNC_RETURN(BluetoothA2dpSinkStub);
}

int32_t BluetoothA2dpSinkStub::ConnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = Connect(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::DisconnectInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");

    int result = Disconnect(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothA2dpSinkObserver> observer = OHOS::iface_cast<IBluetoothA2dpSinkObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    RegisterObserver(observer);

    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::DeregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    const sptr<IBluetoothA2dpSinkObserver> observer = OHOS::iface_cast<IBluetoothA2dpSinkObserver>(remote);
    CHECK_AND_RETURN_LOG_RET(observer != nullptr, ERR_INVALID_VALUE, "observer is nullptr");
    DeregisterObserver(observer);

    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::GetDevicesByStatesInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int32_t> states = {};
    int32_t stateSize = data.ReadInt32();
    if (stateSize > A2DP_MAX_SNK_CONNECTION_NUMS) {
        return ERR_INVALID_STATE;
    }

    for (int i = 0; i < stateSize; i++) {
        int32_t state = data.ReadInt32();
        states.push_back(state);
    }

    std::vector<RawAddress> rawAdds = GetDevicesByStates(states);
    reply.WriteInt32(rawAdds.size());

    for (auto rawAdd : rawAdds) {
        if (!reply.WriteString(rawAdd.GetAddress())) {
            HILOGE("write WriteString failed");
            return ERR_INVALID_STATE;
        }
    }
    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::GetDeviceStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");

    int result = GetDeviceState(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::GetPlayingStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int32_t state = 0;
    int result = GetPlayingState(RawAddress(addr), state);

    (void)reply.WriteInt32(result);
    (void)reply.WriteInt32(state);
    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::SetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int strategy = data.ReadInt32();

    int result = SetConnectStrategy(RawAddress(addr), strategy);

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::GetConnectStrategyInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int result = GetConnectStrategy(RawAddress(addr));

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}

int32_t BluetoothA2dpSinkStub::SendDelayInner(MessageParcel &data, MessageParcel &reply)
{
    std::string addr = data.ReadString();
    CHECK_AND_RETURN_LOG_RET(IsValidAddr(addr), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int delayValue = data.ReadInt32();

    int result = SendDelay(RawAddress(addr), delayValue);

    bool ret = reply.WriteInt32(result);
    if (!ret) {
        HILOGE("reply writing failed");
        return TRANSACTION_ERR;
    }

    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS