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

napi_value ScanReportTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value scanReportTypeObj = nullptr;
    napi_create_object(env, &scanReportTypeObj);
    SetNamedPropertyByInteger(
        env, scanReportTypeObj, static_cast<int32_t>(ScanReportType::ON_FOUND), "ON_FOUND");
    SetNamedPropertyByInteger(
        env, scanReportTypeObj, static_cast<int32_t>(ScanReportType::ON_LOST), "ON_LOST");
    return scanReportTypeObj;
}

napi_value ScanDutyInit(napi_env env)
{
    HILOGD("enter");
    napi_value scanDutyObj = nullptr;
    napi_create_object(env, &scanDutyObj);
    SetNamedPropertyByInteger(
        env, scanDutyObj, static_cast<int>(ScanDuty::SCAN_MODE_LOW_POWER), "SCAN_MODE_LOW_POWER");
    SetNamedPropertyByInteger(
        env, scanDutyObj, static_cast<int>(ScanDuty::SCAN_MODE_BALANCED), "SCAN_MODE_BALANCED");
    SetNamedPropertyByInteger(
        env, scanDutyObj, static_cast<int>(ScanDuty::SCAN_MODE_LOW_LATENCY), "SCAN_MODE_LOW_LATENCY");
    return scanDutyObj;
}

napi_value MatchModeInit(napi_env env)
{
    HILOGD("enter");
    napi_value matchModeObj = nullptr;
    napi_create_object(env, &matchModeObj);
    SetNamedPropertyByInteger(
        env, matchModeObj, static_cast<int>(MatchMode::MATCH_MODE_AGGRESSIVE), "MATCH_MODE_AGGRESSIVE");
    SetNamedPropertyByInteger(
        env, matchModeObj, static_cast<int>(MatchMode::MATCH_MODE_STICKY), "MATCH_MODE_STICKY");
    return matchModeObj;
}

napi_value PhyTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value phyTypeObj = nullptr;
    napi_create_object(env, &phyTypeObj);
    SetNamedPropertyByInteger(
        env, phyTypeObj, static_cast<int32_t>(PhyType::PHY_LE_1M), "PHY_LE_1M");
    SetNamedPropertyByInteger(
        env, phyTypeObj, static_cast<int32_t>(PhyType::PHY_LE_ALL_SUPPORTED), "PHY_LE_ALL_SUPPORTED");
    return phyTypeObj;
}

napi_value ScanReportModeInit(napi_env env)
{
    HILOGD("enter");
    napi_value reportModeObj = nullptr;
    napi_create_object(env, &reportModeObj);
    SetNamedPropertyByInteger(
        env, reportModeObj, static_cast<int32_t>(ScanReportMode::NORMAL), "NORMAL");
    SetNamedPropertyByInteger(
        env, reportModeObj, static_cast<int32_t>(ScanReportMode::FENCE_SENSITIVITY_LOW), "FENCE_SENSITIVITY_LOW");
    SetNamedPropertyByInteger(
        env, reportModeObj, static_cast<int32_t>(ScanReportMode::FENCE_SENSITIVITY_HIGH), "FENCE_SENSITIVITY_HIGH");
    return reportModeObj;
}

void SetGattClientDeviceId(const std::string &deviceId)
{
    deviceAddr = deviceId;
}

std::string GetGattClientDeviceId()
{
    return deviceAddr;
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
