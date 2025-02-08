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
#define LOG_TAG "bt_ipc_ble_advertiser_proxy"
#endif

#include "bluetooth_ble_advertiser_proxy.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "parcel_bt_uuid.h"

namespace OHOS {
namespace Bluetooth {
BluetoothBleAdvertiserProxy::BluetoothBleAdvertiserProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IBluetoothBleAdvertiser>(impl)
{}

BluetoothBleAdvertiserProxy::~BluetoothBleAdvertiserProxy()
{}

void BluetoothBleAdvertiserProxy::RegisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback> &callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGW("[RegisterBleAdvertiserCallback] fail: write interface token failed.");
        return;
    }

    if (!data.WriteRemoteObject(callback->AsObject())) {
        HILOGW("[RegisterBleAdvertiserCallback] fail: write callback failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_REGISTER_BLE_ADVERTISER_CALLBACK, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[RegisterBleAdvertiserCallback] fail: transact ErrCode=%{public}d", result);
    }
}
void BluetoothBleAdvertiserProxy::DeregisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback> &callback)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGW("[DeregisterBleAdvertiserCallback] fail: write interface token failed.");
        return;
    }

    if (!data.WriteRemoteObject(callback->AsObject())) {
        HILOGW("[DeregisterBleAdvertiserCallback] fail: write callback failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_DE_REGISTER_BLE_ADVERTISER_CALLBACK, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[DeregisterBleAdvertiserCallback] fail: transact ErrCode=%{public}d", result);
    }
}

int BluetoothBleAdvertiserProxy::StartAdvertising(const BluetoothBleAdvertiserSettings &settings,
    const BluetoothBleAdvertiserData &advData, const BluetoothBleAdvertiserData &scanResponse, int32_t advHandle,
    uint16_t duration, bool isRawData)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGE("[StartAdvertising] fail: write interface token failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteParcelable(&settings)) {
        HILOGE("[StartAdvertising] fail:write settings failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteParcelable(&advData)) {
        HILOGE("[StartAdvertising] fail:write advData failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteParcelable(&scanResponse)) {
        HILOGE("[StartAdvertising] fail:write scanResponse failed");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteInt32(advHandle)) {
        HILOGE("[StartAdvertising] fail: write advHandle failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteUint16(duration)) {
        HILOGE("[StartAdvertising] fail: write duration failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteBool(isRawData)) {
        HILOGE("[StartAdvertising] fail: write isRawData failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_START_ADVERTISING, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[StartAdvertising] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int BluetoothBleAdvertiserProxy::EnableAdvertising(uint8_t advHandle, uint16_t duration)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGE("[EnableAdvertising] fail: write interface token failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteUint8(advHandle)) {
        HILOGE("[EnableAdvertising] fail: write advHandle failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteUint16(duration)) {
        HILOGE("[EnableAdvertising] fail: write duration failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_ENABLE_ADVERTISING, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[EnableAdvertising] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int BluetoothBleAdvertiserProxy::DisableAdvertising(uint8_t advHandle)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGE("[DisableAdvertising] fail: write interface token failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteUint8(advHandle)) {
        HILOGE("[DisableAdvertising] fail: write advHandle failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_DISABLE_ADVERTISING, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[DisableAdvertising] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

int BluetoothBleAdvertiserProxy::StopAdvertising(int32_t advHandle)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGE("[StopAdvertising] fail: write interface token failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteInt32(advHandle)) {
        HILOGE("[StopAdvertising] fail: write advHandle failed.");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_STOP_ADVERTISING, option, data, reply);
    if (result != NO_ERROR) {
        HILOGE("[StopAdvertising] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    return reply.ReadInt32();
}

void BluetoothBleAdvertiserProxy::Close(int32_t advHandle)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGW("[Close] fail: write interface token failed.");
        return;
    }

    if (!data.WriteInt32(advHandle)) {
        HILOGW("[Close] fail: write advHandle failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_CLOSE, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[Close] fail: transact ErrCode=%{public}d", result);
    }
}

int32_t BluetoothBleAdvertiserProxy::GetAdvertiserHandle(int32_t &advHandle)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGW("[GetAdvertiserHandle] fail: write interface token failed.");
        return BT_ERR_INTERNAL_ERROR;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_GET_ADVERTISER_HANDLE, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[GetAdvertiserHandle] fail: transact ErrCode=%{public}d", result);
        return BT_ERR_INTERNAL_ERROR;
    }
    int32_t ret = reply.ReadInt32();
    advHandle = reply.ReadInt32();
    return ret;
}

void BluetoothBleAdvertiserProxy::SetAdvertisingData(const BluetoothBleAdvertiserData &advData,
    const BluetoothBleAdvertiserData &scanResponse, int32_t advHandle)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGW("[SetAdvertisingData] fail: write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&advData)) {
        HILOGW("[SetAdvertisingData] fail:write advData failed");
        return;
    }

    if (!data.WriteParcelable(&scanResponse)) {
        HILOGW("[SetAdvertisingData] fail:write scanResponse failed");
        return;
    }

    if (!data.WriteInt32(advHandle)) {
        HILOGW("[SetAdvertisingData] fail: write advHandle failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    ErrCode result = InnerTransact(BLE_SET_ADVERTISING_DATA, option, data, reply);
    if (result != NO_ERROR) {
        HILOGW("[SetAdvertisingData] fail: transact ErrCode=%{public}d", result);
    }
}

int BluetoothBleAdvertiserProxy::ChangeAdvertisingParams(uint8_t advHandle,
    const BluetoothBleAdvertiserSettings &settings)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBleAdvertiserProxy::GetDescriptor())) {
        HILOGW("[ChangeAdvertisingParams] fail: transact desCriptor");
        return BT_ERR_IPC_TRANS_FAILED;
    }

    if (!data.WriteUint8(advHandle)) {
        HILOGW("[ChangeAdvertisingParams] fail: transact advHandle:%{public}u", advHandle);
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteParcelable(&settings)) {
        HILOGW("[ChangeAdvertisingParams] fail: transact settings");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    int result = InnerTransact(BLE_CHANGE_ADVERTISING_PARAMS, option, data, reply);
    if (result != BT_NO_ERROR) {
        HILOGW("[ChangeAdvertisingParams] fail: transact ErrCode=%{public}d", result);
    }
    return result;
}

ErrCode BluetoothBleAdvertiserProxy::InnerTransact(
    uint32_t code, MessageOption &flags, MessageParcel &data, MessageParcel &reply)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOGW("[InnerTransact] fail: get Remote fail code %{public}d", code);
        return OBJECT_NULL;
    }
    int err = remote->SendRequest(code, data, reply, flags);
    switch (err) {
        case NO_ERROR: {
            return NO_ERROR;
        }
        case DEAD_OBJECT: {
            HILOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return DEAD_OBJECT;
        }
        default: {
            HILOGW("[InnerTransact] fail: ipcErr=%{public}d code %{public}d", err, code);
            return TRANSACTION_ERR;
        }
    }
}
}  // namespace Bluetooth
}  // namespace OHOS