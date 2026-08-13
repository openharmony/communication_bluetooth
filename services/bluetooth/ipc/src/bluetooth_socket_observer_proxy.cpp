/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_socket_observer_proxy"
#endif

#include "bluetooth_socket_observer_proxy.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
void BluetoothClientSocketObserverProxy::OnConnectionStateChanged(const CallbackParam &callbackParam)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(BluetoothClientSocketObserverProxy::GetDescriptor())) {
        HILOGE(" WriteInterfaceToken error");
        return;
    }
    if (!data.WriteParcelable(&callbackParam.dev)) {
        HILOGE(" Write dev error");
        return;
    }

    BluetoothUuid btUuid(callbackParam.uuid);
    if (!data.WriteParcelable(&btUuid)) {
        HILOGE(" Write uuid error");
        return;
    }
    if (!data.WriteInt32(callbackParam.status)) {
        HILOGE("Write status error");
        return;
    }
    if (!data.WriteInt32(callbackParam.result)) {
        HILOGE("Write result error");
        return;
    }
    if (!data.WriteInt32(callbackParam.type)) {
        HILOGE("Write result error");
        return;
    }
    if (!data.WriteInt32(callbackParam.psm)) {
        HILOGE("Write result error");
        return;
    }

    ErrCode ret = InnerTransact(BT_SOCKET_OBSERVER_CONNECTION_STATE_CHANGED, option, data, reply);
    if (ret != NO_ERROR) {
        HILOGE("OnConnectionStateChanged done fail, error: %{public}d", ret);
        return;
    }

    return;
}

ErrCode BluetoothClientSocketObserverProxy::InnerTransact(
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