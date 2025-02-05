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
#define LOG_TAG "bt_napi_async_callback"
#endif

#include "napi_async_callback.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
void NapiAsyncCallback::CallFunction(int errCode, const std::shared_ptr<NapiNativeObject> &object)
{
    if (callback == nullptr && promise == nullptr) {
        HILOGE("callback & promise is nullptr");
        return;
    }
    if (object == nullptr) {
        HILOGE("napi native object is nullptr");
        return;
    }

    if (callback) {
        callback->CallFunction(errCode, object);
        return;
    }
    if (promise) {
        promise->ResolveOrReject(errCode, object);
    }
}

napi_value NapiAsyncCallback::GetRet(void)
{
    if (promise) {
        return promise->GetPromise();
    }
    return NapiGetUndefinedRet(env);
}

NapiCallback::NapiCallback(napi_env env, napi_value callback) : env_(env)
{
    // Use ID to identify NAPI callback.
    static int idCount = 0;
    id_ = idCount++;

    auto status = napi_create_reference(env, callback, 1, &callbackRef_);
    if (status != napi_ok) {
        HILOGE("napi_create_reference failed, status: %{public}d", status);
    }
}
NapiCallback::~NapiCallback()
{
    auto status = napi_delete_reference(env_, callbackRef_);
    if (status != napi_ok) {
        HILOGE("napi_delete_reference failed, status: %{public}d", status);
    }
}

namespace {
void NapiCallFunction(napi_env env, napi_ref callbackRef, napi_value *argv, size_t argc)
{
    napi_value undefined = nullptr;
    napi_value callRet = nullptr;
    napi_value callback = nullptr;
    auto status = napi_get_reference_value(env, callbackRef, &callback);
    if (status != napi_ok) {
        HILOGE("napi_get_reference_value failed, status: %{public}d", status);
        return;
    }

    status = napi_call_function(env, undefined, callback, argc, argv, &callRet);
    if (status != napi_ok) {
        HILOGE("napi_call_function failed, status: %{public}d", status);
    }

    // Check whether the JS application triggers an exception in callback. If it is, clear it.
    bool isExist = false;
    status = napi_is_exception_pending(env, &isExist);
    HILOGD("napi_is_exception_pending status: %{public}d, isExist: %{public}d", status, isExist);
    if (isExist) {
        HILOGI("Clear JS application's exception");
        napi_value exception = nullptr;
        status = napi_get_and_clear_last_exception(env, &exception);
        HILOGD("napi_get_and_clear_last_exception status: %{public}d, exception: %{public}p", status, exception);
    }
}
}  // namespace {}

void NapiCallback::CallFunction(const std::shared_ptr<NapiNativeObject> &object)
{
    if (object == nullptr) {
        HILOGE("napi native object is nullptr");
        return;
    }

    NapiHandleScope scope(env_);
    napi_value val = object->ToNapiValue(env_);
    NapiCallFunction(env_, callbackRef_, &val, ARGS_SIZE_ONE);
}

void NapiCallback::CallFunction(int errCode, const std::shared_ptr<NapiNativeObject> &object)
{
    if (object == nullptr) {
        HILOGE("napi native object is nullptr");
        return;
    }

    NapiHandleScope scope(env_);
    napi_value code = GetCallbackErrorValue(env_, errCode);
    napi_value val = object->ToNapiValue(env_);
    napi_value argv[ARGS_SIZE_TWO] = {code, val};
    NapiCallFunction(env_, callbackRef_, argv, ARGS_SIZE_TWO);
}

napi_env NapiCallback::GetNapiEnv(void)
{
    return env_;
}

bool NapiCallback::Equal(napi_value &callback) const
{
    NapiHandleScope scope(env_);
    napi_value storedCallback = nullptr;
    napi_get_reference_value(env_, callbackRef_, &storedCallback);

    bool isEqual = false;
    napi_strict_equals(env_, storedCallback, callback, &isEqual);
    return isEqual;
}

std::string NapiCallback::ToLogString(void) const
{
    return "callbackId: " + std::to_string(id_);
}

NapiPromise::NapiPromise(napi_env env) : env_(env)
{
    auto status = napi_create_promise(env, &deferred_, &promise_);
    if (status != napi_ok) {
        HILOGE("napi_create_promise failed, status: %{public}d", status);
    }
}

void NapiPromise::ResolveOrReject(int errCode, const std::shared_ptr<NapiNativeObject> &object)
{
    if (object == nullptr) {
        HILOGE("napi native object is nullptr");
        return;
    }

    if (isResolvedOrRejected_) {
        HILOGE("napi Resolved Or Rejected");
        return;
    }

    NapiHandleScope scope(env_);

    if (errCode == BT_NO_ERROR) {
        napi_value val = object->ToNapiValue(env_);
        Resolve(val);
    } else {
        napi_value code = GetCallbackErrorValue(env_, errCode);
        Reject(code);
    }
    isResolvedOrRejected_ = true;
}

void NapiPromise::Resolve(napi_value resolution)
{
    auto status = napi_resolve_deferred(env_, deferred_, resolution);
    if (status != napi_ok) {
        HILOGE("napi_resolve_deferred failed, status: %{public}d", status);
    }
}
void NapiPromise::Reject(napi_value rejection)
{
    auto status = napi_reject_deferred(env_, deferred_, rejection);
    if (status != napi_ok) {
        HILOGE("napi_reject_deferred failed, status: %{public}d", status);
    }
}
napi_value NapiPromise::GetPromise(void) const
{
    return promise_;
}

NapiHandleScope::NapiHandleScope(napi_env env) : env_(env)
{
    napi_status status = napi_open_handle_scope(env_, &scope_);
    if (status != napi_ok) {
        HILOGE("napi_open_handle_scope failed, status(%{public}d)", status);
    }
}

NapiHandleScope::~NapiHandleScope()
{
    napi_status status = napi_close_handle_scope(env_, scope_);
    if (status != napi_ok) {
        HILOGE("napi_close_handle_scope failed, status(%{public}d)", status);
    }
}

}  // namespace Bluetooth
}  // namespace OHOS
