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

#include "bluetooth_pbap_pse_proxy.h"
#include "bluetooth_log.h"
#include "i_bluetooth_pbap_pse.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int BluetoothPbapPseProxy::GetDeviceState(const BluetoothRawAddress& device)
{
    HILOGI("Enter!");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("GetDeviceState WriteInterfaceToken error");
        return ERROR;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("GetDeviceState error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothPbapPseInterfaceCode::PBAP_PSE_GET_DEVICE_STATE, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("GetDeviceState done fail, error: %{public}d", error);
        return ERROR;
    }
    return reply.ReadInt32();
}

void BluetoothPbapPseProxy::GetDevicesByStates(
    const std::vector<int32_t> tmpStates, std::vector<BluetoothRawAddress> &rawDevices)
{
    HILOGI("start");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("GetServices WriteInterfaceToken error");
        return;
    }
    if (!data.WriteInt32Vector(tmpStates)) {
        HILOGE("GetServices transport error");
        return;
    }
    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(
        BluetoothPbapPseInterfaceCode::PBAP_PSE_GET_DEVICES_BY_STATES, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("GetServices done fail, error: %{public}d", error);
    }
    int DevNum = reply.ReadInt32();
    for (int i = DevNum; i > 0; i--) {
        std::shared_ptr<BluetoothRawAddress> dev(reply.ReadParcelable<BluetoothRawAddress>());
        if (!dev) {
            return;
        }
        rawDevices.push_back(*dev);
    }
}

int BluetoothPbapPseProxy::Disconnect(const BluetoothRawAddress& device)
{
    HILOGI("Enter!");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("Disconnect WriteInterfaceToken error");
        return ERROR;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("Disconnect error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothPbapPseInterfaceCode::PBAP_PSE_DISCONNECT, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("Disconnect done fail, error: %{public}d", error);
        return ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothPbapPseProxy::SetConnectionStrategy(const BluetoothRawAddress& device, int32_t strategy)
{
    HILOGI("Enter!");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("SetConnectionStrategy WriteInterfaceToken error");
        return ERROR;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("SetConnectionStrategy error");
        return ERROR;
    }

    if (!data.WriteInt32(strategy)) {
        HILOGE("SetConnectionStrategy error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothPbapPseInterfaceCode::PBAP_PSE_SET_CONNECTION_STRATEGY, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("SetConnectionStrategy done fail, error: %{public}d", error);
        return ERROR;
    }
    return reply.ReadInt32();
}

int BluetoothPbapPseProxy::GetConnectionStrategy(const BluetoothRawAddress& device)
{
    HILOGI("Enter!");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("GetConnectionStrategy WriteInterfaceToken error");
        return ERROR;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("GetConnectionStrategy error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(
        BluetoothPbapPseInterfaceCode::PBAP_PSE_GET_CONNECTION_STRATEGY, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("GetConnectionStrategy done fail, error: %{public}d", error);
        return ERROR;
    }
    return reply.ReadInt32();
}

void BluetoothPbapPseProxy::GrantPermission(const BluetoothRawAddress& device, bool allow, bool save)
{
    HILOGI("Enter!");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("GrantPermission WriteInterfaceToken error");
        return;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("GrantPermission error");
        return;
    }

    if (!data.WriteBool(allow)) {
            HILOGE("GrantPermission error");
            return;
        }

    if (!data.WriteBool(save)) {
            HILOGE("GrantPermission error");
            return;
        }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothPbapPseInterfaceCode::PBAP_PSE_GRANT_PERMISSION, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("GrantPermission done fail, error: %{public}d", error);
        return;
    }
}

int BluetoothPbapPseProxy::SetDevicePassword(
    const BluetoothRawAddress &device, const std::string &password, const std::string &userId)
{
    HILOGI("Enter!");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("SetDevicePassword WriteInterfaceToken error");
        return ERROR;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("SetDevicePassword error");
        return ERROR;
    }

    if (!data.WriteString(password)) {
        HILOGE("SetDevicePassword error");
        return ERROR;
    }

    if (!data.WriteString(userId)) {
        HILOGE("SetDevicePassword error");
        return ERROR;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    int error = Remote()->SendRequest(BluetoothPbapPseInterfaceCode::PBAP_PSE_SET_DEVICE_PASSWORD, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("SetDevicePassword done fail, error: %{public}d", error);
        return ERROR;
    }
    return reply.ReadInt32();
}

void BluetoothPbapPseProxy::RegisterObserver(const sptr<IBluetoothPbapPseObserver> &observer)
{
    HILOGI("Enter!");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("RegisterObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("RegisterObserver error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(BluetoothPbapPseInterfaceCode::PBAP_PSE_REGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("RegisterObserver done fail, error: %{public}d", error);
        return;
    }
    return;
}

void BluetoothPbapPseProxy::DeregisterObserver(const sptr<IBluetoothPbapPseObserver> &observer)
{
    HILOGI("Enter!");
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothPbapPseProxy::GetDescriptor())) {
        HILOGE("RegisterObserver WriteInterfaceToken error");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("RegisterObserver error");
        return;
    }

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };
    int error = Remote()->SendRequest(BluetoothPbapPseInterfaceCode::PBAP_PSE_DEREGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOGE("RegisterObserver done fail, error: %{public}d", error);
        return;
    }
    return;
}
}  // namespace Bluetooth
}  // namespace OHOS