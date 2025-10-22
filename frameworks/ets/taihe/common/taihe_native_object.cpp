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
#define LOG_TAG "bt_taihe_native_object"
#endif

#include "taihe_native_object.h"

#include "taihe_bluetooth_ble_utils.h"

namespace OHOS {
namespace Bluetooth {
ani_ref TaiheNativeInt::ToTaiheValue(ani_env *env) const
{
    ani_class cls;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass("std.core.Int", &cls)) != ANI_OK) {
        HILOGE("FindClass status : %{public}d", status);
        return nullptr;
    }
    ani_method ctor;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "I:V", &ctor)) != ANI_OK) {
        HILOGE("Class_FindMethod status : %{public}d", status);
        return nullptr;
    }
    ani_object object;
    if ((status = env->Object_New(cls, ctor, &object, value_)) != ANI_OK) {
        HILOGE("Object_New status : %{public}d", status);
        return nullptr;
    }
    return reinterpret_cast<ani_ref>(object);
}

ani_ref TaiheNativeBool::ToTaiheValue(ani_env *env) const
{
    ani_class cls;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass("std.core.boolean", &cls)) != ANI_OK) {
        HILOGE("FindClass status : %{public}d", status);
        return nullptr;
    }
    ani_method ctor;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "z:", &ctor)) != ANI_OK) {
        HILOGE("Class_FindMethod status : %{public}d", status);
        return nullptr;
    }
    ani_object object;
    if ((status = env->Object_New(cls, ctor, &object, value_)) != ANI_OK) {
        HILOGE("Object_New status : %{public}d", status);
        return nullptr;
    }
    return reinterpret_cast<ani_ref>(object);
}

ani_ref TaiheNativeBleCharacteristic::ToTaiheValue(ani_env *env) const
{
    ani_object object = ConvertBLECharacteristicToJS(env, const_cast<GattCharacteristic &>(character_));
    return reinterpret_cast<ani_ref>(object);
}

ani_ref TaiheNativeBleDescriptor::ToTaiheValue(ani_env *env) const
{
    ani_object object = ConvertBLEDescriptorToJS(env, const_cast<GattDescriptor &>(descriptor_));
    return reinterpret_cast<ani_ref>(object);
}
}  // namespace Bluetooth
}  // namespace OHOS