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
#define LOG_TAG "bt_napi_gatt_client_callback"
#endif

#include <uv.h>
#include "bluetooth_log.h"
#include "napi_async_work.h"
#include "napi_bluetooth_event.h"
#include "napi_bluetooth_gatt_client.h"
#include "napi_bluetooth_gatt_client_callback.h"
#include "napi_event_subscribe_module.h"
#include "bluetooth_errorcode.h"

namespace OHOS {
namespace Bluetooth {
NapiGattClientCallback::NapiGattClientCallback()
    : eventSubscribe_({STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE,
        STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE,
        STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE},
        BT_MODULE_NAME)
{}

void NapiGattClientCallback::OnCharacteristicChanged(const GattCharacteristic &characteristic)
{
    auto nativeObject = std::make_shared<NapiNativeBleCharacteristic>(characteristic);
    eventSubscribe_.PublishEvent(STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE, nativeObject);
}

void NapiGattClientCallback::OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    auto napiCharacter = std::make_shared<NapiNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_CHARACTER, napiCharacter, ret);
}

void NapiGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    auto napiDescriptor = std::make_shared<NapiNativeBleDescriptor>(descriptor);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_DESCRIPTOR, napiDescriptor, ret);
}

void NapiGattClientCallback::OnConnectionStateChanged(int connectionState, int ret)
{
    HILOGI("connectionState:%{public}d, ret:%{public}d", connectionState, ret);
    auto nativeObject = std::make_shared<NapiNativeBleConnectionStateChangeParam>(deviceAddr_, connectionState);
    eventSubscribe_.PublishEvent(STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE, nativeObject);
}

void NapiGattClientCallback::OnServicesDiscovered(int status)
{
    HILOGI("status:%{public}d", status);
}

void NapiGattClientCallback::OnReadRemoteRssiValueResult(int rssi, int ret)
{
    HILOGI("rssi: %{public}d, ret: %{public}d", rssi, ret);
    auto napiRssi = std::make_shared<NapiNativeInt>(rssi);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_REMOTE_RSSI_VALUE, napiRssi, ret);
}

void NapiGattClientCallback::OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    auto napiCharacter = std::make_shared<NapiNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_WRITE_CHARACTER, napiCharacter, ret);
#endif
}

void NapiGattClientCallback::OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    auto napiDescriptor = std::make_shared<NapiNativeBleDescriptor>(descriptor);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_WRITE_DESCRIPTOR, napiDescriptor, ret);
#endif
}

void NapiGattClientCallback::OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, status: %{public}d", characteristic.GetUuid().ToString().c_str(), status);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_ENABLE_CHARACTER_CHANGED, nullptr, status);
#endif
}

void NapiGattClientCallback::OnMtuUpdate(int mtu, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("ret: %{public}d, mtu: %{public}d", ret, mtu);
    auto nativeObject = std::make_shared<NapiNativeInt>(mtu);
    eventSubscribe_.PublishEvent(STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE, nativeObject);
#endif
}

} // namespace Bluetooth
} // namespace OHOS
