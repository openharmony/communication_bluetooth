/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_napi_ble_utils"
#endif

#include "napi_bluetooth_utils.h"
#include "napi_bluetooth_ble_utils.h"
#include <algorithm>
#include <functional>
#include <optional>
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_bluetooth_error.h"
#include "securec.h"
#include "bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;
void ConvertGattServiceToJS(napi_env env, napi_value result, GattService& service)
{
    napi_value serviceUuid;
    napi_create_string_utf8(env, service.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &serviceUuid);
    napi_set_named_property(env, result, "serviceUuid", serviceUuid);

    napi_value isPrimary;
    napi_get_boolean(env, service.IsPrimary(), &isPrimary);
    napi_set_named_property(env, result, "isPrimary", isPrimary);
    HILOGI("uuid: %{public}s, isPrimary: %{public}d", service.GetUuid().ToString().c_str(), service.IsPrimary());

    napi_value characteristics;
    napi_create_array(env, &characteristics);
    ConvertBLECharacteristicVectorToJS(env, characteristics, service.GetCharacteristics());
    napi_set_named_property(env, result, "characteristics", characteristics);

    napi_value includeServices;
    napi_create_array(env, &includeServices);
    vector<GattService> services;
    vector<std::reference_wrapper<GattService>> srvs = service.GetIncludedServices();
    for (auto &srv : srvs) {
        services.push_back(srv.get());
    }
    ConvertGattServiceVectorToJS(env, includeServices, services);
    napi_set_named_property(env, result, "includeServices", includeServices);
}

void ConvertGattServiceVectorToJS(napi_env env, napi_value result, vector<GattService>& services)
{
    HILOGI("enter");
    size_t idx = 0;

    if (services.empty()) {
        return;
    }
    HILOGI("size: %{public}zu", services.size());
    for (auto& service : services) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        ConvertGattServiceToJS(env, obj, service);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

void ConvertBLECharacteristicVectorToJS(napi_env env, napi_value result,
    vector<GattCharacteristic>& characteristics)
{
    HILOGI("size: %{public}zu", characteristics.size());
    size_t idx = 0;
    if (characteristics.empty()) {
        return;
    }

    for (auto &characteristic : characteristics) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        ConvertBLECharacteristicToJS(env, obj, characteristic);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

bool HasProperty(int properties, int propertyMask)
{
    if (properties < 0 || propertyMask < 0) {
        HILOGE("properties or propertyMask is less than 0");
        return false;
    }
    return (static_cast<unsigned int>(properties) & static_cast<unsigned int>(propertyMask)) != 0;
}
napi_value ConvertGattPropertiesToJs(napi_env env, int properties)
{
    napi_value object;
    napi_create_object(env, &object);

    napi_value value;
    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::WRITE), &value);
    napi_set_named_property(env, object, "write", value);

    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::WRITE_WITHOUT_RESPONSE), &value);
    napi_set_named_property(env, object, "writeNoResponse", value);

    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::READ), &value);
    napi_set_named_property(env, object, "read", value);

    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::NOTIFY), &value);
    napi_set_named_property(env, object, "notify", value);

    napi_get_boolean(env, HasProperty(properties, GattCharacteristic::INDICATE), &value);
    napi_set_named_property(env, object, "indicate", value);
    return object;
}

void ConvertBLECharacteristicToJS(napi_env env, napi_value result, GattCharacteristic& characteristic)
{
    napi_value characteristicUuid;
    HILOGI("uuid: %{public}s", characteristic.GetUuid().ToString().c_str());
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
    }

    size_t valueSize = 0;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    {
        napi_value value = nullptr;
        uint8_t* bufferData = nullptr;
        napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
        if (valueSize > 0 && memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
            HILOGE("memcpy_s failed");
            return;
        }
        napi_set_named_property(env, result, "characteristicValue", value);
    }

    napi_value propertiesValue = ConvertGattPropertiesToJs(env, characteristic.GetProperties());
    napi_set_named_property(env, result, "properties", propertiesValue);

    napi_value descriptors;
    napi_create_array(env, &descriptors);
    ConvertBLEDescriptorVectorToJS(env, descriptors, characteristic.GetDescriptors());
    napi_set_named_property(env, result, "descriptors", descriptors);

    napi_value characteristicValueHandle;
    napi_create_uint32(env, static_cast<uint32_t>(characteristic.GetHandle()), &characteristicValueHandle);
    napi_set_named_property(env, result, "characteristicValueHandle", characteristicValueHandle);
}

