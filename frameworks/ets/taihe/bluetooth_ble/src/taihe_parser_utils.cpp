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

#include "taihe_async_callback.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_parser_utils.h"
#include "bluetooth_log.h"
#include "taihe/platform/ani.hpp"
#include "taihe/optional.hpp"
#include "taihe/array.hpp"

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
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(element.characteristicUuid), characterUuid));
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
            TAIHE_BT_RETURN_IF(characteristicValueHandle > 0xFFFF, "Invalid characteristicValueHandle",
                taihe_invalid_arg);
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
            TAIHE_BT_RETURN_IF(descriptorHandle > 0xFFFF, "Invalid descriptorHandle", taihe_invalid_arg);
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

std::shared_ptr<TaiheAsyncCallback> TaiheParseAsyncCallback(ani_vm *vm, ani_env *env, ani_object info)
{
    // "argc - 1" is AsyncCallback parameter's index
    auto asyncCallback = std::make_shared<TaiheAsyncCallback>();
    asyncCallback->env = env;
    if (TaiheIsFunction(env, info) == taihe_ok) {
        HILOGD("callback mode");
        asyncCallback->callback = std::make_shared<TaiheCallback>(vm, env, info);
    } else {
        HILOGD("promise mode");
        asyncCallback->promise = std::make_shared<TaihePromise>(vm, env);
    }
    return asyncCallback;
}

static ::ohos::bluetooth::ble::ServiceData TaiheParseServiceData(ani_env *env, ani_object ani_obj)
{
    ani_string ani_field_serviceUuid;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.ServiceData", "<get>serviceUuid", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_serviceUuid));
    size_t cpp_field_serviceUuid_len;
    env->String_GetUTF8Size(ani_field_serviceUuid, &cpp_field_serviceUuid_len);
    TString cpp_field_serviceUuid_tstr;
    char* cpp_field_serviceUuid_buf = tstr_initialize(&cpp_field_serviceUuid_tstr, cpp_field_serviceUuid_len + 1);
    env->String_GetUTF8(ani_field_serviceUuid, cpp_field_serviceUuid_buf,
        cpp_field_serviceUuid_len + 1, &cpp_field_serviceUuid_len);
    cpp_field_serviceUuid_buf[cpp_field_serviceUuid_len] = '\0';
    cpp_field_serviceUuid_tstr.length = cpp_field_serviceUuid_len;
    ::taihe::string cpp_field_serviceUuid = ::taihe::string(cpp_field_serviceUuid_tstr);
    ani_arraybuffer ani_field_serviceValue;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.ServiceData", "<get>serviceValue", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_serviceValue));
    char* cpp_field_serviceValue_data = nullptr;
    size_t cpp_field_serviceValue_length = 0;
    env->ArrayBuffer_GetInfo(ani_field_serviceValue, reinterpret_cast<void **>(&cpp_field_serviceValue_data),
        &cpp_field_serviceValue_length);
    ::taihe::array_view<uint8_t> cpp_field_serviceValue(reinterpret_cast<uint8_t*>(cpp_field_serviceValue_data),
        cpp_field_serviceValue_length);
    return ::ohos::bluetooth::ble::ServiceData { std::move(cpp_field_serviceUuid),
                                                 std::move(cpp_field_serviceValue) };
}

static ::ohos::bluetooth::ble::ManufactureData TaiheParseManufactureData(ani_env *env, ani_object ani_obj)
{
    ani_int ani_field_manufactureId;
    env->Object_CallMethod_Int(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.ManufactureData", "<get>manufactureId", nullptr),
        reinterpret_cast<ani_int*>(&ani_field_manufactureId));
    int cpp_field_manufactureId = (int)ani_field_manufactureId;
    ani_arraybuffer ani_field_manufactureValue;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.ManufactureData", "<get>manufactureValue", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_manufactureValue));
    char* cpp_field_manufactureValue_data = nullptr;
    size_t cpp_field_manufactureValue_length = 0;
    env->ArrayBuffer_GetInfo(ani_field_manufactureValue, reinterpret_cast<void **>(&cpp_field_manufactureValue_data),
        &cpp_field_manufactureValue_length);
    ::taihe::array_view<uint8_t> cpp_field_manufactureValue(reinterpret_cast<uint8_t*>(cpp_field_manufactureValue_data),
        cpp_field_manufactureValue_length);
    return ::ohos::bluetooth::ble::ManufactureData { std::move(cpp_field_manufactureId),
                                                     std::move(cpp_field_manufactureValue) };
}

static ::ohos::bluetooth::ble::AdvertiseData TaiheParseAdvertisingData(ani_env *env, ani_object ani_obj)
{
    ani_array ani_field_serviceUuids;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>serviceUuids", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_serviceUuids));
    size_t cpp_field_serviceUuids_size;
    env->Array_GetLength(ani_field_serviceUuids, &cpp_field_serviceUuids_size);
    ::taihe::string* cpp_field_serviceUuids_buffer =
        reinterpret_cast<::taihe::string*>(malloc(cpp_field_serviceUuids_size * sizeof(::taihe::string)));
    for (size_t cpp_field_serviceUuids_buffer_i = 0;
         cpp_field_serviceUuids_buffer_i < cpp_field_serviceUuids_size;
         cpp_field_serviceUuids_buffer_i++) {
        ani_object cpp_field_serviceUuids_buffer_ani_item;
        env->Array_Get(ani_field_serviceUuids, cpp_field_serviceUuids_buffer_i,
            reinterpret_cast<ani_ref*>(&cpp_field_serviceUuids_buffer_ani_item));
        ani_size cpp_field_serviceUuids_buffer_cpp_item_len;
        env->String_GetUTF8Size(static_cast<ani_string>(cpp_field_serviceUuids_buffer_ani_item),
            &cpp_field_serviceUuids_buffer_cpp_item_len);
        TString cpp_field_serviceUuids_buffer_cpp_item_tstr;
        char *cpp_field_serviceUuids_buffer_cpp_item_buf = tstr_initialize(&cpp_field_serviceUuids_buffer_cpp_item_tstr,
            cpp_field_serviceUuids_buffer_cpp_item_len + 1);
        env->String_GetUTF8(static_cast<ani_string>(cpp_field_serviceUuids_buffer_ani_item),
            cpp_field_serviceUuids_buffer_cpp_item_buf,
            cpp_field_serviceUuids_buffer_cpp_item_len + 1,
            &cpp_field_serviceUuids_buffer_cpp_item_len);
        cpp_field_serviceUuids_buffer_cpp_item_buf[cpp_field_serviceUuids_buffer_cpp_item_len] = '\0';
        cpp_field_serviceUuids_buffer_cpp_item_tstr.length = cpp_field_serviceUuids_buffer_cpp_item_len;
        ::taihe::string cpp_field_serviceUuids_buffer_cpp_item =
            ::taihe::string(cpp_field_serviceUuids_buffer_cpp_item_tstr);
        new (&cpp_field_serviceUuids_buffer[cpp_field_serviceUuids_buffer_i]) ::taihe::string(std::move(cpp_field_serviceUuids_buffer_cpp_item));
    }
    ::taihe::array<::taihe::string> cpp_field_serviceUuids(cpp_field_serviceUuids_buffer, cpp_field_serviceUuids_size);
    ani_array ani_field_manufactureData;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>manufactureData", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_manufactureData));
    size_t cpp_field_manufactureData_size;
    env->Array_GetLength(ani_field_manufactureData, &cpp_field_manufactureData_size);
    ::ohos::bluetooth::ble::ManufactureData* cpp_field_manufactureData_buffer =
        reinterpret_cast<::ohos::bluetooth::ble::ManufactureData*>(malloc(
            cpp_field_manufactureData_size * sizeof(::ohos::bluetooth::ble::ManufactureData)));
    for (size_t cpp_field_manufactureData_buffer_i = 0;
         cpp_field_manufactureData_buffer_i < cpp_field_manufactureData_size;
         cpp_field_manufactureData_buffer_i++) {
        ani_object cpp_field_manufactureData_buffer_ani_item;
        env->Array_Get(ani_field_manufactureData, cpp_field_manufactureData_buffer_i,
            reinterpret_cast<ani_ref*>(&cpp_field_manufactureData_buffer_ani_item));
        ::ohos::bluetooth::ble::ManufactureData cpp_field_manufactureData_buffer_cpp_item =
            TaiheParseManufactureData(env, static_cast<ani_object>(cpp_field_manufactureData_buffer_ani_item));
        new (&cpp_field_manufactureData_buffer[cpp_field_manufactureData_buffer_i]) ::ohos::bluetooth::ble::ManufactureData(std::move(cpp_field_manufactureData_buffer_cpp_item));
    }
    ::taihe::array<::ohos::bluetooth::ble::ManufactureData> cpp_field_manufactureData(cpp_field_manufactureData_buffer,
        cpp_field_manufactureData_size);
    ani_array ani_field_serviceData;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>serviceData", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_serviceData));
    size_t cpp_field_serviceData_size;
    env->Array_GetLength(ani_field_serviceData, &cpp_field_serviceData_size);
    ::ohos::bluetooth::ble::ServiceData* cpp_field_serviceData_buffer =
        reinterpret_cast<::ohos::bluetooth::ble::ServiceData*>(malloc(
            cpp_field_serviceData_size * sizeof(::ohos::bluetooth::ble::ServiceData)));
    for (size_t cpp_field_serviceData_buffer_i = 0;
         cpp_field_serviceData_buffer_i < cpp_field_serviceData_size;
         cpp_field_serviceData_buffer_i++) {
        ani_object cpp_field_serviceData_buffer_ani_item;
        env->Array_Get(ani_field_serviceData, cpp_field_serviceData_buffer_i,
            reinterpret_cast<ani_ref*>(&cpp_field_serviceData_buffer_ani_item));
        ::ohos::bluetooth::ble::ServiceData cpp_field_serviceData_buffer_cpp_item =
            TaiheParseServiceData(env, static_cast<ani_object>(cpp_field_serviceData_buffer_ani_item));
        new (&cpp_field_serviceData_buffer[cpp_field_serviceData_buffer_i]) ::ohos::bluetooth::ble::ServiceData(std::move(cpp_field_serviceData_buffer_cpp_item));
    }
    ::taihe::array<::ohos::bluetooth::ble::ServiceData> cpp_field_serviceData(cpp_field_serviceData_buffer,
        cpp_field_serviceData_size);
    ani_ref ani_field_includeDeviceName;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>includeDeviceName", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_includeDeviceName));
    ani_boolean cpp_field_includeDeviceName_flag;
    bool* cpp_field_includeDeviceName_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_includeDeviceName, &cpp_field_includeDeviceName_flag);
    if (!cpp_field_includeDeviceName_flag) {
        ani_boolean cpp_field_includeDeviceName_spec_ani;
        env->Object_CallMethod_Boolean((ani_object)ani_field_includeDeviceName,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "unboxed", ":z"),
            &cpp_field_includeDeviceName_spec_ani);
        bool cpp_field_includeDeviceName_spec = (bool)cpp_field_includeDeviceName_spec_ani;
        cpp_field_includeDeviceName_ptr = new bool(std::move(cpp_field_includeDeviceName_spec));
    }
    ::taihe::optional<bool> cpp_field_includeDeviceName(cpp_field_includeDeviceName_ptr);
    ani_ref ani_field_includeTxPower;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseData", "<get>includeTxPower", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_includeTxPower));
    ani_boolean cpp_field_includeTxPower_flag;
    bool* cpp_field_includeTxPower_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_includeTxPower, &cpp_field_includeTxPower_flag);
    if (!cpp_field_includeTxPower_flag) {
        ani_boolean cpp_field_includeTxPower_spec_ani;
        env->Object_CallMethod_Boolean((ani_object)ani_field_includeTxPower,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "unboxed", ":z"),
            &cpp_field_includeTxPower_spec_ani);
        bool cpp_field_includeTxPower_spec = (bool)cpp_field_includeTxPower_spec_ani;
        cpp_field_includeTxPower_ptr = new bool(std::move(cpp_field_includeTxPower_spec));
    }
    ::taihe::optional<bool> cpp_field_includeTxPower(cpp_field_includeTxPower_ptr);
    return ::ohos::bluetooth::ble::AdvertiseData{std::move(cpp_field_serviceUuids),
                                                 std::move(cpp_field_manufactureData),
                                                 std::move(cpp_field_serviceData),
                                                 std::move(cpp_field_includeDeviceName),
                                                 std::move(cpp_field_includeTxPower)};
}

static ::ohos::bluetooth::ble::AdvertiseSetting TaiheParseAdvertiseSetting(ani_env *env, ani_object ani_obj)
{
    ani_ref ani_field_interval;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseSetting", "<get>interval", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_interval));
    ani_boolean cpp_field_interval_flag;
    int* cpp_field_interval_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_interval, &cpp_field_interval_flag);
    if (!cpp_field_interval_flag) {
        ani_int cpp_field_interval_spec_ani;
        env->Object_CallMethod_Int((ani_object)ani_field_interval,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "unboxed", ":i"),
            &cpp_field_interval_spec_ani);
        int cpp_field_interval_spec = (int)cpp_field_interval_spec_ani;
        cpp_field_interval_ptr = new int(std::move(cpp_field_interval_spec));
    }
    ::taihe::optional<int> cpp_field_interval(cpp_field_interval_ptr);
    ani_ref ani_field_txPower;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseSetting", "<get>txPower", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_txPower));
    ani_boolean cpp_field_txPower_flag;
    int* cpp_field_txPower_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_txPower, &cpp_field_txPower_flag);
    if (!cpp_field_txPower_flag) {
        ani_int cpp_field_txPower_spec_ani;
        env->Object_CallMethod_Int((ani_object)ani_field_txPower,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "unboxed", ":i"),
            &cpp_field_txPower_spec_ani);
        int cpp_field_txPower_spec = (int)cpp_field_txPower_spec_ani;
        cpp_field_txPower_ptr = new int(std::move(cpp_field_txPower_spec));
    }
    ::taihe::optional<int> cpp_field_txPower(cpp_field_txPower_ptr);
    ani_ref ani_field_connectable;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertiseSetting", "<get>connectable", nullptr),
        reinterpret_cast<ani_ref*>(&ani_field_connectable));
    ani_boolean cpp_field_connectable_flag;
    bool* cpp_field_connectable_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_connectable, &cpp_field_connectable_flag);
    if (!cpp_field_connectable_flag) {
        ani_boolean cpp_field_connectable_spec_ani;
        env->Object_CallMethod_Boolean(
            (ani_object)ani_field_connectable,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Boolean", "unboxed", ":z"),
            &cpp_field_connectable_spec_ani);
        bool cpp_field_connectable_spec = (bool)cpp_field_connectable_spec_ani;
        cpp_field_connectable_ptr = new bool(std::move(cpp_field_connectable_spec));
    }
    ::taihe::optional<bool> cpp_field_connectable(cpp_field_connectable_ptr);
    return ::ohos::bluetooth::ble::AdvertiseSetting{std::move(cpp_field_interval),
                                                    std::move(cpp_field_txPower),
                                                    std::move(cpp_field_connectable)};
}

