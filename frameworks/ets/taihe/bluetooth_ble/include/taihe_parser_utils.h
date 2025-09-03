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

#ifndef TAIHE_PARSER_UTILS_H
#define TAIHE_PARSER_UTILS_H

#include "bluetooth_gatt_characteristic.h"
#include "taihe_bluetooth_utils.h"
#include "ohos.bluetooth.ble.proj.hpp"
#include "ohos.bluetooth.ble.impl.hpp"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {
struct AniBleDescriptor {
    UUID serviceUuid;
    UUID characteristicUuid;
    UUID descriptorUuid;
    std::vector<uint8_t> descriptorValue;
    uint16_t descriptorHandle;
    int permissions;
};

struct AniBleCharacteristic {
    UUID serviceUuid;
    UUID characteristicUuid;
    std::vector<uint8_t> characteristicValue;
    std::vector<AniBleDescriptor> descriptors;
    int properties;
    uint16_t characteristicValueHandle;
    int permissions;
};

struct FileHolder {
    std::string filePath = "";
    int64_t fileSize = 0;
    int32_t fileFd = -1;
};

struct AniGattService {
    UUID serviceUuid;
    bool isPrimary;
    std::vector<AniBleCharacteristic> characteristics;
    std::vector<AniGattService> includeServices;
};

struct AniGattPermission {
    bool readable = false;
    bool readEncrypted = false;
    bool readEncryptedMitm = false;
    bool writeable = false;
    bool writeEncrypted = false;
    bool writeEncryptedMitm = false;
    bool writeSigned = false;
    bool writeSignedMitm = false;
};

struct AniGattProperties {
    bool write = false;
    bool writeNoResponse = false;
    bool read = false;
    bool notify = false;
    bool indicate = false;
    bool broadcast = false;
    bool authenticatedSignedWrite = false;
    bool extendedProperties = false;
};

ani_status AniParseGattService(ohos::bluetooth::ble::GattService object, AniGattService &outService);
ani_status AniParseIncludeService(taihe::array<::ohos::bluetooth::ble::GattService> object,
    std::vector<AniGattService> &outIncludeService);
ani_status AniParseGattCharacteristic(taihe::array<::ohos::bluetooth::ble::BLECharacteristic> object,
    std::vector<AniBleCharacteristic> &outCharacteristic);
ani_status AniParseGattDescriptor(taihe::array<::ohos::bluetooth::ble::BLEDescriptor> object,
    std::vector<AniBleDescriptor> &outDescriptor);
ani_status AniParseObjectGattPermissions(ohos::bluetooth::ble::GattPermissions object,
    AniGattPermission &outPermissions);
ani_status AniParseObjectGattProperties(ohos::bluetooth::ble::GattProperties object, AniGattProperties &outProperties);
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_PARSER_UTILS_H