void ConvertBLEDescriptorVectorToJS(napi_env env, napi_value result, vector<GattDescriptor>& descriptors)
{
    HILOGI("size: %{public}zu", descriptors.size());
    size_t idx = 0;

    if (descriptors.empty()) {
        return;
    }

    for (auto& descriptor : descriptors) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        ConvertBLEDescriptorToJS(env, obj, descriptor);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

void ConvertBLEDescriptorToJS(napi_env env, napi_value result, GattDescriptor& descriptor)
{
    HILOGI("uuid: %{public}s", descriptor.GetUuid().ToString().c_str());

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);

    if (descriptor.GetCharacteristic() != nullptr) {
        napi_value characteristicUuid;
        napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &characteristicUuid);
        napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
        }
    }

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    if (memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }
    napi_set_named_property(env, result, "descriptorValue", value);

    napi_value descriptorHandle;
    napi_create_uint32(env, static_cast<uint32_t>(descriptor.GetHandle()), &descriptorHandle);
    napi_set_named_property(env, result, "descriptorHandle", descriptorHandle);
}

void ConvertCharacteristicReadReqToJS(napi_env env, napi_value result, const std::string &device,
    const GattCharacteristic &characteristic, int requestId)
{
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("offset is %{public}d", 0);

    napi_value characteristicUuid;
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);
    HILOGI("characteristicUuid is %{public}s", characteristic.GetUuid().ToString().c_str());

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
    }
}

void ConvertDescriptorReadReqToJS(napi_env env, napi_value result, const std::string &device,
    const GattDescriptor& descriptor, int requestId)
{
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("offset is %{public}d", 0);

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);
    HILOGI("descriptorUuid is %{public}s", descriptor.GetUuid().ToString().c_str());

    if (descriptor.GetCharacteristic() != nullptr) {
    napi_value characteristicUuid;
    napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
        &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
        }
    }
}

void ConvertCharacteristicWriteReqToJS(napi_env env, napi_value result, const std::string &device,
    const GattCharacteristic& characteristic, int requestId)
{
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("offset is %{public}d", 0);

    napi_value isPrepared;
    napi_get_boolean(env, false, &isPrepared);
    napi_set_named_property(env, result, "isPrepared", isPrepared);

    napi_value needRsp;
    napi_get_boolean(env, characteristic.GetWriteType() == GattCharacteristic::WriteType::DEFAULT, &needRsp);
    napi_set_named_property(env, result, "needRsp", needRsp);

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    if (memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }
    napi_set_named_property(env, result, "value", value);

    napi_value characteristicUuid;
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);
    HILOGI("characteristicUuid is %{public}s", characteristic.GetUuid().ToString().c_str());

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
    }
}

void ConvertDescriptorWriteReqToJS(napi_env env, napi_value result, const std::string &device,
    const GattDescriptor &descriptor, int requestId)
{
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("offset is %{public}d", 0);

    napi_value isPrepared;
    napi_get_boolean(env, false, &isPrepared);
    napi_set_named_property(env, result, "isPrepared", isPrepared);

    napi_value needRsp;
    napi_get_boolean(env, true, &needRsp);
    napi_set_named_property(env, result, "needRsp", needRsp);

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    if (memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
        HILOGE("memcpy_s error");
    }
    napi_set_named_property(env, result, "value", value);

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);
    HILOGI("descriptorUuid is %{public}s", descriptor.GetUuid().ToString().c_str());

    if (descriptor.GetCharacteristic() != nullptr) {
        napi_value characteristicUuid;
        napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &characteristicUuid);
        napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
        }
    }
}

void SetGattClientDeviceId(const std::string &deviceId)
{
    deviceAddr = deviceId;
}

std::string GetGattClientDeviceId()
{
    return deviceAddr;
}

napi_value GetPropertyValueByNamed(napi_env env, napi_value object, std::string_view propertyName, napi_valuetype type)
{
    napi_value value = nullptr;
    bool hasProperty = false;
    napi_valuetype paraType = napi_undefined;

    NAPI_CALL(env, napi_has_named_property(env, object, propertyName.data(), &hasProperty));
    if (hasProperty) {
        NAPI_CALL(env, napi_get_named_property(env, object, propertyName.data(), &value));
        NAPI_CALL(env, napi_typeof(env, value, &paraType));
        if (paraType != type) {
            return NapiGetNull(env);
        }
    }
    return value;
}

