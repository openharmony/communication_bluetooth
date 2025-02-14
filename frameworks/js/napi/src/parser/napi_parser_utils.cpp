/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_parser_utils"
#endif

#include "napi_parser_utils.h"

#include <string>
#include <vector>

#include "napi_async_callback.h"

namespace OHOS {
namespace Bluetooth {

// shall check object type is napi_object before, if it's other type, return false
// If the 'name' field is not exist, or napi function call error, return false
bool NapiIsObjectPropertyExist(napi_env env, napi_value object, const char *name)
{
    auto status = NapiIsObject(env, object);
    if (status != napi_ok) {
        HILOGE("expect object");
        return false;
    }
    bool exist = false;
    status = napi_has_named_property(env, object, name, &exist);
    if (status != napi_ok) {
        HILOGE("Get object property failed, name: %{public}s", name);
        return false;
    }
    return exist;
}

napi_status NapiParseGattService(napi_env env, napi_value object, NapiGattService &outService)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"serviceUuid", "isPrimary", "characteristics",
        "includeServices"}));

    std::string uuid {};
    bool isPrimary = true;
    std::vector<NapiBleCharacteristic> characteristics {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "serviceUuid", uuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectBoolean(env, object, "isPrimary", isPrimary));
    NAPI_BT_CALL_RETURN(NapiParseObjectArray(env, object, "characteristics", characteristics));
    if (NapiIsObjectPropertyExist(env, object, "includeServices")) {
        std::vector<NapiGattService> includeServices {};
        NAPI_BT_CALL_RETURN(NapiParseObjectArray(env, object, "includeServices", includeServices));
        outService.includeServices = std::move(includeServices);
    }
    outService.serviceUuid = UUID::FromString(uuid);
    outService.isPrimary = isPrimary;
    outService.characteristics = std::move(characteristics);
    return napi_ok;
}

namespace {
const NapiGattPermission DEFAULT_GATT_PERMISSIONS = {
    .readable = true,
    .writeable = true,
};
const NapiGattProperties DEFAULT_GATT_PROPERTIES = {
    .write = true,
    .writeNoResponse = true,
    .read = true,
};
uint16_t ConvertGattPermissions(const NapiGattPermission &napiPermissions)
{
    uint16_t permissions = 0;
    if (napiPermissions.readable) {
        permissions |= static_cast<uint16_t>(GattPermission::READABLE);
    }
    if (napiPermissions.writeable) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITEABLE);
    }
    if (napiPermissions.readEncrypted) {
        permissions |= static_cast<uint16_t>(GattPermission::READ_ENCRYPTED_MITM);
    }
    if (napiPermissions.writeEncrypted) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_ENCRYPTED_MITM);
    }
    return permissions;
}
uint16_t ConvertGattProperties(const NapiGattProperties &napiProperties)
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
    return properties;
}
}  // namespace {}

