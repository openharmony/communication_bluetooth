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
#define LOG_TAG "bt_ipc_gatt_client_callback_stub"
#endif

#include "bluetooth_gatt_client_callback_stub.h"
#include "bluetooth_log.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
const int32_t GATT_CLIENT_CALLBACK_READ_DATA_SIZE_MAX_LEN = 0x100;
BluetoothGattClientCallbackStub::BluetoothGattClientCallbackStub()
{
    HILOGD("start.");
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_CONNECT_STATE_CHANGE)] =
        BluetoothGattClientCallbackStub::OnConnectionStateChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_CHARACTER_CHANGE)] =
        BluetoothGattClientCallbackStub::OnCharacteristicChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_CHARACTER_READ)] =
        BluetoothGattClientCallbackStub::OnCharacteristicReadInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_CHARACTER_WRITE)] =
        BluetoothGattClientCallbackStub::OnCharacteristicWriteInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_DESCRIPTOR_READ)] =
        BluetoothGattClientCallbackStub::OnDescriptorReadInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_DESCRIPTOR_WRITE)] =
        BluetoothGattClientCallbackStub::OnDescriptorWriteInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_MTU_UPDATE)] =
        BluetoothGattClientCallbackStub::OnMtuChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_SERVICES_DISCOVER)] =
        BluetoothGattClientCallbackStub::OnServicesDiscoveredInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_CONNECTION_PARA_CHANGE)] =
        BluetoothGattClientCallbackStub::OnConnectionParameterChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_SERVICES_CHANGED)] =
        BluetoothGattClientCallbackStub::OnServicesChangedInner;
    memberFuncMap_[static_cast<uint32_t>(
        BluetoothGattClientCallbackInterfaceCode::BT_GATT_CLIENT_CALLBACK_READ_REMOTE_RSSI_VALUE)] =
        BluetoothGattClientCallbackStub::OnReadRemoteRssiValueInner;
}

BluetoothGattClientCallbackStub::~BluetoothGattClientCallbackStub()
{
    HILOGD("start.");
    memberFuncMap_.clear();
}

int BluetoothGattClientCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGD("BluetoothGattClientCallbackStub::OnRemoteRequest, cmd = %{public}d, flags= %{public}d",
        code, option.GetFlags());
    if (BluetoothGattClientCallbackStub::GetDescriptor() != data.ReadInterfaceToken()) {
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
    HILOGW("BluetoothGattClientCallbackStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnConnectionStateChangedInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGD("BluetoothGattClientCallbackStub::OnConnectionStateChangedInner Triggered!");
    int32_t state = data.ReadInt32();
    int32_t newState = data.ReadInt32();
    stub->OnConnectionStateChanged(state, newState);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnCharacteristicChangedInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    stub->OnCharacteristicChanged(*characteristic);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnCharacteristicReadInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothGattClientCallbackStub::OnCharacteristicReadInner Triggered!");
    int32_t ret = data.ReadInt32();
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    stub->OnCharacteristicRead(ret, *characteristic);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnCharacteristicWriteInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothGattClientCallbackStub::OnCharacteristicWriteInner Triggered!");
    int32_t ret = data.ReadInt32();
    std::shared_ptr<BluetoothGattCharacteristic> characteristic(data.ReadParcelable<BluetoothGattCharacteristic>());
    if (!characteristic) {
        return TRANSACTION_ERR;
    }
    stub->OnCharacteristicWrite(ret, *characteristic);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnDescriptorReadInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothGattClientCallbackStub::OnDescriptorReadInner Triggered!");
    int32_t ret = data.ReadInt32();
    std::shared_ptr<BluetoothGattDescriptor> descriptor(data.ReadParcelable<BluetoothGattDescriptor>());
    if (!descriptor) {
        return TRANSACTION_ERR;
    }
    stub->OnDescriptorRead(ret, *descriptor);

    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnDescriptorWriteInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGD("BluetoothGattClientCallbackStub::OnDescriptorWriteInner Triggered!");
    int32_t ret = data.ReadInt32();
    std::shared_ptr<BluetoothGattDescriptor> descriptor(data.ReadParcelable<BluetoothGattDescriptor>());
    if (!descriptor) {
        return TRANSACTION_ERR;
    }
    stub->OnDescriptorWrite(ret, *descriptor);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnMtuChangedInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothGattClientCallbackStub::OnMtuChangedInner Triggered!");
    int32_t state = data.ReadInt32();
    int32_t mtu = data.ReadInt32();
    stub->OnMtuChanged(state, mtu);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnServicesDiscoveredInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothGattClientCallbackStub::OnServicesDiscoveredInner Triggered!");
    int32_t status = data.ReadInt32();
    stub->OnServicesDiscovered(status);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnConnectionParameterChangedInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGD("BluetoothGattClientCallbackStub::OnConnectionParameterChangedInner Triggered!");
    int32_t interval = data.ReadInt32();
    int32_t latency = data.ReadInt32();
    int32_t timeout = data.ReadInt32();
    int32_t status = data.ReadInt32();
    stub->OnConnectionParameterChanged(interval, latency, timeout, status);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnServicesChangedInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothGattClientCallbackStub::OnServicesChangedInner Triggered!");
    int32_t num = 0;
    if (!data.ReadInt32(num) || num > GATT_CLIENT_CALLBACK_READ_DATA_SIZE_MAX_LEN) {
        HILOGE("read Parcelable size failed.");
        return TRANSACTION_ERR;
    }
    std::vector<BluetoothGattService> service;
    for (int i = num; i > 0; i--) {
        std::shared_ptr<BluetoothGattService> dev(data.ReadParcelable<BluetoothGattService>());
        if (!dev) {
            return TRANSACTION_ERR;
        }
        service.push_back(*dev);
    }
    stub->OnServicesChanged(service);
    return NO_ERROR;
}

__attribute__((no_sanitize("cfi")))
ErrCode BluetoothGattClientCallbackStub::OnReadRemoteRssiValueInner(
    BluetoothGattClientCallbackStub *stub, MessageParcel &data, MessageParcel &reply)
{
    HILOGI("BluetoothGattClientCallbackStub::OnReadRemoteRssiValueInner Triggered!");
    bluetooth::RawAddress address(data.ReadString());
    int32_t rssi = data.ReadInt32();
    int32_t state = data.ReadInt32();
    stub->OnReadRemoteRssiValue(address, rssi, state);
    return NO_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS
