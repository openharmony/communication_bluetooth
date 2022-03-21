/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "napi_bluetooth_utils.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "bluetooth_log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_bluetooth_spp_client.h"
#include "securec.h"

namespace OHOS {
namespace Bluetooth {
using namespace std;

napi_value GetCallbackErrorValue(napi_env env, int errCode)
{
    HILOGE("GetCallbackErrorValue");
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    napi_create_int32(env, errCode, &eCode);
    napi_create_object(env, &result);
    napi_set_named_property(env, result, "code", eCode);
    return result;
}

bool ParseString(napi_env env, string &param, napi_value args)
{
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);

    HILOGI("param=%{public}d.", valuetype);
    if (valuetype != napi_string) {
        HILOGE("Wrong argument type. String expected.");
        return false;
    }
    size_t size = 0;

    if (napi_get_value_string_utf8(env, args, nullptr, 0, &size) != napi_ok) {
        HILOGE("can not get string size");
        param = "";
        return false;
    }
    param.reserve(size + 1);
    param.resize(size);
    if (napi_get_value_string_utf8(env, args, param.data(), (size + 1), &size) != napi_ok) {
        HILOGE("can not get string value");
        param = "";
        return false;
    }
    return true;
}

bool ParseInt32(napi_env env, int32_t &param, napi_value args)
{
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);

    HILOGI("param=%{public}d.", valuetype);
    if (valuetype != napi_number) {
        HILOGE("Wrong argument type. Int32 expected.");
        return false;
    }
    napi_get_value_int32(env, args, &param);
    return true;
}

bool ParseBool(napi_env env, bool &param, napi_value args)
{
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);

    HILOGI("param=%{public}d.", valuetype);
    if (valuetype != napi_boolean) {
        HILOGE("Wrong argument type. bool expected.");
        return false;
    }
    napi_get_value_bool(env, args, &param);
    return true;
}


bool ParseArrayBuffer(napi_env env, uint8_t** data, size_t &size, napi_value args)
{
    napi_status status;
    napi_valuetype valuetype;
    napi_typeof(env, args, &valuetype);

    HILOGI("param=%{public}d.", valuetype);
    if (valuetype != napi_object) {
        HILOGE("Wrong argument type. object expected.");
        return false;
    }

    status = napi_get_arraybuffer_info(env, args, (void**)data, &size);
    if (status != napi_ok) {
        HILOGE("can not get arraybuffer, error is %{public}d", status);
        (*data)[0] = -1;
        return false;
    }
    HILOGE("arraybuffer size is %{public}zu", size);
    HILOGE("arraybuffer is %{public}d", (*data)[0]);
    return true;
}

