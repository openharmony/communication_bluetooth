/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "bluetooth_socket_observer_stub.h"
#include "bluetooth_log.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_bt_uuid.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
int32_t BluetoothClientSocketObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothClientSocketObserverStub: transaction of code: %{public}d is received", code);
    if (BluetoothClientSocketObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGE("BluetoothClientSocketObserverStub::OnRemoteRequest, local descriptor is not equal to remote");
        return BT_ERR_INVALID_STATE;
    }

    int32_t errcode = BT_NO_ERROR;
    switch (code) {
        case static_cast<uint32_t>(BluetoothSocketObserverInterfaceCode::BT_SOCKET_OBSERVER_CONNECTION_STATE_CHANGED):
            errcode = OnConnectionStateChangedInner(data, reply);
            break;
        default:
            HILOGW("default case, need check.");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return errcode;
}

int32_t BluetoothClientSocketObserverStub::OnConnectionStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGD("enter");

    std::shared_ptr<BluetoothRawAddress> addr(data.ReadParcelable<BluetoothRawAddress>());
    if (!addr) {
        HILOGE("read addr failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    std::shared_ptr<BluetoothUuid> uuid(data.ReadParcelable<BluetoothUuid>());
    if (!uuid) {
        HILOGE("read uuid failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    bluetooth::Uuid btUuid = bluetooth::Uuid(*uuid);
    int status = data.ReadInt32();
    int result = data.ReadInt32();
    OnConnectionStateChanged(*addr, btUuid, status, result);
    return BT_NO_ERROR;
}

int32_t BluetoothServerSocketObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothServerSocketObserverStub: transaction of code: %{public}d is received", code);
    if (BluetoothServerSocketObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGE("BluetoothServerSocketObserverStub::OnRemoteRequest, local descriptor is not equal to remote");
        return BT_ERR_INVALID_STATE;
    }

    HILOGW("BluetoothServerSocketObserverStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace Bluetooth
}  // namespace OHOS