napi_status NapiParseGattCharacteristic(napi_env env, napi_value object, NapiBleCharacteristic &outCharacteristic)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"serviceUuid", "characteristicUuid",
        "characteristicValue", "descriptors", "properties", "characteristicValueHandle", "permissions"}));

    std::string serviceUuid {};
    std::string characterUuid {};
    std::vector<uint8_t> characterValue {};
    std::vector<NapiBleDescriptor> descriptors {};
    uint32_t characteristicValueHandle = 0;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "serviceUuid", serviceUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "characteristicUuid", characterUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectArrayBuffer(env, object, "characteristicValue", characterValue));
    NAPI_BT_CALL_RETURN(NapiParseObjectArray(env, object, "descriptors", descriptors));

    NapiGattProperties properties = DEFAULT_GATT_PROPERTIES;
    if (NapiIsObjectPropertyExist(env, object, "properties"))  {
        NAPI_BT_CALL_RETURN(NapiParseObjectGattProperties(env, object, "properties", properties));
    }
    if (NapiIsObjectPropertyExist(env, object, "characteristicValueHandle")) {
        NAPI_BT_CALL_RETURN(NapiParseObjectUint32(env, object, "characteristicValueHandle", characteristicValueHandle));
        NAPI_BT_RETURN_IF(characteristicValueHandle > 0xFFFF, "Invalid characteristicValueHandle", napi_invalid_arg);
    }
    NapiGattPermission permissions = DEFAULT_GATT_PERMISSIONS;
    if (NapiIsObjectPropertyExist(env, object, "permissions")) {
        NAPI_BT_CALL_RETURN(NapiParseObjectGattPermissions(env, object, "permissions", permissions));
    }

    outCharacteristic.serviceUuid = UUID::FromString(serviceUuid);
    outCharacteristic.characteristicUuid = UUID::FromString(characterUuid);
    outCharacteristic.characteristicValue = std::move(characterValue);
    outCharacteristic.descriptors = std::move(descriptors);
    outCharacteristic.properties = ConvertGattProperties(properties);
    outCharacteristic.characteristicValueHandle = static_cast<uint16_t>(characteristicValueHandle);
    outCharacteristic.permissions = ConvertGattPermissions(permissions);
    return napi_ok;
}

napi_status NapiParseGattDescriptor(napi_env env, napi_value object, NapiBleDescriptor &outDescriptor)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"serviceUuid", "characteristicUuid",
        "descriptorUuid", "descriptorValue", "descriptorHandle", "permissions"}));

    std::string serviceUuid {};
    std::string characterUuid {};
    std::string descriptorUuid {};
    std::vector<uint8_t> descriptorValue {};
    uint32_t descriptorHandle = 0;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "serviceUuid", serviceUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "characteristicUuid", characterUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "descriptorUuid", descriptorUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectArrayBuffer(env, object, "descriptorValue", descriptorValue));

    if (NapiIsObjectPropertyExist(env, object, "descriptorHandle")) {
        NAPI_BT_CALL_RETURN(NapiParseObjectUint32(env, object, "descriptorHandle", descriptorHandle));
        NAPI_BT_RETURN_IF(descriptorHandle > 0xFFFF, "Invalid descriptorHandle", napi_invalid_arg);
    }
    NapiGattPermission permissions = DEFAULT_GATT_PERMISSIONS;
    if (NapiIsObjectPropertyExist(env, object, "permissions")) {
        NAPI_BT_CALL_RETURN(NapiParseObjectGattPermissions(env, object, "permissions", permissions));
    }

    outDescriptor.serviceUuid = UUID::FromString(serviceUuid);
    outDescriptor.characteristicUuid = UUID::FromString(characterUuid);
    outDescriptor.descriptorUuid = UUID::FromString(descriptorUuid);
    outDescriptor.descriptorValue = std::move(descriptorValue);
    outDescriptor.descriptorHandle = static_cast<uint16_t>(descriptorHandle);
    outDescriptor.permissions = ConvertGattPermissions(permissions);
    return napi_ok;
}

napi_status NapiParseNotifyCharacteristic(napi_env env, napi_value object, NapiNotifyCharacteristic &outCharacter)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"serviceUuid", "characteristicUuid",
        "characteristicValue", "confirm"}));

    std::string serviceUuid {};
    std::string characterUuid {};
    std::vector<uint8_t> characterValue {};
    bool confirm = false;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "serviceUuid", serviceUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "characteristicUuid", characterUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectArrayBuffer(env, object, "characteristicValue", characterValue));
    NAPI_BT_CALL_RETURN(NapiParseObjectBoolean(env, object, "confirm", confirm));

    outCharacter.serviceUuid = UUID::FromString(serviceUuid);
    outCharacter.characterUuid = UUID::FromString(characterUuid);
    outCharacter.characterValue = std::move(characterValue);
    outCharacter.confirm = confirm;
    return napi_ok;
}

