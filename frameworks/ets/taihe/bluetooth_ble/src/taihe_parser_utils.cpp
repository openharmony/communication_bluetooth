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
#define LOG_TAG "bt_taihe_parser_utils"
#endif

#include "taihe_parser_utils.h"

#include <string>
#include <vector>

namespace OHOS {
namespace Bluetooth {
ani_status AniParseGattService(ohos::bluetooth::ble::GattService object, AniGattService &outService)
{
    std::string uuid {};
    bool isPrimary = true;
    std::vector<AniBleCharacteristic> characteristics {};

    if (!object.serviceUuid.empty()) {
        uuid = object.serviceUuid;
    }

    if (object.isPrimary) {
        isPrimary = object.isPrimary;
    }

    taihe::array<::ohos::bluetooth::ble::BLECharacteristic> objectCharacteristics = object.characteristics;
    if (objectCharacteristics.size() > 0) {
        AniParseGattCharacteristic(objectCharacteristics, characteristics);
    }

    if (object.includeServices.has_value()) {
        taihe::array<::ohos::bluetooth::ble::GattService> objectIncludeServices = object.includeServices.value();
        if (objectIncludeServices.size() > 0) {
            std::vector<AniGattService> includeServices {};
            AniParseIncludeService(objectIncludeServices, includeServices);
            outService.includeServices = std::move(includeServices);
        }
    }

    outService.serviceUuid = UUID::FromString(uuid);
    outService.isPrimary = isPrimary;
    outService.characteristics = std::move(characteristics);

    return ani_ok;
}

namespace {
const AniGattPermission DEFAULT_GATT_PERMISSIONS = {
    .readable = true,
    .writeable = true,
};
const AniGattProperties DEFAULT_GATT_PROPERTIES = {
    .write = true,
    .writeNoResponse = true,
    .read = true,
};
uint16_t ConvertGattPermissions(const AniGattPermission &napiPermissions)
{
    uint16_t permissions = 0;
    if (napiPermissions.readable) {
        permissions |= static_cast<uint16_t>(GattPermission::READABLE);
    }
    if (napiPermissions.readEncrypted) {
        permissions |= static_cast<uint16_t>(GattPermission::READ_ENCRYPTED);
    }
    if (napiPermissions.readEncryptedMitm) {
        permissions |= static_cast<uint16_t>(GattPermission::READ_ENCRYPTED_MITM);
    }
    if (napiPermissions.writeable) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITEABLE);
    }
    if (napiPermissions.writeEncrypted) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_ENCRYPTED);
    }
    if (napiPermissions.writeEncryptedMitm) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_ENCRYPTED_MITM);
    }
    if (napiPermissions.writeSigned) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_SIGNED);
    }
    if (napiPermissions.writeSignedMitm) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_SIGNED_MITM);
    }
    return permissions;
}

uint16_t ConvertGattProperties(const AniGattProperties &napiProperties)
{
    uint16_t properties = 0;
    if (napiProperties.read) {
        properties |= static_cast<uint16_t>(GattCharacteristic::READ);
    }
    if (napiProperties.write) {
        properties |= static_cast<uint16_t>(GattCharacteristic::WRITE);
    }
    if (napiProperties.writeNoResponse) {
        properties |= static_cast<uint16_t>(GattCharacteristic::WRITE_WITHOUT_RESPONSE);
    }
    if (napiProperties.notify) {
        properties |= static_cast<uint16_t>(GattCharacteristic::NOTIFY);
    }
    if (napiProperties.indicate) {
        properties |= static_cast<uint16_t>(GattCharacteristic::INDICATE);
    }
    if (napiProperties.broadcast) {
        properties |= static_cast<uint16_t>(GattCharacteristic::BROADCAST);
    }
    if (napiProperties.authenticatedSignedWrite) {
        properties |= static_cast<uint16_t>(GattCharacteristic::AUTHENTICATED_SIGNED_WRITES);
    }
    if (napiProperties.extendedProperties) {
        properties |= static_cast<uint16_t>(GattCharacteristic::EXTENDED_PROPERTIES);
    }
    return properties;
}
}  // namespace {}

ani_status AniParseIncludeService(taihe::array<::ohos::bluetooth::ble::GattService> object,
                                  std::vector<AniGattService> &outIncludeService)
{
    for (size_t i = 0; i < object.size(); i++) {
        ohos::bluetooth::ble::GattService element = object[i];
        AniGattService includeService {};
        AniParseGattService(element, includeService);
        outIncludeService.push_back(std::move(includeService));
    }
    return ani_ok;
}

