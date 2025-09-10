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
#include "taihe_bluetooth_utils.h"

#include <string>
#include <vector>

namespace OHOS {
namespace Bluetooth {
taihe_status TaiheParseGattService(ohos::bluetooth::ble::GattService object, TaiheGattService &outService)
{
    UUID uuid {};
    bool isPrimary = true;
    std::vector<TaiheBleCharacteristic> characteristics {};

    TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(object.serviceUuid), uuid));

    isPrimary = object.isPrimary;

    taihe::array<::ohos::bluetooth::ble::BLECharacteristic> objectCharacteristics = object.characteristics;
    if (objectCharacteristics.size() > 0) {
        TAIHE_BT_CALL_RETURN(TaiheParseGattCharacteristic(objectCharacteristics, characteristics));
    }

    if (object.includeServices.has_value()) {
        taihe::array<::ohos::bluetooth::ble::GattService> objectIncludeServices = object.includeServices.value();
        if (objectIncludeServices.size() > 0) {
            std::vector<TaiheGattService> includeServices {};
            TAIHE_BT_CALL_RETURN(TaiheParseIncludeService(objectIncludeServices, includeServices));
            outService.includeServices = std::move(includeServices);
        }
    }

    outService.serviceUuid = uuid;
    outService.isPrimary = isPrimary;
    outService.characteristics = std::move(characteristics);

    return taihe_ok;
}

namespace {
const TaiheGattPermission DEFAULT_GATT_PERMISSIONS = {
    .readable = true,
    .writeable = true,
};
const TaiheGattProperties DEFAULT_GATT_PROPERTIES = {
    .write = true,
    .writeNoResponse = true,
    .read = true,
};
uint16_t ConvertGattPermissions(const TaiheGattPermission &taihePermissions)
{
    uint16_t permissions = 0;
    if (taihePermissions.readable) {
        permissions |= static_cast<uint16_t>(GattPermission::READABLE);
    }
    if (taihePermissions.readEncrypted) {
        permissions |= static_cast<uint16_t>(GattPermission::READ_ENCRYPTED);
    }
    if (taihePermissions.readEncryptedMitm) {
        permissions |= static_cast<uint16_t>(GattPermission::READ_ENCRYPTED_MITM);
    }
    if (taihePermissions.writeable) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITEABLE);
    }
    if (taihePermissions.writeEncrypted) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_ENCRYPTED);
    }
    if (taihePermissions.writeEncryptedMitm) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_ENCRYPTED_MITM);
    }
    if (taihePermissions.writeSigned) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_SIGNED);
    }
    if (taihePermissions.writeSignedMitm) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_SIGNED_MITM);
    }
    return permissions;
}

uint16_t ConvertGattProperties(const TaiheGattProperties &taiheProperties)
{
    uint16_t properties = 0;
    if (taiheProperties.read) {
        properties |= static_cast<uint16_t>(GattCharacteristic::READ);
    }
    if (taiheProperties.write) {
        properties |= static_cast<uint16_t>(GattCharacteristic::WRITE);
    }
    if (taiheProperties.writeNoResponse) {
        properties |= static_cast<uint16_t>(GattCharacteristic::WRITE_WITHOUT_RESPONSE);
    }
    if (taiheProperties.notify) {
        properties |= static_cast<uint16_t>(GattCharacteristic::NOTIFY);
    }
    if (taiheProperties.indicate) {
        properties |= static_cast<uint16_t>(GattCharacteristic::INDICATE);
    }
    if (taiheProperties.broadcast) {
        properties |= static_cast<uint16_t>(GattCharacteristic::BROADCAST);
    }
    if (taiheProperties.authenticatedSignedWrite) {
        properties |= static_cast<uint16_t>(GattCharacteristic::AUTHENTICATED_SIGNED_WRITES);
    }
    if (taiheProperties.extendedProperties) {
        properties |= static_cast<uint16_t>(GattCharacteristic::EXTENDED_PROPERTIES);
    }
    return properties;
}
}  // namespace {}

taihe_status TaiheParseIncludeService(taihe::array<::ohos::bluetooth::ble::GattService> object,
                                      std::vector<TaiheGattService> &outIncludeService)
{
    for (size_t i = 0; i < object.size(); i++) {
        ohos::bluetooth::ble::GattService element = object[i];
        TaiheGattService includeService {};
        TAIHE_BT_CALL_RETURN(TaiheParseGattService(element, includeService));
        outIncludeService.push_back(std::move(includeService));
    }
    return taihe_ok;
}