::ohos::bluetooth::ble::AdvertisingParams TaiheParseAdvertisingParams(ani_env *env, ani_object ani_obj)
{
    ani_object ani_field_advertisingSettings;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingParams", "<get>advertisingSettings", nullptr),
        reinterpret_cast<ani_ref *>(&ani_field_advertisingSettings));
    ::ohos::bluetooth::ble::AdvertiseSetting cpp_field_advertisingSettings =
        TaiheParseAdvertiseSetting(env, ani_field_advertisingSettings);
    ani_object ani_field_advertisingData;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingParams", "<get>advertisingData", nullptr),
        reinterpret_cast<ani_ref *>(&ani_field_advertisingData));
    ::ohos::bluetooth::ble::AdvertiseData cpp_field_advertisingData =
        TaiheParseAdvertisingData(env, ani_field_advertisingData);
    ani_ref ani_field_advertisingResponse;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingParams", "<get>advertisingResponse", nullptr),
        reinterpret_cast<ani_ref *>(&ani_field_advertisingResponse));
    ani_boolean cpp_field_advertisingResponse_flag;
    ::ohos::bluetooth::ble::AdvertiseData* cpp_field_advertisingResponse_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_advertisingResponse, &cpp_field_advertisingResponse_flag);
    if (!cpp_field_advertisingResponse_flag) {
        ::ohos::bluetooth::ble::AdvertiseData cpp_field_advertisingResponse_spec =
            TaiheParseAdvertisingData(env, static_cast<ani_object>(ani_field_advertisingResponse));
        cpp_field_advertisingResponse_ptr =
            new ::ohos::bluetooth::ble::AdvertiseData(cpp_field_advertisingResponse_spec);
    }
    ::taihe::optional<::ohos::bluetooth::ble::AdvertiseData> cpp_field_advertisingResponse(
        cpp_field_advertisingResponse_ptr);
    ani_ref ani_field_duration;
    env->Object_CallMethod_Ref(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingParams", "<get>duration", nullptr),
        reinterpret_cast<ani_ref *>(&ani_field_duration));
    ani_boolean cpp_field_duration_flag;
    int* cpp_field_duration_ptr = nullptr;
    env->Reference_IsUndefined(ani_field_duration, &cpp_field_duration_flag);
    if (!cpp_field_duration_flag) {
        ani_int cpp_field_duration_spec_ani;
        env->Object_CallMethod_Int((ani_object)ani_field_duration,
            TH_ANI_FIND_CLASS_METHOD(env, "std.core.Int", "unboxed", ":i"),
            &cpp_field_duration_spec_ani);
        int cpp_field_duration_spec = (int)cpp_field_duration_spec_ani;
        cpp_field_duration_ptr = new int(std::move(cpp_field_duration_spec));
    }
    ::taihe::optional<int> cpp_field_duration(cpp_field_duration_ptr);
    return ::ohos::bluetooth::ble::AdvertisingParams{std::move(cpp_field_advertisingSettings),
                                                     std::move(cpp_field_advertisingData),
                                                     std::move(cpp_field_advertisingResponse),
                                                     std::move(cpp_field_duration)};
}

::ohos::bluetooth::ble::AdvertisingDisableParams TaiheParseAdvertisingDisableParams(ani_env *env, ani_object ani_obj)
{
    ani_int ani_field_advertisingId = {};
    env->Object_CallMethod_Int(ani_obj,
        TH_ANI_FIND_CLASS_METHOD(env, "@ohos.bluetooth.ble.ble.AdvertisingDisableParams", "<get>advertisingId", nullptr),
        reinterpret_cast<ani_int*>(&ani_field_advertisingId));
    int32_t cpp_field_advertisingId = static_cast<int32_t>(ani_field_advertisingId);
    return ::ohos::bluetooth::ble::AdvertisingDisableParams{std::move(cpp_field_advertisingId)};
}

}  // namespace Bluetooth
}  // namespace OHOS