napi_status NapiParseGattsServerResponse(napi_env env, napi_value object, NapiGattsServerResponse &rsp)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"deviceId", "transId", "status", "offset",
        "value"}));

    std::string deviceId {};
    int transId = 0;
    int status = 0;
    int offset = 0;
    std::vector<uint8_t> value {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectBdAddr(env, object, "deviceId", deviceId));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, object, "transId", transId));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, object, "status", status));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, object, "offset", offset));
    NAPI_BT_CALL_RETURN(NapiParseObjectArrayBuffer(env, object, "value", value));

    rsp.deviceId = deviceId;
    rsp.transId = transId;
    rsp.status = status;
    rsp.offset = offset;
    rsp.value = std::move(value);
    return napi_ok;
}

napi_status NapiParseObjectGattPermissions(napi_env env, napi_value object, const char *name,
    NapiGattPermission &outPermissions)
{
    napi_value permissionObject;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, permissionObject));
    // Parse permission object
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, permissionObject, {"readable", "writeable",
        "readEncrypted", "writeEncrypted"}));

    bool isExist;
    NapiGattPermission permissions {};
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, permissionObject, "readable", permissions.readable, isExist));
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, permissionObject, "writeable", permissions.writeable, isExist));
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, permissionObject, "readEncrypted", permissions.readEncrypted, isExist));
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, permissionObject, "writeEncrypted", permissions.writeEncrypted, isExist));
    outPermissions = permissions;
    return napi_ok;
}

napi_status NapiParseObjectGattProperties(napi_env env, napi_value object, const char *name,
    NapiGattProperties &outProperties)
{
    napi_value propertiesObject;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, propertiesObject));
    // Parse properties object
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, propertiesObject, {"write", "writeNoResponse",
        "read", "notify", "indicate"}));

    bool isExist;
    NapiGattProperties properties {};
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, propertiesObject, "write", properties.write, isExist));
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, propertiesObject, "writeNoResponse", properties.writeNoResponse, isExist));
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, propertiesObject, "read", properties.read, isExist));
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, propertiesObject, "notify", properties.notify, isExist));
    NAPI_BT_CALL_RETURN(
        NapiParseObjectBooleanOptional(env, propertiesObject, "indicate", properties.indicate, isExist));
    outProperties = properties;
    return napi_ok;
}

napi_status NapiParseBoolean(napi_env env, napi_value value, bool &outBoolean)
{
    bool boolean = false;
    NAPI_BT_CALL_RETURN(NapiIsBoolean(env, value));
    NAPI_BT_CALL_RETURN(napi_get_value_bool(env, value, &boolean));
    outBoolean = boolean;
    return napi_ok;
}

napi_status NapiParseInt32(napi_env env, napi_value value, int32_t &outNum)
{
    int32_t num = 0;
    NAPI_BT_CALL_RETURN(NapiIsNumber(env, value));
    NAPI_BT_CALL_RETURN(napi_get_value_int32(env, value, &num));
    outNum = num;
    return napi_ok;
}

napi_status NapiParseUint32(napi_env env, napi_value value, uint32_t &outNum)
{
    uint32_t num = 0;
    NAPI_BT_CALL_RETURN(NapiIsNumber(env, value));
    NAPI_BT_CALL_RETURN(napi_get_value_uint32(env, value, &num));
    outNum = num;
    return napi_ok;
}

napi_status NapiParseString(napi_env env, napi_value value, std::string &outStr)
{
    std::string str {};
    NAPI_BT_CALL_RETURN(NapiIsString(env, value));
    NAPI_BT_RETURN_IF(!ParseString(env, str, value), "parse string failed", napi_invalid_arg);
    outStr = std::move(str);
    return napi_ok;
}

napi_status NapiParseStringArray(napi_env env, napi_value value, std::vector<std::string> &outStrVec)
{
    NAPI_BT_CALL_RETURN(NapiIsArray(env, value));
    uint32_t length = 0;
    std::string param {};
    std::vector<std::string> strVec {};
    napi_get_array_length(env, value, &length);
    for (size_t i = 0; i < length; i++) {
        napi_value result;
        napi_get_element(env, value, i, &result);
        NAPI_BT_CALL_RETURN(NapiParseString(env, result, param));
        strVec.push_back(param);
    }
    outStrVec = std::move(strVec);
    return napi_ok;
}

