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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_pan_proxy"
#endif

#include "bluetooth_errorcode.h"
#include "bluetooth_pan_proxy.h"
#include "bluetooth_log.h"
#include "refbase.h"

namespace OHOS {
namespace Bluetooth {
int32_t BluetoothPanProxy::Disconnect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothPanInterfaceCode::COMMAND_DISCONNECT),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothPanProxy::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothPanInterfaceCode::COMMAND_GET_DEVICE_STATE),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    // read error code
    int32_t errCode = reply.ReadInt32();
    if (errCode != BT_NO_ERROR) {
        HILOGE("reply errCode: %{public}d", errCode);
        return errCode;
    }
    // read state
    state = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothPanProxy::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress>& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(WriteParcelableInt32Vector(states, data), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothPanInterfaceCode::COMMAND_GET_DEVICES_BY_STATES),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    // read error code
    int32_t errCode = reply.ReadInt32();
    if (errCode != BT_NO_ERROR) {
        HILOGE("reply errCode: %{public}d", errCode);
        return errCode;
    }

    // read size
    int32_t rawAddsSize = reply.ReadInt32();
    const int32_t maxSize = 100;
    if (rawAddsSize > maxSize) {
        return BT_ERR_INVALID_PARAM;
    }

    // read devices
    for (int i = 0; i < rawAddsSize; i++) {
        std::unique_ptr<BluetoothRawAddress> address(reply.ReadParcelable<BluetoothRawAddress>());
        if (!address) {
            return TRANSACTION_ERR;
        }
        result.push_back(*address);
    }
    return BT_NO_ERROR;
}

int32_t BluetoothPanProxy::RegisterObserver(const sptr<IBluetoothPanObserver> observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteRemoteObject(observer->AsObject()),
        BT_ERR_IPC_TRANS_FAILED, "Write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothPanInterfaceCode::COMMAND_REGISTER_OBSERVER),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return BT_NO_ERROR;
}

int32_t BluetoothPanProxy::DeregisterObserver(const sptr<IBluetoothPanObserver> observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteRemoteObject(observer->AsObject()),
        BT_ERR_IPC_TRANS_FAILED, "Write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothPanInterfaceCode::COMMAND_DEREGISTER_OBSERVER),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return BT_NO_ERROR;
}

int32_t BluetoothPanProxy::SetTethering(const bool value)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteBool(value), BT_ERR_IPC_TRANS_FAILED, "write value error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothPanInterfaceCode::COMMAND_SET_TETHERING),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothPanProxy::IsTetheringOn(bool &result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothPanProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothPanInterfaceCode::COMMAND_IS_TETHERING_ON),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t ret = reply.ReadInt32();
    if (ret != BT_NO_ERROR) {
        HILOGE("internal error. ret:%{public}d", ret);
        return ret;
    }

    result = reply.ReadInt32();
    return BT_NO_ERROR;
}

bool BluetoothPanProxy::WriteParcelableInt32Vector(const std::vector<int32_t> &parcelableVector, Parcel &reply)
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