/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bluetooth_ble_clientDevice.h"

#include "bluetooth_ble_common.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "cj_lambda.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BluetoothHost;
using Bluetooth::BT_ERR_INTERNAL_ERROR;
using Bluetooth::BT_NO_ERROR;
using Bluetooth::BT_TRANSPORT_BLE;
using Bluetooth::GATT_TRANSPORT_TYPE_LE;
using Bluetooth::UUID;

int32_t FfiClientDevice::Connect()
{
    if (client_ == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return client_->Connect(callback_, false, GATT_TRANSPORT_TYPE_LE);
}

int32_t FfiClientDevice::Disconnect()
{
    if (client_ == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return client_->Disconnect();
}

int32_t FfiClientDevice::Close()
{
    if (client_ == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return client_->Close();
}

std::string FfiClientDevice::GetDeviceName(int32_t *errCode)
{
    std::string deviceName = "";
    std::string deviceAddr = GetGattClientDeviceId();
    *errCode = BluetoothHost::GetDefaultHost().GetRemoteDevice(deviceAddr, BT_TRANSPORT_BLE).GetDeviceName(deviceName);

    HILOGI("err: %{public}d, deviceName: %{public}s", *errCode, deviceName.c_str());
    return deviceName;
}

int32_t FfiClientDevice::GetServices(CArrGattService &service)
{
    if (client_ == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = client_->DiscoverServices();
    if (ret == BT_NO_ERROR) {
        HILOGI("start get services");
        std::vector<GattService> nativeServices = client_->GetService();
        service = Convert2CArrGattService(nativeServices);
    }
    return ret;
}

static GattCharacteristic *GetCharacteristic(const std::shared_ptr<GattClient> &client, const UUID &serviceUuid,
                                             const UUID &characterUuid)
{
    GattCharacteristic *character = nullptr;
    if (client) {
        auto service = client->GetService(serviceUuid);
        if (service.has_value()) {
            character = service->get().GetCharacteristic(characterUuid);
        }
    }
    return character;
}

static GattCharacteristic *GetGattcCharacteristic(const std::shared_ptr<GattClient> &client,
                                                  NativeBLECharacteristic &nativeCharacteristic)
{
    GattCharacteristic *character = GetCharacteristic(client, UUID::FromString(nativeCharacteristic.serviceUuid),
                                                      UUID::FromString(nativeCharacteristic.characteristicUuid));
    if (character) {
        character->SetValue(nativeCharacteristic.characteristicValue.head,
                            nativeCharacteristic.characteristicValue.size);
    }
    return character;
}

int32_t FfiClientDevice::ReadCharacteristicValue(NativeBLECharacteristic &characteristic, void (*callback)())
{
    GattCharacteristic *character = GetGattcCharacteristic(client_, characteristic);

    if (character == nullptr) {
        HILOGE("character is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = BT_ERR_INTERNAL_ERROR;
    if (client_) {
        ret = client_->ReadCharacteristic(*character);
    }
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    auto readCharacteristicFunc = CJLambda::Create(reinterpret_cast<void (*)(RetNativeBLECharacteristic)>(callback));
    callback_->RegisterReadCharacteristicCallback(readCharacteristicFunc);
    return BT_NO_ERROR;
}

static GattDescriptor *GetGattcDescriptor(const std::shared_ptr<GattClient> &client,
                                          const NativeBLEDescriptor &nativeDescriptor)
{
    GattDescriptor *descriptor = nullptr;
    if (client) {
        auto *character = GetCharacteristic(client, UUID::FromString(nativeDescriptor.serviceUuid),
                                            UUID::FromString(nativeDescriptor.characteristicUuid));
        if (character == nullptr) {
            HILOGE("character is nullptr");
            return nullptr;
        }
        descriptor = character->GetDescriptor(UUID::FromString(nativeDescriptor.descriptorUuid));
        if (descriptor) {
            descriptor->SetValue(nativeDescriptor.descriptorValue.head, nativeDescriptor.descriptorValue.size);
        }
    }
    return descriptor;
}

int32_t FfiClientDevice::ReadDescriptorValue(NativeBLEDescriptor &inputDescriptor, void (*callback)())
{
    GattDescriptor *descriptor = GetGattcDescriptor(client_, inputDescriptor);

    if (descriptor == nullptr) {
        HILOGE("descriptor is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = BT_ERR_INTERNAL_ERROR;
    if (client_) {
        ret = client_->ReadDescriptor(*descriptor);
    }
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    auto readDescriptorFunc = CJLambda::Create(reinterpret_cast<void (*)(RetNativeBLEDescriptor)>(callback));
    callback_->RegisterReadDescriptorCallback(readDescriptorFunc);
    return BT_NO_ERROR;
}

int32_t FfiClientDevice::WriteCharacteristicValue(NativeBLECharacteristic characteristic, int32_t writeType,
                                                  void (*callback)())
{
    GattCharacteristic *character = GetGattcCharacteristic(client_, characteristic);
    std::vector<uint8_t> value{};

    if (character == nullptr) {
        HILOGE("character is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    character->SetWriteType(writeType);

    int ret = BT_ERR_INTERNAL_ERROR;
    if (client_) {
        ret = client_->WriteCharacteristic(*character);
    }
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    auto writeCharacteristicFunc = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
    callback_->RegisterWriteCharacteristicCallback(writeCharacteristicFunc);
    return BT_NO_ERROR;
}

int32_t FfiClientDevice::WriteDescriptorValue(NativeBLEDescriptor inputDescriptor, void (*callback)())
{
    GattDescriptor *descriptor = GetGattcDescriptor(client_, inputDescriptor);

    if (descriptor == nullptr) {
        HILOGE("descriptor is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = BT_ERR_INTERNAL_ERROR;
    if (client_) {
        ret = client_->WriteDescriptor(*descriptor);
    }
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    auto writeDescriptorValueFunc = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
    callback_->RegisterWriteDescriptorCallback(writeDescriptorValueFunc);
    return BT_NO_ERROR;
}

int32_t FfiClientDevice::GetRssiValue(void (*callback)())
{
    int ret = BT_ERR_INTERNAL_ERROR;
    if (client_) {
        ret = client_->ReadRemoteRssiValue();
    }
    if (ret != BT_NO_ERROR) {
        return ret;
    }
    auto getRssiValueFunc = CJLambda::Create(reinterpret_cast<void (*)(RetDataI32)>(callback));
    callback_->RegisterGetRemoteRssicallback(getRssiValueFunc);
    return BT_NO_ERROR;
}

int32_t FfiClientDevice::SetBLEMtuSize(int32_t mut)
{
    int ret = client_->RequestBleMtuSize(mut);
    return ret;
}

int32_t FfiClientDevice::SetCharacteristicChangeNotification(NativeBLECharacteristic characteristic, bool enable)
{
    GattCharacteristic *character = GetGattcCharacteristic(client_, characteristic);
    if (character == nullptr) {
        HILOGE("character is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = BT_ERR_INTERNAL_ERROR;
    if (client_) {
        ret = client_->SetNotifyCharacteristic(*character, enable);
    }
    return ret;
}

int32_t FfiClientDevice::SetCharacteristicChangeIndication(NativeBLECharacteristic characteristic, bool enable)
{
    GattCharacteristic *character = GetGattcCharacteristic(client_, characteristic);
    if (character == nullptr) {
        HILOGE("character is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = BT_ERR_INTERNAL_ERROR;
    if (client_) {
        ret = client_->SetIndicateCharacteristic(*character, enable);
    }
    return ret;
}

int32_t FfiClientDevice::RegisterBleGattClientDeviceObserver(int32_t callbackType, void (*callback)())
{
    if (callbackType == BLE_CHARACTERISTIC_CHANGE) {
        auto bleCharacteristicChangeFunc =
            CJLambda::Create(reinterpret_cast<void (*)(NativeBLECharacteristic)>(callback));
        if (!bleCharacteristicChangeFunc) {
            HILOGD("Register bleCharacteristicChange event failed");
            return BT_ERR_INTERNAL_ERROR;
        }
        callback_->RegisterBLECharacteristicChangeFunc(bleCharacteristicChangeFunc);
        return BT_NO_ERROR;
    }

    if (callbackType == BLE_CONNECTION_STATE_CHANGE) {
        auto bleConnectionStateChangeFunc =
            CJLambda::Create(reinterpret_cast<void (*)(NativeBLEConnectionChangeState)>(callback));
        if (!bleConnectionStateChangeFunc) {
            HILOGD("Register bleConnectionStateChange event failed");
            return BT_ERR_INTERNAL_ERROR;
        }
        callback_->RegisterBLEConnectionStateChangeFunc(bleConnectionStateChangeFunc);
        return BT_NO_ERROR;
    }

    if (callbackType == CLIENT_BLE_MTU_CHANGE) {
        auto bleMtuChangeFunc = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
        if (!bleMtuChangeFunc) {
            HILOGD("Register bleMtuChange event failed");
            return BT_ERR_INTERNAL_ERROR;
        }
        callback_->RegisterBLEMtuChangeFunc(bleMtuChangeFunc);
        return BT_NO_ERROR;
    }

    return BT_NO_ERROR;
}
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS