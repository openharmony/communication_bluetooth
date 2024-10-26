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
#define LOG_TAG "bt_ipc_pan_observer_stub"
#endif

#include "bluetooth_pan_observer_stub.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
int BluetoothPanObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    switch (code) {
        case static_cast<uint32_t>(BluetoothPanObserverInterfaceCode::COMMAND_ON_CONNECTION_STATE_CHANGED): {
            if (BluetoothPanObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
                HILOGE("local descriptor is not equal to remote");
                return IPC_INVOKER_TRANSLATE_ERR;
            }
            std::shared_ptr<BluetoothRawAddress> address(data.ReadParcelable<BluetoothRawAddress>());
            if (address == nullptr) {
                HILOGE("null pointer");
                return ERR_TRANSACTION_FAILED;
            }
            int state = data.ReadInt32();
            int cause = data.ReadInt32();
            OnConnectionStateChanged(*address, state, cause);
            return NO_ERROR;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return ERR_TRANSACTION_FAILED;
}
}  // namespace Bluetooth
}  // namespace OHOS