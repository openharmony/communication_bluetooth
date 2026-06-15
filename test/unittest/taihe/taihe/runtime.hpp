/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef TAIHE_RUNTIME_HPP_
#define TAIHE_RUNTIME_HPP_

#include <cstddef>
#include <cstdint>

using ani_ref = void *;
using ani_class = void *;
using ani_method = void *;
using ani_object = void *;
using ani_int = int32_t;
using ani_string = ani_ref;
using ani_status = int;

constexpr ani_status ANI_OK = 0;
constexpr ani_status ANI_ERROR = -1;

struct ani_env {
    ani_status GetNull(ani_ref *result)
    {
        *result = nullptr;
        return ANI_OK;
    }

    ani_status GetUndefined(ani_ref *result)
    {
        *result = nullptr;
        return ANI_OK;
    }

    ani_status FindClass(const char *name, ani_class *result)
    {
        *result = nullptr;
        return ANI_OK;
    }

    ani_status Class_FindMethod(ani_class cls, const char *name, const char *sig, ani_method *result)
    {
        *result = nullptr;
        return ANI_OK;
    }

    ani_status Object_New(ani_class cls, ani_method method, ani_object *result)
    {
        *result = nullptr;
        return ANI_OK;
    }

    ani_status String_NewUTF8(const char *str, size_t length, ani_string *result)
    {
        *result = nullptr;
        return ANI_OK;
    }

    ani_status Object_SetPropertyByName_Int(ani_object obj, const char *name, ani_int value)
    {
        return ANI_OK;
    }

    ani_status Object_SetPropertyByName_Ref(ani_object obj, const char *name, ani_ref value)
    {
        return ANI_OK;
    }
};

#endif // TAIHE_RUNTIME_HPP_