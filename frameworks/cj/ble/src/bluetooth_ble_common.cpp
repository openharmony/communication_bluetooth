/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bluetooth_ble_common.h"

#include "bluetooth_ble_ffi.h"
#include "bluetooth_log.h"

#include <regex>
#include <vector>

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BTConnectState;
using Bluetooth::GattCharacteristic;
using Bluetooth::GattDescriptor;
using Bluetooth::GattPermission;
using Bluetooth::GattService;
using namespace std;

namespace {
std::string g_deviceAddr;
} // namespace

char *MallocCString(const std::string &origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto length = origin.length() + 1;
    char *res = static_cast<char *>(malloc(sizeof(char) * length));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), length);
}

CArrString Convert2CArrString(std::vector<std::string> &tids)
{
    CArrString res{};
    if (tids.empty()) {
        return res;
    }

    size_t size = tids.size();
    if (size == 0 || size > std::numeric_limits<size_t>::max() / sizeof(char *)) {
        return res;
    }
    res.head = static_cast<char **>(malloc(sizeof(char *) * size));
    if (res.head == nullptr) {
        return res;
    }

    size_t i = 0;
    for (; i < size; ++i) {
        res.head[i] = MallocCString(tids[i]);
    }
    res.size = static_cast<int64_t>(i);

    return res;
}

CArrUI8 Convert2CArrUI8(std::vector<uint8_t> vec)
{
    CArrUI8 res{};
    if (vec.empty()) {
        return res;
    }
    size_t size = vec.size();
    if (size == 0 || size > std::numeric_limits<size_t>::max() / sizeof(uint8_t)) {
        return res;
    }
    res.head = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * vec.size()));
    if (res.head == nullptr) {
        return res;
    }
    size_t i = 0;
    for (; i < vec.size(); i++) {
        res.head[i] = vec[i];
    }
    res.size = static_cast<int64_t>(i);
    return res;
}

NativeBLEDescriptor ConvertBLEDescriptorToCJ(GattDescriptor &descriptor)
{
    NativeBLEDescriptor outDescriptor{};
    outDescriptor.descriptorUuid = MallocCString(descriptor.GetUuid().ToString());
    if (descriptor.GetCharacteristic() != nullptr) {
        outDescriptor.characteristicUuid = MallocCString(descriptor.GetCharacteristic()->GetUuid().ToString());
        if (descriptor.GetCharacteristic()->GetService() != nullptr) {
            outDescriptor.serviceUuid =
                MallocCString(descriptor.GetCharacteristic()->GetService()->GetUuid().ToString());
        }
    }

    size_t valueSize;
    uint8_t *valueData = descriptor.GetValue(&valueSize).get();
    vector<uint8_t> vec(valueData, valueData + valueSize);
    outDescriptor.descriptorValue = Convert2CArrUI8(vec);
    return outDescriptor;
}

CArrBLEDescriptor Convert2CArrBLEDescriptor(vector<GattDescriptor> &descriptors)
{
    CArrBLEDescriptor res{};
    if (descriptors.empty()) {
        return res;
    }
    size_t size = descriptors.size();
    if (size == 0 || size > (std::numeric_limits<size_t>::max() / sizeof(NativeBLEDescriptor))) {
        return res;
    }
    res.head = static_cast<NativeBLEDescriptor *>(malloc(sizeof(NativeBLEDescriptor) * size));
    if (res.head == nullptr) {
        return res;
    }
    size_t i = 0;
    for (; i < descriptors.size(); i++) {
        res.head[i] = ConvertBLEDescriptorToCJ(descriptors[i]);
    }
    res.size = static_cast<int64_t>(i);
    return res;
}

bool HasProperty(int properties, int propertyMask)
{
    if (properties < 0 || propertyMask < 0) {
        HILOGE("properties or propertyMask is less than 0");
        return false;
    }
    return (static_cast<unsigned int>(properties) & static_cast<unsigned int>(propertyMask)) != 0;
}

NativeGattProperties ConvertGattPropertiesToCJ(int properties)
{
    NativeGattProperties outProperties{};
    outProperties.write = HasProperty(properties, GattCharacteristic::WRITE);
    outProperties.writeNoResponse = HasProperty(properties, GattCharacteristic::WRITE_WITHOUT_RESPONSE);
    outProperties.read = HasProperty(properties, GattCharacteristic::READ);
    outProperties.notify = HasProperty(properties, GattCharacteristic::NOTIFY);
    outProperties.indicate = HasProperty(properties, GattCharacteristic::INDICATE);
    return outProperties;
}

NativeBLECharacteristic ConvertBLECharacteristicToCJ(GattCharacteristic &characteristic)
{
    NativeBLECharacteristic outCharacteristic{};
    outCharacteristic.characteristicUuid = MallocCString(characteristic.GetUuid().ToString());
    if (characteristic.GetService() != nullptr) {
        outCharacteristic.serviceUuid = MallocCString(characteristic.GetService()->GetUuid().ToString());
    }

    size_t valueSize = 0;
    uint8_t *valueData = characteristic.GetValue(&valueSize).get();
    vector<uint8_t> vec(valueData, valueData + valueSize);
    outCharacteristic.characteristicValue = Convert2CArrUI8(vec);

    outCharacteristic.descriptors = Convert2CArrBLEDescriptor(characteristic.GetDescriptors());

    outCharacteristic.properties = ConvertGattPropertiesToCJ(characteristic.GetProperties());

    return outCharacteristic;
}

CArrBLECharacteristic Convert2CArrBLECharacteristic(std::vector<GattCharacteristic> characteristics)
{
    CArrBLECharacteristic res{};
    if (characteristics.empty()) {
        return res;
    }
    size_t size = characteristics.size();
    if (size == 0 || size > std::numeric_limits<size_t>::max() / sizeof(NativeBLECharacteristic)) {
        return res;
    }
    res.head = static_cast<NativeBLECharacteristic *>(malloc(sizeof(NativeBLECharacteristic) * size));
    if (res.head == nullptr) {
        return res;
    }
    size_t i = 0;
    for (; i < characteristics.size(); i++) {
        res.head[i] = ConvertBLECharacteristicToCJ(characteristics[i]);
    }
    res.size = static_cast<int64_t>(i);
    return res;
}

NativeGattService ConvertGattServiceToCJ(GattService service)
{
    NativeGattService outService{};
    outService.serviceUuid = MallocCString(service.GetUuid().ToString());
    outService.isPrimary = service.IsPrimary();
    outService.characteristics = Convert2CArrBLECharacteristic(service.GetCharacteristics());
    vector<GattService> services;
    vector<std::reference_wrapper<GattService>> srvs = service.GetIncludedServices();
    for (auto &srv : srvs) {
        services.push_back(srv.get());
    }
    outService.includeServices = Convert2CArrGattService(services);
    return outService;
}

CArrGattService Convert2CArrGattService(std::vector<GattService> services)
{
    CArrGattService res{};
    if (services.empty()) {
        return res;
    }
    size_t size = services.size();
    if (size == 0 || size > std::numeric_limits<size_t>::max() / sizeof(NativeGattService)) {
        return res;
    }
    res.head = static_cast<NativeGattService *>(malloc(sizeof(NativeGattService) * services.size()));
    if (res.head == nullptr) {
        return res;
    }
    size_t i = 0;
    for (; i < services.size(); i++) {
        res.head[i] = ConvertGattServiceToCJ(services[i]);
    }
    res.size = static_cast<int64_t>(i);
    return res;
}

std::string GetGattClientDeviceId()
{
    return g_deviceAddr;
}

uint16_t ConvertGattPermissions(const NativeGattPermission &nativePermissions)
{
    uint16_t permissions = 0;
    if (nativePermissions.readable) {
        permissions |= static_cast<uint16_t>(GattPermission::READABLE);
    }
    if (nativePermissions.writeable) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITEABLE);
    }
    if (nativePermissions.readEncrypted) {
        permissions |= static_cast<uint16_t>(GattPermission::READ_ENCRYPTED_MITM);
    }
    if (nativePermissions.writeEncrypted) {
        permissions |= static_cast<uint16_t>(GattPermission::WRITE_ENCRYPTED_MITM);
    }
    return permissions;
}

uint16_t ConvertGattProperties(const NativeGattProperties &nativeProperties)
{
    uint16_t properties = 0;
    if (nativeProperties.read) {
        properties |= static_cast<uint16_t>(GattCharacteristic::READ);
    }
    if (nativeProperties.write) {
        properties |= static_cast<uint16_t>(GattCharacteristic::WRITE);
    }
    if (nativeProperties.writeNoResponse) {
        properties |= static_cast<uint16_t>(GattCharacteristic::WRITE_WITHOUT_RESPONSE);
    }
    if (nativeProperties.notify) {
        properties |= static_cast<uint16_t>(GattCharacteristic::NOTIFY);
    }
    if (nativeProperties.indicate) {
        properties |= static_cast<uint16_t>(GattCharacteristic::INDICATE);
    }
    return properties;
}

int GetProfileConnectionState(int state)
{
    int32_t profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
    switch (state) {
        case static_cast<int32_t>(BTConnectState::CONNECTING):
            HILOGD("STATE_CONNECTING(1)");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::CONNECTED):
            HILOGD("STATE_CONNECTED(2)");
            profileConnectionState = ProfileConnectionState::STATE_CONNECTED;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTING):
            HILOGD("STATE_DISCONNECTING(3)");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTING;
            break;
        case static_cast<int32_t>(BTConnectState::DISCONNECTED):
            HILOGD("STATE_DISCONNECTED(0)");
            profileConnectionState = ProfileConnectionState::STATE_DISCONNECTED;
            break;
        default:
            break;
    }
    return profileConnectionState;
}

void FreeNativeBLECharacteristic(NativeBLECharacteristic characteristic)
{
    free(characteristic.characteristicUuid);
    free(characteristic.serviceUuid);
    for (int64_t i = 0; i < characteristic.descriptors.size; i++) {
        NativeBLEDescriptor descriptor = characteristic.descriptors.head[i];
        FreeNativeBLEDescriptor(descriptor);
    }
    free(characteristic.descriptors.head);
    characteristic.characteristicUuid = nullptr;
    characteristic.serviceUuid = nullptr;
    characteristic.descriptors.head = nullptr;
}

void FreeNativeBLEDescriptor(NativeBLEDescriptor descriptor)
{
    free(descriptor.descriptorUuid);
    free(descriptor.characteristicUuid);
    free(descriptor.serviceUuid);
    free(descriptor.descriptorValue.head);
    descriptor.descriptorUuid = nullptr;
    descriptor.characteristicUuid = nullptr;
    descriptor.serviceUuid = nullptr;
    descriptor.descriptorValue.head = nullptr;
}

} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS