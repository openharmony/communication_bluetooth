/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "bluetooth_errorcode.h"
#include "bluetooth_pan_proxy.h"
#include "bluetooth_log.h"
#include "refbase.h"

namespace OHOS {
namespace Bluetooth {
int32_t BluetoothPanProxy::Disconnect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor())) {
        HILOGE("BluetoothPanProxy::Disconnect WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothPanProxy::Disconnect write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(COMMAND_DISCONNECT, data, reply, option);
    if (error != BT_SUCCESS) {
        HILOGE("BluetoothPanProxy::Disconnect done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothPanProxy::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteParcelable(&device)) {
        HILOGE("write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(COMMAND_GET_DEVICE_STATE, data, reply, option);
    if (error != BT_SUCCESS) {
        HILOGE("done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    // read error code
    int32_t errCode = reply.ReadInt32();
    if (errCode != BT_SUCCESS) {
        HILOGE("reply errCode: %{public}d", errCode);
        return errCode;
    }
    // read state
    state = reply.ReadInt32();
    return BT_SUCCESS;
}

int32_t BluetoothPanProxy::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress>& result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!WriteParcelableInt32Vector(states, data)) {
        HILOGE("write result failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int error = Remote()->SendRequest(COMMAND_GET_DEVICES_BY_STATES, data, reply, option);
    if (error != BT_SUCCESS) {
        HILOGE("SendRequest failed, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    // read error code
    int32_t errCode = reply.ReadInt32();
    if (errCode != BT_SUCCESS) {
        HILOGE("reply errCode: %{public}d", errCode);
        return errCode;
    }

    // read size
    int32_t rawAddsSize = reply.ReadInt32();

    // read devices
    for (int i = 0; i < rawAddsSize; i++) {
        std::unique_ptr<BluetoothRawAddress> address(reply.ReadParcelable<BluetoothRawAddress>());
        result.push_back(*address);
    }
    return BT_SUCCESS;
}

ErrCode BluetoothPanProxy::RegisterObserver(
    const sptr<IBluetoothPanObserver> observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor())) {
        HILOGE("BluetoothPanProxy::RegisterObserver WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothPanProxy::RegisterObserver error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(COMMAND_REGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothPanProxy::RegisterObserver done fail, error: %{public}d", error);
        return INVALID_DATA;
    }
    return error;
}

ErrCode BluetoothPanProxy::DeregisterObserver(
    const sptr<IBluetoothPanObserver> observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor())) {
        HILOGE("BluetoothPanProxy::DeregisterObserver WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothPanProxy::DeregisterObserver error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(COMMAND_DEREGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothPanProxy::DeregisterObserver done fail, error: %{public}d", error);
        return INVALID_DATA;
    }
    return error;
}

int32_t BluetoothPanProxy::SetTethering(const bool value)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor())) {
        HILOGE("BluetoothPanProxy::SetTethering WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteBool(value)) {
        HILOGE("BluetoothPanProxy::SetTethering error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(COMMAND_SET_TETHERING, data, reply, option);
    if (error != BT_SUCCESS) {
        HILOGE("BluetoothPanProxy::SetTethering done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int32_t BluetoothPanProxy::IsTetheringOn(bool &result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor())) {
        HILOGE("BluetoothPanProxy::IsTetheringOn WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(COMMAND_IS_TETHERING_ON, data, reply, option);
    if (error != BT_SUCCESS) {
        HILOGE("BluetoothPanProxy::IsTetheringOn done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    if (ret != BT_SUCCESS) {
        HILOGE("internal error. ret:%{public}d", ret);
        return ret;
    }

    result = reply.ReadInt32();
    return BT_SUCCESS;
}

bool BluetoothPanProxy::WriteParcelableInt32Vector(
    const std::vector<int32_t> &parcelableVector, Parcel &reply)
{
    if (!reply.WriteInt32(parcelableVector.size())) {
        HILOGE("write ParcelableVector failed");
        return false;
    }

    for (auto parcelable : parcelableVector) {
        if (!reply.WriteInt32(parcelable)) {
            HILOGE("write ParcelableVector failed");
            return false;
        }
    }
    return true;
}
}  // namespace Bluetooth
}  // namespace OHOS