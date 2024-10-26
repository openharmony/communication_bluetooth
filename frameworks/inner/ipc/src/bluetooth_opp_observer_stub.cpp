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
#define LOG_TAG "bt_ipc_opp_observer_stub"
#endif

#include "bluetooth_errorcode.h"
#include "bluetooth_opp_observer_stub.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {

BluetoothOppObserverStub::BluetoothOppObserverStub()
{
    HILOGI("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothOppObserverInterfaceCode::OPP_ON_RECEIVE_INCOMING_FILE_CHANGED)] =
        BluetoothOppObserverStub::OnReceiveIncomingFileChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothOppObserverInterfaceCode::OPP_ON_TRANSFER_STATE_CHANGED)] =
        BluetoothOppObserverStub::OnTransferStateChangedInner;
}

BluetoothOppObserverStub::~BluetoothOppObserverStub()
{
    HILOGI("start.");
    memberFuncMap_.clear();
}

int32_t BluetoothOppObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("cmd = %{public}d, flags= %{public}d", code, option.GetFlags());
    if (BluetoothOppObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
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
    HILOGW("OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t BluetoothOppObserverStub::OnReceiveIncomingFileChangedInner(BluetoothOppObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    HILOGD("Enter.");
    std::shared_ptr<BluetoothIOppTransferInformation> oppInformation(
        data.ReadParcelable<BluetoothIOppTransferInformation>());
    CHECK_AND_RETURN_LOG_RET((oppInformation != nullptr), BT_ERR_INTERNAL_ERROR, "Read oppInformation error");
    stub->OnReceiveIncomingFileChanged(*oppInformation);
    return BT_NO_ERROR;
}

int32_t BluetoothOppObserverStub::OnTransferStateChangedInner(BluetoothOppObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    HILOGD("Enter.");
    std::shared_ptr<BluetoothIOppTransferInformation> oppInformation(
        data.ReadParcelable<BluetoothIOppTransferInformation>());
    CHECK_AND_RETURN_LOG_RET((oppInformation != nullptr), BT_ERR_INTERNAL_ERROR, "Read oppInformation error");
    stub->OnTransferStateChanged(*oppInformation);
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS