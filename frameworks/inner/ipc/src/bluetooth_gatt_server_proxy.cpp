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
#define LOG_TAG "bt_ipc_gatt_server_proxy"
#endif

#include "bluetooth_gatt_server_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int BluetoothGattServerProxy::AddService(int32_t appId, BluetoothGattService *services)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(services), BT_ERR_IPC_TRANS_FAILED, "write services error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_ADD_SERVICE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
void BluetoothGattServerProxy::ClearServices(int appId)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(
        data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(appId), "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothGattServerInterfaceCode::GATT_SERVER_CLEAR_SERVICES, data, reply, option);

    return;
}

int BluetoothGattServerProxy::Connect(int appId, const BluetoothGattDevice &device, bool isDirect)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteBool(isDirect), BT_ERR_IPC_TRANS_FAILED, "write isDirect error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattServerProxy::CancelConnection(int appId, const BluetoothGattDevice &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_CANCEL_CONNECTION,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
int BluetoothGattServerProxy::RegisterApplication(const sptr<IBluetoothGattServerCallback> &callback)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(callback->AsObject()), BT_ERR_IPC_TRANS_FAILED, "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_REGISTER,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
int BluetoothGattServerProxy::DeregisterApplication(int appId)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_DEREGISTER,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
int BluetoothGattServerProxy::NotifyClient(
    const BluetoothGattDevice &device, BluetoothGattCharacteristic *characteristic, bool needConfirm)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(characteristic),
        BT_ERR_IPC_TRANS_FAILED, "write characteristic error");
    CHECK_AND_RETURN_LOG_RET(data.WriteBool(needConfirm), BT_ERR_IPC_TRANS_FAILED, "write needConfirm error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_NOTIFY_CLIENT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
int BluetoothGattServerProxy::RemoveService(int32_t appId, const BluetoothGattService &services)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&services), BT_ERR_IPC_TRANS_FAILED, "write services error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_REMOVE_SERVICE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
int BluetoothGattServerProxy::RespondCharacteristicRead(
    const BluetoothGattDevice &device, BluetoothGattCharacteristic *characteristic, int32_t ret)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(characteristic),
        BT_ERR_IPC_TRANS_FAILED, "write characteristic error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(ret), BT_ERR_IPC_TRANS_FAILED, "write ret error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_RESPOND_CHARACTERISTIC_READ,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
int BluetoothGattServerProxy::RespondCharacteristicWrite(
    const BluetoothGattDevice &device, const BluetoothGattCharacteristic &characteristic, int32_t ret)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&characteristic),
        BT_ERR_IPC_TRANS_FAILED, "write characteristic error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(ret), BT_ERR_IPC_TRANS_FAILED, "write ret error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
int BluetoothGattServerProxy::RespondDescriptorRead(
    const BluetoothGattDevice &device, BluetoothGattDescriptor *descriptor, int32_t ret)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(descriptor), BT_ERR_IPC_TRANS_FAILED, "write descriptor error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(ret), BT_ERR_IPC_TRANS_FAILED, "write ret error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_RESPOND_DESCRIPTOR_READ,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
int BluetoothGattServerProxy::RespondDescriptorWrite(
    const BluetoothGattDevice &device, const BluetoothGattDescriptor &descriptor, int32_t ret)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattServerProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&descriptor), BT_ERR_IPC_TRANS_FAILED, "write descriptor error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(ret), BT_ERR_IPC_TRANS_FAILED, "write ret error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattServerInterfaceCode::GATT_SERVER_RESPOND_DESCRIPTOR_WRITE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}
}  // namespace Bluetooth
}  // namespace OHOS