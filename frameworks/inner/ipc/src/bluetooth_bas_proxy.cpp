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
#define LOG_TAG "bt_ipc_bas_proxy"
#endif

#include "bluetooth_bas_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

int32_t BluetoothBasProxy::RegisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBasProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (observer == nullptr) {
        HILOGE("observer is nullptr");
        return BT_ERR_INVALID_PARAM;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("write RemoteObject error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error =
        InnerTransact(BluetoothHostInterfaceCode::BT_REGISTER_DEVICE_BATTERY_OBSERVER, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("fail error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t BluetoothBasProxy::DeregisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBasProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (observer == nullptr) {
        HILOGE("observer is nullptr");
        return BT_ERR_INVALID_PARAM;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOGE("write RemoteObject error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error =
        InnerTransact(BluetoothHostInterfaceCode::BT_DEREGISTER_DEVICE_BATTERY_OBSERVER, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("fail error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t BluetoothBasProxy::GetBatteryLevel(const std::string &address)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBasProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    if (!data.WriteString(address)) {
        HILOGE("write address error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error = InnerTransact(BluetoothHostInterfaceCode::BT_GET_BATTERY_LEVEL, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("fail error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t BluetoothBasProxy::GetConnectedDeviceBatteryInfos(std::map<std::string, int32_t> &batteryInfos)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(BluetoothBasProxy::GetDescriptor())) {
        HILOGE("WriteInterfaceToken error");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int32_t error =
        InnerTransact(BluetoothHostInterfaceCode::GET_CONNECTED_DEVICE_BATTERY_INFOS, option, data, reply);
    if (error != BT_NO_ERROR) {
        HILOGE("fail error: %{public}d", error);
        return error;
    }
    int32_t exception = reply.ReadInt32();
    if (exception != BT_NO_ERROR) {
        HILOGE("fail result: %{public}d", exception);
        return exception;
    }
    int32_t size = reply.ReadInt32();
    const int32_t maxSize = 100;
    if (size < 0 || size > maxSize) {
        HILOGE("invalid size");
        return BT_ERR_IPC_TRANS_FAILED;
    }
    for (int32_t i = 0; i < size; i++) {
        std::string address = reply.ReadString();
        int32_t batteryLevel = reply.ReadInt32();
        batteryInfos[address] = batteryLevel;
    }
    return exception;
}
} // namespace Bluetooth
} // namespace OHOS