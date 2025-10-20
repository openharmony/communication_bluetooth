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
#include "taihe_bluetooth_utils.h"

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
        HILOGE("taihe native object is nullptr");
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
    return reinterpret_cast<ani_object>(TaiheGetUndefined(env));
}

TaiheCallback::TaiheCallback(ani_vm *vm, ani_env *env, std::thread::id tid, ani_object callback) : vm_(vm), env_(env), threadId_(tid)
{
    auto status = env->GlobalReference_Create(static_cast<ani_ref>(callback), &callbackRef_);
    if (status != ANI_OK) {
        HILOGE("GlobalReference_Create failed, status: %{public}d", status);
    }
}

TaiheCallback::~TaiheCallback()
{
    bool isSameThread = (threadId_ == std::this_thread::get_id());
    ani_env* curEnv = isSameThread ? env_ : GetCurrentEnv(vm_);
    if (curEnv == nullptr) {
        HILOGE("Failed to GetCurrentEnv");
        return;
    }

    if (callbackRef_ != nullptr) {
        auto status = curEnv->GlobalReference_Delete(callbackRef_);
        if (status != ANI_OK) {
            HILOGE("GlobalReference_Delete failed, status: %{public}d", status);
        }
        callbackRef_ = nullptr;
    }

    if (!isSameThread) {
        vm_->DetachCurrentThread();
    }
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

    ani_boolean isExist = false;
    status = env->ExistUnhandledError(&isExist);
    HILOGD("ExistUnhandledError status: %{public}d, isExist: %{public}d", status, isExist);
    if (isExist) {
        HILOGI("Clear application's exception");
        status = env->ResetError();
        HILOGD("ResetError status: %{public}d", status);
    }
    HILOGI("TaiheCallFunction leave");
}
}  // namespace {}

void TaiheCallback::CallFunction(int errCode, const std::shared_ptr<TaiheNativeObject> &object)
{
    if (object == nullptr) {
        HILOGE("taihe native object is nullptr");
        return;
    }

    bool isSameThread = (threadId_ == std::this_thread::get_id());
    ani_env* curEnv = isSameThread ? env_ : GetCurrentEnv(vm_);
    if (curEnv == nullptr) {
        HILOGE("taihe get current env is nullptr");
        return;
    }

    TaiheCreateLocalScope(curEnv);
    ani_ref code = GetCallbackErrorValue(curEnv, errCode);
    ani_ref val = object->ToTaiheValue(curEnv);
    ani_ref argv[ARGS_SIZE_TWO] = {code, val};
    TaiheCallFunction(curEnv, callbackRef_, argv, ARGS_SIZE_TWO);
    TaiheDestroyLocalScope(curEnv);

    if (!isSameThread) {
        vm_->DetachCurrentThread();
    }
}

TaihePromise::TaihePromise(ani_vm *vm, ani_env *env, std::thread::id tid) : vm_(vm), env_(env), threadId_(tid)
{
    auto status = env->Promise_New(&bindDeferred_, &promise_);
    if (status != ANI_OK) {
        HILOGE("Promise_New failed, status: %{public}d", status);
    }
}

TaihePromise::~TaihePromise()
{
}

void TaihePromise::ResolveOrReject(int errCode, const std::shared_ptr<TaiheNativeObject> &object)
{
    HILOGI("ResolveOrReject enter");
    if (object == nullptr) {
        HILOGE("taihe native object is nullptr");
        return;
    }

    if (isResolvedOrRejected_) {
        HILOGE("taihe Resolved Or Rejected");
        return;
    }

    bool isSameThread = (threadId_ == std::this_thread::get_id());
    ani_env* curEnv = isSameThread ? env_ : GetCurrentEnv(vm_);
    if (curEnv == nullptr) {
        HILOGE("taihe get current env is nullptr");
        return;
    }

    TaiheCreateLocalScope(curEnv);
    if (errCode == BT_NO_ERROR) {
        ani_ref val = object->ToTaiheValue(curEnv);
        Resolve(curEnv, val);
    } else {
        ani_ref code = GetCallbackErrorValue(curEnv, errCode);
        Reject(curEnv, code);
    }
    isResolvedOrRejected_ = true;
    TaiheDestroyLocalScope(curEnv);

    if (!isSameThread) {
        vm_->DetachCurrentThread();
    }
}

void TaihePromise::Resolve(ani_env *env, ani_ref resolution)
{
    HILOGI("Resolve enter");
    auto status = env->PromiseResolver_Resolve(bindDeferred_, resolution);
    if (status != ANI_OK) {
        HILOGE("PromiseResolver_Resolve failed, status: %{public}d", status);
    }
    HILOGI("Resolve leave");
}

void TaihePromise::Reject(ani_env *env, ani_ref rejection)
{
    HILOGI("Reject enter");
    auto status = env->PromiseResolver_Reject(bindDeferred_, reinterpret_cast<ani_error>(rejection));
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

void TaiheCreateLocalScope(ani_env* env)
{
    ani_size nr_refs = 16;
    ani_status status = env->CreateLocalScope(nr_refs);
    if (status != ANI_OK) {
        HILOGE("CreateLocalScope failed, status(%{public}d)", status);
    }
}

void TaiheDestroyLocalScope(ani_env* env)
{
    ani_status status = env->DestroyLocalScope();
    if (status != ANI_OK) {
        HILOGE("DestroyLocalScope failed, status(%{public}d)", status);
    }
}

ani_env* GetCurrentEnv(ani_vm *vm)
{
    if (vm == nullptr) {
        HILOGE("null vm");
        return nullptr;
    }
    ani_env *threadEnv;
    ani_options aniArgs {0, nullptr};
    ani_status status = vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &threadEnv);
    if (status != ANI_OK) {
        HILOGE("GetCurrentEnv failed, status(%{public}d)", status);
        return nullptr;
    }
    return threadEnv;
}
}  // namespace Bluetooth
}  // namespace OHOS
