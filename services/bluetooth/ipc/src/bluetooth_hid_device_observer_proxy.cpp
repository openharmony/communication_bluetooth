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
#define LOG_TAG "bt_ipc_hid_device_observer_proxy"
#endif

#include "bluetooth_errorcode.h"
#include "bluetooth_hid_device_observer_proxy.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {

ErrCode BluetoothHidDeviceObserverProxy::OnAppStatusChanged(int state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidDeviceObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothHidDeviceObserverProxy::onAppStatusChanged WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(state)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onAppStatusChanged WriteInt32 error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {MessageOption::TF_ASYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, ERR_DEAD_OBJECT, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_APP_STATUS_CHANGED, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidDeviceObserverProxy::onAppStatusChanged done fail, error: %{public}d", error);
        return error;
    }
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverProxy::OnConnectionStateChanged(const BluetoothRawAddress &device, int state)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidDeviceObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothHidDeviceObserverProxy::OnConnectionStateChanged WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothHidDeviceObserverProxy::OnConnectionStateChanged WriteParcelable error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(state)) {
        HILOGE("BluetoothHidDeviceObserverProxy::OnConnectionStateChanged WriteInt32 error");
        return BT_ERR_INTERNAL_ERROR;
    }
    MessageParcel reply;
    MessageOption option {MessageOption::TF_ASYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, ERR_DEAD_OBJECT, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_CONNECTION_STATE_CHANGED, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidDeviceObserverProxy::OnConnectionStateChanged done fail, error: %{public}d", error);
        return error;
    }
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverProxy::OnGetReport(int type, int id, uint16_t bufferSize)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidDeviceObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothHidDeviceObserverProxy::onGetReport WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(type)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onGetReport WriteInt32 error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteInt32(id)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onGetReport WriteInt32 error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!data.WriteUint32(bufferSize)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onGetReport WriteUint32 error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {MessageOption::TF_ASYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, ERR_DEAD_OBJECT, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_GET_REPORT, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidDeviceObserverProxy::onGetReport done fail, error: %{public}d", error);
        return error;
    }
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverProxy::OnInterruptDataReceived(int reportId, std::vector<uint8_t> data)
{
    MessageParcel messageData;
    if (!messageData.WriteInterfaceToken(BluetoothHidDeviceObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothHidDeviceObserverProxy::onInterruptData WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!messageData.WriteInt32(reportId)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onInterruptData WriteInt32 error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!messageData.WriteUInt8Vector(data)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onInterruptData WriteUInt8Vector error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {MessageOption::TF_ASYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, ERR_DEAD_OBJECT, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_INTERRUPT_DATA, messageData, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidDeviceObserverProxy::onInterruptData done fail, error: %{public}d", error);
        return error;
    }
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverProxy::OnSetProtocol(int protocol)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidDeviceObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothHidDeviceObserverProxy::onSetProtocol WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!data.WriteInt32(protocol)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onSetProtocol WriteInt32 error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {MessageOption::TF_ASYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, ERR_DEAD_OBJECT, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_SET_PROTOCOL, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidDeviceObserverProxy::onSetProtocol done fail, error: %{public}d", error);
        return error;
    }
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverProxy::OnSetReport(int type, int id, std::vector<uint8_t> data)
{
    MessageParcel messageData;
    if (!messageData.WriteInterfaceToken(BluetoothHidDeviceObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothHidDeviceObserverProxy::onSetReport WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!messageData.WriteInt32(type)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onSetReport WriteInt32 error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!messageData.WriteInt32(id)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onSetReport WriteInt32 error");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (!messageData.WriteUInt8Vector(data)) {
        HILOGE("BluetoothHidDeviceObserverProxy::onSetReport WriteUInt8Vector error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {MessageOption::TF_ASYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, ERR_DEAD_OBJECT, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_SET_REPORT, messageData, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidDeviceObserverProxy::onSetReport done fail, error: %{public}d", error);
        return error;
    }
    return BT_NO_ERROR;
}

ErrCode BluetoothHidDeviceObserverProxy::OnVirtualCableUnplug() 
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothHidDeviceObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothHidDeviceObserverProxy::onVirtualCableUnplug WriteInterfaceToken error");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option {MessageOption::TF_ASYNC};
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, ERR_DEAD_OBJECT, "remote nullptr");
    int error = remote->SendRequest(
        BluetoothHidDeviceObserverInterfaceCode::COMMAND_ON_VIRTUAL_CABLE_UNPLUG, data, reply, option);
    if (error != BT_NO_ERROR) {
        HILOGE("BluetoothHidDeviceObserverProxy::onVirtualCableUnplug done fail, error: %{public}d", error);
        return error;
    }
    return BT_NO_ERROR;
}
} // Bluetooth
} // OHOS