void RegisterBLEObserver(napi_env env, napi_value val, int32_t callbackIndex, const std::string &type)
{
    std::shared_ptr<BluetoothCallbackInfo> pCallbackInfo = std::make_shared<BluetoothCallbackInfo>();
    pCallbackInfo->env_ = env;
    napi_create_reference(env, val, 1, &pCallbackInfo->callback_);
    RegisterSysBLEObserver(pCallbackInfo, callbackIndex, type);
}

bool ParseScanParameters(napi_env env, napi_value arg, ScanOptions &info)
{
    napi_value interval = GetPropertyValueByNamed(env, arg, "interval", napi_number);
    if (interval) {
        napi_get_value_int32(env, interval, &info.interval);
        HILOGI("Scan interval is %{public}d", info.interval);
    } else {
        info.interval = 0;
    }

    std::array<std::string, ARGS_SIZE_THREE> funcArray {"success", "fail", "complete"};

    for (size_t i = 0; i < funcArray.size(); i++) {
        napi_value value = GetPropertyValueByNamed(env, arg, funcArray[i], napi_function);
        if (value) {
            RegisterBLEObserver(env, value, i, REGISTER_SYS_BLE_SCAN_TYPE);
        } else {
            UnregisterSysBLEObserver(REGISTER_SYS_BLE_SCAN_TYPE);
            return false;
        }
    }
    return true;
}

napi_status ParseScanFilterDeviceIdParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    std::string deviceId {};
    NAPI_BT_CALL_RETURN(ParseStringParams(env, scanFilter, "deviceId", exist, deviceId));
    if (exist) {
        if (!IsValidAddress(deviceId)) {
            HILOGE("Invalid deviceId: %{public}s", deviceId.c_str());
            return napi_invalid_arg;
        }

        HILOGI("Scan filter device id is %{public}s", GetEncryptAddr(deviceId).c_str());
        bleScanFilter.SetDeviceId(deviceId);
    }
    return napi_ok;
}

napi_status ParseScanFilterLocalNameParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    std::string name {};
    NAPI_BT_CALL_RETURN(ParseStringParams(env, scanFilter, "name", exist, name));
    if (exist) {
        if (name.empty()) {
            HILOGE("name is empty");
            return napi_invalid_arg;
        }
        HILOGI("Scan filter name is %{public}s", name.c_str());
        bleScanFilter.SetName(name);
    }
    return napi_ok;
}

napi_status ParseScanFilterServiceUuidParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    UUID uuid {};
    NAPI_BT_CALL_RETURN(ParseUuidParams(env, scanFilter, "serviceUuid", exist, uuid));
    if (exist) {
        HILOGI("Scan filter serviceUuid is %{public}s", uuid.ToString().c_str());
        bleScanFilter.SetServiceUuid(uuid);
    }

    UUID uuidMask {};
    NAPI_BT_CALL_RETURN(ParseUuidParams(env, scanFilter, "serviceUuidMask", exist, uuidMask));
    if (exist) {
        HILOGI("Scan filter serviceUuidMask is %{public}s", uuidMask.ToString().c_str());
        bleScanFilter.SetServiceUuidMask(uuidMask);
    }
    return napi_ok;
}

napi_status ParseScanFilterSolicitationUuidParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    UUID uuid {};
    NAPI_BT_CALL_RETURN(ParseUuidParams(env, scanFilter, "serviceSolicitationUuid", exist, uuid));
    if (exist) {
        HILOGI("Scan filter serviceSolicitationUuid is %{public}s", uuid.ToString().c_str());
        bleScanFilter.SetServiceSolicitationUuid(uuid);
    }

    UUID uuidMask {};
    NAPI_BT_CALL_RETURN(ParseUuidParams(env, scanFilter, "serviceSolicitationUuidMask", exist, uuidMask));
    if (exist) {
        HILOGI("Scan filter serviceSolicitationUuidMask is %{public}s", uuidMask.ToString().c_str());
        bleScanFilter.SetServiceSolicitationUuidMask(uuidMask);
    }
    return napi_ok;
}

napi_status ParseScanFilterServiceDataParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    std::vector<uint8_t> data {};
    NAPI_BT_CALL_RETURN(ParseArrayBufferParams(env, scanFilter, "serviceData", exist, data));
    if (exist) {
        bleScanFilter.SetServiceData(std::move(data));
    }

    std::vector<uint8_t> dataMask {};
    NAPI_BT_CALL_RETURN(ParseArrayBufferParams(env, scanFilter, "serviceDataMask", exist, dataMask));
    if (exist) {
        bleScanFilter.SetServiceDataMask(std::move(dataMask));
    }
    return napi_ok;
}

napi_status ParseScanFilterManufactureDataParameters(
    const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    bool exist = false;
    napi_value result;
    NAPI_BT_CALL_RETURN(ParseNumberParams(env, scanFilter, "manufactureId", exist, result));
    if (exist) {
        uint32_t manufacturerId = 0;
        NAPI_BT_CALL_RETURN(napi_get_value_uint32(env, result, &manufacturerId));
        bleScanFilter.SetManufacturerId(manufacturerId);
        HILOGI("Scan filter manufacturerId is %{public}#x", manufacturerId);
    }

    exist = false;
    std::vector<uint8_t> data {};
    NAPI_BT_CALL_RETURN(ParseArrayBufferParams(env, scanFilter, "manufactureData", exist, data));
    if (exist) {
        bleScanFilter.SetManufactureData(std::move(data));
    }

    std::vector<uint8_t> dataMask {};
    NAPI_BT_CALL_RETURN(ParseArrayBufferParams(env, scanFilter, "manufactureDataMask", exist, dataMask));
    if (exist) {
        bleScanFilter.SetManufactureDataMask(std::move(dataMask));
    }
    return napi_ok;
}

napi_status ParseScanFilter(const napi_env &env, napi_value &scanFilter, BleScanFilter &bleScanFilter)
{
    HILOGD("enter");
    std::vector<std::string> expectedNames {"deviceId", "name", "serviceUuid", "serviceUuidMask",
        "serviceSolicitationUuid", "serviceSolicitationUuidMask", "serviceData", "serviceDataMask", "manufactureId",
        "manufactureData", "manufactureDataMask"};
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, scanFilter, expectedNames));

    NAPI_BT_CALL_RETURN(ParseScanFilterDeviceIdParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterLocalNameParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterServiceUuidParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterSolicitationUuidParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterServiceDataParameters(env, scanFilter, bleScanFilter));
    NAPI_BT_CALL_RETURN(ParseScanFilterManufactureDataParameters(env, scanFilter, bleScanFilter));
    return napi_ok;
}

napi_status ParseScanFilterParameters(const napi_env &env, napi_value &args, std::vector<BleScanFilter> &params)
{
    HILOGD("enter");
    NAPI_BT_CALL_RETURN(NapiIsArray(env, args));

    uint32_t length = 0;
    NAPI_BT_CALL_RETURN(napi_get_array_length(env, args, &length));
    NAPI_BT_RETURN_IF(length == 0, "Requires array length > 0", napi_invalid_arg);
    for (uint32_t i = 0; i < length; i++) {
        napi_value scanFilter;
        NAPI_BT_CALL_RETURN(napi_get_element(env, args, i, &scanFilter));
        NAPI_BT_CALL_RETURN(NapiIsObject(env, scanFilter));
        BleScanFilter bleScanFilter;
        NAPI_BT_CALL_RETURN(ParseScanFilter(env, scanFilter, bleScanFilter));
        params.push_back(bleScanFilter);
    }
    return napi_ok;
}

napi_status ParseScanParameters(
    const napi_env &env, const napi_callback_info &info, const napi_value &scanArg, ScanOptions &params)
{
    (void)info;
    NAPI_BT_CALL_RETURN(NapiCheckObjectPropertiesName(env, scanArg, {"interval", "dutyMode", "matchMode", "phyType"}));

    bool exist = false;
    int32_t interval = 0;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, scanArg, "interval", exist, interval));
    if (exist) {
        HILOGI("Scan interval is %{public}d", interval);
        params.interval = interval;
    }

    int32_t dutyMode = 0;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, scanArg, "dutyMode", exist, dutyMode));
    if (exist) {
        HILOGI("Scan dutyMode is %{public}d", dutyMode);
        ConvertDutyMode(params, dutyMode);
    }

    int32_t matchMode = 0;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, scanArg, "matchMode", exist, matchMode));
    if (exist) {
        HILOGI("Scan matchMode is %{public}d", matchMode);
        ConvertMatchMode(params, matchMode);
    }

    int32_t phyType = 0;
    NAPI_BT_CALL_RETURN(ParseInt32Params(env, scanArg, "phyType", exist, phyType));
    if (exist) {
        HILOGI("Scan phyType is %{public}d", phyType);
        ConvertPhyType(params, phyType);
    }
    return napi_ok;
}

