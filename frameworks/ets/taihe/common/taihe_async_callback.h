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
#include <thread>

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
    TaiheCallback(ani_vm *vm, ani_object callback);
    ~TaiheCallback();

    void CallFunction(int errCode, const std::shared_ptr<TaiheNativeObject> &object);

private:
    TaiheCallback(const TaiheCallback &) = delete;
    TaiheCallback &operator=(const TaiheCallback &) = delete;
    TaiheCallback(TaiheCallback &&) = delete;
    TaiheCallback &operator=(TaiheCallback &&) noexcept = delete;

    ani_vm *vm_;
    ani_ref callbackRef_;
    bool isAttach_ = false;
};

class TaihePromise {
public:
    explicit TaihePromise(ani_vm *vm);
    ~TaihePromise();

    void ResolveOrReject(int errCode, const std::shared_ptr<TaiheNativeObject> &object);
    void Resolve(ani_env *env, ani_ref resolution);
    void Reject(ani_env *env, ani_ref rejection);
    ani_object GetPromise(void) const;

private:
    ani_vm *vm_;
    ani_object promise_;
    ani_resolver bindDeferred_;
    bool isResolvedOrRejected_ = false;
    bool isAttach_ = false;
};

void TaiheCreateLocalScope(ani_env *env);
void TaiheDestroyLocalScope(ani_env *env);
ani_env* GetCurrentEnv(ani_vm *vm, bool &isAttach);
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_ASYNC_CALLBACK_H