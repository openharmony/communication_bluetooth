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

#ifndef NAPI_PARSER_UTILS_H
#define NAPI_PARSER_UTILS_H

#include "napi_bluetooth_ble_utils.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace Bluetooth {

struct NapiBleDescriptor {
    UUID serviceUuid;
    UUID characteristicUuid;
    UUID descriptorUuid;
    std::vector<uint8_t> descriptorValue;
    uint16_t descriptorHandle;
    int permissions;
};

struct NapiBleCharacteristic {
    UUID serviceUuid;
    UUID characteristicUuid;
    std::vector<uint8_t> characteristicValue;
    std::vector<NapiBleDescriptor> descriptors;
    int properties;
    uint16_t characteristicValueHandle;
    int permissions;
};

struct FileHolder {
    std::string filePath = "";
    int64_t fileSize = 0;
    int32_t fileFd = -1;
};

struct NapiGattService {
    UUID serviceUuid;
    bool isPrimary;
    std::vector<NapiBleCharacteristic> characteristics;
    std::vector<NapiGattService> includeServices;
};

struct NapiGattPermission {
    bool readable = false;
    bool writeable = false;
    bool readEncrypted = false;
    bool writeEncrypted = false;
};

struct NapiGattProperties {
    bool write = false;
    bool writeNoResponse = false;
    bool read = false;
    bool notify = false;
    bool indicate = false;
};

bool NapiIsObjectPropertyExist(napi_env env, napi_value object, const char *name);
napi_status NapiParseBoolean(napi_env env, napi_value value, bool &outBoolean);
napi_status NapiParseInt32(napi_env env, napi_value value, int32_t &outNum);
napi_status NapiParseUint32(napi_env env, napi_value value, uint32_t &outNum);
napi_status NapiParseString(napi_env env, napi_value value, std::string &outStr);
napi_status NapiParseBdAddr(napi_env env, napi_value value, std::string &outAddr);
napi_status NapiParseUuid(napi_env env, napi_value value, std::string &outUuid);
napi_status NapiParseArrayBuffer(napi_env env, napi_value value, std::vector<uint8_t> &outVec);
napi_status NapiParseGattService(napi_env env, napi_value object, NapiGattService &outService);
napi_status NapiParseGattCharacteristic(napi_env env, napi_value object, NapiBleCharacteristic &outCharacteristic);
napi_status NapiParseGattDescriptor(napi_env env, napi_value object, NapiBleDescriptor &outDescriptor);
napi_status NapiParseNotifyCharacteristic(napi_env env, napi_value object, NapiNotifyCharacteristic &outCharacter);
napi_status NapiParseGattsServerResponse(napi_env env, napi_value object, NapiGattsServerResponse &rsp);
napi_status NapiParseGattWriteType(napi_env env, napi_value value, int &outWriteType);

struct NapiAsyncCallback;
std::shared_ptr<NapiAsyncCallback> NapiParseAsyncCallback(napi_env env, napi_callback_info info);

// must have the 'name' property
napi_status NapiGetObjectProperty(napi_env env, napi_value object, const char *name, napi_value &outProperty);
napi_status NapiParseObjectBoolean(napi_env env, napi_value object, const char *name, bool &outBoolean);
napi_status NapiParseObjectUuid(napi_env env, napi_value object, const char *name, std::string &outUuid);
napi_status NapiParseObjectArrayBuffer(napi_env env, napi_value object, const char *name, std::vector<uint8_t> &outVec);
napi_status NapiParseObjectBdAddr(napi_env env, napi_value object, const char *name, std::string &outAddr);
napi_status NapiParseStringArray(napi_env env, napi_value value, std::vector<std::string> &outStrVec);
napi_status NapiParseFileHolder(napi_env env, napi_value object, FileHolder &outFileHolder);
napi_status NapiParseFileHolderArray(napi_env env, napi_value value, std::vector<FileHolder> &outFileHolderVec);
napi_status NapiParseObjectInt32(napi_env env, napi_value object, const char *name, int32_t &outNum);
napi_status NapiParseObjectString(napi_env env, napi_value object, const char *name, std::string &outString);
napi_status NapiParseObjectUint32(napi_env env, napi_value object, const char *name, uint32_t &outNum);
napi_status NapiParseObjectGattPermissions(napi_env env, napi_value object, const char *name,
    NapiGattPermission &outPermissions);
napi_status NapiParseObjectGattProperties(napi_env env, napi_value object, const char *name,
    NapiGattProperties &outProperties);
napi_status NapiParseObjectStr(napi_env env, napi_value object, const char *name, std::string &outStr);
napi_status NapiParseInt64(napi_env env, napi_value object, const char *name, int64_t &outNum);
napi_status NapiParseObjectInt64(napi_env env, napi_value object, const char *name, int64_t &outNum);
napi_status NapiParseTrustPairDevice(napi_env env, napi_value object, std::vector<TrustPairDeviceParam> &outService);
napi_status ParseAndCheckUuids(const std::string &uuids, std::vector<std::string> &res);
napi_status NapiParseObjectUuids(napi_env env, napi_value object, std::vector<std::string> &outUuid);

struct NapiObject {
    napi_env env;
    napi_value object;
};
napi_status NapiParseObjectUint32Check(NapiObject napiObject, const char *name, uint32_t &outNum,
    uint32_t min, uint32_t max);

napi_status NapiGetObjectPropertyOptional(napi_env env, napi_value object, const char *name, napi_value &outProperty,
    bool &outExist);
napi_status NapiParseObjectBooleanOptional(napi_env env, napi_value object, const char *name, bool &outBoolean,
    bool &outExist);
napi_status NapiParseObjectInt32Optional(napi_env env, napi_value object, const char *name, int32_t &outNum,
    bool &outExist);
napi_status NapiParseObjectUint32Optional(napi_env env, napi_value object, const char *name, uint32_t &outNum,
    bool &outExist);

// Parse type Array<XXX>, must implete NapiParseObject<XXX> in napi_parser_utils.cpp.
template <typename T>
napi_status NapiParseArray(napi_env env, napi_value array, std::vector<T> &outVec);
template <typename T>
napi_status NapiParseObjectArray(napi_env env, napi_value object, const char *name, std::vector<T> &outVec);
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_PARSER_UTILS_H