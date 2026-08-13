/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_map_mse_observer_proxy"
#endif

#include "bluetooth_map_mse_observer_proxy.h"
#include "bluetooth_log.h"
#include "i_bluetooth_map_mse_observer.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
void BluetoothMapMseObserverProxy::OnConnectionStateChanged(const BluetoothRawAddress &device,
    int32_t state, int32_t cause)
{
    MessageParcel data;
    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(BluetoothMapMseObserverProxy::GetDescriptor()),
        "WriteInterfaceToken error");
    CHECK_AND_RETURN_LOG(data.WriteParcelable(&device), "Writedevice error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(state), "Writestate error");
    CHECK_AND_RETURN_LOG(data.WriteInt32(cause), "Writecause error");

    MessageParcel reply;
    MessageOption option {
        MessageOption::TF_SYNC
    };

    auto remote = Remote();
    CHECK_AND_RETURN_LOG(remote, "remote nullptr");
    int32_t error = remote->SendRequest(
        BluetoothMapMseObserverInterfaceCode::MSE_ON_CONNECTION_STATE_CHANGED, data, reply, option);
    CHECK_AND_RETURN_LOG((error == BT_NO_ERROR), "error: %{public}d", error);
}

}  // namespace Bluetooth
}  // namespace OHOS