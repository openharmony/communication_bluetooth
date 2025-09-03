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
#include "taihe_bluetooth_error.h"
#include "bluetooth_utils.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_parser_utils.h"

namespace OHOS {
namespace Bluetooth {

std::vector<std::string> GattServerImpl::deviceList_;
std::mutex GattServerImpl::deviceListMutex_;

void GattServerImpl::AddService(ohos::bluetooth::ble::GattService service)
{
    ANI_BT_ASSERT(server_ != nullptr, BT_ERR_INVALID_PARAM, "AddService ani assert failed");
    std::unique_ptr<GattService> gattService {nullptr};
    GattServiceType type = service.isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY;
    UUID serviceUuid = UUID::FromString(std::string(service.serviceUuid));
    gattService = std::make_unique<GattService>(serviceUuid, type);

    TaiheGattService aniGattService;
    ANI_BT_ASSERT(TaiheParseGattService(service, aniGattService) == ani_ok, BT_ERR_INTERNAL_ERROR,
                  "AniParseGattService ani parase failed");

    for (auto &characteristic : aniGattService.characteristics) {
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
    ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "AddService return error");
}

void GattServerImpl::Close()
{
    ANI_BT_ASSERT(server_ != nullptr, BT_ERR_INVALID_PARAM, "Close ani assert failed");
    int ret = server_->Close();
    HILOGI("ret: %{public}d", ret);

    ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "Close return error");
}

void GattServerImpl::RemoveService(string_view serviceUuid)
{
    UUID uuid;
    auto status = ParseUuidParams(std::string(serviceUuid), uuid);
    ANI_BT_ASSERT((status && server_ != nullptr), BT_ERR_INVALID_PARAM, "RemoveService ani assert failed");

    int ret = BT_NO_ERROR;
    auto primaryService = server_->GetService(uuid, true);
    if (primaryService.has_value()) {
        ret = server_->RemoveGattService(primaryService.value());
        ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "Primary RemoveService return error");
    }

    auto secondService = server_->GetService(uuid, false);
    if (secondService.has_value()) {
        ret = server_->RemoveGattService(secondService.value());
        ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "Second RemoveService return error");
    }
    ANI_BT_ASSERT((primaryService.has_value() || secondService.has_value()),
        BT_ERR_INVALID_PARAM, "RemoveGattService return error");
}

void GattServerImpl::SendResponse(ServerResponse serverResponse)
{
    ANI_BT_ASSERT(server_ != nullptr, BT_ERR_INVALID_PARAM, "SendResponse ani assert failed");
    BluetoothRemoteDevice remoteDevice(std::string(serverResponse.deviceId), BTTransport::ADAPTER_BLE);
    int32_t transId = serverResponse.transId;
    int32_t status = serverResponse.status;
    int32_t offset = serverResponse.offset;
    int ret = server_->SendResponse(remoteDevice, transId, status, offset,
        serverResponse.value.data(), serverResponse.value.size());
    ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "SendResponse return error");
}
} // namespace Bluetooth
} // namespace OHOS