void ConvertStringVectorToJS(napi_env env, napi_value result, std::vector<std::string>& stringVector)
{
    HILOGI("ConvertStringVectorToJS called");
    size_t idx = 0;

    if (stringVector.empty()) {
        return;
    }
    HILOGI("ConvertStringVectorToJS size is %{public}zu", stringVector.size());
    for (auto& str : stringVector) {
        napi_value obj = nullptr;
        napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &obj);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

void ConvertGattServiceVectorToJS(napi_env env, napi_value result, vector<GattService>& services)
{
    HILOGI("ConverGattServiceVectorToJS called");
    size_t idx = 0;

    if (services.empty()) {
        return;
    }
    HILOGI("ConverGattServiceVectorToJS size is %{public}zu", services.size());
    for (auto& service : services) {
        napi_value obj = nullptr;
        napi_create_object(env, &obj);
        ConvertGattServiceToJS(env, obj, service);
        napi_set_element(env, result, idx, obj);
        idx++;
    }
}

void ConvertGattServiceToJS(napi_env env, napi_value result, GattService& service)
{
    HILOGI("ConvertGattServiceToJS called");

    napi_value serviceUuid;
    napi_create_string_utf8(env, service.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &serviceUuid);
    napi_set_named_property(env, result, "serviceUuid", serviceUuid);
    HILOGI("ConvertGattServiceToJS serviceUuid is %{public}s", service.GetUuid().ToString().c_str());

    napi_value isPrimary;
    napi_get_boolean(env, service.IsPrimary(), &isPrimary);
    napi_set_named_property(env, result, "isPrimary", isPrimary);
    HILOGI("ConvertGattServiceToJS isPrimary is %{public}d", service.IsPrimary());

    napi_value characteristics;
    napi_create_array(env, &characteristics);
    ConvertBLECharacteristicVectorToJS(env, characteristics, service.GetCharacteristics());
    napi_set_named_property(env, result, "characteristics", characteristics);

    napi_value includedServices;
    napi_create_array(env, &includedServices);
    vector<GattService> services;
    vector<std::reference_wrapper<GattService>> srvs = service.GetIncludedServices();
    for (auto &srv : srvs) {
        services.push_back(srv.get());
    }
    ConvertGattServiceVectorToJS(env, includedServices, services);
    napi_set_named_property(env, result, "includedServices", includedServices);
}

void ConvertBLECharacteristicVectorToJS(napi_env env, napi_value result,
    vector<GattCharacteristic>& characteristics)
{
    HILOGI("ConvertBLECharacteristicVectorToJS called");
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

void ConvertBLECharacteristicToJS(napi_env env, napi_value result, GattCharacteristic& characteristic)
{
    HILOGI("ConvertBLECharacteristicToJS called");
    
    napi_value characteristicUuid;
    HILOGI("ConvertBLECharacteristicToJS uuid is %{public}s", characteristic.GetUuid().ToString().c_str());
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
    }

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    if (memcpy_s(bufferData, valueSize, valueData, valueSize) != EOK) {
        HILOGE("ConvertBLECharacteristicToJS memcpy_s failed");
        return;
    }
    napi_set_named_property(env, result, "characteristicValue", value);

    napi_value descriptors;
    napi_create_array(env, &descriptors);
    ConvertBLEDescriptorVectorToJS(env, descriptors, characteristic.GetDescriptors());
    napi_set_named_property(env, result, "descriptors", descriptors);
}


void ConvertBLEDescriptorVectorToJS(napi_env env, napi_value result, vector<GattDescriptor>& descriptors)
{
    HILOGI("ConvertBLEDescriptorVectorToJS called");
    size_t idx = 0;

    if (descriptors.empty()) {
        return;
    }
    HILOGI("ConvertBLEDescriptorVectorToJS size is %{public}zu", descriptors.size());
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
    HILOGI("ConvertBLEDescriptorToJS called");

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);
    HILOGI("ConvertBLEDescriptorToJS descriptorUuid is %{public}s", descriptor.GetUuid().ToString().c_str());
    
    if (descriptor.GetCharacteristic() != nullptr) {
        napi_value characteristicUuid;
        napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &characteristicUuid);
        napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);

        HILOGI("ConvertBLEDescriptorToJS characteristicUuid is %{public}s",
            descriptor.GetCharacteristic()->GetUuid().ToString().c_str());

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
            HILOGI("ConvertBLEDescriptorToJS serviceUuid is %{public}s", 
                descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str());
        }
    }

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    memcpy_s(bufferData, valueSize, valueData, valueSize);
    napi_set_named_property(env, result, "descriptorValue", value);
}

void ConvertCharacteristicReadReqToJS(napi_env env, napi_value result, const std::string &device,
    GattCharacteristic &characteristic, int requestId)
{
    HILOGI("ConvertCharacteristicReadReqToJS called");
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);
    HILOGI("ConvertCharacteristicReadReqToJS deviceId is %{public}s", device.c_str());

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);
    HILOGI("ConvertCharacteristicReadReqToJS transId is %{public}d", requestId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("ConvertCharacteristicReadReqToJS offset is %{public}d", 0);

    napi_value characteristicUuid;
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);
    HILOGI("ConvertCharacteristicReadReqToJS characteristicUuid is %{public}s",
        characteristic.GetUuid().ToString().c_str());

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
        HILOGI("ConvertCharacteristicReadReqToJS serviceUuid is %{public}s",
            characteristic.GetService()->GetUuid().ToString().c_str());
    }
}

