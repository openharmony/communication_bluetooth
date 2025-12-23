/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_hid_device_proxy"
#endif

#include "bluetooth_hid_device_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "refbase.h"

namespace OHOS {
namespace Bluetooth {
int32_t BluetoothHidDeviceProxy::Connect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_INTERNAL_ERROR, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_CONNECT),
        data, reply, option, BT_ERR_INTERNAL_ERROR);

    return reply.ReadInt32();
}

int32_t BluetoothHidDeviceProxy::Disconnect()
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_DISCONNECT),
        data, reply, option, BT_ERR_INTERNAL_ERROR);

    return reply.ReadInt32();
}

int32_t BluetoothHidDeviceProxy::GetConnectionState(const BluetoothRawAddress &device, int32_t &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_INTERNAL_ERROR, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_GET_CONNECTION_STATE),
        data, reply, option, BT_ERR_INTERNAL_ERROR);

    int32_t errCode = reply.ReadInt32();
    if (errCode != BT_NO_ERROR) {
        HILOGE("reply errCode: %{public}d", errCode);
        return errCode;
    }

    state = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceProxy::GetConnectedDevices(std::vector<BluetoothRawAddress> &result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(
        BluetoothHidDeviceInterfaceCode::COMMAND_GET_CONNECTED_DEVICES),
        data, reply, option, BT_ERR_INTERNAL_ERROR);

    int32_t errCode = reply.ReadInt32();
    if (errCode != BT_NO_ERROR) {
        HILOGE("reply errCode: %{public}d", errCode);
        return errCode;
    }

    int32_t rawAddrSize = reply.ReadInt32();
    const int32_t maxSize = 100;
    if (rawAddrSize > maxSize) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < rawAddrSize; i++) {
        std::unique_ptr<BluetoothRawAddress> address(reply.ReadParcelable<BluetoothRawAddress>());
        if (!address) {
            return BT_ERR_INTERNAL_ERROR;
        }
        result.push_back(*address);
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceProxy::RegisterHidDevice(const BluetoothHidDeviceSdp &sdpSetting,
    const BluetoothHidDeviceQos &inQos, const BluetoothHidDeviceQos &outQos)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&sdpSetting), BT_ERR_INTERNAL_ERROR, "write sdpSetting error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&inQos), BT_ERR_INTERNAL_ERROR, "write inQos error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&outQos), BT_ERR_INTERNAL_ERROR, "write outQos error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_REGISTER_APP),
        data, reply, option, BT_ERR_INTERNAL_ERROR);

    return reply.ReadInt32();
}

int32_t BluetoothHidDeviceProxy::UnregisterHidDevice()
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_UNREGISTER_APP),
        data, reply, option, BT_ERR_INTERNAL_ERROR);

    return reply.ReadInt32();
}

int32_t BluetoothHidDeviceProxy::RegisterObserver(
    const sptr<IBluetoothHidDeviceObserver> observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(observer->AsObject()), BT_ERR_INTERNAL_ERROR, "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_REGISTER_OBSERVER),
        data, reply, option, INVALID_DATA);

    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceProxy::DeregisterObserver(
    const sptr<IBluetoothHidDeviceObserver> observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(observer->AsObject()), BT_ERR_INTERNAL_ERROR, "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_DEREGISTER_OBSERVER),
        data, reply, option, INVALID_DATA);

    return BT_NO_ERROR;
}

int32_t BluetoothHidDeviceProxy::SendReport(int id, const std::vector<uint8_t> &data)
{
    MessageParcel messageData;
    CHECK_AND_RETURN_LOG_RET(messageData.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(messageData.WriteInt32(id), BT_ERR_INTERNAL_ERROR, "write id error");
    CHECK_AND_RETURN_LOG_RET(messageData.WriteUInt8Vector(data), BT_ERR_INTERNAL_ERROR, "write data error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_SEND_REPORT),
        messageData, reply, option, BT_ERR_INTERNAL_ERROR);

    return reply.ReadInt32();
}

int32_t BluetoothHidDeviceProxy::ReplyReport(ReportType type, int id, const std::vector<uint8_t> &data)
{
    MessageParcel messageData;
    CHECK_AND_RETURN_LOG_RET(messageData.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(messageData.WriteInt32(static_cast<int32_t>(type)),
        BT_ERR_INTERNAL_ERROR, "write type error");
    CHECK_AND_RETURN_LOG_RET(messageData.WriteInt32(id), BT_ERR_INTERNAL_ERROR, "write id error");
    CHECK_AND_RETURN_LOG_RET(messageData.WriteUInt8Vector(data), BT_ERR_INTERNAL_ERROR, "write data error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_REPLY_REPORT),
        messageData, reply, option, BT_ERR_INTERNAL_ERROR);

    return reply.ReadInt32();
}

int32_t BluetoothHidDeviceProxy::ReportError(ErrorReason type)
{
    MessageParcel messageData;
    CHECK_AND_RETURN_LOG_RET(messageData.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(messageData.WriteInt32(static_cast<int32_t>(type)),
        BT_ERR_INTERNAL_ERROR, "write type error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_REPORT_ERROR),
        messageData, reply, option, BT_ERR_INTERNAL_ERROR);

    return reply.ReadInt32();
}

int32_t BluetoothHidDeviceProxy::SetConnectStrategy(const BluetoothRawAddress &device, int strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_INTERNAL_ERROR, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(strategy), BT_ERR_INTERNAL_ERROR, "write strategy error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_SET_CONNECT_STRATEGY),
        data, reply, option, BT_ERR_INTERNAL_ERROR);

    return reply.ReadInt32();
}

int32_t BluetoothHidDeviceProxy::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidDeviceProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_INTERNAL_ERROR, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidDeviceInterfaceCode::COMMAND_GET_CONNECT_STRATEGY),
        data, reply, option, BT_ERR_INTERNAL_ERROR);

    int32_t res = reply.ReadInt32();
    if (res == NO_ERROR) {
        strategy = reply.ReadInt32();
    }
    return res;
}
}  // namespace Bluetooth
}  // namespace OHOS