napi_status NapiParseFileHolderArray(napi_env env, napi_value value, std::vector<FileHolder> &outFileHolderVec)
{
    NAPI_BT_CALL_RETURN(NapiIsArray(env, value));
    uint32_t length = 0;
    std::vector<FileHolder> fileHolderVec {};
    napi_get_array_length(env, value, &length);
    for (size_t i = 0; i < length; i++) {
        FileHolder fileHolder;
        napi_value result;
        napi_get_element(env, value, i, &result);
        NAPI_BT_CALL_RETURN(NapiParseFileHolder(env, result, fileHolder));
        fileHolderVec.push_back(fileHolder);
    }
    outFileHolderVec = std::move(fileHolderVec);
    return napi_ok;
}

napi_status NapiParseFileHolder(napi_env env, napi_value object, FileHolder &outFileHolder)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"filePath", "fileSize", "fileFd"}));
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    std::string filePath {};
    int64_t fileSize = 0;
    int32_t fileFd = 0;
    std::string fileType {};

    NAPI_BT_CALL_RETURN(NapiParseObjectString(env, object, "filePath", filePath));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt64(env, object, "fileSize", fileSize));
    NAPI_BT_CALL_RETURN(NapiParseObjectInt32(env, object, "fileFd", fileFd));
    HILOGI("filePath: %{public}s fileSize: %{public}Ld fileFd: %{public}d",
        filePath.c_str(), fileSize, fileFd);

    FileHolder fileHolder;
    fileHolder.filePath = filePath;
    fileHolder.fileSize = fileSize;
    fileHolder.fileFd = fileFd;
    outFileHolder = std::move(fileHolder);
    return napi_ok;
}


napi_status NapiParseBdAddr(napi_env env, napi_value value, std::string &outAddr)
{
    std::string bdaddr {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, value, bdaddr));
    NAPI_BT_RETURN_IF(!IsValidAddress(bdaddr), "Invalid bdaddr", napi_invalid_arg);
    outAddr = std::move(bdaddr);
    return napi_ok;
}

napi_status NapiParseUuid(napi_env env, napi_value value, std::string &outUuid)
{
    std::string uuid {};
    NAPI_BT_CALL_RETURN(NapiParseString(env, value, uuid));
    NAPI_BT_RETURN_IF(!IsValidUuid(uuid), "Invalid uuid", napi_invalid_arg);
    outUuid = std::move(uuid);
    return napi_ok;
}

napi_status NapiParseInt64(napi_env env, napi_value value, int64_t &outNum)
{
    int64_t num = 0;
    NAPI_BT_CALL_RETURN(NapiIsNumber(env, value));
    NAPI_BT_CALL_RETURN(napi_get_value_int64(env, value, &num));
    outNum = num;
    return napi_ok;
}

napi_status NapiParseObjectInt64(napi_env env, napi_value object, const char *name, int64_t &outNum)
{
    napi_value property;
    int64_t num = 0;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseInt64(env, property, num));
    outNum = num;
    return napi_ok;
}

napi_status NapiParseObjectStr(napi_env env, napi_value object, const char *name, std::string &outStr)
{
    napi_value property;
    std::string str;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseString(env, property, str));
    outStr = std::move(str);
    return napi_ok;
}

napi_status NapiParseObjectUuids(napi_env env, napi_value object, const char *name, std::vector<std::string> &outUuid)
{
    std::string uuids;
    NAPI_BT_CALL_RETURN(NapiParseObjectStr(env, object, name, uuids));
    NAPI_BT_CALL_RETURN(ParseAndCheckUuids(uuids, outUuid));
    return napi_ok;
}

napi_status ParseAndCheckUuids(const std::string &uuids, std::vector<std::string> &res)
{
    HILOGI("[CLOUD_PAIR] ParseAndCheckUuids %{public}s", uuids.c_str());
    if (uuids.empty()) {
        return napi_ok;
    }
    const std::regex pattern("[,]");
    std::vector<std::string> result(
        std::sregex_token_iterator(uuids.begin(), uuids.end(), pattern, -1),
        std::sregex_token_iterator()
    );
    for (const auto &str : result) {
        NAPI_BT_RETURN_IF(!IsValidUuid(str), "Invalid uuid", napi_invalid_arg);
    }
    res = std::move(result);
    return napi_ok;
}

napi_status NapiParseTrustPairDevice(napi_env env, napi_value object, std::vector<TrustPairDeviceParam> &outService)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"trustedPairedDevices"}));
    NAPI_BT_CALL_RETURN(NapiParseObjectArray(env, object, "trustedPairedDevices", outService));
    return napi_ok;
}

napi_status NapiParseArrayBuffer(napi_env env, napi_value value, std::vector<uint8_t> &outVec)
{
    uint8_t *data = nullptr;
    size_t size = 0;
    NAPI_BT_CALL_RETURN(NapiIsArrayBuffer(env, value));
    bool isSuccess = ParseArrayBuffer(env, &data, size, value);
    if (!isSuccess) {
        HILOGE("Parse arraybuffer failed");
        return napi_invalid_arg;
    }
    std::vector<uint8_t> vec(data, data + size);
    outVec = std::move(vec);
    return napi_ok;
}

napi_status NapiParseGattWriteType(napi_env env, napi_value value, int &outWriteType)
{
    int writeType = -1;
    NAPI_BT_CALL_RETURN(NapiParseInt32(env, value, writeType));
    if (writeType == static_cast<int>(NapiGattWriteType::WRITE)) {
        HILOGI("gattWriteType: WRITE");
        outWriteType = GattCharacteristic::WriteType::DEFAULT;
    } else if (writeType == static_cast<int>(NapiGattWriteType::WRITE_NO_RESPONSE)) {
        HILOGI("gattWriteType: WRITE_NO_RESPONSE");
        outWriteType = GattCharacteristic::WriteType::NO_RESPONSE;
    } else {
        HILOGE("Invalid gattWriteType: %{public}d", writeType);
        return napi_invalid_arg;
    }
    return napi_ok;
}

std::shared_ptr<NapiAsyncCallback> NapiParseAsyncCallback(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_FOUR;
    napi_value argv[ARGS_SIZE_FOUR] = {nullptr};
    auto status = napi_get_cb_info(env, info, &argc, argv, nullptr, NULL);
    if (status != napi_ok) {
        HILOGE("napi_get_cb_info failed");
        return nullptr;
    }
    if (argc > ARGS_SIZE_FOUR) {
        HILOGE("size of parameters is larger than ARGS_SIZE_FOUR");
        return nullptr;
    }

    // "argc - 1" is AsyncCallback parameter's index
    auto asyncCallback = std::make_shared<NapiAsyncCallback>();
    asyncCallback->env = env;
    if (argc > 0 && NapiIsFunction(env, argv[argc - 1]) == napi_ok) {
        HILOGD("callback mode");
        asyncCallback->callback = std::make_shared<NapiCallback>(env, argv[argc - 1]);
    } else {
        HILOGD("promise mode");
        asyncCallback->promise = std::make_shared<NapiPromise>(env);
    }
    return asyncCallback;
}

static napi_status NapiGetObjectProperty(napi_env env, napi_value object, const char *name, napi_value &outProperty,
    bool &outExist)
{
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(napi_has_named_property(env, object, name, &exist));
    if (exist) {
        napi_value property;
        NAPI_BT_CALL_RETURN(napi_get_named_property(env, object, name, &property));
        outProperty = property;
    }
    outExist = exist;
    return napi_ok;
}
napi_status NapiGetObjectProperty(napi_env env, napi_value object, const char *name, napi_value &outProperty)
{
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, outProperty, exist));
    NAPI_BT_RETURN_IF(!exist, "no needed property", napi_invalid_arg);
    return napi_ok;
}
napi_status NapiGetObjectPropertyOptional(napi_env env, napi_value object, const char *name, napi_value &outProperty,
    bool &outExist)
{
    return NapiGetObjectProperty(env, object, name, outProperty, outExist);
}

