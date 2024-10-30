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

#include "bluetooth_ble_gattServer.h"

#include "bluetooth_ble_common.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_service.h"
#include "bluetooth_log.h"
#include "cj_lambda.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BT_ERR_INTERNAL_ERROR;
using Bluetooth::BT_ERR_INVALID_PARAM;
using Bluetooth::BT_NO_ERROR;
using Bluetooth::BTTransport;
using Bluetooth::GattServiceType;
using Bluetooth::UUID;

static int CheckGattsAddService(NativeGattService service, std::unique_ptr<GattService> &outService)
{
    GattServiceType type = service.isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY;
    outService = std::make_unique<GattService>(UUID::FromString(service.serviceUuid), type);

    CArrBLECharacteristic characteristics = service.characteristics;
    for (int64_t i = 0; i < characteristics.size; i++) {
        NativeBLECharacteristic nativeCharacter = characteristics.head[i];

        NativeGattPermission permissions = {
            .readable = true,
            .writeable = true,
        };
        int charPermissions = ConvertGattPermissions(permissions);
        int charProperties = ConvertGattProperties(nativeCharacter.properties);
        GattCharacteristic character(UUID::FromString(nativeCharacter.characteristicUuid), charPermissions,
                                     charProperties);
        character.SetValue(nativeCharacter.characteristicValue.head, nativeCharacter.characteristicValue.size);

        CArrBLEDescriptor descriptors = nativeCharacter.descriptors;
        for (int64_t j = 0; j < descriptors.size; j++) {
            NativeBLEDescriptor nativeDescriptor = descriptors.head[j];
            GattDescriptor descriptor(UUID::FromString(nativeDescriptor.descriptorUuid), 0);
            descriptor.SetValue(nativeDescriptor.descriptorValue.head, nativeDescriptor.descriptorValue.size);
            character.AddDescriptor(descriptor);
        }
        outService->AddCharacteristic(character);
    }
    return BT_NO_ERROR;
}

