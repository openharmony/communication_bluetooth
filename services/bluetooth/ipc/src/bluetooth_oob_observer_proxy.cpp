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
#define LOG_TAG "bt_ipc_oob_observer_proxy"
#endif

#include "bluetooth_oob_observer_proxy.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {

void BluetoothOobObserverProxy::OnGenerateLocalOobData(int32_t ret, const BluetoothOobData &oobData)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothOobObserverProxy::GetDescriptor())) {
        HILOGE("BluetoothOobObserverProxy::OnGenerateLocalOobData WriteInterfaceToken error");
        return;
    }

    if (!data.WriteInt32(ret)) {
        HILOGE("BluetoothOobObserverProxy::OnGenerateLocalOobData ret error");
        return;
    }

    if (!data.WriteParcelable(&oobData)) {
        HILOGE("BluetoothOobObserverProxy::OnGenerateLocalOobData oobData error");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    ErrCode result = InnerTransact(BT_OOB_OBSERVER_GENERATE_LOCAL_OOB_DATA, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("BluetoothOobObserverProxy::OnGenerateLocalOobData done fail, error: %{public}d", result);
        return;
    }
}

ErrCode BluetoothOobObserverProxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGE("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return ERR_DEAD_OBJECT;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return ERR_OK;
        }
        case DEAD_OBJECT: {
            HILOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return ERR_DEAD_OBJECT;
        }
        default: {
            HILOGE("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return TRANSACTION_ERR;
        }
    }
}
}  // namespace Bluetooth
}  // namespace OHOS