void ConvertDescriptorReadReqToJS(napi_env env, napi_value result, const std::string &device,
    GattDescriptor& descriptor, int requestId)
{
    HILOGI("ConvertDescriptorReadReqToJS called");
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);
    HILOGI("ConvertDescriptorReadReqToJS deviceId is %{public}s", device.c_str());

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);
    HILOGI("ConvertDescriptorReadReqToJS transId is %{public}d", requestId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("ConvertDescriptorReadReqToJS offset is %{public}d", 0);

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);
    HILOGI("ConvertDescriptorReadReqToJS descriptorUuid is %{public}s",
        descriptor.GetUuid().ToString().c_str());

    if (descriptor.GetCharacteristic() != nullptr) {
    napi_value characteristicUuid;
    napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
        &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);
    HILOGI("ConvertDescriptorReadReqToJS characteristicUuid is %{public}s",
        descriptor.GetCharacteristic()->GetUuid().ToString().c_str());

        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(), 
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
            HILOGI("ConvertDescriptorReadReqToJS serviceUuid is %{public}s",
                descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str());
        }
    }
}

void ConvertCharacteristicWriteReqToJS(napi_env env, napi_value result, const std::string &device,
    GattCharacteristic& characteristic, int requestId)
{
    HILOGI("ConvertCharacteristicWriteReqToJS called");
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);
    HILOGI("ConvertCharacteristicWriteReqToJS deviceId is %{public}s", device.c_str());

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);
    HILOGI("ConvertCharacteristicWriteReqToJS transId is %{public}d", requestId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("ConvertCharacteristicWriteReqToJS offset is %{public}d", 0);


    napi_value isPrep;
    napi_get_boolean(env, false, &isPrep);
    napi_set_named_property(env, result, "isPrep", isPrep);

    napi_value needRsp;
    napi_get_boolean(env, true, &needRsp);
    napi_set_named_property(env, result, "needRsp", needRsp);

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = characteristic.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    memcpy_s(bufferData, valueSize, valueData, valueSize);
    napi_set_named_property(env, result, "value", value);

    napi_value characteristicUuid;
    napi_create_string_utf8(env, characteristic.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &characteristicUuid);
    napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);
    HILOGI("ConvertCharacteristicWriteReqToJS characteristicUuid is %{public}s",
        characteristic.GetUuid().ToString().c_str());

    if (characteristic.GetService() != nullptr) {
        napi_value serviceUuid;
        napi_create_string_utf8(env, characteristic.GetService()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &serviceUuid);
        napi_set_named_property(env, result, "serviceUuid", serviceUuid);
        HILOGI("ConvertCharacteristicWriteReqToJS serviceUuid is %{public}s",
            characteristic.GetService()->GetUuid().ToString().c_str());
    }
}

void ConvertDescriptorWriteReqToJS(napi_env env, napi_value result, const std::string &device,
    GattDescriptor &descriptor, int requestId)
{
    HILOGI("ConvertDescriptorWriteReqToJS called");
    napi_value deviceId;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);
    HILOGI("ConvertCharacteristicWriteReqToJS deviceId is %{public}s", device.c_str());

    napi_value transId;
    napi_create_int32(env, requestId, &transId);
    napi_set_named_property(env, result, "transId", transId);
    HILOGI("ConvertCharacteristicWriteReqToJS transId is %{public}d", requestId);

    napi_value offset;
    napi_create_int32(env, 0, &offset);
    napi_set_named_property(env, result, "offset", offset);
    HILOGI("ConvertCharacteristicWriteReqToJS offset is %{public}d", 0);

    napi_value isPrep;
    napi_get_boolean(env, false, &isPrep);
    napi_set_named_property(env, result, "isPrep", isPrep);

    napi_value needRsp;
    napi_get_boolean(env, true, &needRsp);
    napi_set_named_property(env, result, "needRsp", needRsp);

    napi_value value;
    size_t valueSize;
    uint8_t* valueData = descriptor.GetValue(&valueSize).get();
    uint8_t* bufferData = nullptr;
    napi_create_arraybuffer(env, valueSize, (void**)&bufferData, &value);
    memcpy_s(bufferData, valueSize, valueData, valueSize);
    napi_set_named_property(env, result, "value", value);

    napi_value descriptorUuid;
    napi_create_string_utf8(env, descriptor.GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH, &descriptorUuid);
    napi_set_named_property(env, result, "descriptorUuid", descriptorUuid);
    HILOGI("ConvertDescriptorWriteReqToJS descriptorUuid is %{public}s",
        descriptor.GetUuid().ToString().c_str());

    if (descriptor.GetCharacteristic() != nullptr) {
        napi_value characteristicUuid;
        napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetUuid().ToString().c_str(), NAPI_AUTO_LENGTH,
            &characteristicUuid);
        napi_set_named_property(env, result, "characteristicUuid", characteristicUuid);
        HILOGI("ConvertDescriptorWriteReqToJS characteristicUuid is %{public}s",
            descriptor.GetCharacteristic()->GetUuid().ToString().c_str());
        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            napi_value serviceUuid;
            napi_create_string_utf8(env, descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str(),
                NAPI_AUTO_LENGTH, &serviceUuid);
            napi_set_named_property(env, result, "serviceUuid", serviceUuid);
            HILOGI("ConvertDescriptorWriteReqToJS serviceUuid is %{public}s",
                descriptor.GetCharacteristic()->GetService()->GetUuid().ToString().c_str());
        }
    }
}

void ConvertStateChangeParamToJS(napi_env env, napi_value result, const std::string &device, int state)
{
    HILOGI("ConvertStateChangeParamToJS called");
    napi_value deviceId = nullptr;
    napi_create_string_utf8(env, device.c_str(), NAPI_AUTO_LENGTH, &deviceId);
    napi_set_named_property(env, result, "deviceId", deviceId);
    HILOGI("ConvertStateChangeParamToJS deviceId is %{public}s", device.c_str());

    napi_value statue = nullptr;
    napi_create_int32(env, state, &statue);
    napi_set_named_property(env, result, "state", statue);
    HILOGI("ConvertStateChangeParamToJS state is %{public}d", state);
}

void GetServiceVectorFromJS(napi_env env, napi_value object, vector<GattService>& services,
    std::shared_ptr<GattServer> server, std::shared_ptr<GattClient> client)
{
    HILOGI("GetServiceVectorFromJS called");
    size_t idx = 0;
    bool hasElement = false;
    HILOGI("GetServiceVectorFromJS size is %{public}zu", services.size());
    napi_has_element(env, object, idx, &hasElement);
    while (hasElement) {
        napi_value result = nullptr;
        napi_create_object(env, &result);
        napi_get_element(env, object, idx, &result);
        GattService* service = GetServiceFromJS(env, result, nullptr, nullptr);
        services.push_back(*service);
        delete service;
        idx++;
        napi_has_element(env, object, idx, &hasElement);
    }
}

GattService* GetServiceFromJS(napi_env env, napi_value object, std::shared_ptr<GattServer> server,
    std::shared_ptr<GattClient> client)
{
    HILOGI("GetServiceFromJS called");
    string serviceUuid;
    bool isPrimary = false;

    napi_value propertyNameValue = nullptr;
    napi_value value = nullptr;
    bool hasProperty = false;

    napi_create_string_utf8(env, "serviceUuid", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseString(env, serviceUuid, value);
    HILOGI("serviceUuid is %{public}s", serviceUuid.c_str());

    napi_create_string_utf8(env, "isPrimary", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseBool(env, isPrimary, value);
    HILOGI("isPrimary is %{public}d", isPrimary);

    GattServiceType serviceType = GattServiceType::PRIMARY;
    if (!isPrimary) {
        serviceType = GattServiceType::SECONDARY;
    }

    GattService* service;
    if (server == nullptr && client == nullptr) {
        service =  new GattService(UUID::FromString(serviceUuid), serviceType);

        napi_create_string_utf8(env, "characteristics", NAPI_AUTO_LENGTH, &propertyNameValue);
        napi_has_property(env, object, propertyNameValue, &hasProperty);
        if (hasProperty) {
            napi_get_property(env, object, propertyNameValue, &value);
            vector<GattCharacteristic> characteristics;
            GetCharacteristicVectorFromJS(env, value, characteristics, server, client);
            for (auto& characteristic : characteristics) {
                service->AddCharacteristic(characteristic);
            }
        }

        napi_create_string_utf8(env, "includeServices", NAPI_AUTO_LENGTH, &propertyNameValue);
        napi_has_property(env, object, propertyNameValue, &hasProperty);
        if (hasProperty) {
            napi_get_property(env, object, propertyNameValue, &value);
            vector<GattService> services;
            GetServiceVectorFromJS(env, value, services, server, client);
            for (auto& serv : services) {
                service->AddService(serv);
            }
        }
    } else {
        std::optional<std::reference_wrapper<GattService>> obtainedService;
        if (server != nullptr) {
            obtainedService = server->GetService(UUID::FromString(serviceUuid), isPrimary);
        } else if (client != nullptr) {
            if (client->DiscoverServices()) {
                obtainedService = client->GetService(UUID::FromString(serviceUuid));
            } else {
                return nullptr;
            }
        }
        
        if (obtainedService == std::nullopt) {
            return nullptr;
        } else {
            service = &(obtainedService->get());
        }
    }
    return service;
}

void GetCharacteristicVectorFromJS(napi_env env, napi_value object, vector<GattCharacteristic>& characteristics,
    std::shared_ptr<GattServer> server, std::shared_ptr<GattClient> client)
{
    HILOGI("GetCharacteristicVectorFromJS called");
    size_t idx = 0;

    bool hasElement = false;
    napi_has_element(env, object, idx, &hasElement);
    HILOGI("GetCharacteristicVectorFromJS size is %{public}zu", characteristics.size());
    while(hasElement) {
        napi_value result = nullptr;
        napi_create_object(env, &result);
        napi_get_element(env, object, idx, &result);
        GattCharacteristic* characteristic = GetCharacteristicFromJS(env, result, server, client);
        characteristics.push_back(*characteristic);
        if (server == nullptr && client == nullptr) {
            delete characteristic;
        }
        idx++;
        napi_has_element(env, object, idx, &hasElement);
    };
}

GattCharacteristic* GetCharacteristicFromJS(napi_env env, napi_value object, std::shared_ptr<GattServer> server,
    std::shared_ptr<GattClient> client)
{
    HILOGI("GetCharacteristicFromJS called");
    string serviceUuid;
    string characteristicUuid;
    uint8_t *characteristicValue = nullptr;
    size_t characteristicValueSize = 0;
    
    napi_value propertyNameValue = nullptr;
    napi_value value = nullptr;
    bool hasProperty = false;

    napi_create_string_utf8(env, "serviceUuid", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseString(env, serviceUuid, value);
    HILOGI("serviceUuid is %{public}s", serviceUuid.c_str());

    napi_create_string_utf8(env, "characteristicUuid", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseString(env, characteristicUuid, value);
    HILOGI("characteristicUuid is %{public}s", characteristicUuid.c_str());

    GattCharacteristic* characteristic = nullptr;
    std::optional<std::reference_wrapper<GattService>> service = nullopt;

    if (server == nullptr && client == nullptr) {
        characteristic = new GattCharacteristic(UUID::FromString(characteristicUuid),
            (GattCharacteristic::Permission::READABLE | GattCharacteristic::Permission::WRITEABLE),
                GattCharacteristic::Propertie::NOTIFY);

        napi_create_string_utf8(env, "descriptors", NAPI_AUTO_LENGTH, &propertyNameValue);
        napi_has_property(env, object, propertyNameValue, &hasProperty);
        if (hasProperty) {
            napi_get_property(env, object, propertyNameValue, &value);
            vector<GattDescriptor> descriptors;
            GetDescriptorVectorFromJS(env, value, descriptors);
            for (auto& descriptor : descriptors) {
                characteristic->AddDescriptor(descriptor);
        }
    }
    } else {
        if (server != nullptr) {
            service = server->GetService(UUID::FromString(serviceUuid), true);
            if (service == std::nullopt) {
                service = server->GetService(UUID::FromString(serviceUuid), false);
            }
        } else if (client != nullptr) {
            service = client->GetService(UUID::FromString(serviceUuid));
        }
        
        if (service == std::nullopt) {
            return nullptr;
        } else {
            characteristic = service->get().GetCharacteristic(UUID::FromString(characteristicUuid));
        }
    }
 
    if (characteristic == nullptr) {
        return nullptr;
    }
    napi_create_string_utf8(env, "characteristicValue", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    if (!ParseArrayBuffer(env, &characteristicValue, characteristicValueSize, value)) {
        HILOGE("ParseArrayBuffer failed");
    } else {
        HILOGI("characteristicValue is %{public}d", characteristicValue[0]);
    }
    characteristic->SetValue(characteristicValue, characteristicValueSize);
    
    return characteristic;
}

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_int32(env, objName, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value RegisterObserver(napi_env env, napi_callback_info info)
{
    HILOGI("RegisterObserver called");
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == expectedArgsCount + 1) {
        NapiSppClient::On(env, info);
    } else {
        NAPI_ASSERT(env, argc == expectedArgsCount, "Requires 2 arguments.");
        std::string type;
        ParseString(env, type, argv[PARAM0]);
        std::shared_ptr<BluetoothCallbackInfo> pCallbackInfo = std::make_shared<BluetoothCallbackInfo>();
        pCallbackInfo->env_ = env;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM1], &valueType);
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[PARAM1], 1, &pCallbackInfo->callback_);
        g_Observer[type] = pCallbackInfo;

        HILOGI("%{public}s is registered", type.c_str());
    }
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    HILOGI("RegisterObserver called end");
    return ret;
}

napi_value DeregisterObserver(napi_env env, napi_callback_info info)
{
    HILOGI("DeregisterObserver called");
    size_t expectedArgsCount = ARGS_SIZE_TWO;
    size_t argc = expectedArgsCount;
    napi_value argv[ARGS_SIZE_TWO] = {0};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == expectedArgsCount + 1) {
        NapiSppClient::Off(env, info);
    } else {
        NAPI_ASSERT(env, argc == expectedArgsCount, "Requires 2 argument.");
        std::string type;
        ParseString(env, type, argv[PARAM0]);

        std::shared_ptr<BluetoothCallbackInfo> pCallbackInfo = std::make_shared<BluetoothCallbackInfo>();
        pCallbackInfo->env_ = env;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM1], &valueType);
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[PARAM1], 1, &pCallbackInfo->callback_);
        if (pCallbackInfo != nullptr) {
            napi_value callback = 0;
            napi_value undefined = 0;
            napi_value callResult = 0;
            napi_get_undefined(pCallbackInfo->env_, &undefined);
            napi_value result = NapiGetNull(pCallbackInfo->env_);
            napi_get_reference_value(pCallbackInfo->env_, pCallbackInfo->callback_, &callback);
            napi_call_function(pCallbackInfo->env_, undefined, callback, ARGS_SIZE_ONE, &result, &callResult);
        }

        if (g_Observer.find(type) != g_Observer.end()) {
            g_Observer[type] = nullptr;
        }
    }
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    HILOGI("DeregisterObserver called end");
    return ret;
}

