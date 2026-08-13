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
#define LOG_TAG "bt_ipc_pan_observer_proxy"
#endif

#include "bluetooth_pan_observer_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
ErrCode BluetoothPanObserverProxy::OnConnectionStateChanged(
    const BluetoothRawAddress &device, int state, int cause, int role)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(BluetoothPanObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothPanObserverProxy::OnConnectionStateChanged WriteInterfaceToken error");
        return IPC_PROXY_TRANSACTION_ERR;
    }
    data.WriteParcelable(&device);
    data.WriteInt32(state);
    data.WriteInt32(cause);
    data.WriteInt32(role);
    auto remote = Remote();
    CHECK_AND_RETURN_LOG_RET(remote, BT_ERR_INTERNAL_ERROR, "remote nullptr");
    int32_t error = remote->SendRequest(static_cast<uint32_t>(
        BluetoothPanObserverInterfaceCode::COMMAND_ON_CONNECTION_STATE_CHANGED), data, reply, option);
    CHECK_AND_RETURN_LOG_RET((error == BT_NO_ERROR), BT_ERR_INTERNAL_ERROR, "error: %{public}d", error);

    int32_t ret = reply.ReadInt32();
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "reply errCode: %{public}d", ret);
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS