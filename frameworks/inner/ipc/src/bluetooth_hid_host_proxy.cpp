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
#define LOG_TAG "bt_ipc_hid_host_proxy"
#endif

#include "bluetooth_hid_host_proxy.h"

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "refbase.h"

namespace OHOS {
namespace Bluetooth {
int32_t BluetoothHidHostProxy::Connect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_CONNECT),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothHidHostProxy::Disconnect(const BluetoothRawAddress &device)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_DISCONNECT),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothHidHostProxy::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_GET_DEVICE_STATE),
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

int32_t BluetoothHidHostProxy::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress>& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(WriteParcelableInt32Vector(states, data), BT_ERR_IPC_TRANS_FAILED, "write states error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_GET_DEVICES_BY_STATES),
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

int32_t BluetoothHidHostProxy::RegisterObserver(
    const sptr<IBluetoothHidHostObserver> observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(observer->AsObject()), BT_ERR_IPC_TRANS_FAILED, "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_REGISTER_OBSERVER),
        data, reply, option, INVALID_DATA);

    return BT_NO_ERROR;
}

int32_t BluetoothHidHostProxy::DeregisterObserver(
    const sptr<IBluetoothHidHostObserver> observer)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(
        data.WriteRemoteObject(observer->AsObject()), BT_ERR_IPC_TRANS_FAILED, "write object error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_DEREGISTER_OBSERVER),
        data, reply, option, INVALID_DATA);

    return BT_NO_ERROR;
}

int32_t BluetoothHidHostProxy::HidHostVCUnplug(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device), BT_ERR_IPC_TRANS_FAILED, "Write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(id), BT_ERR_IPC_TRANS_FAILED, "Write id error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint16(size), BT_ERR_IPC_TRANS_FAILED, "Write size error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(type), BT_ERR_IPC_TRANS_FAILED, "Write type error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_VCUN_PLUG),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    result = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothHidHostProxy::HidHostSendData(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device), BT_ERR_IPC_TRANS_FAILED, "write device errorr");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(id), BT_ERR_IPC_TRANS_FAILED, "write id error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint16(size), BT_ERR_IPC_TRANS_FAILED, "write size error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(type), BT_ERR_IPC_TRANS_FAILED, "write type error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_SEND_DATA),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return BT_NO_ERROR;
}

int32_t BluetoothHidHostProxy::HidHostSetReport(std::string &device,
    uint8_t &type, std::string &report, int& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(type), BT_ERR_IPC_TRANS_FAILED, "write type error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(report), BT_ERR_IPC_TRANS_FAILED, "write report error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_SET_REPORT),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    result = reply.ReadInt32();
    return BT_NO_ERROR;
}

int32_t BluetoothHidHostProxy::HidHostGetReport(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteString(device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(id), BT_ERR_IPC_TRANS_FAILED, "write id error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint16(size), BT_ERR_IPC_TRANS_FAILED, "write size error");
    CHECK_AND_RETURN_LOG_RET(data.WriteUint8(type), BT_ERR_IPC_TRANS_FAILED, "write type error");
    
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_GET_REPORT),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    result = reply.ReadInt32();
    return BT_NO_ERROR;
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
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");
    CHECK_AND_RETURN_LOG_RET(data.WriteInt32(strategy), BT_ERR_IPC_TRANS_FAILED, "write strategy error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_SET_CONNECT_STRATEGY),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    return reply.ReadInt32();
}

int32_t BluetoothHidHostProxy::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG_RET(data.WriteInterfaceToken(BluetoothHidHostProxy::GetDescriptor()),
        BT_ERR_IPC_TRANS_FAILED, "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG_RET(data.WriteParcelable(&device), BT_ERR_IPC_TRANS_FAILED, "write device error");

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    SEND_IPC_REQUEST_RETURN_RESULT(static_cast<uint32_t>(BluetoothHidHostInterfaceCode::COMMAND_GET_CONNECT_STRATEGY),
        data, reply, option, BT_ERR_IPC_TRANS_FAILED);

    int32_t res = reply.ReadInt32();
    if (res == NO_ERROR) {
        strategy = reply.ReadInt32();
    }
    return res;
}
} // Bluetooth
} // OHOS
