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

#ifndef NAPI_NATIVE_API_H
#define NAPI_NATIVE_API_H

struct napi_env {};
using napi_value = void *;
enum napi_status {
    napi_ok = 0,
    napi_invalid_arg,
};
struct napi_async_work {};
struct napi_deferred {};
using napi_ref = int;
struct uv_work_t {
    void *data;
};
struct napi_callback_info {};
using napi_handle_scope = void *;
typedef enum {
    napi_undefined,
    napi_null,
} napi_valuetype;

napi_status napi_create_reference(napi_env env, napi_value value, uint32_t initial_refcount, napi_ref* result);
napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result);
napi_status napi_call_function(napi_env env, napi_value recv, napi_value func, size_t argc,
    const napi_value* argv, napi_value* result);
napi_status napi_is_exception_pending(napi_env env, bool* result);
napi_status napi_delete_reference(napi_env env, napi_ref ref);
napi_status napi_get_and_clear_last_exception(napi_env env, napi_value* result);
napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result);
napi_status napi_create_promise(napi_env env, napi_deferred* deferred, napi_value* promise);
napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution);
napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection);
napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result);
napi_status napi_get_cb_info(napi_env env, napi_callback_info cbinfo, size_t* argc, napi_value* argv,
    napi_value* this_arg, void** data);
napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope);

using napi_threadsafe_function = void *;
#endif // NAPI_NATIVE_API_H