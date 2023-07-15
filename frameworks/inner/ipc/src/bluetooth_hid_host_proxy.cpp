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
#include "bluetooth_hid_host_proxy.h"

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "refbase.h"

namespace OHOS {
namespace Bluetooth {
int32_t BluetoothHidHostProxy::Connect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::Connect WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothHidHostProxy::Connect write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_CONNECT), data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::Connect done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return reply.ReadInt32();
}

int32_t BluetoothHidHostProxy::Disconnect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::Disconnect WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothHidHostProxy::Disconnect write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_DISCONNECT), data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::Disconnect done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return reply.ReadInt32();
}

int32_t BluetoothHidHostProxy::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::GetDeviceState WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothHidHostProxy::GetDeviceState write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_GET_DEVICE_STATE), data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::GetDeviceState done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

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

int32_t BluetoothHidHostProxy::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress>& result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::GetDevicesByStates WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!WriteParcelableInt32Vector(states, data)) {
        HILOGE("[GetDevicesByStates] fail: write result failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_GET_DEVICES_BY_STATES), data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::GetDevicesByStates done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    // read error code
    int32_t errCode = reply.ReadInt32();
    if (errCode != BT_NO_ERROR) {
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
    return BT_NO_ERROR;
}

ErrCode BluetoothHidHostProxy::RegisterObserver(
    const sptr<IBluetoothHidHostObserver> observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::RegisterObserver WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHidHostProxy::RegisterObserver error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_REGISTER_OBSERVER), data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::RegisterObserver done fail, error: %{public}d", error);
        return INVALID_DATA;
    }
    return error;
}

ErrCode BluetoothHidHostProxy::DeregisterObserver(
    const sptr<IBluetoothHidHostObserver> observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::DeregisterObserver WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHidHostProxy::DeregisterObserver error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_DEREGISTER_OBSERVER), data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::DeregisterObserver done fail, error: %{public}d", error);
        return INVALID_DATA;
    }
    return error;
}

ErrCode BluetoothHidHostProxy::HidHostVCUnplug(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::HidHostVCUnplug WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }

    if (!data.WriteString(device)) {
        HILOGE("BluetoothHidHostProxy::HidHostVCUnplug error");
        return INVALID_DATA;
    }

    if (!data.WriteUint8(id)) {
        HILOGE("BluetoothHidHostProxy::HidHostVCUnplug error");
        return INVALID_DATA;
    }

    if (!data.WriteUint16(size)) {
        HILOGE("BluetoothHidHostProxy::HidHostVCUnplug error");
        return INVALID_DATA;
    }

    if (!data.WriteUint8(type)) {
        HILOGE("BluetoothHidHostProxy::HidHostVCUnplug error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_VCUN_PLUG), data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::HidHostVCUnplug done fail, error: %{public}d", error);
        return error;
    }
    result = reply.ReadInt32();
    return ERR_OK;
}

ErrCode BluetoothHidHostProxy::HidHostSendData(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::HidHostSendData WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }

    if (!data.WriteString(device)) {
        HILOGE("BluetoothHidHostProxy::HidHostSendData error");
        return INVALID_DATA;
    }

    if (!data.WriteUint8(id)) {
        HILOGE("BluetoothHidHostProxy::HidHostSendData error");
        return INVALID_DATA;
    }

    if (!data.WriteUint16(size)) {
        HILOGE("BluetoothHidHostProxy::HidHostSendData error");
        return INVALID_DATA;
    }

    if (!data.WriteUint8(type)) {
        HILOGE("BluetoothHidHostProxy::HidHostSendData error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_SEND_DATA), data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::HidHostSendData done fail, error: %{public}d", error);
        return error;
    }
    result = reply.ReadInt32();
    return ERR_OK;
}

ErrCode BluetoothHidHostProxy::HidHostSetReport(std::string &device,
    uint8_t &type, uint16_t &size, uint8_t &report, int& result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::HidHostSetReport WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }

    if (!data.WriteString(device)) {
        HILOGE("BluetoothHidHostProxy::HidHostSetReport error");
        return INVALID_DATA;
    }

    if (!data.WriteUint8(type)) {
        HILOGE("BluetoothHidHostProxy::HidHostSetReport error");
        return INVALID_DATA;
    }

    if (!data.WriteUint16(size)) {
        HILOGE("BluetoothHidHostProxy::HidHostSetReport error");
        return INVALID_DATA;
    }

    if (!data.WriteUint8(report)) {
        HILOGE("BluetoothHidHostProxy::HidHostSetReport error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_SET_REPORT), data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::HidHostSetReport done fail, error: %{public}d", error);
        return error;
    }
    result = reply.ReadInt32();
    return ERR_OK;
}

ErrCode BluetoothHidHostProxy::HidHostGetReport(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("BluetoothHidHostProxy::HidHostGetReport WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }

    if (!data.WriteString(device)) {
        HILOGE("BluetoothHidHostProxy::HidHostGetReport error");
        return INVALID_DATA;
    }

    if (!data.WriteUint8(id)) {
        HILOGE("BluetoothHidHostProxy::HidHostGetReport error");
        return INVALID_DATA;
    }

    if (!data.WriteUint16(size)) {
        HILOGE("BluetoothHidHostProxy::HidHostGetReport error");
        return INVALID_DATA;
    }

    if (!data.WriteUint8(type)) {
        HILOGE("BluetoothHidHostProxy::HidHostGetReport error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_GET_REPORT), data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::HidHostGetReport done fail, error: %{public}d", error);
        return error;
    }
    result = reply.ReadInt32();
    return ERR_OK;
}

bool BluetoothHidHostProxy::WriteParcelableInt32Vector(
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

int32_t BluetoothHidHostProxy::SetConnectStrategy(const BluetoothRawAddress &device, int strategy)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteParcelable(&device)) {
        HILOGE("write device error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteInt32(strategy)) {
        HILOGE("write strategy error");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_SET_CONNECT_STRATEGY), data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHfpAgProxy::SetConnectStrategy done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    return reply.ReadInt32();
}

int32_t BluetoothHidHostProxy::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor())) {
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

    int error = Remote()->SendRequest(
        static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_GET_CONNECT_STRATEGY), data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHfpAgProxy::GetConnectStrategy done fail, error: %{public}d", error);
        return BT_ERR_IPC_TRANS_FAILED;
    }

    int32_t res = reply.ReadInt32();
    if (res == NO_ERROR) {
        strategy = reply.ReadInt32();
    }

    return res;
}
} // Bluetooth
} // OHOS
