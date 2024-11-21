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
#define LOG_TAG "bt_ipc_gatt_client_proxy"
#endif

#include "bluetooth_gatt_client_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"

namespace OHOS {
namespace Bluetooth {
const int GATT_CLIENT_READ_DATA_SIZE_MAX_LEN = 0xFF;
int BluetoothGattClientProxy::RegisterApplication(
    const sptr<IBluetoothGattClientCallback> &callback, const BluetoothRawAddress &addr, int32_t transport, int &appId)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(callback->AsObject()), BT_ERR_IPC_TRANS_FAILED, "write object error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&addr), BT_ERR_IPC_TRANS_FAILED, "write addr error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(transport), BT_ERR_IPC_TRANS_FAILED, "write transport error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_REGISTER_APP,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t result = reply.ReadInt32();
    appId = reply.ReadInt32();
    return result;
}

int BluetoothGattClientProxy::RegisterApplication(
    const sptr<IBluetoothGattClientCallback> &callback, const BluetoothRawAddress &addr, int32_t transport)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(callback->AsObject()), BT_ERR_IPC_TRANS_FAILED, "write object error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&addr), BT_ERR_IPC_TRANS_FAILED, "write addr error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(transport), BT_ERR_IPC_TRANS_FAILED, "write transport error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_REGISTER_APP,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::DeregisterApplication(int32_t appId)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_DEREGISTER_APP,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::Connect(int32_t appId, bool autoConnect)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteBool(autoConnect), BT_ERR_IPC_TRANS_FAILED, "write autoConnect error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::Disconnect(int32_t appId)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_DIS_CONNECT,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::DiscoveryServices(int32_t appId)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_DISCOVERY_SERVICES,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::ReadCharacteristic(int32_t appId, const BluetoothGattCharacteristic &characteristic)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&characteristic),
        BT_ERR_IPC_TRANS_FAILED, "write characteristic error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_READ_CHARACTERISTIC,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::WriteCharacteristic(
    int32_t appId, BluetoothGattCharacteristic *characteristic, bool withoutRespond)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(characteristic),
        BT_ERR_IPC_TRANS_FAILED, "write characteristic error");
    CHECK_AND_RETURN_LOG_RET(data.WriteBool(withoutRespond),
        BT_ERR_IPC_TRANS_FAILED, "write withoutRespond error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_WRITE_CHARACTERISTIC,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::SignedWriteCharacteristic(int32_t appId, BluetoothGattCharacteristic *characteristic)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId),
        BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(characteristic),
        BT_ERR_IPC_TRANS_FAILED, "write characteristic error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_SIGNED_WRITE_CHARACTERISTIC,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::ReadDescriptor(int32_t appId, const BluetoothGattDescriptor &descriptor)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&descriptor), BT_ERR_IPC_TRANS_FAILED, "write descriptor error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_READ_DESCRIPTOR,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::WriteDescriptor(int32_t appId, BluetoothGattDescriptor *descriptor)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(descriptor), BT_ERR_IPC_TRANS_FAILED, "write descriptor error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_WRITE_DESCRIPTOR,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::RequestExchangeMtu(int32_t appId, int32_t mtu)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(mtu), BT_ERR_IPC_TRANS_FAILED, "write mtu error");
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_REQUEST_EXCHANGE_MTU,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

void BluetoothGattClientProxy::GetAllDevice(std::vector<BluetoothGattDevice> &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(
        data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()), "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_GET_ALL_DEVICE, data, reply, option);
    
    int devNum = 0;
    if (!reply.ReadInt32(devNum) || devNum > GATT_CLIENT_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return;
    }
    for (int i = devNum; i > 0; i--) {
        std::shared_ptr<BluetoothGattDevice> dev(reply.ReadParcelable<BluetoothGattDevice>());
        if (!dev) {
            return;
        }
        device.push_back(*dev);
    }
}

int BluetoothGattClientProxy::RequestConnectionPriority(int32_t appId, int32_t connPriority)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(connPriority), BT_ERR_IPC_TRANS_FAILED, "write connPriority error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_REQUEST_CONNECTION_PRIORITY,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::GetServices(int32_t appId, std::vector<BluetoothGattService> &service)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_GET_SERVICES,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);
    
    int ret = reply.ReadInt32();
    int devNum = 0;
    if (!reply.ReadInt32(devNum) || devNum > GATT_CLIENT_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    for (int i = devNum; i > 0; i--) {
        std::shared_ptr<BluetoothGattService> dev(reply.ReadParcelable<BluetoothGattService>());
        if (!dev) {
            return BT_ERR_IPC_TRANS_FAILED;
        }
        service.push_back(*dev);
    }
    return ret;
}

int BluetoothGattClientProxy::RequestFastestConn(const BluetoothRawAddress &addr)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&addr), BT_ERR_IPC_TRANS_FAILED, "write addr error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_REQUEST_FASTEST_CONNECTION,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::ReadRemoteRssiValue(int32_t appId)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_READ_REMOTE_RSSI_VALUE,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int BluetoothGattClientProxy::RequestNotification(int32_t appId, uint16_t characterHandle, bool enable)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothGattClientProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(appId), BT_ERR_IPC_TRANS_FAILED, "write appId error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint16(characterHandle), BT_ERR_IPC_TRANS_FAILED, "write characterHandle error");
    CHECK_AND_RETURN_LOG_RET(data.WriteBool(enable), BT_ERR_IPC_TRANS_FAILED, "write enable error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(BluetoothGattClientInterfaceCode::BT_GATT_CLIENT_REQUEST_NOTIFICATION,
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

}  // namespace Bluetooth
}  // namespace OHOS