std::map<std::string, std::shared_ptr<BluetoothCallbackInfo>> GetObserver()
{
    return g_Observer;
}

void GetDescriptorVectorFromJS(napi_env env, napi_value object, vector<GattDescriptor>& descriptors)
{
    HILOGI("GetDescriptorVectorFromJS called");

    size_t idx = 0;
    bool hasElement = false;
    napi_has_element(env, object, idx, &hasElement);

    HILOGI("GetDescriptorVectorFromJS size is %{public}zu", descriptors.size());

    while (hasElement) {
        napi_value result = nullptr;
        napi_create_object(env, &result);
        napi_get_element(env, object, idx, &result);
        GattDescriptor* descriptor = GetDescriptorFromJS(env, result, nullptr, nullptr);
        descriptors.push_back(*descriptor);
        delete descriptor;
        idx++;
        napi_has_element(env, object, idx, &hasElement);
    }
}

GattDescriptor* GetDescriptorFromJS(napi_env env, napi_value object, std::shared_ptr<GattServer> server,
    std::shared_ptr<GattClient> client)
{
    HILOGI("GetDescriptorFromJS called");
    
    string serviceUuid;
    string characteristicUuid;
    string descriptorUuid;
    uint8_t *descriptorValue = nullptr;
    size_t descriptorValueSize = 0;

    napi_value propertyNameValue = nullptr;
    napi_value value = nullptr;

    napi_create_string_utf8(env, "serviceUuid", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseString(env, serviceUuid, value);
    HILOGI("descriptorUuid is %{public}s", serviceUuid.c_str());

    napi_create_string_utf8(env, "characteristicUuid", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseString(env, characteristicUuid, value);
    HILOGI("descriptorUuid is %{public}s", characteristicUuid.c_str());

    napi_create_string_utf8(env, "descriptorUuid", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseString(env, descriptorUuid, value);
    HILOGI("descriptorUuid is %{public}s", descriptorUuid.c_str());
    
    GattDescriptor* descriptor = nullptr;
    GattCharacteristic* characteristic = nullptr;
    std::optional<std::reference_wrapper<GattService>> service = nullopt;

    if (server == nullptr && client == nullptr) {
        descriptor = new GattDescriptor(UUID::FromString(descriptorUuid),
            GattCharacteristic::Permission::READABLE | GattCharacteristic::Permission::WRITEABLE);
    } else {
        if (server != nullptr) {
            service = server->GetService(UUID::FromString(serviceUuid), true);
            if (service == std::nullopt) {
                service = server->GetService(UUID::FromString(serviceUuid), false);
            }
        } else if (client != nullptr) {
            service = client->GetService(UUID::FromString(serviceUuid));
        }
        
        if (service == std::nullopt) {
            return nullptr;
        } else {
            characteristic = service->get().GetCharacteristic(UUID::FromString(characteristicUuid));
        }

        if (characteristic == nullptr) {
            return nullptr;
        } else {
            descriptor = characteristic->GetDescriptor(UUID::FromString(descriptorUuid));
        }
    }

    if (descriptor == nullptr) {
        return nullptr;
    }

    napi_create_string_utf8(env, "descriptorValue", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    if (!ParseArrayBuffer(env, &descriptorValue, descriptorValueSize, value)) {
        HILOGE("ParseArrayBuffer failed");
    } else {
        HILOGI("descriptorValue is %{public}d", descriptorValue[0]);
    }
    descriptor->SetValue(descriptorValue, descriptorValueSize);

    return descriptor;
}

ServerResponse GetServerResponseFromJS(napi_env env, napi_value object)
{
    HILOGI("GetServerResponseFromJS called");
    ServerResponse serverResponse;
    napi_value propertyNameValue = nullptr;
    napi_value value = nullptr;
    uint8_t *values = nullptr;
    size_t valuesSize = 0;

    napi_create_string_utf8(env, "deviceId", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseString(env, serverResponse.deviceId, value);
    HILOGI("deviceId is %{public}s", serverResponse.deviceId.c_str());

    napi_create_string_utf8(env, "transId", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseInt32(env, serverResponse.transId, value);
    HILOGI("transId is %{public}d", serverResponse.transId);

    napi_create_string_utf8(env, "status", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseInt32(env, serverResponse.status, value);
    HILOGI("status is %{public}d", serverResponse.status);

    napi_create_string_utf8(env, "offset", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseInt32(env, serverResponse.offset, value);
    HILOGI("offset is %{public}d", serverResponse.offset);

    napi_create_string_utf8(env, "value", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    if (!ParseArrayBuffer(env, &values, valuesSize, value)) {
        HILOGE("ParseArrayBuffer failed");
    } else {
        HILOGI("value is %{public}d", values[0]);
    }
    serverResponse.SetValue(values, valuesSize);

    return serverResponse;
}

std::shared_ptr<SppOption> GetSppOptionFromJS(napi_env env, napi_value object)
{
    HILOGI("GetSppOptionFromJS called");
    std::shared_ptr<SppOption> sppOption = std::make_shared<SppOption>();
    napi_value propertyNameValue = nullptr;
    napi_value value = nullptr;

    napi_create_string_utf8(env, "uuid", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseString(env, sppOption->uuid_, value);
    HILOGI("uuid is %{public}s", sppOption->uuid_.c_str());

    napi_create_string_utf8(env, "secure", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseBool(env, sppOption->secure_, value);
    HILOGI("secure is %{public}d", sppOption->secure_);

    int type;
    napi_create_string_utf8(env, "type", NAPI_AUTO_LENGTH, &propertyNameValue);
    napi_get_property(env, object, propertyNameValue, &value);
    ParseInt32(env, type, value);
    sppOption->type_ = SppSocketType(type);
    HILOGI("type is %{public}d", sppOption->type_);

    return sppOption;
}

void SetGattClinetDeviceId(const std::string &deviceId)
{
    deviceAddr = deviceId;
}

std::string GetGattClientDeviceId()
{
    return deviceAddr;
}

void SetRssiValueCallbackInfo(std::shared_ptr<GattGetRssiValueCallbackInfo> &callback)
{
    callbackInfo = callback;
}

std::shared_ptr<GattGetRssiValueCallbackInfo> GetRssiValueCallbackInfo()
{
    return callbackInfo;
}
}  // namespace Bluetooth
}  // namespace OHOS
