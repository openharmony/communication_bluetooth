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
#define LOG_TAG "bt_taihe_async_callback"
#endif

#include "taihe_async_callback.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
void TaiheAsyncCallback::CallFunction(int errCode, const std::shared_ptr<TaiheNativeObject> &object)
{
    HILOGI("CallFunction enter");
    if (callback == nullptr && promise == nullptr) {
        HILOGE("callback & promise is nullptr");
        return;
    }
    if (object == nullptr) {
        HILOGE("napi native object is nullptr");
        return;
    }

    if (callback) {
        HILOGI("CallFunction callback");
        callback->CallFunction(errCode, object);
        return;
    }
    if (promise) {
        HILOGI("CallFunction promise");
        promise->ResolveOrReject(errCode, object);
    }
    HILOGI("CallFunction leave");
}

ani_object TaiheAsyncCallback::GetRet(void)
{
    if (promise) {
        return promise->GetPromise();
    }
    return reinterpret_cast<ani_object>(TaiheGetUndefinedRet(env));
}

/*************************** env_cleanup_hook ********************************/
void TaiheCallbackEnvCleanupHook(void *data)
{
    CHECK_AND_RETURN_LOG(data, "data is nullptr");

    TaiheCallback *callback = static_cast<TaiheCallback *>(data);
    callback->SetTaiheEnvValidity(false);
}
/*************************** env_cleanup_hook ********************************/

TaiheCallback::TaiheCallback(ani_vm *vm, ani_env *env, ani_object callback) : vm_(vm), env_(env)
{
    HILOGI("TaiheCallback::TaiheCallback enter");
    // Use ID to identify NAPI callback.
    static int idCount = 0;
    id_ = idCount++;

    auto status = env->GlobalReference_Create(static_cast<ani_ref>(callback), &callbackRef_);
    if (status != ANI_OK) {
        HILOGE("GlobalReference_Create failed, status: %{public}d", status);
    }
    HILOGI("TaiheCallback::TaiheCallback leave");
}
TaiheCallback::~TaiheCallback()
{
    HILOGI("TaiheCallback::~TaiheCallback enter");
    if (!IsValidTaiheEnv()) {
        HILOGE("TaiheCallback::~TaiheCallback env is exit");
        return;
    }

    TaiheHandleScope scope(vm_, env_);
    auto status = env_->GlobalReference_Delete(callbackRef_);
    if (status != ANI_OK) {
        HILOGE("GlobalReference_Delete failed, status: %{public}d", status);
    }
    HILOGI("TaiheCallback::~TaiheCallback leave");
}

namespace {
void TaiheCallFunction(ani_env *env, ani_ref callbackRef, ani_ref *argv, size_t argc)
{
    HILOGI("TaiheCallFunction enter");
    ani_ref callRet = nullptr;
    ani_fn_object callback = reinterpret_cast<ani_fn_object>(callbackRef);

    auto status = env->FunctionalObject_Call(callback, argc, argv, &callRet);
    if (status != ANI_OK) {
        HILOGE("FunctionalObject_Call failed, status: %{public}d", status);
    }
    if (status == ANI_INVALID_ARGS) {
        HILOGI("TaiheCallFunction status == ANI_INVALID_ARGS");
        return;
    } else {
        HILOGI("TaiheCallFunction status != ANI_INVALID_ARGS");
    }
    HILOGI("TaiheCallFunction leave");
}
}  // namespace {}

void TaiheCallback::CallFunction(const std::shared_ptr<TaiheNativeObject> &object)
{
    if (!IsValidTaiheEnv()) {
        HILOGW("taihe env is exit");
        return;
    }
    if (object == nullptr) {
        HILOGE("taihe native object is nullptr");
        return;
    }

    TaiheHandleScope scope(vm_, env_);
    ani_ref val = object->ToTaiheValue(env_);
    TaiheCallFunction(env_, callbackRef_, &val, ARGS_SIZE_ONE);
}

void TaiheCallback::CallFunction(int errCode, const std::shared_ptr<TaiheNativeObject> &object)
{
    if (!IsValidTaiheEnv()) {
        HILOGW("taihe env is exit");
        return;
    }
    if (object == nullptr) {
        HILOGE("taihe native object is nullptr");
        return;
    }

    TaiheHandleScope scope(vm_, env_);

    ani_ref code = GetCallbackErrorValue(env_, errCode);
    ani_ref val = object->ToTaiheValue(env_);
    ani_ref argv[ARGS_SIZE_TWO] = {code, val};
    TaiheCallFunction(env_, callbackRef_, argv, ARGS_SIZE_TWO);
}

