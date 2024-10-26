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
#ifndef LOG_TAG
#define LOG_TAG "bt_ipc_avrcp_tg_observer_stub"
#endif

#include "bluetooth_avrcp_tg_observer_stub.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
BluetoothAvrcpTgObserverStub::BluetoothAvrcpTgObserverStub()
{
    HILOGD("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothAvrcpTgObserverInterfaceCode::BT_AVRCP_TG_OBSERVER_CONNECTION_STATE_CHANGED)] =
        BluetoothAvrcpTgObserverStub::OnConnectionStateChangedInner;
}

BluetoothAvrcpTgObserverStub::~BluetoothAvrcpTgObserverStub()
{
    HILOGD("start.");
    memberFuncMap_.clear();
}

int BluetoothAvrcpTgObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothAvrcpTgObserverStub::OnRemoteRequest, cmd = %{public}d, flags= %{public}d",
        code, option.GetFlags());
    if (BluetoothAvrcpTgObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return memberFunc(this, data, reply);
        }
    }
    HILOGW("BluetoothAvrcpTgObserverStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothAvrcpTgObserverStub::OnConnectionStateChangedInner(
    BluetoothAvrcpTgObserverStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothAvrcpTgObserverStub::OnConnectionStateChangedInner Triggered!");
    std::shared_ptr<BluetoothRawAddress> addr(data.ReadParcelable<BluetoothRawAddress>());
    if (!addr) {
        return TRANSACTION_ERR;
    }
    int state = data.ReadInt32();
    int cause = data.ReadInt32();

    stub->OnConnectionStateChanged(*addr, state, cause);
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS