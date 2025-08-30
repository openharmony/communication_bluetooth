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

namespace OHOS {
namespace Bluetooth {

std::vector<std::string> TaiheGattServer::deviceList_;
std::mutex TaiheGattServer::deviceListMutex_;

void TaiheGattServer::AddService(ohos::bluetooth::ble::GattService service)
{
    ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR, "AddService ani assert failed");
    std::unique_ptr<GattService> gattService {nullptr};
    GattServiceType type = service.isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY;
    UUID serviceUuid = UUID::FromString(std::string(service.serviceUuid));
    gattService = std::make_unique<GattService>(serviceUuid, type);

    for (auto &characteristic : service.characteristics) {
        int charPermissions = 0;
        int charProperties = 0;
        UUID characteristicUuid = UUID::FromString(std::string(characteristic.serviceUuid));
        GattCharacteristic character(characteristicUuid, charPermissions, charProperties);
        character.SetValue(characteristic.characteristicValue.data(), characteristic.characteristicValue.size());

        for (const auto &descriptor : characteristic.descriptors) {
            UUID descriptorUuid = UUID::FromString(std::string(descriptor.descriptorUuid));
            GattDescriptor gattDescriptor(descriptorUuid, 0);
            gattDescriptor.SetValue(descriptor.descriptorValue.data(), descriptor.descriptorValue.size());
            character.AddDescriptor(gattDescriptor);
        }
        gattService->AddCharacteristic(character);
    }
    int ret = server_->AddService(*gattService);
    ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "AddService return error");
}

void TaiheGattServer::Close()
{
    ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR, "Close ani assert failed");
    int ret = server_->Close();
    HILOGI("ret: %{public}d", ret);

    ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Close return error");
}

void TaiheGattServer::RemoveService(string_view serviceUuid)
{
    HILOGI("[BTTEST] RemoveService enter");
    ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
        "RemoveService ani assert failed");
    UUID uuid = UUID::FromString(std::string(serviceUuid));

    int ret = BT_NO_ERROR;
    auto primaryService = server_->GetService(uuid, true);
    if (primaryService.has_value()) {
        ret = server_->RemoveGattService(primaryService.value());
        ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "Primary RemoveService return error");
    }

    auto secondService = server_->GetService(uuid, false);
    if (secondService.has_value()) {
        ret = server_->RemoveGattService(secondService.value());
        ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "Second RemoveService return error");
    }
    HILOGI("[BTTEST] RemoveService leave");
}

void TaiheGattServer::SendResponse(ServerResponse serverResponse)
{
    HILOGI("[BTTEST] SendResponse enter");
    ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
        "SendResponse ani assert failed");
    BluetoothRemoteDevice remoteDevice(std::string(serverResponse.deviceId), BTTransport::ADAPTER_BLE);
    int32_t transId = serverResponse.transId;
    int32_t status = serverResponse.status;
    int32_t offset = serverResponse.offset;
    int ret = server_->SendResponse(remoteDevice, transId, status, offset,
        serverResponse.value.data(), serverResponse.value.size());
    ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "SendResponse return error");
}
} // namespace Bluetooth
} // namespace OHOS
