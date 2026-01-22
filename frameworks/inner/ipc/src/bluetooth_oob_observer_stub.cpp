/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_oob_observer_stub"
#endif

#include "bluetooth_oob_observer_stub.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
BluetoothOobObserverStub::BluetoothOobObserverStub()
{
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothOobObserverInterfaceCode::BT_OOB_OBSERVER_GENERATE_LOCAL_OOB_DATA)]
        = BluetoothOobObserverStub::OnGenerateLocalOobDataInner;
}

BluetoothOobObserverStub::~BluetoothOobObserverStub()
{
    memberFuncMap_.clear();
}

int32_t BluetoothOobObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("transaction of code: %{public}u is received", code);
    if (BluetoothOobObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGE("BluetoothOobObserverStub::OnRemoteRequest, local descriptor is not equal to remote");
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

int32_t BluetoothOobObserverStub::OnGenerateLocalOobDataInner(BluetoothOobObserverStub *stub,
    MessageParcel &data, MessageParcel &reply)
{
    int32_t ret;
    CHECK_AND_RETURN_LOG_RET(data.ReadInt32(ret), BT_ERR_IPC_TRANS_FAILED,
        "BluetoothOobObserverStub::OnGenerateLocalOobDataInner read ret failed");
    std::shared_ptr<BluetoothOobData> oobData(data.ReadParcelable<BluetoothOobData>());
    CHECK_AND_RETURN_LOG_RET(oobData != nullptr, BT_ERR_IPC_TRANS_FAILED, "oobData is nullptr");
    HILOGD("onGenerateLocalOobDataInner starts");
    stub->OnGenerateLocalOobData(ret, *oobData);
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
