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
#define LOG_TAG "bt_ipc_ble_peripheral_obs_proxy"
#endif

#include "bluetooth_ble_peripheral_observer_proxy.h"

#include "bluetooth_raw_address.h"
#include "bluetooth_log.h"
#include "ipc_types.h"

namespace OHOS {
namespace Bluetooth {
BluetoothBlePeripheralObserverProxy::BluetoothBlePeripheralObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IBluetoothBlePeripheralObserver>(impl)
{}

BluetoothBlePeripheralObserverProxy::~BluetoothBlePeripheralObserverProxy()
{}

void BluetoothBlePeripheralObserverProxy::OnReadRemoteRssiEvent(const BluetoothRawAddress &device, int rssi, int status)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBlePeripheralObserverProxy::GetDescriptor())) {
        HILOGE("[OnReadRemoteRssiEvent] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("[OnReadRemoteRssiEvent] fail: write device failed.");
        return;
    }

    if (!data.WriteInt32(rssi)) {
        HILOGE("[OnReadRemoteRssiEvent] fail: write rssi failed.");
        return;
    }

    if (!data.WriteInt32(status)) {
        HILOGE("[OnReadRemoteRssiEvent] fail: write status failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int error =
        InnerTransact(BluetoothBlePeripheralObserverInterfaceCode::BLE_ON_READ_REMOTE_RSSI_EVENT, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothBlePeripheralObserverProxy::OnReadRemoteRssiEvent done fail, error: %{public}d", error);
        return;
    }
}

void BluetoothBlePeripheralObserverProxy::OnPairStatusChanged(
    const int32_t transport, const BluetoothRawAddress &device, int status, int cause, const std::string &causeMessage)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBlePeripheralObserverProxy::GetDescriptor())) {
        HILOGE("[OnPairStatusChanged] fail: write interface token failed.");
        return;
    }

    if (!data.WriteInt32(transport)) {
        HILOGE("[OnPairStatusChanged] fail: write transport failed.");
        return;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("[OnPairStatusChanged] fail: write device failed.");
        return;
    }

    if (!data.WriteInt32(status)) {
        HILOGE("[OnPairStatusChanged] fail: write status failed.");
        return;
    }

    if (!data.WriteInt32(cause)) {
        HILOGE("[OnPairStatusChanged] fail: write cause failed.");
        return;
    }

    if (!data.WriteString(causeMessage)) {
        HILOGE("[OnPairStatusChanged] fail: write causeMessage failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int error = InnerTransact(
        BluetoothBlePeripheralObserverInterfaceCode::BLE_PAIR_STATUS_CHANGED, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothBlePeripheralObserverProxy::OnPairStatusChanged done fail, error: %{public}d", error);
        return;
    }
}

void BluetoothBlePeripheralObserverProxy::OnAclStateChanged(
    const BluetoothRawAddress &device, int state, unsigned int reason)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBlePeripheralObserverProxy::GetDescriptor())) {
        HILOGE("[OnAclStateChanged] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("[OnAclStateChanged] fail: write device failed.");
        return;
    }

    if (!data.WriteInt32(state)) {
        HILOGE("[OnAclStateChanged] fail: write state failed.");
        return;
    }

    if (!data.WriteUint32(reason)) {
        HILOGE("[OnAclStateChanged] fail: write reason failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int error = InnerTransact(BluetoothBlePeripheralObserverInterfaceCode::BLE_ACL_STATE_CHANGED, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothBlePeripheralObserverProxy::OnAclStateChanged done fail, error: %{public}d", error);
        return;
    }
}

ErrCode BluetoothBlePeripheralObserverProxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGW("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return OBJECT_NULL;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return NO_ERROR;
        }
        case DEAD_OBJECT: {
            HILOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return DEAD_OBJECT;
        }
        default: {
            HILOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return TRANSACTION_ERR;
        }
    }
}
}  // namespace Bluetooth
}  // namespace OHOS