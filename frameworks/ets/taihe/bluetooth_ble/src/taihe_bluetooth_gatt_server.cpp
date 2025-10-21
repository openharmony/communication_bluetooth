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

#ifndef LOG_TAG
#define LOG_TAG "bt_taihe_gatt_server"
#endif

#include "taihe_bluetooth_gatt_server.h"

#include "bluetooth_utils.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_parser_utils.h"

namespace OHOS {
namespace Bluetooth {

std::vector<std::string> GattServerImpl::deviceList_;
std::mutex GattServerImpl::deviceListMutex_;

void GattServerImpl::AddService(ohos::bluetooth::ble::GattService service)
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(server_ != nullptr, BT_ERR_INVALID_PARAM);
    std::unique_ptr<GattService> gattService {nullptr};
    GattServiceType type = service.isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY;
    UUID serviceUuid = UUID::FromString(std::string(service.serviceUuid));
    gattService = std::make_unique<GattService>(serviceUuid, type);

    TaiheGattService taiheGattService;
    TAIHE_BT_ASSERT_RETURN_VOID(!TaiheParseGattService(service, taiheGattService), BT_ERR_INTERNAL_ERROR);

    for (auto &characteristic : taiheGattService.characteristics) {
        int charPermissions = characteristic.permissions;
        int charProperties = characteristic.properties;
        GattCharacteristic character(characteristic.characteristicUuid, charPermissions, charProperties);
        character.SetValue(characteristic.characteristicValue.data(), characteristic.characteristicValue.size());

        for (const auto &descriptor : characteristic.descriptors) {
            GattDescriptor gattDescriptor(descriptor.descriptorUuid, descriptor.permissions);
            gattDescriptor.SetValue(descriptor.descriptorValue.data(), descriptor.descriptorValue.size());
            character.AddDescriptor(gattDescriptor);
        }
        gattService->AddCharacteristic(character);
    }
    int ret = server_->AddService(*gattService);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void GattServerImpl::Close()
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(server_ != nullptr, BT_ERR_INTERNAL_ERROR);
    int ret = server_->Close();
    HILOGI("ret: %{public}d", ret);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

void GattServerImpl::RemoveService(taihe::string_view serviceUuid)
{
    HILOGI("enter");
    UUID uuid;
    auto status = ParseUuidParams(std::string(serviceUuid), uuid);
    TAIHE_BT_ASSERT_RETURN_VOID((status == taihe_ok && server_ != nullptr), BT_ERR_INVALID_PARAM);

    int ret = BT_NO_ERROR;
    auto primaryService = server_->GetService(uuid, true);
    if (primaryService.has_value()) {
        ret = server_->RemoveGattService(primaryService.value());
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    auto secondService = server_->GetService(uuid, false);
    if (secondService.has_value()) {
        ret = server_->RemoveGattService(secondService.value());
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }
    TAIHE_BT_ASSERT_RETURN_VOID((primaryService.has_value() || secondService.has_value()),
        BT_ERR_INVALID_PARAM);
}

void GattServerImpl::SendResponse(ohos::bluetooth::ble::ServerResponse serverResponse)
{
    HILOGI("enter");
    TAIHE_BT_ASSERT_RETURN_VOID(server_ != nullptr, BT_ERR_INVALID_PARAM);
    BluetoothRemoteDevice remoteDevice(std::string(serverResponse.deviceId), BTTransport::ADAPTER_BLE);
    HILOGI("Remote device address: %{public}s", GET_ENCRYPT_ADDR(remoteDevice));
    int32_t transId = serverResponse.transId;
    int32_t status = serverResponse.status;
    int32_t offset = serverResponse.offset;
    int ret = server_->SendResponse(remoteDevice, transId, status, offset,
        serverResponse.value.data(), serverResponse.value.size());
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
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

static taihe_status CheckNotifyCharacteristicChangedEx(
    ::taihe::string_view id, ::ohos::bluetooth::ble::NotifyCharacteristic const& info,
    std::string &outDeviceId, TaiheNotifyCharacteristic &outCharacter)
{
    std::string deviceId(id.c_str());
    TaiheNotifyCharacteristic character;
    TAIHE_BT_RETURN_IF(!IsValidAddress(deviceId), "Invalid bdaddr", taihe_invalid_arg);
    TAIHE_BT_CALL_RETURN(TaiheParseNotifyCharacteristic(info, character));

    outDeviceId = std::move(deviceId);
    outCharacter = std::move(character);
    return taihe_ok;
}

static TaihePromiseAndCallback TaiheNotifyCharacteristicChanged(::taihe::string_view deviceId,
    ::ohos::bluetooth::ble::NotifyCharacteristic const& notifyCharacteristic, uintptr_t cb,
    GattServerImpl* taiheServer, bool asPromise = true)
{
    HILOGI("enter");
    std::string devId {};
    TaiheNotifyCharacteristic notifyCharacter;
    auto status = CheckNotifyCharacteristicChangedEx(deviceId, notifyCharacteristic, devId, notifyCharacter);
    if ((status != taihe_ok) && (taiheServer == nullptr) && (taiheServer->GetServer() == nullptr)) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INVALID_PARAM);
    }
    HILOGI("[BTTEST] NotifyCharacteristicChangedAsyncPromise check params status: %{public}d", status);

    auto func = [server = taiheServer->GetServer(), notifyCharacter, devId]() {
        int ret = BT_ERR_INTERNAL_ERROR;
        auto character = GetGattCharacteristic(server, notifyCharacter.serviceUuid, notifyCharacter.characterUuid);
        if (character == nullptr) {
            HILOGI("character is null!");
            return TaiheAsyncWorkRet(ret);
        }
        character->SetValue(notifyCharacter.characterValue.data(), notifyCharacter.characterValue.size());
        BluetoothRemoteDevice remoteDevice(devId, BTTransport::ADAPTER_BLE);
        ret = server->NotifyCharacteristicChanged(remoteDevice, *character, notifyCharacter.confirm);
        return TaiheAsyncWorkRet(ret);
    };
    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(taihe::get_env(), nullptr, func);
    if (asyncWork == nullptr) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }
    bool success = taiheServer->GetCallback()->asyncWorkMap_.TryPush(TaiheAsyncType::GATT_SERVER_NOTIFY_CHARACTERISTIC,
        asyncWork);
    if (!success) {
        return TaihePromiseAndCallback::Failure(BT_ERR_INTERNAL_ERROR);
    }

