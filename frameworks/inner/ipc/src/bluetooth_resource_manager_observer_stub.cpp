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

#include "bluetooth_resource_manager_observer_stub.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
BluetoothResourceManagerObserverStub::BluetoothResourceManagerObserverStub()
{
    HILOGD("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothResourceManagerObserverInterfaceCode::SENSING_STATE_CHANGED)] =
        &BluetoothResourceManagerObserverStub::OnSensingStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothResourceManagerObserverInterfaceCode::BLUETOOTH_RESOURCE_DECISION)] =
        &BluetoothResourceManagerObserverStub::OnBluetoothResourceDecisionInner;
}

BluetoothResourceManagerObserverStub::~BluetoothResourceManagerObserverStub()
{
    HILOGD("start.");
    memberFuncMap_.clear();
}

int BluetoothResourceManagerObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    if (BluetoothResourceManagerObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
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

ErrCode BluetoothResourceManagerObserverStub::OnSensingStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    uint8_t eventId = data.ReadUint8();
    std::shared_ptr<BluetoothSensingInfo> info(data.ReadParcelable<BluetoothSensingInfo>());
    if (!info) {
        return TRANSACTION_ERR;
    }
    OnSensingStateChanged(eventId, *info);
    return NO_ERROR;
}

ErrCode BluetoothResourceManagerObserverStub::OnBluetoothResourceDecisionInner(
    MessageParcel &data, MessageParcel &reply)
{
    uint8_t eventId = data.ReadUint8();
    std::shared_ptr<BluetoothSensingInfo> info(data.ReadParcelable<BluetoothSensingInfo>());
    if (!info) {
        return TRANSACTION_ERR;
    }
    uint32_t result = bluetooth::CONNECTION_ACCEPT;
    OnBluetoothResourceDecision(eventId, *info, result);
    if (!reply.WriteUint32(result)) {
        HILOGE("reply write failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return NO_ERROR;
}
}
}