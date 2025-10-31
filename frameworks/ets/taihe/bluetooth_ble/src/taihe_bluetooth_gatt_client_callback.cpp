/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "taihe_bluetooth_gatt_client_callback.h"

#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_gatt_client.h"

namespace OHOS {
namespace Bluetooth {
TaiheGattClientCallback::TaiheGattClientCallback()
{}

void TaiheGattClientCallback::OnCharacteristicChanged(const GattCharacteristic &characteristic)
{
    HILOGI("UUID: %{public}s", characteristic.GetUuid().ToString().c_str());
}

void TaiheGattClientCallback::OnCharacteristicReadResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
    auto taiheCharacter = std::make_shared<TaiheNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_READ_CHARACTER, taiheCharacter, ret);
}

void TaiheGattClientCallback::OnDescriptorReadResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
    auto taiheDescriptor = std::make_shared<TaiheNativeBleDescriptor>(descriptor);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_READ_DESCRIPTOR, taiheDescriptor, ret);
}

void TaiheGattClientCallback::OnConnectionStateChanged(int connectionState, int ret)
{
    HILOGI("connectionState:%{public}d, ret:%{public}d", connectionState, ret);
}

void TaiheGattClientCallback::OnServicesDiscovered(int status)
{
    HILOGI("status:%{public}d", status);
}

void TaiheGattClientCallback::OnReadRemoteRssiValueResult(int rssi, int ret)
{
    HILOGI("rssi: %{public}d, ret: %{public}d", rssi, ret);
}

void TaiheGattClientCallback::OnCharacteristicWriteResult(const GattCharacteristic &characteristic, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", characteristic.GetUuid().ToString().c_str(), ret);
    ret = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(ret)); // Adaptation for old sdk
    auto taiheCharacter = std::make_shared<TaiheNativeBleCharacteristic>(characteristic);
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_WRITE_CHARACTER, taiheCharacter, ret);
}

void TaiheGattClientCallback::OnDescriptorWriteResult(const GattDescriptor &descriptor, int ret)
{
    HILOGI("UUID: %{public}s, ret: %{public}d", descriptor.GetUuid().ToString().c_str(), ret);
}

void TaiheGattClientCallback::OnSetNotifyCharacteristic(const GattCharacteristic &characteristic, int status)
{
    HILOGI("UUID: %{public}s, status: %{public}d", characteristic.GetUuid().ToString().c_str(), status);
    status = GetSDKAdaptedStatusCode(GattClientDeviceImpl::GattStatusFromService(status)); // Adaptation for old sdk
    AsyncWorkCallFunction(asyncWorkMap_, TaiheAsyncType::GATT_CLIENT_ENABLE_CHARACTER_CHANGED, nullptr, status);
}

void TaiheGattClientCallback::OnMtuUpdate(int mtu, int ret)
{
    HILOGI("ret: %{public}d, mtu: %{public}d", ret, mtu);
}

} // namespace Bluetooth
} // namespace OHOS