napi_status NapiParseObjectBoolean(napi_env env, napi_value object, const char *name, bool &outBoolean)
{
    napi_value property;
    bool boolean = true;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseBoolean(env, property, boolean));
    outBoolean = boolean;
    return napi_ok;
}

napi_status NapiParseObjectUuid(napi_env env, napi_value object, const char *name, std::string &outUuid)
{
    napi_value property;
    std::string uuid {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseUuid(env, property, uuid));
    outUuid = std::move(uuid);
    return napi_ok;
}

napi_status NapiParseObjectArrayBuffer(napi_env env, napi_value object, const char *name, std::vector<uint8_t> &outVec)
{
    napi_value property;
    std::vector<uint8_t> vec {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseArrayBuffer(env, property, vec));
    outVec = std::move(vec);
    return napi_ok;
}

napi_status NapiParseObjectBdAddr(napi_env env, napi_value object, const char *name, std::string &outAddr)
{
    napi_value property;
    std::string bdaddr {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseBdAddr(env, property, bdaddr));
    outAddr = std::move(bdaddr);
    return napi_ok;
}

napi_status NapiParseObjectInt32(napi_env env, napi_value object, const char *name, int32_t &outNum)
{
    napi_value property;
    int32_t num = 0;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseInt32(env, property, num));
    outNum = num;
    return napi_ok;
}

napi_status NapiParseObjectString(napi_env env, napi_value object, const char *name, std::string &outString)
{
    napi_value property;
    std::string str = "";
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseString(env, property, str));
    outString = str;
    return napi_ok;
}

napi_status NapiParseObjectUint32(napi_env env, napi_value object, const char *name, uint32_t &outNum)
{
    napi_value property;
    uint32_t num = 0;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseUint32(env, property, num));
    outNum = num;
    return napi_ok;
}

napi_status NapiParseObjectUint32Check(NapiObject napiObject, const char *name, uint32_t &outNum,
    uint32_t min, uint32_t max)
{
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32(napiObject.env, napiObject.object, name, outNum));
    if (outNum < min || outNum > max) {
        return napi_invalid_arg;
    }
    return napi_ok;
}

napi_status NapiParseObjectBooleanOptional(napi_env env, napi_value object, const char *name, bool &outBoolean,
    bool &outExist)
{
    napi_value property;
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectPropertyOptional(env, object, name, property, exist));
    if (exist) {
        bool boolean = true;
        NAPI_BT_CALL_RETURN(NapiParseBoolean(env, property, boolean));
        outBoolean = boolean;
    }
    outExist = exist;
    return napi_ok;
}
napi_status NapiParseObjectInt32Optional(napi_env env, napi_value object, const char *name, int32_t &outNum,
    bool &outExist)
{
    napi_value property;
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property, exist));
    if (exist) {
        int32_t num = 0;
        NAPI_BT_CALL_RETURN(NapiParseInt32(env, property, num));
        outNum = num;
    }
    outExist = exist;
    return napi_ok;
}
napi_status NapiParseObjectUint32Optional(napi_env env, napi_value object, const char *name, uint32_t &outNum,
    bool &outExist)
{
    napi_value property;
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property, exist));
    if (exist) {
        uint32_t num = 0;
        NAPI_BT_CALL_RETURN(NapiParseUint32(env, property, num));
        outNum = num;
    }
    outExist = exist;
    return napi_ok;
}

// Parse params template, used for NapiParseArray
template <typename T>
napi_status NapiParseObject(napi_env env, napi_value object, T &outObj)
{
    HILOGE("Unimpleted type");
    return napi_invalid_arg;
}

template <>
napi_status NapiParseObject<std::string>(napi_env env, napi_value object, std::string &outObj)
{
    return NapiParseString(env, object, outObj);
}

