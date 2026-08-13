/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_device_battery_obs_proxy"
#endif

#include "bluetooth_device_battery_observer_proxy.h"

#include "bluetooth_raw_address.h"
#include "bluetooth_log.h"
#include "ipc_types.h"

namespace OHOS {
namespace Bluetooth {
BluetoothDeviceBatteryObserverProxy::BluetoothDeviceBatteryObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IBluetoothDeviceBatteryObserver>(impl)
{}

BluetoothDeviceBatteryObserverProxy::~BluetoothDeviceBatteryObserverProxy()
{}

void BluetoothDeviceBatteryObserverProxy::OnGetBatteryLevelEvent(
    const BluetoothRawAddress &device, int32_t batteryLevel)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothDeviceBatteryObserverProxy::GetDescriptor())) {
        HILOGE("[OnGetBatteryLevelEvent] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("[OnGetBatteryLevelEvent] fail: write device failed.");
        return;
    }

    if (!data.WriteInt32(batteryLevel)) {
        HILOGE("[OnGetBatteryLevelEvent] fail: write batteryLevel failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int error = InnerTransact(
        BluetoothDeviceBatteryObserverInterfaceCode::BT_DEVICE_GET_BATTERYLEVEL_EVENT, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothDeviceBatteryObserverProxy::OnGetBatteryLevelEvent done fail, error: %{public}d", error);
        return;
    }
}

void BluetoothDeviceBatteryObserverProxy::OnBatteryLevelChanged(
    const BluetoothRawAddress &device, int32_t batteryLevel)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothDeviceBatteryObserverProxy::GetDescriptor())) {
        HILOGE("[OnBatteryLevelChanged] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&device)) {
        HILOGE("[OnBatteryLevelChanged] fail: write device failed.");
        return;
    }

    if (!data.WriteInt32(batteryLevel)) {
        HILOGE("[OnBatteryLevelChanged] fail: write batteryLevel failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int error = InnerTransact(
        BluetoothDeviceBatteryObserverInterfaceCode::BT_DEVICE_BATTERYLEVEL_CHANGED, option, data, reply);
    if (error != NO_ERROR) {
        HILOGE("BluetoothDeviceBatteryObserverProxy::OnBatteryLevelChanged done fail, error: %{public}d", error);
        return;
    }
}

ErrCode BluetoothDeviceBatteryObserverProxy::InnerTransact(
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