ani_env* TaiheCallback::GetTaiheEnv(void)
{
    return env_;
}

bool TaiheCallback::Equal(ani_env *env, ani_value &callback) const
{
    if (!IsValidTaiheEnv()) {
        HILOGW("napi env is exit");
        return false;
    }
    if (env != env_) {
        HILOGD("Callback is not in the same thread, not uqual");
        return false;
    }

    bool isEqual = false;
    return isEqual;
}

std::string TaiheCallback::ToLogString(void) const
{
    return "callbackId: " + std::to_string(id_);
}

/*************************** env_cleanup_hook ********************************/
void TaihePromiseEnvCleanupHook(void *data)
{
    CHECK_AND_RETURN_LOG(data, "data is nullptr");

    TaihePromise *callback = static_cast<TaihePromise *>(data);
    callback->SetTaiheEnvValidity(false);
}
/*************************** env_cleanup_hook ********************************/

TaihePromise::TaihePromise(ani_vm *vm, ani_env *env) : vm_(vm), env_(env)
{
    auto status = env->Promise_New(&bindDeferred_, &promise_);
    if (status != ANI_OK) {
        HILOGE("Promise_New failed, status: %{public}d", status);
    }
}

TaihePromise::~TaihePromise()
{
    if (!IsValidTaiheEnv()) {
        return;
    }
}

void TaihePromise::ResolveOrReject(int errCode, const std::shared_ptr<TaiheNativeObject> &object)
{
    HILOGI("ResolveOrReject enter");
    if (!IsValidTaiheEnv()) {
        HILOGW("napi env is exit");
        return;
    }
    if (object == nullptr) {
        HILOGE("napi native object is nullptr");
        return;
    }

    if (isResolvedOrRejected_) {
        HILOGE("napi Resolved Or Rejected");
        return;
    }

    TaiheHandleScope scope(vm_, env_);

    if (errCode == BT_NO_ERROR) {
        HILOGI("ResolveOrReject Resolve");
        ani_ref val = object->ToTaiheValue(env_);
        Resolve(val);
    } else {
        HILOGI("ResolveOrReject Reject");
        ani_ref code = GetCallbackErrorValue(env_, errCode);
        Reject(code);
    }
    isResolvedOrRejected_ = true;
    HILOGI("ResolveOrReject leave");
}

void TaihePromise::Resolve(ani_ref resolution)
{
    HILOGI("Resolve enter");
    auto status = env_->PromiseResolver_Resolve(bindDeferred_, resolution);
    if (status != ANI_OK) {
        HILOGE("PromiseResolver_Resolve failed, status: %{public}d", status);
    }
    HILOGI("Resolve leave");
}

void TaihePromise::Reject(ani_ref rejection)
{
    HILOGI("Reject enter");
    auto status = env_->PromiseResolver_Reject(bindDeferred_, reinterpret_cast<ani_error>(rejection));
    if (status != ANI_OK) {
        HILOGE("PromiseResolver_Reject failed, status: %{public}d", status);
    }
    HILOGI("Reject leave");
}

ani_object TaihePromise::GetPromise(void) const
{
    HILOGI("GetPromise enter");
    return promise_;
}

TaiheHandleScope::TaiheHandleScope(ani_vm *vm, ani_env*& env) : vm_(vm)
{
    ani_options aniArgs {0, nullptr};
    ani_status status = vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env);
    HILOGI("attach current thread ret: %{public}d", status);
    if (status != ANI_OK) {
        HILOGE("AttachCurrentThread failed, status(%{public}d)", status);
        status = vm->GetEnv(ANI_VERSION_1, &env);
        HILOGI("get new env ret: %{public}d", status);
        if (status != ANI_OK) {
            HILOGE("GetEnv failed, status(%{public}d)", status);
        }
    }
}

TaiheHandleScope::~TaiheHandleScope()
{
    ani_status status = vm_->DetachCurrentThread();
    HILOGI("detach current thread ret: %{public}d", status);
    if (status != ANI_OK) {
        HILOGE("DetachCurrentThread failed, status(%{public}d)", status);
    }
}
}  // namespace Bluetooth
}  // namespace OHOS