template <>
napi_status NapiParseObject<UUID>(napi_env env, napi_value object, UUID &outObj)
{
    std::string uuid {};
    NAPI_BT_CALL_RETURN(NapiParseUuid(env, object, uuid));
    outObj = UUID::FromString(uuid);
    return napi_ok;
}

template <>
napi_status NapiParseObject<NapiAdvManufactureData>(napi_env env, napi_value object, NapiAdvManufactureData &outObj)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"manufactureId", "manufactureValue"}));
    uint32_t num = 0;
    std::vector<uint8_t> vec {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectUint32(env, object, "manufactureId", num));
    NAPI_BT_RETURN_IF(num > 0xFFFF, "Invalid manufactureId", napi_invalid_arg);
    NAPI_BT_CALL_RETURN(NapiParseObjectArrayBuffer(env, object, "manufactureValue", vec));

    outObj.id = static_cast<uint16_t>(num);
    outObj.value = std::string(vec.begin(), vec.end());
    return napi_ok;
}

template <>
napi_status NapiParseObject<NapiAdvServiceData>(napi_env env, napi_value object, NapiAdvServiceData &outObj)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"serviceUuid", "serviceValue"}));
    std::string uuid {};
    std::vector<uint8_t> vec {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "serviceUuid", uuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectArrayBuffer(env, object, "serviceValue", vec));

    outObj.uuid = std::move(uuid);
    outObj.value = std::move(vec);
    return napi_ok;
}

template <>
napi_status NapiParseObject<NapiGattService>(napi_env env, napi_value object, NapiGattService &outObj)
{
    return NapiParseGattService(env, object, outObj);
}

template <>
napi_status NapiParseObject<NapiBleCharacteristic>(napi_env env, napi_value object, NapiBleCharacteristic &outObj)
{
    return NapiParseGattCharacteristic(env, object, outObj);
}

template <>
napi_status NapiParseObject<NapiBleDescriptor>(napi_env env, napi_value object, NapiBleDescriptor &outObj)
{
    return NapiParseGattDescriptor(env, object, outObj);
}

template <>
napi_status NapiParseObject<TrustPairDeviceParam>(napi_env env, napi_value object, TrustPairDeviceParam &outObj)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"sn", "deviceType",
        "modelId", "manufactory", "productId", "hiLinkVersion", "macAddress", "serviceType",
        "serviceId", "deviceName", "uuids", "bluetoothClass", "token", "deviceNameTime",
        "secureAdvertisingInfo", "pairState"}));
    NapiParseObjectStr(env, object, "sn", outObj.sn_);
    NapiParseObjectStr(env, object, "deviceType", outObj.deviceType_);
    NapiParseObjectStr(env, object, "modelId", outObj.modelId_);
    NapiParseObjectStr(env, object, "manufactory", outObj.manufactory_);
    NapiParseObjectStr(env, object, "productId", outObj.productId_);
    NapiParseObjectStr(env, object, "hiLinkVersion", outObj.hiLinkVersion_);
    NAPI_BT_CALL_RETURN(NapiParseObjectBdAddr(env, object, "macAddress", outObj.macAddress_));
    NapiParseObjectStr(env, object, "serviceType", outObj.serviceType_);
    NapiParseObjectStr(env, object, "serviceId", outObj.serviceId_);
    NapiParseObjectStr(env, object, "deviceName", outObj.deviceName_);
    NAPI_BT_CALL_RETURN(NapiParseObjectUuids(env, object, "uuids", outObj.uuids_));
    NapiParseObjectInt32(env, object, "bluetoothClass", outObj.bluetoothClass_);
    NapiParseObjectInt64(env, object, "deviceNameTime", outObj.deviceNameTime_);
    NapiParseObjectInt32(env, object, "pairState", outObj.pairState_);
    std::vector<uint8_t> tokenValue;
    std::vector<uint8_t> secureAdvertisingInfoValue;
    NapiParseObjectArrayBuffer(env, object, "token", tokenValue);
    NapiParseObjectArrayBuffer(env, object, "secureAdvertisingInfo", secureAdvertisingInfoValue);
    outObj.token_ = std::move(tokenValue);
    outObj.secureAdvertisingInfo_ = std::move(secureAdvertisingInfoValue);
    return napi_ok;
}

