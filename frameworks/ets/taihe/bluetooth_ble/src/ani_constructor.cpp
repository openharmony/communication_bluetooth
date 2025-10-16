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

#include "ohos.bluetooth.ble.ani.hpp"
#include "ohos.bluetooth.ble.impl.h"
#include "taihe_bluetooth_gatt_server.h"

static ani_status ANIFuncsRegister(ani_env *env)
{
    ani_namespace scope;
    if (ANI_OK != env->FindNamespace("@ohos.bluetooth.ble.ble", &scope)) {
        return ANI_ERROR;
    }
    ani_native_function methods[] = {
        {"StartAdvertisingReturnsPromise_inner", nullptr,
            reinterpret_cast<ani_object *>(OHOS::Bluetooth::StartAdvertisingAsyncPromise)},
        {"StartAdvertisingWithCallback_inner", nullptr,
            reinterpret_cast<ani_object *>(OHOS::Bluetooth::StartAdvertisingAsyncCallback)},
        {"DisableAdvertisingReturnsPromise_inner", nullptr,
            reinterpret_cast<ani_object *>(OHOS::Bluetooth::DisableAdvertisingAsyncPromise)},
        {"DisableAdvertisingWithCallback_inner", nullptr,
            reinterpret_cast<ani_object *>(OHOS::Bluetooth::DisableAdvertisingAsyncCallback)},
        {"EnableAdvertisingReturnsPromise_inner", nullptr,
            reinterpret_cast<ani_object *>(OHOS::Bluetooth::EnableAdvertisingAsyncPromise)},
        {"EnableAdvertisingWithCallback_inner", nullptr,
            reinterpret_cast<ani_object *>(OHOS::Bluetooth::EnableAdvertisingAsyncCallback)},
    };
    return env->Namespace_BindNativeFunctions(scope, methods, sizeof(methods) / sizeof(ani_native_function));
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    if (ANI_OK != ohos::bluetooth::ble::ANIRegister(env)) {
        std::cerr << "Error from ohos::bluetooth::ble::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    ani_status status = ANI_OK;
    if (ani_status ret = ANIFuncsRegister(env); ret != ANI_OK && ret != ANI_ALREADY_BINDED) {
        std::cerr << "Error from ANIFuncsRegister, code: " << ret << std::endl;
        status = ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
