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
#include "bluetooth_log.h"
#include "refbase.h"

namespace OHOS {
namespace Bluetooth {
ErrCode BluetoothHidHostProxy::Connect(const BluetoothRawAddress &device, bool& result)
{
    MessageParcel data;
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothHidHostProxy::Connect write device error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(COMMAND_CONNECT, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::Connect done fail, error: %{public}d", error);
        return error;
    }

    result = reply.ReadInt32() == 1 ? true : false;
    return ERR_OK;
}

ErrCode BluetoothHidHostProxy::Disconnect(const BluetoothRawAddress &device, bool& result)
{
    MessageParcel data;
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothHidHostProxy::Disconnect write device error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(COMMAND_DISCONNECT, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::Disconnect done fail, error: %{public}d", error);
        return error;
    }

    result = reply.ReadInt32() == 1 ? true : false;
    return ERR_OK;
}

ErrCode BluetoothHidHostProxy::GetDeviceState(const BluetoothRawAddress &device, int& result)
{
    MessageParcel data;
    if (!data.WriteParcelable(&device)) {
        HILOGE("BluetoothHidHostProxy::GetDeviceState write device error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(COMMAND_GET_DEVICE_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::GetDeviceState done fail, error: %{public}d", error);
        return error;
    }

    ErrCode ec = reply.ReadInt32();
    if (FAILED(ec)) {
        return ec;
    }

    result = reply.ReadInt32();
    return ERR_OK;
}

ErrCode BluetoothHidHostProxy::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress>& result)
{
    MessageParcel data;
    if (!WriteParcelableInt32Vector(states, data)) {
        HILOGE("[GetDevicesByStates] fail: write result failed");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int error = Remote()->SendRequest(COMMAND_GET_DEVICES_BY_STATES, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::GetDevicesByStates done fail, error: %{public}d", error);
        return INVALID_DATA;
    }
    int32_t rawAddsSize = reply.ReadInt32();
    for (int i = 0; i < rawAddsSize; i++) {
        std::unique_ptr<BluetoothRawAddress> address(reply.ReadParcelable<BluetoothRawAddress>());
        result.push_back(*address);
    }
    return ERR_OK;
}

ErrCode BluetoothHidHostProxy::RegisterObserver(
    const sptr<IBluetoothHidHostObserver> observer)
{
    MessageParcel data;
if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHidHostProxy::RegisterObserver error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(COMMAND_REGISTER_OBSERVER, data, reply, option);
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
if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("BluetoothHidHostProxy::DeregisterObserver error");
        return INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_ASYNC
    };

    int error = Remote()->SendRequest(COMMAND_DEREGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("BluetoothHidHostProxy::DeregisterObserver done fail, error: %{public}d", error);
        return INVALID_DATA;
    }
    return error;
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
} // Bluetooth
} // OHOS