template <typename T>
napi_status NapiParseArray(napi_env env, napi_value array, std::vector<T> &outVec)
{
    std::vector<T> vec {};

    NAPI_BT_CALL_RETURN(NapiIsArray(env, array));
    uint32_t length = 0;
    NAPI_BT_CALL_RETURN(napi_get_array_length(env, array, &length));
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        NAPI_BT_CALL_RETURN(napi_get_element(env, array, i, &element));
        T object;
        NAPI_BT_CALL_RETURN(NapiParseObject(env, element, object));
        vec.push_back(std::move(object));
    }
    outVec = std::move(vec);
    return napi_ok;
}
// // Declaration, ohters will undefined synbol
template napi_status NapiParseArray<NapiBleDescriptor>(napi_env env, napi_value array,
    std::vector<NapiBleDescriptor> &outVec);
template napi_status NapiParseArray<NapiBleCharacteristic>(napi_env env, napi_value array,
    std::vector<NapiBleCharacteristic> &outVec);
template napi_status NapiParseArray<NapiGattService>(napi_env env, napi_value array,
    std::vector<NapiGattService> &outVec);
template napi_status NapiParseArray<NapiAdvServiceData>(napi_env env, napi_value array,
    std::vector<NapiAdvServiceData> &outVec);
template napi_status NapiParseArray<NapiAdvManufactureData>(napi_env env, napi_value array,
    std::vector<NapiAdvManufactureData> &outVec);
template napi_status NapiParseArray<UUID>(napi_env env, napi_value array,
    std::vector<UUID> &outVec);
template napi_status NapiParseArray<std::string>(napi_env env, napi_value array,
    std::vector<std::string> &outVec);
template napi_status NapiParseArray<TrustPairDeviceParam>(napi_env env, napi_value array,
    std::vector<TrustPairDeviceParam> &outVec);

template <typename T>
napi_status NapiParseObjectArray(napi_env env, napi_value object, const char *name, std::vector<T> &outVec)
{
    napi_value property;
    std::vector<T> vec {};
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property));
    NAPI_BT_CALL_RETURN(NapiParseArray(env, property, vec));
    outVec = std::move(vec);
    return napi_ok;
}
// Declaration, ohters will undefined synbol
template napi_status NapiParseObjectArray<NapiBleDescriptor>(napi_env env, napi_value object, const char *name,
    std::vector<NapiBleDescriptor> &outVec);
template napi_status NapiParseObjectArray<NapiBleCharacteristic>(napi_env env, napi_value object, const char *name,
    std::vector<NapiBleCharacteristic> &outVec);
template napi_status NapiParseObjectArray<NapiGattService>(napi_env env, napi_value object, const char *name,
    std::vector<NapiGattService> &outVec);
template napi_status NapiParseObjectArray<NapiAdvServiceData>(napi_env env, napi_value object, const char *name,
    std::vector<NapiAdvServiceData> &outVec);
template napi_status NapiParseObjectArray<NapiAdvManufactureData>(napi_env env, napi_value object, const char *name,
    std::vector<NapiAdvManufactureData> &outVec);
template napi_status NapiParseObjectArray<UUID>(napi_env env, napi_value object, const char *name,
    std::vector<UUID> &outVec);
template napi_status NapiParseObjectArray<std::string>(napi_env env, napi_value object, const char *name,
    std::vector<std::string> &outVec);
template napi_status NapiParseObjectArray<TrustPairDeviceParam>(napi_env env, napi_value object, const char *name,
    std::vector<TrustPairDeviceParam> &outVec);
}  // namespace Bluetooth
}  // namespace OHOS
