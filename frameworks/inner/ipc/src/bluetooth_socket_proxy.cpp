/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_socket_proxy"
#endif

#include "bluetooth_socket_proxy.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int BluetoothSocketProxy::Connect(ConnectSocketParam &param, int &fd)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(param.addr), BT_ERR_IPC_TRANS_FAILED, "write param.addr error");
    BluetoothUuid btUuid(param.uuid);
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&btUuid), BT_ERR_IPC_TRANS_FAILED, "write btUuid error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteInt32(param.securityFlag), BT_ERR_IPC_TRANS_FAILED, "write param.securityFlag error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(param.type), BT_ERR_IPC_TRANS_FAILED, "write param.type error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(param.psm), BT_ERR_IPC_TRANS_FAILED, "write param.psm error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothSocketInterfaceCode::SOCKET_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int errorCode = reply.ReadInt32();
    if (errorCode == NO_ERROR) {
        fd = reply.ReadFileDescriptor();
    }
    return errorCode;
}

int BluetoothSocketProxy::Listen(ListenSocketParam &param, int &fd)
{
    fd = BT_INVALID_SOCKET_FD;
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(param.name), BT_ERR_IPC_TRANS_FAILED, "write param.name error");
    BluetoothUuid btUuid(param.uuid);
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&btUuid), BT_ERR_IPC_TRANS_FAILED, "write btUuid error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteInt32(param.securityFlag), BT_ERR_IPC_TRANS_FAILED, "write param.securityFlag error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(param.type), BT_ERR_IPC_TRANS_FAILED, "write param.type error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(param.observer->AsObject()), BT_ERR_IPC_TRANS_FAILED, "write object errorr");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothSocketInterfaceCode::SOCKET_LISTEN,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int errorCode = reply.ReadInt32();
    if (errorCode == NO_ERROR) {
        fd = reply.ReadFileDescriptor();
    }

    return errorCode;
}

int BluetoothSocketProxy::DeregisterServerObserver(const sptr<IBluetoothServerSocketObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteRemoteObject(observer->AsObject()),
        BT_ERR_IPC_TRANS_FAILED, "write object errorr");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothSocketInterfaceCode::DEREGISTER_SERVER_OBSERVER,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothSocketProxy::RegisterClientObserver(const BluetoothRawAddress &dev, const bluetooth::Uuid uuid,
    const sptr<IBluetoothClientSocketObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&dev), BT_ERR_IPC_TRANS_FAILED, "write dev error");
    BluetoothUuid btUuid(uuid);
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&btUuid), BT_ERR_IPC_TRANS_FAILED, "write btUuid error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(observer->AsObject()), BT_ERR_IPC_TRANS_FAILED, "write object errorr");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothSocketInterfaceCode::REGISTER_CLIENT_OBSERVER,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothSocketProxy::DeregisterClientObserver(const BluetoothRawAddress &dev, const bluetooth::Uuid uuid,
    const sptr<IBluetoothClientSocketObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&dev), BT_ERR_IPC_TRANS_FAILED, "write dev error");

    BluetoothUuid btUuid(uuid);
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&btUuid), BT_ERR_IPC_TRANS_FAILED, "write btUuid error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(observer->AsObject()), BT_ERR_IPC_TRANS_FAILED, "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothSocketInterfaceCode::DEREGISTER_CLIENT_OBSERVER,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothSocketProxy::UpdateCocConnectionParams(const BluetoothSocketCocInfo &info)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&info), BT_ERR_IPC_TRANS_FAILED, "write info error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothSocketInterfaceCode::SOCKET_UPDATE_COC_PARAMS,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothSocketProxy::IsAllowSocketConnect(int socketType, const std::string &addr, bool &isAllowed)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(socketType), BT_ERR_IPC_TRANS_FAILED, "write socketType error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(addr), BT_ERR_IPC_TRANS_FAILED, "write addr error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothSocketInterfaceCode::SOCKET_IS_ALLOW_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t exception = reply.ReadInt32();
    if (exception == BT_NO_ERROR) {
        isAllowed = reply.ReadBool();
    }
    return exception;
}
}  // namespace Bluetooth
}  // namespace OHOS