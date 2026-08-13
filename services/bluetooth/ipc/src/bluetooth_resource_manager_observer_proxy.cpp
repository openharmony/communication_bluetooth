/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_resource_manag_obs_proxy"
#endif

#include "bluetooth_resource_manager_observer_proxy.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
BluetoothResourceManagerObserverproxy::BluetoothResourceManagerObserverproxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IBluetoothResourceManagerObserver>(impl)
{}
BluetoothResourceManagerObserverproxy::~BluetoothResourceManagerObserverproxy()
{}

void BluetoothResourceManagerObserverproxy::OnSensingStateChanged(uint8_t eventId, const BluetoothSensingInfo &info)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothResourceManagerObserverproxy::GetDescriptor()),
        "write InterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteUint8(eventId), "write eventId error");
    CHECK_AND_RETURN_LOG(data.WriteParcelable(&info), "write info error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t error = InnerTransact(
        BluetoothResourceManagerObserverInterfaceCode::SENSING_STATE_CHANGED, option, data, reply);
    CHECK_AND_RETURN_LOG(error == NO_ERROR, "InnerTransact fail, error: %{public}d", error);
}

void BluetoothResourceManagerObserverproxy::OnBluetoothResourceDecision(
    uint8_t eventId, const BluetoothSensingInfo &info, uint32_t &result)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothResourceManagerObserverproxy::GetDescriptor()),
        "write InterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteUint8(eventId), "write eventId error");
    CHECK_AND_RETURN_LOG(data.WriteParcelable(&info), "write info error");

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(
        BluetoothResourceManagerObserverInterfaceCode::BLUETOOTH_RESOURCE_DECISION, option, data, reply);
    CHECK_AND_RETURN_LOG(error == NO_ERROR, "InnerTransact fail, error: %{public}d", error);
    result = reply.ReadUint32();
}

ErrCode BluetoothResourceManagerObserverproxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGW("fail: get Remote fail code %{public}d", code);
        return OBJECT_NULL;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return NO_ERROR;
        }
        case DEAD_OBJECT: {
            HILOGW("fail: ipcErr=%{public}d code %{public}d", err, code);
            return DEAD_OBJECT;
        }
        default: {
            HILOGW("fail: ipcErr=%{public}d code %{public}d", err, code);
            return TRANSACTION_ERR;
        }
    }
}
}
}