taihe_status TaiheParseGattCharacteristic(taihe::array<::ohos::bluetooth::ble::BLECharacteristic> object,
                                          std::vector<TaiheBleCharacteristic> &outCharacteristic)
{
    for (size_t i = 0; i < object.size(); i++) {
        ohos::bluetooth::ble::BLECharacteristic element = object[i];
        TaiheBleCharacteristic characteristic {};
        UUID serviceUuid {};
        UUID characterUuid {};
        std::vector<uint8_t> characterValue {};
        std::vector<TaiheBleDescriptor> descriptors {};
        uint32_t characteristicValueHandle = 0;
        TaiheGattProperties properties = DEFAULT_GATT_PROPERTIES;
        TaiheGattPermission permissions = DEFAULT_GATT_PERMISSIONS;
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(element.serviceUuid), serviceUuid));
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(element.characteristicUuid), serviceUuid));
        if (element.characteristicValue.size() > 0) {
            ParseArrayBufferParams(element.characteristicValue, characterValue);
        }
        if (element.descriptors.size() > 0) {
            taihe::array<::ohos::bluetooth::ble::BLEDescriptor> objectDescriptors = element.descriptors;
            TAIHE_BT_CALL_RETURN(TaiheParseGattDescriptor(objectDescriptors, descriptors));
        }
        if (element.properties.has_value()) {
            ohos::bluetooth::ble::GattProperties objectProperties = element.properties.value();
            TaiheParseObjectGattProperties(objectProperties, properties);
        }
        if (element.characteristicValueHandle.has_value()) {
            characteristicValueHandle = element.characteristicValueHandle.value();
        }
        if (element.permissions.has_value()) {
            ohos::bluetooth::ble::GattPermissions objectPermissions = element.permissions.value();
            TaiheParseObjectGattPermissions(objectPermissions, permissions);
        }

        characteristic.serviceUuid = serviceUuid;
        characteristic.characteristicUuid = characterUuid;
        characteristic.characteristicValue = std::move(characterValue);
        characteristic.descriptors = std::move(descriptors);
        characteristic.properties = ConvertGattProperties(properties);
        characteristic.characteristicValueHandle = static_cast<uint16_t>(characteristicValueHandle);
        characteristic.permissions = ConvertGattPermissions(permissions);

        outCharacteristic.push_back(std::move(characteristic));
    }
    return taihe_ok;
}

taihe_status TaiheParseGattDescriptor(taihe::array<::ohos::bluetooth::ble::BLEDescriptor> object,
                                      std::vector<TaiheBleDescriptor> &outDescriptor)
{
    for (size_t i = 0; i < object.size(); i++) {
        ohos::bluetooth::ble::BLEDescriptor element = object[i];
        TaiheBleDescriptor descriptor {};
        UUID serviceUuid {};
        UUID characterUuid {};
        UUID descriptorUuid {};
        std::vector<uint8_t> descriptorValue {};
        uint32_t descriptorHandle = 0;
        TaiheGattPermission permissions = DEFAULT_GATT_PERMISSIONS;

        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(element.serviceUuid), serviceUuid));
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(element.characteristicUuid), characterUuid));
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(element.descriptorUuid), descriptorUuid));
        if (element.descriptorValue.size() > 0) {
            ParseArrayBufferParams(element.descriptorValue, descriptorValue);
        }
        if (element.descriptorHandle.has_value()) {
            descriptorHandle = element.descriptorHandle.value();
            TAIHE_BT_RETURN_IF(descriptorHandle > 0xFFFF, "Invalid characteristicValueHandle", taihe_invalid_arg);
        }
        if (element.permissions.has_value()) {
            ohos::bluetooth::ble::GattPermissions objectPermissions = element.permissions.value();
            TaiheParseObjectGattPermissions(objectPermissions, permissions);
        }

        descriptor.serviceUuid = serviceUuid;
        descriptor.characteristicUuid = characterUuid;
        descriptor.descriptorUuid = descriptorUuid;
        descriptor.descriptorValue = std::move(descriptorValue);
        descriptor.descriptorHandle = static_cast<uint16_t>(descriptorHandle);
        descriptor.permissions = ConvertGattPermissions(permissions);

        outDescriptor.push_back(std::move(descriptor));
    }
    return taihe_ok;
}

void TaiheParseObjectGattPermissions(ohos::bluetooth::ble::GattPermissions object,
                                     TaiheGattPermission &outPermissions)
{
    TaiheGattPermission permissions {};
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
}

void TaiheParseObjectGattProperties(ohos::bluetooth::ble::GattProperties object,
                                    TaiheGattProperties &outProperties)
{
    TaiheGattProperties properties {};
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
    if (object.broadcast.has_value()) {
        properties.broadcast = object.broadcast.value();
    }
    if (object.authenticatedSignedWrite.has_value()) {
        properties.authenticatedSignedWrite = object.authenticatedSignedWrite.value();
    }
    if (object.extendedProperties.has_value()) {
        properties.extendedProperties = object.extendedProperties.value();
    }
    outProperties = properties;
}
}  // namespace Bluetooth
}  // namespace OHOS
