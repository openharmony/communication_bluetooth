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

#include "napi_parser_utils.h"

#include <string>
#include <vector>

namespace OHOS {
namespace Bluetooth {

// shall check object type is napi_object before, if it's other type, return false
// If the 'name' field is not exist, or napi function call error, return false
static bool NapiIsObjectPropertyExist(napi_env env, napi_value object, const char *name)
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

napi_status NapiParseGattCharacteristic(napi_env env, napi_value object, NapiBleCharacteristic &outCharacteristic)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"serviceUuid", "characteristicUuid",
        "characteristicValue", "descriptors"}));

    std::string serviceUuid {};
    std::string characterUuid {};
    std::vector<uint8_t> characterValue {};
    std::vector<NapiBleDescriptor> descriptors {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "serviceUuid", serviceUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "characteristicUuid", characterUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectArrayBuffer(env, object, "characteristicValue", characterValue));
    NAPI_BT_CALL_RETURN(NapiParseObjectArray(env, object, "descriptors", descriptors));

    outCharacteristic.serviceUuid = UUID::FromString(serviceUuid);
    outCharacteristic.characteristicUuid = UUID::FromString(characterUuid);
    outCharacteristic.characteristicValue = std::move(characterValue);
    outCharacteristic.descriptors = std::move(descriptors);
    return napi_ok;
}

napi_status NapiParseGattDescriptor(napi_env env, napi_value object, NapiBleDescriptor &outDescriptor)
{
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, object, {"serviceUuid", "characteristicUuid",
        "descriptorUuid", "descriptorValue"}));

    std::string serviceUuid {};
    std::string characterUuid {};
    std::string descriptorUuid {};
    std::vector<uint8_t> descriptorValue {};
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "serviceUuid", serviceUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "characteristicUuid", characterUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectUuid(env, object, "descriptorUuid", descriptorUuid));
    NAPI_BT_CALL_RETURN(NapiParseObjectArrayBuffer(env, object, "descriptorValue", descriptorValue));

    outDescriptor.serviceUuid = UUID::FromString(serviceUuid);
    outDescriptor.characteristicUuid = UUID::FromString(characterUuid);
    outDescriptor.descriptorUuid = UUID::FromString(descriptorUuid);
    outDescriptor.descriptorValue = std::move(descriptorValue);
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

napi_status NapiParseObjectBooleanOptional(napi_env env, napi_value object, const char *name, bool &outBoolean,
    bool &outExist)
{
    napi_value property;
    bool boolean = true;
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectPropertyOptional(env, object, name, property, exist));
    if (exist) {
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
    int32_t num = 0;
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property, exist));
    if (exist) {
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
    uint32_t num = 0;
    bool exist = false;
    NAPI_BT_CALL_RETURN(NapiIsObject(env, object));
    NAPI_BT_CALL_RETURN(NapiGetObjectProperty(env, object, name, property, exist));
    if (exist) {
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
}  // namespace Bluetooth
}  // namespace OHOS
