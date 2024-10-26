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
#define LOG_TAG "bt_ipc_a2dp_sink_observer_stub"
#endif

#include "bluetooth_a2dp_sink_observer_stub.h"
#include "bluetooth_log.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
BluetoothA2dpSinkObserverStub::BluetoothA2dpSinkObserverStub()
{
    HILOGD("start.");
}

BluetoothA2dpSinkObserverStub::~BluetoothA2dpSinkObserverStub()
{
    HILOGD("start.");
}

int BluetoothA2dpSinkObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGE("cmd = %{public}d, flags= %{public}d", code, option.GetFlags());

    if (BluetoothA2dpSinkObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGI("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    ErrCode errCode = NO_ERROR;
    switch (code) {
        case static_cast<uint32_t>(
            BluetoothA2dpSinkObserverInterfaceCode::BT_A2DP_SINK_OBSERVER_CONNECTION_STATE_CHANGED):
            errCode = OnConnectionStateChangedInner(data, reply);
            break;
        default:
            HILOGW("default case, need check.");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return errCode;
}

ErrCode BluetoothA2dpSinkObserverStub::OnConnectionStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    if (BluetoothA2dpSinkObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGE("check interface token failed");
        return ERR_INVALID_STATE;
    }
    std::string addr = data.ReadString();
    int state = data.ReadInt32();
    int cause = data.ReadInt32();

    OnConnectionStateChanged(RawAddress(addr), state, cause);

    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS