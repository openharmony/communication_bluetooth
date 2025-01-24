/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_ipc_ble_advertiser_cb_stub"
#endif

#include "bluetooth_ble_advertise_callback_stub.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "string_ex.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
namespace OHOS {
namespace Bluetooth {
const std::map<uint32_t, std::function<ErrCode(
    BluetoothBleAdvertiseCallbackStub *, MessageParcel &, MessageParcel &)>>
    BluetoothBleAdvertiseCallbackStub::memberFuncMap_ = {
        {static_cast<uint32_t>(
            BluetoothBleAdvertiseCallbackInterfaceCode::BT_BLE_ADVERTISE_CALLBACK_AUTO_STOP_EVENT),
            BluetoothBleAdvertiseCallbackStub::OnAutoStopAdvEventInner},
        {static_cast<uint32_t>(
            BluetoothBleAdvertiseCallbackInterfaceCode::BT_BLE_ADVERTISE_CALLBACK_START_RESULT_EVENT),
            BluetoothBleAdvertiseCallbackStub::OnStartResultEventInner},
        {static_cast<uint32_t>(
            BluetoothBleAdvertiseCallbackInterfaceCode::BT_BLE_ADVERTISE_CALLBACK_ENABLE_RESULT_EVENT),
            BluetoothBleAdvertiseCallbackStub::OnEnableResultEventInner},
        {static_cast<uint32_t>(
            BluetoothBleAdvertiseCallbackInterfaceCode::BT_BLE_ADVERTISE_CALLBACK_DISABLE_RESULT_EVENT),
            BluetoothBleAdvertiseCallbackStub::OnDisableResultEventInner},
        {static_cast<uint32_t>(
            BluetoothBleAdvertiseCallbackInterfaceCode::BT_BLE_ADVERTISE_CALLBACK_STOP_RESULT_EVENT),
            BluetoothBleAdvertiseCallbackStub::OnStopResultEventInner},
        {static_cast<uint32_t>(
            BluetoothBleAdvertiseCallbackInterfaceCode::BT_BLE_ADVERTISE_CALLBACK_SET_ADV_DATA),
            BluetoothBleAdvertiseCallbackStub::OnSetAdvDataEventInner},
        {static_cast<uint32_t>(
            BluetoothBleAdvertiseCallbackInterfaceCode::BT_BLE_ADVERTISE_CALLBACK_CHANGE_ADV_RESULT),
            BluetoothBleAdvertiseCallbackStub::OnChangeAdvResultEventInner},
};

BluetoothBleAdvertiseCallbackStub::BluetoothBleAdvertiseCallbackStub()
{
    HILOGD("start.");
}

BluetoothBleAdvertiseCallbackStub::~BluetoothBleAdvertiseCallbackStub()
{
    HILOGD("start.");
}

int BluetoothBleAdvertiseCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothBleAdvertiseCallbackStub::OnRemoteRequest, cmd = %{public}d, flags= %{public}d",
        code, option.GetFlags());
    if (BluetoothBleAdvertiseCallbackStub::GetDescriptor() != data.ReadInterfaceToken()) {
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

    HILOGW("BluetoothBleAdvertiseCallbackStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothBleAdvertiseCallbackStub::OnStartResultEventInner(
    BluetoothBleAdvertiseCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    const int32_t result = static_cast<int32_t>(data.ReadInt32());
    const int32_t advHandle = static_cast<int32_t>(data.ReadInt32());
    const int32_t opcode = static_cast<int32_t>(data.ReadInt32());

    stub->OnStartResultEvent(result, advHandle, opcode);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothBleAdvertiseCallbackStub::OnEnableResultEventInner(
    BluetoothBleAdvertiseCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    const int32_t result = static_cast<int32_t>(data.ReadInt32());
    const int32_t advHandle = static_cast<int32_t>(data.ReadInt32());
    stub->OnEnableResultEvent(result, advHandle);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothBleAdvertiseCallbackStub::OnDisableResultEventInner(
    BluetoothBleAdvertiseCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    const int32_t result = static_cast<int32_t>(data.ReadInt32());
    const int32_t advHandle = static_cast<int32_t>(data.ReadInt32());
    stub->OnDisableResultEvent(result, advHandle);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothBleAdvertiseCallbackStub::OnStopResultEventInner(
    BluetoothBleAdvertiseCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    const int32_t result = static_cast<int32_t>(data.ReadInt32());
    const int32_t advHandle = static_cast<int32_t>(data.ReadInt32());
    stub->OnStopResultEvent(result, advHandle);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothBleAdvertiseCallbackStub::OnAutoStopAdvEventInner(
    BluetoothBleAdvertiseCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    const int32_t advHandle = static_cast<int32_t>(data.ReadInt32());
    stub->OnAutoStopAdvEvent(advHandle);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothBleAdvertiseCallbackStub::OnSetAdvDataEventInner(
    BluetoothBleAdvertiseCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    const int32_t result = static_cast<int32_t>(data.ReadInt32());
    const int32_t advHandle = static_cast<int32_t>(data.ReadInt32());
    stub->OnSetAdvDataEvent(result, advHandle);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothBleAdvertiseCallbackStub::OnChangeAdvResultEventInner(
    BluetoothBleAdvertiseCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    const int32_t result = static_cast<int32_t>(data.ReadInt32());
    const int32_t advHandle = static_cast<int32_t>(data.ReadInt32());
    stub->OnChangeAdvResultEvent(result, advHandle);
    return NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
