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
        STR_BT_GATT_CLIENT_CALLBACK_BLE_MTU_CHANGE,
        STR_BT_GATT_CLIENT_CALLBACK_SERVICE_CHANGE,
        STR_BT_GATT_CLIENT_CALLBACK_BLE_PHY_UPDATE},
        BT_MODULE_NAME)
{}

void NapiGattClientCallback::OnCharacteristicChanged(const GattCharacteristic &characteristic)
{
    auto nativeObject = std::make_shared<NapiNativeBleCharacteristic>(characteristic);
    eventSubscribe_.PublishEvent(STR_BT_GATT_CLIENT_CALLBACK_BLE_CHARACTERISTIC_CHANGE, nativeObject);
}

void NapiGattClientCallback::OnServicesChanged()
{
    auto nativeObject = std::make_shared<NapiNativeInt>(BT_NO_ERROR);
    eventSubscribe_.PublishEvent(STR_BT_GATT_CLIENT_CALLBACK_SERVICE_CHANGE, nativeObject);
}

void NapiGattClientCallback::OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(NapiGattClient::GattStatusFromService(ret)); // Adaptation for old sdk
    auto napiCharacter = std::make_shared<NapiNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_CHARACTER, napiCharacter, ret);
}

void NapiGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(NapiGattClient::GattStatusFromService(ret)); // Adaptation for old sdk
    auto napiDescriptor = std::make_shared<NapiNativeBleDescriptor>(descriptor);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_DESCRIPTOR, napiDescriptor, ret);
}

void NapiGattClientCallback::OnConnectionStateChangedWithReason(int connectionState, int ret, int disconnectReason)
{
    HILOGI("connectionState:%{public}d, disconnectReason:%{public}d, ret:%{public}d",
        connectionState, disconnectReason, ret);
    if (connectionState == static_cast<int>(BTConnectState::DISCONNECTED)) {
        auto nativeObject = std::make_shared<NapiNativeBleConnectionStateChangeParam>(
            deviceAddr_, connectionState, 0, true, disconnectReason);
        eventSubscribe_.PublishEvent(STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE, nativeObject);
        return;
    }
    auto nativeObject = std::make_shared<NapiNativeBleConnectionStateChangeParam>(deviceAddr_, connectionState);
    eventSubscribe_.PublishEvent(STR_BT_GATT_CLIENT_CALLBACK_BLE_CONNECTIION_STATE_CHANGE, nativeObject);
}

void NapiGattClientCallback::OnServicesDiscovered(int status)
{
    HILOGI("status:%{public}d", status);
}

void NapiGattClientCallback::OnConnectionParameterChanged(int interval, int latency, int timeout, int status)
{
    int ret = -1;
    if (status == static_cast<int>(GattStatus::GATT_SUCCESS)) {
        ret = static_cast<int>(BT_NO_ERROR);
    } else {
        ret = static_cast<int>(BT_ERR_INTERNAL_ERROR);
    }
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_UPDATE_CONNECTION_PRIORITY, nullptr, ret);
}

void NapiGattClientCallback::OnReadRemoteRssiValueResult(int rssi, int ret)
{
    HILOGI("rssi: %{public}d, ret: %{public}d", rssi, ret);
    ret = GetSDKAdaptedStatusCode(NapiGattClient::GattStatusFromService(ret)); // Adaptation for old sdk
    auto napiRssi = std::make_shared<NapiNativeInt>(rssi);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_REMOTE_RSSI_VALUE, napiRssi, ret);
}

void NapiGattClientCallback::OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(NapiGattClient::GattStatusFromService(ret)); // Adaptation for old sdk
    auto napiCharacter = std::make_shared<NapiNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_WRITE_CHARACTER, napiCharacter, ret);
#endif
}

void NapiGattClientCallback::OnCharacteristicWriteResultWithContext(const GattCharacteristic &characteristic,
    const BluetoothGattRspContext &rspContext, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, timeStamp:%{public}lld, ret: %{public}d", characteristic.GetUuid().ToString().c_str(),
        static_cast<long long>(rspContext.GetTimeStamp()), ret);
    ret = GetSDKAdaptedStatusCode(NapiGattClient::GattStatusFromService(ret)); // Adaptation for old sdk
    auto napiRspContext = std::make_shared<NapiNativeGattResponseContext>(rspContext);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_WRITE_CHARACTER_WITH_CONTEXT, napiRspContext, ret);
#endif
}

void NapiGattClientCallback::OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(NapiGattClient::GattStatusFromService(ret)); // Adaptation for old sdk
    auto napiDescriptor = std::make_shared<NapiNativeBleDescriptor>(descriptor);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_WRITE_DESCRIPTOR, napiDescriptor, ret);
#endif
}

void NapiGattClientCallback::OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status)
{
#ifdef BLUETOOTH_API_SINCE_10
    HILOGI("UUID: %{public}s, status: %{public}d", characteristic.GetUuid().ToString().c_str(), status);
    status = GetSDKAdaptedStatusCode(NapiGattClient::GattStatusFromService(status)); // Adaptation for old sdk
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

void NapiGattClientCallback::OnBlePhyUpdate(int32_t txPhy, int32_t rxPhy, int32_t status)
{
    HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_SET_PHY, nullptr, status);
    auto napiBlePhy = std::make_shared<NapiNativeBlePhyInfo>(txPhy, rxPhy);
    eventSubscribe_.PublishEvent(STR_BT_GATT_CLIENT_CALLBACK_BLE_PHY_UPDATE, napiBlePhy);
}

void NapiGattClientCallback::OnBlePhyRead(int32_t txPhy, int32_t rxPhy, int32_t status)
{
    HILOGI("txPhy: %{public}d, rxPhy: %{public}d, status: %{public}d", txPhy, rxPhy, status);
    auto napiBlePhy = std::make_shared<NapiNativeBlePhyInfo>(txPhy, rxPhy);
    AsyncWorkCallFunction(asyncWorkMap_, NapiAsyncType::GATT_CLIENT_READ_PHY, napiBlePhy, status);
}

} // namespace Bluetooth
} // namespace OHOS
