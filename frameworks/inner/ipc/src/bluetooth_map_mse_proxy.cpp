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

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_map_mse_proxy.h"

namespace OHOS {
namespace Bluetooth {
const int32_t MAP_MSE_READ_DEVICE_MAX_SIZE = 0x100;
int32_t BluetoothMapMseProxy::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "Write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothMapMseInterfaceCode::MSE_GET_DEVICE_STATE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "reply errCode: %{public}d", ret);
    state = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseProxy::GetDevicesByStates(
    const std::vector<int32_t> &states, std::vector<BluetoothRawAddress> &rawDevices)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32Vector(states), BT_ERR_IPC_TRANS_FAILED, "WriteInt32Vector error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothMapMseInterfaceCode::MSE_GET_DEVICES_BY_STATES,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "reply errCode: %{public}d", ret);
    int32_t DevNum = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((DevNum >= 0 && DevNum < MAP_MSE_READ_DEVICE_MAX_SIZE),
        BT_ERR_INTERNAL_ERROR, "Invalid DevNum: %{public}d", DevNum);

    for (int32_t i = 0; i < DevNum; i++) {
        std::shared_ptr<BluetoothRawAddress> address(reply.ReadParcelable<BluetoothRawAddress>());
        CHECK_AND_RETURN_LOG_RET((address != nullptr), BT_ERR_INTERNAL_ERROR, "address is nullptr");
        rawDevices.push_back(*address);
    }
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseProxy::Disconnect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "Write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothMapMseInterfaceCode::MSE_DISCONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothMapMseProxy::SetConnectionStrategy(const BluetoothRawAddress &device, int32_t strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "Write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(strategy), BT_ERR_IPC_TRANS_FAILED, "Write strategy error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothMapMseInterfaceCode::MSE_SET_CONNECTION_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothMapMseProxy::GetConnectionStrategy(const BluetoothRawAddress &device, int32_t &strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "Write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothMapMseInterfaceCode::MSE_GET_CONNECTION_STRATEGY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "reply errCode: %{public}d", ret);
    strategy = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothMapMseProxy::SetMessageAccessAuthorization(const BluetoothRawAddress &device,
    int32_t accessAuthorization)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "Write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(accessAuthorization),
        BT_ERR_IPC_TRANS_FAILED, "Write accessAuthorization error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothMapMseInterfaceCode::MSE_SET_ACCESS_AUTHORIZATION,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothMapMseProxy::GetMessageAccessAuthorization(const BluetoothRawAddress &device,
    int32_t &accessAuthorization)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_INTERNAL_ERROR, "Write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothMapMseInterfaceCode::MSE_GET_ACCESS_AUTHORIZATION,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "reply errCode: %{public}d", ret);
    accessAuthorization = reply.ReadInt32();
    return BT_NO_ERROR;
}

void BluetoothMapMseProxy::RegisterObserver(const sptr<IBluetoothMapMseObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "Write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothMapMseInterfaceCode::MSE_REGISTER_OBSERVER, data, reply, option);
}

void BluetoothMapMseProxy::DeregisterObserver(const sptr<IBluetoothMapMseObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothMapMseProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "Write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothMapMseInterfaceCode::MSE_DEREGISTER_OBSERVER, data, reply, option);
}
}  // namespace Bluetooth
}  // namespace OHOS