ani_status AniParseGattCharacteristic(taihe::array<::ohos::bluetooth::ble::BLECharacteristic> object,
                                      std::vector<AniBleCharacteristic> &outCharacteristic)
{
    for (size_t i = 0; i < object.size(); i++) {
        ohos::bluetooth::ble::BLECharacteristic element = object[i];
        AniBleCharacteristic characteristic {};
        std::string serviceUuid {};
        std::string characterUuid {};
        std::vector<uint8_t> characterValue {};
        std::vector<AniBleDescriptor> descriptors {};
        uint32_t characteristicValueHandle = 0;
        AniGattProperties properties = DEFAULT_GATT_PROPERTIES;
        AniGattPermission permissions = DEFAULT_GATT_PERMISSIONS;
        if (!element.serviceUuid.empty()) {
            serviceUuid = element.serviceUuid;
        }
        if (!element.characteristicUuid.empty()) {
            characterUuid = element.characteristicUuid;
        }
        if (element.characteristicValue.size() > 0) {
            ParseArrayBufferParams(element.characteristicValue, characterValue);
        }
        if (element.descriptors.size() > 0) {
            taihe::array<::ohos::bluetooth::ble::BLEDescriptor> objectDescriptors = element.descriptors;
            AniParseGattDescriptor(objectDescriptors, descriptors);
        }
        if (element.properties.has_value()) {
            ohos::bluetooth::ble::GattProperties objectProperties = element.properties.value();
            AniParseObjectGattProperties(objectProperties, properties);
        }
        if (element.characteristicValueHandle.has_value()) {
            characteristicValueHandle = element.characteristicValueHandle.value();
        }
        if (element.permissions.has_value()) {
            ohos::bluetooth::ble::GattPermissions objectPermissions = element.permissions.value();
            AniParseObjectGattPermissions(objectPermissions, permissions);
        }

        characteristic.serviceUuid = UUID::FromString(serviceUuid);
        characteristic.characteristicUuid = UUID::FromString(characterUuid);
        characteristic.characteristicValue = std::move(characterValue);
        characteristic.descriptors = std::move(descriptors);
        characteristic.properties = ConvertGattProperties(properties);
        characteristic.characteristicValueHandle = static_cast<uint16_t>(characteristicValueHandle);
        characteristic.permissions = ConvertGattPermissions(permissions);

        outCharacteristic.push_back(std::move(characteristic));
    }
    return ani_ok;
}

ani_status AniParseGattDescriptor(taihe::array<::ohos::bluetooth::ble::BLEDescriptor> object,
                                  std::vector<AniBleDescriptor> &outDescriptor)
{
    for (size_t i = 0; i < object.size(); i++) {
        ohos::bluetooth::ble::BLEDescriptor element = object[i];
        AniBleDescriptor descriptor {};
        std::string serviceUuid {};
        std::string characterUuid {};
        std::string descriptorUuid {};
        std::vector<uint8_t> descriptorValue {};
        uint32_t descriptorHandle = 0;
        AniGattPermission permissions = DEFAULT_GATT_PERMISSIONS;

        if (!element.serviceUuid.empty()) {
            serviceUuid = element.serviceUuid;
        }
        if (!element.characteristicUuid.empty()) {
            characterUuid = element.characteristicUuid;
        }
        if (!element.descriptorUuid.empty()) {
            descriptorUuid = element.descriptorUuid;
        }
        if (element.descriptorValue.size() > 0) {
            ParseArrayBufferParams(element.descriptorValue, descriptorValue);
        }
        if (element.descriptorHandle.has_value()) {
            descriptorHandle = element.descriptorHandle.value();
        }
        if (element.permissions.has_value()) {
            ohos::bluetooth::ble::GattPermissions objectPermissions = element.permissions.value();
            AniParseObjectGattPermissions(objectPermissions, permissions);
        }

        descriptor.serviceUuid = UUID::FromString(serviceUuid);
        descriptor.characteristicUuid = UUID::FromString(characterUuid);
        descriptor.descriptorUuid = UUID::FromString(descriptorUuid);
        descriptor.descriptorValue = std::move(descriptorValue);
        descriptor.descriptorHandle = static_cast<uint16_t>(descriptorHandle);
        descriptor.permissions = ConvertGattPermissions(permissions);

        outDescriptor.push_back(std::move(descriptor));
    }
    return ani_ok;
}

ani_status AniParseObjectGattPermissions(ohos::bluetooth::ble::GattPermissions object,
                                         AniGattPermission &outPermissions)
{
    AniGattPermission permissions {};
    if (object.read.has_value()) {
        permissions.readable = object.read.value();
    }
    if (object.readEncrypted.has_value()) {
        permissions.readEncrypted = object.readEncrypted.value();
    }
    if (object.readEncryptedMitm.has_value()) {
        permissions.readEncryptedMitm = object.readEncryptedMitm.value();
    }
    if (object.write.has_value()) {
        permissions.writeable = object.write.value();
    }
    if (object.writeEncrypted.has_value()) {
        permissions.writeEncrypted = object.writeEncrypted.value();
    }
    if (object.writeEncryptedMitm.has_value()) {
        permissions.writeEncryptedMitm = object.writeEncryptedMitm.value();
    }
    if (object.writeSigned.has_value()) {
        permissions.writeSigned = object.writeSigned.value();
    }
    if (object.writeSignedMitm.has_value()) {
        permissions.writeSignedMitm = object.writeSignedMitm.value();
    }
    outPermissions = permissions;
    return ani_ok;
}

ani_status AniParseObjectGattProperties(ohos::bluetooth::ble::GattProperties object,
                                        AniGattProperties &outProperties)
{
    AniGattProperties properties {};
    if (object.write.has_value()) {
        properties.write = object.write.value();
    }
    if (object.writeNoResponse.has_value()) {
        properties.writeNoResponse = object.writeNoResponse.value();
    }
    if (object.read.has_value()) {
        properties.read = object.read.value();
    }
    if (object.notify.has_value()) {
        properties.notify = object.notify.value();
    }
    if (object.indicate.has_value()) {
        properties.indicate = object.indicate.value();
    }
    outProperties = properties;
    return ani_ok;
}
}  // namespace Bluetooth
}  // namespace OHOS
