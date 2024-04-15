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

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_map_mse_observer_stub.h"

namespace OHOS {
namespace Bluetooth {
BluetoothMapMseObserverStub::BluetoothMapMseObserverStub()
{
    HILOGI("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothMapMseObserverInterfaceCode::MSE_ON_CONNECTION_STATE_CHANGED)] =
        &BluetoothMapMseObserverStub::OnConnectionStateChangedInner;
}

BluetoothMapMseObserverStub::~BluetoothMapMseObserverStub()
{
    HILOGI("start.");
    memberFuncMap_.clear();
}

int32_t BluetoothMapMseObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("MapMseObserverStub::OnRemoteRequest, cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    if (BluetoothMapMseObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t BluetoothMapMseObserverStub::OnConnectionStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    HILOGD("Enter.");
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    CHECK_AND_RETURN_LOG_RET((device != nullptr), BT_ERR_INTERNAL_ERROR, "Read device error");
    int32_t state = data.ReadInt32();
    int32_t cause = data.ReadInt32();
    OnConnectionStateChanged(*device, state, cause);
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