int32_t FfiGattServer::AddService(NativeGattService service)
{
    std::unique_ptr<GattService> gattService{nullptr};
    auto status = CheckGattsAddService(service, gattService);
    if (status != BT_NO_ERROR) {
        return status;
    }
    if (server_ == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = server_->AddService(*gattService);
    return ret;
}

int32_t FfiGattServer::RemoveService(std::string serviceUuid)
{
    UUID realServiceUuid = UUID::FromString(serviceUuid);
    int ret = BT_ERR_INTERNAL_ERROR;
    if (server_ == nullptr) {
        return ret;
    }
    auto primaryService = server_->GetService(realServiceUuid, true);
    if (primaryService.has_value()) {
        ret = server_->RemoveGattService(*primaryService);
        if (ret != BT_NO_ERROR) {
            return ret;
        }
    }
    auto secondService = server_->GetService(realServiceUuid, false);
    if (secondService.has_value()) {
        ret = server_->RemoveGattService(*secondService);
        if (ret != BT_NO_ERROR) {
            return ret;
        }
    }
    return ret;
}

int32_t FfiGattServer::Close()
{
    if (server_ == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return server_->Close();
}

static GattCharacteristic *GetGattCharacteristic(const std::shared_ptr<GattServer> &server, const UUID &serviceUuid,
                                                 const UUID &characterUuid)
{
    auto service = server->GetService(serviceUuid, true);
    if (!service.has_value()) {
        service = server->GetService(serviceUuid, false);
    }
    if (!service.has_value()) {
        HILOGE("not found service uuid: %{public}s", serviceUuid.ToString().c_str());
        return nullptr;
    }
    GattCharacteristic *character = service.value().get().GetCharacteristic(characterUuid);
    return character;
}

int32_t FfiGattServer::NotifyCharacteristicChanged(std::string deviceId, NativeNotifyCharacteristic characteristic)
{
    if (server_ == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    auto character = GetGattCharacteristic(server_, UUID::FromString(characteristic.serviceUuid),
                                           UUID::FromString(characteristic.characteristicUuid));
    if (character == nullptr) {
        return BT_ERR_INVALID_PARAM;
    }
    character->SetValue(characteristic.characteristicValue.head, characteristic.characteristicValue.size);
    BluetoothRemoteDevice remoteDevice(deviceId, BTTransport::ADAPTER_BLE);
    int ret = server_->NotifyCharacteristicChanged(remoteDevice, *character, characteristic.confirm);
    return ret;
}

int32_t FfiGattServer::SendResponse(NativeServerResponse serverResponse)
{
    BluetoothRemoteDevice remoteDevice(serverResponse.deviceId, BTTransport::ADAPTER_BLE);
    if (server_ == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return server_->SendResponse(remoteDevice, serverResponse.transId, serverResponse.status, serverResponse.offset,
                                 serverResponse.value.head, static_cast<int>(serverResponse.value.size));
    ;
}

int32_t FfiGattServer::CreateCharacteristicReadFunc(void (*callback)())
{
    auto characteristicReadFunc =
        CJLambda::Create(reinterpret_cast<void (*)(NativeCharacteristicReadRequest)>(callback));
    if (!characteristicReadFunc) {
        HILOGD("Register characteristicRead event failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    callback_->RegisterCharacteristicReadFunc(characteristicReadFunc);
    return BT_NO_ERROR;
}

int32_t FfiGattServer::CreateCharacteristicWriteFunc(void (*callback)())
{
    auto characteristicWriteFunc =
        CJLambda::Create(reinterpret_cast<void (*)(NativeCharacteristicWriteRequest)>(callback));
    if (!characteristicWriteFunc) {
        HILOGD("Register characteristicWrite event failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    callback_->RegisterCharacteristicWriteFunc(characteristicWriteFunc);
    return BT_NO_ERROR;
}

int32_t FfiGattServer::CreateDescriptorReadFunc(void (*callback)())
{
    auto descriptorReadFunc = CJLambda::Create(reinterpret_cast<void (*)(NativeDescriptorReadRequest)>(callback));
    if (!descriptorReadFunc) {
        HILOGD("Register descriptorRead event failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    callback_->RegisterDescriptorReadFunc(descriptorReadFunc);
    return BT_NO_ERROR;
}

int32_t FfiGattServer::CreateRegisterDescriptorWriteFunc(void (*callback)())
{
    auto descriptorWriteFunc = CJLambda::Create(reinterpret_cast<void (*)(NativeDescriptorWriteRequest)>(callback));
    if (!descriptorWriteFunc) {
        HILOGD("Register descriptorWrite event failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    callback_->RegisterDescriptorWriteFunc(descriptorWriteFunc);
    return BT_NO_ERROR;
}

int32_t FfiGattServer::CreateConnectionStateChangeFunc(void (*callback)())
{
    auto connectionStateChangeFunc =
        CJLambda::Create(reinterpret_cast<void (*)(NativeBLEConnectionChangeState)>(callback));
    if (!connectionStateChangeFunc) {
        HILOGD("Register connectionStateChange event failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    callback_->RegisterConnectionStateChangeFunc(connectionStateChangeFunc);
    return BT_NO_ERROR;
}

int32_t FfiGattServer::CreateBLEMtuChangeFunc(void (*callback)())
{
    auto bleMtuChangeFunc = CJLambda::Create(reinterpret_cast<void (*)(int32_t)>(callback));
    if (!bleMtuChangeFunc) {
        HILOGD("Register bleMtuChange event failed");
        return BT_ERR_INTERNAL_ERROR;
    }
    callback_->RegisterBLEMtuChangeFunc(bleMtuChangeFunc);
    return BT_NO_ERROR;
}

int32_t FfiGattServer::RegisterBleGattServerObserver(int32_t callbackType, void (*callback)())
{
    if (callbackType == CHARACTERISTIC_READ) {
        return CreateCharacteristicReadFunc(callback);
    }

    if (callbackType == CHARACTERISTIC_WRITE) {
        return CreateCharacteristicWriteFunc(callback);
    }

    if (callbackType == DESCRIPTOR_READ) {
        return CreateDescriptorReadFunc(callback);
    }

    if (callbackType == DESCRIPTOR_WRITE) {
        return CreateRegisterDescriptorWriteFunc(callback);
    }

    if (callbackType == CONNECTION_STATE_CHANGE) {
        return CreateConnectionStateChangeFunc(callback);
    }

    if (callbackType == SERVER_BLE_MTU_CHANGE) {
        return CreateBLEMtuChangeFunc(callback);
    }

    return BT_NO_ERROR;
}

std::vector<std::string> FfiGattServer::deviceList_;
std::mutex FfiGattServer::deviceListMutex_;
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS