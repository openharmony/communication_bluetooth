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
#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "taihe_bluetooth_utils.h"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {
struct TaiheBleDescriptor {
    UUID serviceUuid;
    UUID characteristicUuid;
    UUID descriptorUuid;
    std::vector<uint8_t> descriptorValue;
    uint16_t descriptorHandle;
    int permissions;
};

struct TaiheBleCharacteristic {
    UUID serviceUuid;
    UUID characteristicUuid;
    std::vector<uint8_t> characteristicValue;
    std::vector<TaiheBleDescriptor> descriptors;
    int properties;
    uint16_t characteristicValueHandle;
    int permissions;
};

struct TaiheGattService {
    UUID serviceUuid;
    bool isPrimary;
    std::vector<TaiheBleCharacteristic> characteristics;
    std::vector<TaiheGattService> includeServices;
};

struct TaiheGattPermission {
    bool readable = false;
    bool readEncrypted = false;
    bool readEncryptedMitm = false;
    bool writeable = false;
    bool writeEncrypted = false;
    bool writeEncryptedMitm = false;
    bool writeSigned = false;
    bool writeSignedMitm = false;
};

struct TaiheGattProperties {
    bool write = false;
    bool writeNoResponse = false;
    bool read = false;
    bool notify = false;
    bool indicate = false;
    bool broadcast = false;
    bool authenticatedSignedWrite = false;
    bool extendedProperties = false;
};

taihe_status TaiheParseGattService(ohos::bluetooth::ble::GattService object, TaiheGattService &outService);
taihe_status TaiheParseIncludeService(taihe::array<::ohos::bluetooth::ble::GattService> object,
    std::vector<TaiheGattService> &outIncludeService);
taihe_status TaiheParseGattCharacteristic(taihe::array<::ohos::bluetooth::ble::BLECharacteristic> object,
    std::vector<TaiheBleCharacteristic> &outCharacteristic);
taihe_status TaiheParseGattDescriptor(taihe::array<::ohos::bluetooth::ble::BLEDescriptor> object,
    std::vector<TaiheBleDescriptor> &outDescriptor);
void TaiheParseObjectGattPermissions(ohos::bluetooth::ble::GattPermissions object,
    TaiheGattPermission &outPermissions);
void TaiheParseObjectGattProperties(ohos::bluetooth::ble::GattProperties object,
    TaiheGattProperties &outProperties);

struct TaiheAsyncCallback;
std::shared_ptr<TaiheAsyncCallback> TaiheParseAsyncCallback(ani_vm *vm, ani_env *env, ani_object info);
::ohos::bluetooth::ble::AdvertisingParams TaiheParseAdvertisingParams(ani_env *env, ani_object ani_obj);
::ohos::bluetooth::ble::AdvertisingDisableParams TaiheParseAdvertisingDisableParams(ani_env *env, ani_object ani_obj);
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_PARSER_UTILS_H