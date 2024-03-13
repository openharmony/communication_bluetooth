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

#include "bluetooth_log.h"
#include "napi_bluetooth_audio_manager.h"
 
namespace OHOS {
namespace Bluetooth {
EXTERN_C_START
/*
 * Module initialization function
 */
static napi_value Init(napi_env env, napi_value exports)
{
    HILOGI("-----wear detection Init start------");
    napi_property_descriptor desc[] = {};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    NapiBluetoothAudioManager::DefineSystemWearDetectionInterface(env, exports);
    HILOGI("-----wear detection Init end------");
    return exports;
}
EXTERN_C_END
/*
 * Module define
 */
static napi_module bluetoothWearDetectionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = Init,
    .nm_modname = "bluetooth.wearDetection",
    .nm_priv = ((void *)0),
    .reserved = {0}};
/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    HILOGI("Register bluetoothConnectionModule nm_modname:%{public}s", bluetoothWearDetectionModule.nm_modname);
    napi_module_register(&bluetoothWearDetectionModule);
}
}  // namespace Bluetooth
}  // namespace OHOS