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
#define LOG_TAG "bt_napi_module_base_profile"
#endif

#include <map>
#include <string>
#include <thread>

#include "napi_bluetooth_base_profile.h"
#include "bluetooth_log.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
EXTERN_C_START
/*
 * Module initialization function
 */
static napi_value Init(napi_env env, napi_value exports)
{
    HITRACE_METER_NAME(HITRACE_TAG_OHOS, "baseProfile init");
    HILOGI("-----Base Profile Init start------");
    napi_property_descriptor desc[] = {};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    NapiBaseProfile::DefineBaseProfileJSFunction(env, exports);
    HILOGI("-----Base Profile Init end------");
    return exports;
}
EXTERN_C_END
/*
 * Module define
 */
static napi_module bluetoothBaseProfileModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = Init,
    .nm_modname = "bluetooth.baseProfile",
    .nm_priv = ((void *)0),
    .reserved = {0}};
/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    HILOGI("Register bluetoothBaseProfileModule nm_modname:%{public}s", bluetoothBaseProfileModule.nm_modname);
    napi_module_register(&bluetoothBaseProfileModule);
}
}  // namespace Bluetooth
}  // namespace OHOS
