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
#define LOG_TAG "bt_napi_async_work"
#endif

#include "napi_async_work.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "napi_async_callback.h"
#include "napi_bluetooth_utils.h"
#include "napi_timer.h"
#include "../parser/napi_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<NapiAsyncWork> NapiAsyncWorkFactory::CreateAsyncWork(napi_env env, napi_callback_info info,
    std::function<NapiAsyncWorkRet(void)> asyncWork, bool needCallback)
{
    auto asyncCallback = NapiParseAsyncCallback(env, info);
    if (!asyncCallback) {
        HILOGE("asyncCallback is nullptr!");
        return nullptr;
    }
    auto napiAsyncWork = std::make_shared<NapiAsyncWork>(env, asyncWork, asyncCallback, needCallback);
    return napiAsyncWork;
}

void NapiAsyncWork::Info::Execute(void)
{
    if (napiAsyncWork == nullptr) {
        HILOGE("napiAsyncWork is nullptr");
        errCode = BT_ERR_INTERNAL_ERROR;
        object = nullptr;
        return;
    }
    auto ret = napiAsyncWork->func_();
    errCode = ret.errCode;
    object = ret.object;
}

void NapiAsyncWork::Info::Complete(void)
{
    HILOGD("needCallback: %{public}d, errCode: %{public}d", needCallback, errCode);
    if (napiAsyncWork == nullptr) {
        HILOGE("napiAsyncWork is nullptr");
        return;
    }

    // need wait callback
    if (needCallback && (errCode == BT_NO_ERROR)) {
        if (napiAsyncWork->triggered_) {
            HILOGE("NapiAsyncWork is triggered, Callback is earlier than Complete in thread scheduling");
            return;
        }
        // start timer to avoid the callback is lost.
        std::weak_ptr<NapiAsyncWork> asyncWorkWptr = napiAsyncWork;
        auto func = [asyncWorkWptr]() {
            auto asyncWorkSptr = asyncWorkWptr.lock();
            if (asyncWorkSptr == nullptr) {
                HILOGE("asyncWorkSptr is nullptr");
                return;
            }
            asyncWorkSptr->TimeoutCallback();
        };
        NapiTimer::GetInstance()->Register(func, napiAsyncWork->timerId_);
        return;
    }

    if (object == nullptr) {
        HILOGD("napi native object is nullptr");
        object = std::make_shared<NapiNativeEmpty>();
    }

    if (napiAsyncWork->napiAsyncCallback_) {
        napiAsyncWork->triggered_ = true;
        napiAsyncWork->napiAsyncCallback_->CallFunction(errCode, object);
    }
}

void NapiAsyncWork::Run(void)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env_, "napiAsyncWork", NAPI_AUTO_LENGTH, &resource);

    NapiAsyncWork::Info *info = new NapiAsyncWork::Info();
    info->needCallback = needCallback_.load();
    info->napiAsyncWork = shared_from_this();

    napi_status status = napi_create_async_work(env_, nullptr, resource,
        [](napi_env env, void* data) {
            NapiAsyncWork::Info *info = static_cast<NapiAsyncWork::Info *>(data);
            if (info == nullptr) {
                HILOGE("Async work info is nullptr");
                return;
            }
            info->Execute();
        },
        [](napi_env env, napi_status status, void* data) {
            NapiAsyncWork::Info *info = static_cast<NapiAsyncWork::Info *>(data);
            if (info == nullptr) {
                HILOGE("info is nullptr");
                return;
            }
            info->Complete();
            napi_delete_async_work(env, info->asyncWork);
            delete info;
        },
        static_cast<void *>(info), &info->asyncWork);
    if (status != napi_ok) {
        HILOGE("napi_create_async_work failed, status(%{public}d)", status);
        delete info;
        return;
    }

    status = napi_queue_async_work(env_, info->asyncWork);
    if (status != napi_ok) {
        HILOGE("napi_queue_async_work failed, status(%{public}d)", status);
        napi_delete_async_work(env_, info->asyncWork);
        delete info;
        return;
    }
}

void NapiAsyncWork::TimeoutCallback(void)
{
    HILOGI("enter");
    CallFunction(BT_ERR_TIMEOUT, nullptr);
}

void NapiAsyncWork::CallFunction(int errCode, std::shared_ptr<NapiNativeObject> object)
{
    if (!needCallback_.load()) {
        HILOGE("Unsupported in no needCallback mode");
        return;
    }

    HILOGI("enter");
    auto nativeObj = object;
    if (nativeObj == nullptr) {
        HILOGD("napi native object is nullptr");
        nativeObj = std::make_shared<NapiNativeEmpty>();
    }
    // Check timer triggered & remove timer if supported
    NapiTimer::GetInstance()->Unregister(timerId_);

    triggered_ = true;
    auto func = [errCode, nativeObj, asyncWorkPtr = shared_from_this()]() {
        if (asyncWorkPtr && asyncWorkPtr->napiAsyncCallback_) {
            asyncWorkPtr->napiAsyncCallback_->CallFunction(errCode, nativeObj);
        }
    };
    DoInJsMainThread(env_, std::move(func));
}

napi_value NapiAsyncWork::GetRet(void)
{
    if (!napiAsyncCallback_) {
        HILOGI("napiAsyncCallback_ is nullptr");
        return NapiGetUndefinedRet(env_);
    }
    return napiAsyncCallback_->GetRet();
}

void AsyncWorkCallFunction(NapiAsyncWorkMap &map, NapiAsyncType type, std::shared_ptr<NapiNativeObject> nativeObject,
    int status)
{
    HILOGD("type: %{public}d", type);
    auto asyncWork = map.Get(type);
    if (!asyncWork) {
        HILOGE("async work(%{public}d) is nullptr", type);
        return;
    }
    map.Erase(type);

    asyncWork->CallFunction(status, nativeObject);
}

bool NapiAsyncWorkMap::TryPush(NapiAsyncType type, std::shared_ptr<NapiAsyncWork> asyncWork)
{
    if (!asyncWork) {
        HILOGE("asyncWork is nullptr");
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(type);
    if (it != map_.end()) {
        auto const &storedAsyncWork = it->second;
        if (storedAsyncWork != nullptr && !storedAsyncWork->triggered_) {
            HILOGE("Async work(%{public}d) hasn't been triggered", type);
            return false;
        }
        HILOGI("Async work(%{public}d) hasn't been removed, but triggered, remove it", type);
        map_.erase(it);
    }

    map_[type] = std::move(asyncWork);
    return true;
}

void NapiAsyncWorkMap::Erase(NapiAsyncType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    map_.erase(type);
}

std::shared_ptr<NapiAsyncWork> NapiAsyncWorkMap::Get(NapiAsyncType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(type);
    return it != map_.end() ? it->second : nullptr;
}

}  // namespace Bluetooth
}  // namespace OHOS