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

#ifndef TAIHE_ASYNC_CALLBACK_H
#define TAIHE_ASYNC_CALLBACK_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "taihe_async_work.h"

namespace OHOS {
namespace Bluetooth {
class TaiheCallback;
class TaihePromise;
class TaiheNativeObject;

struct TaiheAsyncCallback {
    enum class Type {
        CALLBACK = 1,
        PROMISE,
    };

    void CallFunction(int errCode, const std::shared_ptr<TaiheNativeObject> &object);
    ani_object GetRet(void);

    Type type;
    ani_env *env;
    std::shared_ptr<TaiheCallback> callback = nullptr;
    std::shared_ptr<TaihePromise> promise = nullptr;
};

class TaiheCallback {
public:
    // napi_value 'callback' shall be type of napi_founction, check it use NapiIsFunction().
    TaiheCallback(ani_vm *vm, ani_env *env, ani_object callback);
    ~TaiheCallback();

    void CallFunction(const std::shared_ptr<TaiheNativeObject> &object);
    void CallFunction(int errCode, const std::shared_ptr<TaiheNativeObject> &object);
    ani_env* GetTaiheEnv(void);
    bool Equal(ani_env *env, ani_value &callback) const;
    std::string ToLogString(void) const;
    void SetTaiheEnvValidity(bool isValid)
    {
        isValid_ = isValid;
    }
    bool IsValidTaiheEnv(void) const
    {
        return isValid_;
    }

private:
    TaiheCallback(const TaiheCallback &) = delete;
    TaiheCallback &operator=(const TaiheCallback &) = delete;
    TaiheCallback(TaiheCallback &&) = delete;
    TaiheCallback &operator=(TaiheCallback &&) noexcept = delete;

    ani_vm *vm_;
    ani_env *env_;
    ani_ref callbackRef_;
    int id_;
    /*************************** env_cleanup_hook ********************************/
    bool isValid_ = true;
    /*************************** env_cleanup_hook ********************************/
};

class TaihePromise {
public:
    explicit TaihePromise(ani_vm *vm, ani_env *env);
    ~TaihePromise();

    void ResolveOrReject(int errCode, const std::shared_ptr<TaiheNativeObject> &object);
    void Resolve(ani_ref resolution);
    void Reject(ani_ref rejection);
    ani_object GetPromise(void) const;
    void SetTaiheEnvValidity(bool isValid)
    {
        isValid_ = isValid;
    }
    bool IsValidTaiheEnv(void) const
    {
        return isValid_;
    }

private:
    ani_vm *vm_;
    ani_env *env_;
    ani_object promise_;
    ani_resolver bindDeferred_;
    bool isResolvedOrRejected_ = false;
    bool isValid_ = true;
};

class TaiheHandleScope {
public:
    explicit TaiheHandleScope(ani_vm *vm, ani_env*& env);
    ~TaiheHandleScope();

private:
    ani_vm *vm_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_ASYNC_CALLBACK_H