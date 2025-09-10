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
#define LOG_TAG "bt_napi_common"
#endif

#include "napi_bluetooth_common.h"

#include "bluetooth_log.h"
#include "bluetooth_def.h"
#include "napi_bluetooth_utils.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
napi_value NapiCommon::DefineJSCommon(napi_env env, napi_value exports)
{
    HILOGD("enter");
    CommonPropertyValueInit(env, exports);
    napi_property_descriptor desc[] = {};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

napi_value NapiCommon::CommonPropertyValueInit(napi_env env, napi_value exports)
{
    HILOGD("enter");
    napi_value bluetoothAddressTypeObj = BluetoothAddressTypeInit(env);
    napi_property_descriptor exporProperties[] = {
        DECLARE_NAPI_PROPERTY("BluetoothAddressType", bluetoothAddressTypeObj),
    };
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "common:napi_define_properties");
    napi_define_properties(env, exports, sizeof(exporProperties) / sizeof(*exporProperties), exporProperties);
    return exports;
}

napi_value NapiCommon::BluetoothAddressTypeInit(napi_env env)
{
    HILOGD("enter");
    napi_value bluetoothAddressType = nullptr;
    napi_create_object(env, &bluetoothAddressType);
    SetNamedPropertyByInteger(env, bluetoothAddressType, BluetoothAddressType::VIRTUAL_ADDRESS_TYPE, "VIRTUAL");
    SetNamedPropertyByInteger(env, bluetoothAddressType, BluetoothAddressType::REAL_ADDRESS_TYPE, "REAL");
    return bluetoothAddressType;
}
}  // namespace Bluetooth
}  // namespace OHOS