    asyncWork->Run();

    if (asPromise) {
        return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(asyncWork->GetRet()));
    } else {
        return TaihePromiseAndCallback::Success(reinterpret_cast<uintptr_t>(nullptr));
    }
}

uintptr_t GattServerImpl::NotifyCharacteristicChangedPromise(::taihe::string_view deviceId,
    ::ohos::bluetooth::ble::NotifyCharacteristic const& notifyCharacteristic)
{    TaihePromiseAndCallback result = TaiheNotifyCharacteristicChanged(deviceId, notifyCharacteristic,
            reinterpret_cast<uintptr_t>(nullptr), this, false);
    if (!result.success || !result.handle.has_value()) {
        TAIHE_BT_ASSERT_RETURN(false, result.errorCode, reinterpret_cast<uintptr_t>(nullptr));
    }
    return result.handle.value();
}

void GattServerImpl::NotifyCharacteristicChangedAsync(::taihe::string_view deviceId, ::ohos::bluetooth::ble::NotifyCharacteristic const& notifyCharacteristic, uintptr_t callback)
{
    TaihePromiseAndCallback result = TaiheNotifyCharacteristicChanged(deviceId, notifyCharacteristic,
        reinterpret_cast<uintptr_t>(callback), this, false);
    if (!result.success) {
        TAIHE_BT_ASSERT_RETURN_VOID(false, result.errorCode);
    }
}
} // namespace Bluetooth
} // namespace OHOS