void ConvertMatchMode(ScanOptions &params, int32_t matchMode)
{
    switch (matchMode) {
        case MatchMode::MATCH_MODE_AGGRESSIVE:
            params.matchMode = MatchMode::MATCH_MODE_AGGRESSIVE;
            break;
        case MatchMode::MATCH_MODE_STICKY:
            params.matchMode = MatchMode::MATCH_MODE_STICKY;
            break;
        default:
            break;
    }
}

void ConvertPhyType(ScanOptions &params, int32_t phyType)
{
    switch (phyType) {
        case static_cast<int32_t>(PhyType::PHY_LE_1M):
            params.phyType = PhyType::PHY_LE_1M;
            break;
        case static_cast<int32_t>(PhyType::PHY_LE_2M):
            params.phyType = PhyType::PHY_LE_2M;
            break;
        case static_cast<int32_t>(PhyType::PHY_LE_CODED):
            params.phyType = PhyType::PHY_LE_CODED;
            break;
        case static_cast<int32_t>(PhyType::PHY_LE_ALL_SUPPORTED):
            params.phyType = PhyType::PHY_LE_ALL_SUPPORTED;
            break;
        default:
            break;
    }
}

void ConvertDutyMode(ScanOptions &params, int32_t dutyMode)
{
    switch (dutyMode) {
        case static_cast<int32_t>(ScanDuty::SCAN_MODE_LOW_POWER):
            params.dutyMode = ScanDuty::SCAN_MODE_LOW_POWER;
            break;
        case static_cast<int32_t>(ScanDuty::SCAN_MODE_BALANCED):
            params.dutyMode = ScanDuty::SCAN_MODE_BALANCED;
            break;
        case static_cast<int32_t>(ScanDuty::SCAN_MODE_LOW_LATENCY):
            params.dutyMode = ScanDuty::SCAN_MODE_LOW_LATENCY;
            break;
        default:
            break;
    }
}

napi_value NapiNativeBleCharacteristic::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_object(env, &object);
    ConvertBLECharacteristicToJS(env, object, const_cast<GattCharacteristic &>(character_));
    return object;
}

napi_value NapiNativeBleDescriptor::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_object(env, &object);
    ConvertBLEDescriptorToJS(env, object, const_cast<GattDescriptor &>(descriptor_));
    return object;
}

napi_value NapiNativeGattServiceArray::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_array(env, &object);
    ConvertGattServiceVectorToJS(env, object, const_cast<vector<GattService> &>(gattServices_));
    return object;
}

napi_value NapiNativeAdvertisingStateInfo::ToNapiValue(napi_env env) const
{
    napi_value object;
    napi_create_object(env, &object);

    napi_value value;
    napi_create_int32(env, advHandle_, &value);
    napi_set_named_property(env, object, "advertisingId", value);

    napi_create_int32(env, advState_, &value);
    napi_set_named_property(env, object, "state", value);
    return object;
}

napi_value NapiNativeGattsCharacterReadRequest::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    ConvertCharacteristicReadReqToJS(env, result, deviceAddr_, character_, transId_);
    return result;
}

napi_value NapiNativeGattsCharacterWriteRequest::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    ConvertCharacteristicWriteReqToJS(env, result, deviceAddr_, character_, transId_);
    return result;
}

napi_value NapiNativeGattsDescriptorWriteRequest::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    ConvertDescriptorWriteReqToJS(env, result, deviceAddr_, descriptor_, transId_);
    return result;
}

napi_value NapiNativeGattsDescriptorReadRequest::ToNapiValue(napi_env env) const
{
    napi_value result = nullptr;
    napi_create_object(env, &result);

    ConvertDescriptorReadReqToJS(env, result, deviceAddr_, descriptor_, transId_);
    return result;
}
}  // namespace Bluetooth
}  // namespace OHOS
