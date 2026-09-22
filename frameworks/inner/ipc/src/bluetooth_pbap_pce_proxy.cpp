/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_pbap_pce_proxy"
#endif

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_pbap_pce_proxy.h"

namespace OHOS {
namespace Bluetooth {
const int32_t PBAP_PCE_READ_DEVICE_MAX_SIZE = 0x100;

int32_t BluetoothPbapPceProxy::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "GetDeviceState: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED,
        "GetDeviceState: write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothPbapPceInterfaceCode::PBAP_PCE_GET_DEVICE_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "GetDeviceState: reply errCode: %{public}d", ret);
    state = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothPbapPceProxy::GetDevicesByStates(
    const std::vector<int32_t> &states, std::vector<BluetoothRawAddress> &rawDevices)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "GetDevicesByStates: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32Vector(states), BT_ERR_IPC_TRANS_FAILED,
        "GetDevicesByStates: WriteInt32Vector error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothPbapPceInterfaceCode::PBAP_PCE_GET_DEVICES_BY_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "GetDevicesByStates: reply errCode: %{public}d", ret);
    int32_t devNum = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((devNum >= 0 && devNum < PBAP_PCE_READ_DEVICE_MAX_SIZE),
        BT_ERR_IPC_TRANS_FAILED, "GetDevicesByStates: Invalid devNum: %{public}d", devNum);

    for (int32_t i = 0; i < devNum; i++) {
        std::shared_ptr<BluetoothRawAddress> address(reply.ReadParcelable<BluetoothRawAddress>());
        CHECK_AND_RETURN_LOG_RET((address != nullptr), BT_ERR_IPC_TRANS_FAILED,
            "GetDevicesByStates: address is nullptr");
        rawDevices.push_back(*address);
    }
    return BT_NO_ERROR;
}

int32_t BluetoothPbapPceProxy::Connect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "Connect: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "Connect: write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothPbapPceInterfaceCode::PBAP_PCE_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothPbapPceProxy::Disconnect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "Disconnect: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "Disconnect: write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothPbapPceInterfaceCode::PBAP_PCE_DISCONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothPbapPceProxy::GetPhoneBookSyncState(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "GetPhoneBookSyncState: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED,
        "GetPhoneBookSyncState: write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothPbapPceInterfaceCode::PBAP_PCE_GET_PHONEBOOK_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothPbapPceProxy::SetConnectionStrategy(const BluetoothRawAddress &device, int32_t strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "SetConnectionStrategy: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED,
        "SetConnectionStrategy: Write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(strategy), BT_ERR_IPC_TRANS_FAILED,
        "SetConnectionStrategy: Write strategy error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothPbapPceInterfaceCode::PBAP_PCE_SET_CONNECT_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothPbapPceProxy::GetConnectionStrategy(const BluetoothRawAddress &device, int32_t &strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "GetConnectionStrategy: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED,
        "GetConnectionStrategy: Write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothPbapPceInterfaceCode::PBAP_PCE_GET_CONNECT_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "GetConnectionStrategy: reply errCode: %{public}d", ret);
    strategy = reply.ReadInt32();
    return BT_NO_ERROR;
}

void BluetoothPbapPceProxy::RegisterObserver(const sptr<IBluetoothPbapPceObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        "RegisterObserver: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "RegisterObserver: Write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothPbapPceInterfaceCode::PBAP_PCE_REGISTER_OBSERVER, data, reply, option);
}

void BluetoothPbapPceProxy::DeregisterObserver(const sptr<IBluetoothPbapPceObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothPbapPceProxy::GetDescriptor()),
        "DeregisterObserver: WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "DeregisterObserver: Write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothPbapPceInterfaceCode::PBAP_PCE_DEREGISTER_OBSERVER, data, reply, option);
}
}  // namespace Bluetooth
}  // namespace OHOS