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

#include "bluetooth_socket_proxy.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int BluetoothSocketProxy::Connect(ConnectSocketParam &param, int &fd)
{
    HILOGI("Connect starts");
    MessageParcel data;

    if (!data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor())) {
        HILOGE("Connect WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteString(param.addr)) {
        HILOGE("Connect write addr error");
        return BT_ERR_INTERNAL_ERROR;
    }

    BluetoothUuid btUuid(param.uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("Connect write uuid error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(param.securityFlag)) {
        HILOGE("Connect write securityFlag error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(param.type)) {
        HILOGE("Connect write type error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(param.psm)) {
        HILOGE("Connect write psm error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothSocketInterfaceCode::SOCKET_CONNECT, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("Connect done fail, error: %{public}d", error);
        return BT_ERR_INTERNAL_ERROR;
    }

    int errorCode = reply.ReadInt32();
    if (errorCode == NO_ERROR) {
        fd = reply.ReadFileDescriptor();
    }
    return errorCode;
}

int BluetoothSocketProxy::Listen(ListenSocketParam &param, int &fd)
{
    HILOGI("starts");
    fd = BT_INVALID_SOCKET_FD;
    MessageParcel data;

    if (!data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteString(param.name)) {
        HILOGE("write name error");
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothUuid btUuid(param.uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("write uuid error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(param.securityFlag)) {
        HILOGE("write securityFlag error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(param.type)) {
        HILOGE("write type error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteRemoteObject(param.observer->AsObject())) {
        HILOGE("write remote object error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothSocketInterfaceCode::SOCKET_LISTEN, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("Listen done fail, error: %{public}d", error);
        return BT_ERR_INTERNAL_ERROR;
    }

    int errorCode = reply.ReadInt32();
    if (errorCode == NO_ERROR) {
        fd = reply.ReadFileDescriptor();
    }

    return errorCode;
}

int BluetoothSocketProxy::DeregisterServerObserver(const sptr<IBluetoothServerSocketObserver> &observer)
{
    HILOGD("BluetoothSocketProxy::DeregisterServerObserver start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor())) {
        HILOGE("BluetoothSocketProxy::DeregisterServerObserver WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothSocketProxy::DeregisterServerObserver error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(BluetoothSocketInterfaceCode::DEREGISTER_SERVER_OBSERVER, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothSocketProxy::DeregisterServerObserver done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    HILOGD("BluetoothSocketProxy::DeregisterServerObserver success");
    return reply.ReadInt32();
}

int BluetoothSocketProxy::RegisterClientObserver(const BluetoothRawAddress &dev, const bluetooth::Uuid uuid,
    const sptr<IBluetoothClientSocketObserver> &observer)
{
    HILOGD("BluetoothSocketProxy::RegisterClientObserver start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor())) {
        HILOGE("BluetoothSocketProxy::RegisterClientObserver WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteParcelable(&dev)) {
        HILOGE("write dev error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    BluetoothUuid btUuid(uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("write uuid error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("write observer error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(BluetoothSocketInterfaceCode::REGISTER_CLIENT_OBSERVER, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothSocketProxy::RegisterClientObserver done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    HILOGD("BluetoothSocketProxy::RegisterClientObserver success");
    return reply.ReadInt32();
}

int BluetoothSocketProxy::DeregisterClientObserver(const BluetoothRawAddress &dev, const bluetooth::Uuid uuid,
        const sptr<IBluetoothClientSocketObserver> &observer)
{
    HILOGD("BluetoothSocketProxy::DeregisterClientObserver start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor())) {
        HILOGE("BluetoothSocketProxy::DeregisterClientObserver WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteParcelable(&dev)) {
        HILOGE("write dev error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    BluetoothUuid btUuid(uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE("write uuid error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("write observer error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(BluetoothSocketInterfaceCode::DEREGISTER_CLIENT_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothSocketProxy::DeregisterClientObserver done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    HILOGD("BluetoothSocketProxy::DeregisterClientObserver success");
    return reply.ReadInt32();
}

int BluetoothSocketProxy::UpdateCocConnectionParams(const BluetoothSocketCocInfo &info)
{
    HILOGI("UpdateCocConnectionParams starts");
    MessageParcel data;

    if (!data.WriteInterfaceToken(BluetoothSocketProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteParcelable(&info)) {
        HILOGE("WriteParcelable error");
        return BT_ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int error = Remote()->SendRequest(BluetoothSocketInterfaceCode::SOCKET_UPDATE_COC_PARAMS, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("UpdateCocConnectionParams done fail, error: %{public}d", error);
        return BT_ERR_INTERNAL_ERROR;
    }

    return reply.ReadInt32();
}
}  // namespace Bluetooth
}  // namespace OHOS