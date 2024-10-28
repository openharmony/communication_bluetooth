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
#ifndef LOG_TAG
#define LOG_TAG "bt_napi_gatt_server_callback"
#endif

#include <uv.h>
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "napi_bluetooth_gatt_server.h"
#include "napi_bluetooth_gatt_server_callback.h"
#include "napi_event_subscribe_module.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

NapiGattServerCallback::NapiGattServerCallback()
    : eventSubscribe_({STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ,
        STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE,
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ,
        STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE,
        STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE,
        STR_BT_GATT_SERVER_CALLBACK_MTU_CHANGE},
        BT_MODULE_NAME)
{}

void NapiGattServerCallback::OnCharacteristicReadRequest(
    const BluetoothRemoteDevice &device, GattCharacteristic &characteristic, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    auto nativeObject =
        std::make_shared<NapiNativeGattsCharacterReadRequest>(requestId, device.GetDeviceAddr(), characteristic);
    eventSubscribe_.PublishEvent(STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_READ, nativeObject);
}

void NapiGattServerCallback::OnCharacteristicWriteRequest(const BluetoothRemoteDevice &device,
    GattCharacteristic &characteristic, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    auto nativeObject =
        std::make_shared<NapiNativeGattsCharacterWriteRequest>(requestId, device.GetDeviceAddr(), characteristic);
    eventSubscribe_.PublishEvent(STR_BT_GATT_SERVER_CALLBACK_CHARACTERISTIC_WRITE, nativeObject);
}

void NapiGattServerCallback::OnConnectionStateUpdate(const BluetoothRemoteDevice &device, int state)
{
    HILOGI("enter, state: %{public}d, remote device address: %{public}s", state, GET_ENCRYPT_ADDR(device));
    std::lock_guard<std::mutex> lock(NapiGattServer::deviceListMutex_);
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        HILOGI("connected");
        bool hasAddr = false;
        for (auto it = NapiGattServer::deviceList_.begin(); it != NapiGattServer::deviceList_.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                hasAddr = true;
                break;
            }
        }
        if (!hasAddr) {
            HILOGI("add devices");
            NapiGattServer::deviceList_.push_back(device.GetDeviceAddr());
        }
    } else if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        HILOGI("disconnected");
        for (auto it = NapiGattServer::deviceList_.begin(); it != NapiGattServer::deviceList_.end(); ++it) {
            if (*it == device.GetDeviceAddr()) {
                HILOGI("romove device");
                NapiGattServer::deviceList_.erase(it);
                break;
            }
        }
    }

    auto nativeObject =
        std::make_shared<NapiNativeBleConnectionStateChangeParam>(device.GetDeviceAddr(), state);
    eventSubscribe_.PublishEvent(STR_BT_GATT_SERVER_CALLBACK_CONNECT_STATE_CHANGE, nativeObject);
}

void NapiGattServerCallback::OnDescriptorWriteRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    auto nativeObject =
        std::make_shared<NapiNativeGattsDescriptorWriteRequest>(requestId, device.GetDeviceAddr(), descriptor);
    eventSubscribe_.PublishEvent(STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_WRITE, nativeObject);
}

void NapiGattServerCallback::OnDescriptorReadRequest(const BluetoothRemoteDevice &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("enter, remote device address: %{public}s, requestId: %{public}d",
        GET_ENCRYPT_ADDR(device), requestId);
    auto nativeObject =
        std::make_shared<NapiNativeGattsDescriptorReadRequest>(requestId, device.GetDeviceAddr(), descriptor);
    eventSubscribe_.PublishEvent(STR_BT_GATT_SERVER_CALLBACK_DESCRIPTOR_READ, nativeObject);
}

void NapiGattServerCallback::OnMtuUpdate(const BluetoothRemoteDevice &device, int mtu)
{
    HILOGI("enter, remote device address: %{public}s, mtu: %{public}d",
        GET_ENCRYPT_ADDR(device), mtu);
    auto nativeObject = std::make_shared<NapiNativeInt>(mtu);
    eventSubscribe_.PublishEvent(STR_BT_GATT_SERVER_CALLBACK_MTU_CHANGE, nativeObject);
}

void NapiGattServerCallback::OnNotificationCharacteristicChanged(const BluetoothRemoteDevice &device, int ret)
{
    HILOGI("ret: %{public}d", ret);
    auto napiRet = std::make_shared<NapiNativeInt>(ret);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_SERVER_NOTIFY_CHARACTERISTIC, napiRet, ret);
}
} // namespace Bluetooth
} // namespace OHOS
