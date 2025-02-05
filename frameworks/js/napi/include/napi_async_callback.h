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

#ifndef NAPI_ASYNC_CALLBACK_H
#define NAPI_ASYNC_CALLBACK_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "napi/native_api.h"
#include "napi_async_work.h"

namespace OHOS {
namespace Bluetooth {
class NapiCallback;
class NapiPromise;
class NapiNativeObject;

struct NapiAsyncCallback {
    enum class Type {
        CALLBACK = 1,
        PROMISE,
    };

    void CallFunction(int errCode, const std::shared_ptr<NapiNativeObject> &object);
    napi_value GetRet(void);

    Type type;
    napi_env env;
    std::shared_ptr<NapiCallback> callback = nullptr;
    std::shared_ptr<NapiPromise> promise = nullptr;
};

class NapiCallback {
public:
    // napi_value 'callback' shall be type of napi_founction, check it use NapiIsFunction().
    NapiCallback(napi_env env, napi_value callback);
    ~NapiCallback();

    void CallFunction(const std::shared_ptr<NapiNativeObject> &object);
    void CallFunction(int errCode, const std::shared_ptr<NapiNativeObject> &object);
    napi_env GetNapiEnv(void);
    bool Equal(napi_value &callback) const;
    std::string ToLogString(void) const;
private:
    NapiCallback(const NapiCallback &) = delete;
    NapiCallback &operator=(const NapiCallback &) = delete;
    NapiCallback(NapiCallback &&) = delete;
    NapiCallback &operator=(NapiCallback &&) noexcept = delete;

    napi_env env_;
    napi_ref callbackRef_;
    int id_;
};

class NapiPromise {
public:
    explicit NapiPromise(napi_env env);
    ~NapiPromise() = default;

    void ResolveOrReject(int errCode, const std::shared_ptr<NapiNativeObject> &object);
    void Resolve(napi_value resolution);
    void Reject(napi_value rejection);
    napi_value GetPromise(void) const;
private:
    napi_env env_;
    napi_value promise_;
    napi_deferred deferred_;
    bool isResolvedOrRejected_ = false;
};

class NapiHandleScope {
public:
    explicit NapiHandleScope(napi_env env);
    ~NapiHandleScope();

private:
    napi_env env_;
    napi_handle_scope scope_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_ASYNC_CALLBACK_H