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
#define LOG_TAG "bt_ipc_opp_proxy"
#endif

#include "bluetooth_opp_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int32_t BluetoothOppProxy::SendFile(std::string &device,
    std::vector<BluetoothIOppTransferFileHolder> fileHolders, bool& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device), BT_ERR_INTERNAL_ERROR, "Write device error");

    if (!WriteParcelableFileHolderVector(fileHolders, data)) {
        HILOGE("[SendFile] fail: write fileHolders failed");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothOppInterfaceCode::COMMAND_SEND_FILE), data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "reply errCode: %{public}d", ret);
    result = ret;
    return BT_NO_ERROR;
}

int32_t BluetoothOppProxy::SetIncomingFileConfirmation(bool accept, int fd)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteBool(accept), BT_ERR_INTERNAL_ERROR,
        "setIncomingFileConfirmation write bool error");
    data.WriteFileDescriptor(fd);
    HILOGE("write fd %{public}d success", fd);
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothOppInterfaceCode::COMMAND_SET_INCOMING_FILE_CONFIRMATION), data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    return reply.ReadInt32();
}

int32_t BluetoothOppProxy::GetCurrentTransferInformation(BluetoothIOppTransferInformation &oppInformation)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int error = Remote()->SendRequest(static_cast<uint32_t>(
        BluetoothOppInterfaceCode::COMMAND_GET_CURRENT_TRANSFER_INFORMATION), data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    std::unique_ptr<BluetoothIOppTransferInformation>
        oppInformation_(reply.ReadParcelable<BluetoothIOppTransferInformation>());
    CHECK_AND_RETURN_LOG_RET((oppInformation_ != nullptr), BT_ERR_DEVICE_DISCONNECTED, "oppInformation is nullptr");
    oppInformation = *oppInformation_;
    return BT_NO_ERROR;
}

int32_t BluetoothOppProxy::CancelTransfer(bool &result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothOppInterfaceCode::COMMAND_CANCEL_TRANSFER), data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);

    result = reply.ReadInt32() == BT_NO_ERROR ? true : false;
    return BT_NO_ERROR;
}

int32_t BluetoothOppProxy::SetLastReceivedFileUri(const std::string &uri)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(uri), BT_ERR_INTERNAL_ERROR, "Write uri error");

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothOppInterfaceCode::COMMAND_SET_LAST_RECEIVED_URI), data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "reply errCode: %{public}d", ret);
    return BT_NO_ERROR;
}

void BluetoothOppProxy::RegisterObserver(const sptr<IBluetoothOppObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "Write object error");

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothOppInterfaceCode::COMMAND_REGISTER_OBSERVER), data, reply, option);
    CHECK_AND_RETURN_LOG((error == BT_NO_ERROR), "error: %{public}d", error);
}

void BluetoothOppProxy::DeregisterObserver(const sptr<IBluetoothOppObserver> &observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()), "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteRemoteObject(observer->AsObject()), "Write object error");

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothOppInterfaceCode::COMMAND_DEREGISTER_OBSERVER), data, reply, option);
    CHECK_AND_RETURN_LOG((error == BT_NO_ERROR), "error: %{public}d", error);
}

int32_t BluetoothOppProxy::GetDeviceState(const BluetoothRawAddress &device, int& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothOppProxy::GetDeviceState write device error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothOppInterfaceCode::COMMAND_GET_DEVICE_STATE), data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);

    int32_t ec = reply.ReadInt32();
    if (FAILED(ec)) {
        return ec;
    }

    result = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothOppProxy::GetDevicesByStates(
    const std::vector<int32_t> &states, std::vector<BluetoothRawAddress>& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothOppProxy::GetDescriptor()),
        BT_ERR_INTERNAL_ERROR, "WriteInterfaceToken error");
    if (!WriteParcelableInt32Vector(states, data)) {
        HILOGE("[GetDevicesByStates] fail: write result failed");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothOppInterfaceCode::COMMAND_GET_DEVICES_BY_STATES), data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);
    int32_t rawAddsSize = reply.ReadInt32();
    const int32_t maxSize = 100;
    if (rawAddsSize > maxSize) {
        return BT_ERR_INVALID_PARAM;
    }
    for (int i = 0; i < rawAddsSize; i++) {
        std::unique_ptr<BluetoothRawAddress> address(reply.ReadParcelable<BluetoothRawAddress>());
        if (!address) {
            return TRANSACTION_ERR;
        }
        result.push_back(*address);
    }
    return BT_NO_ERROR;
}

bool BluetoothOppProxy::WriteParcelableStringVector(const std::vector<std::string> &parcelableVector, Parcel &reply)
{
    if (!reply.WriteInt32(parcelableVector.size())) {
        HILOGE("write ParcelableVector failed");
        return false;
    }

    for (auto parcelable : parcelableVector) {
        if (!reply.WriteString(parcelable)) {
            HILOGE("write ParcelableVector failed");
            return false;
        }
    }
    return true;
}

bool BluetoothOppProxy::WriteParcelableFileHolderVector(const std::vector<BluetoothIOppTransferFileHolder> &fileHolders,
    MessageParcel &reply)
{
    if (!reply.WriteInt32(fileHolders.size())) {
        HILOGE("write ParcelableVector failed");
        return false;
    }

    for (auto fileHolder : fileHolders) {
        if (!reply.WriteString(fileHolder.GetFilePath())) {
            HILOGE("write file path failed");
            return false;
        }
        if (!reply.WriteInt64(fileHolder.GetFileSize())) {
            HILOGE("write file size failed");
            return false;
        }
        if (!reply.WriteFileDescriptor(fileHolder.GetFileFd())) {
            HILOGE("write file fd failed");
            return false;
        }
    }
    return true;
}

bool BluetoothOppProxy::WriteParcelableInt32Vector(const std::vector<int32_t> &parcelableVector, Parcel &reply)
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