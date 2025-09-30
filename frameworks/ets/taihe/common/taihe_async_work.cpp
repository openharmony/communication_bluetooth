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
#define LOG_TAG "bt_taihe_async_work"
#endif

#include "taihe_async_work.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "taihe_async_callback.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_timer.h"
#include "taihe_parser_utils.h"

namespace OHOS {
namespace Bluetooth {
std::shared_ptr<TaiheAsyncWork> TaiheAsyncWorkFactory::CreateAsyncWork(ani_vm *vm, ani_env *env, ani_object info,
    std::function<TaiheAsyncWorkRet(void)> asyncWork, bool needCallback, std::shared_ptr<TaiheHaEventUtils> haUtils)
{
    HILOGI("CreateAsyncWork enter");
    auto asyncCallback = TaiheParseAsyncCallback(vm, env, info);
    if (!asyncCallback) {
        HILOGE("asyncCallback is nullptr!");
        return nullptr;
    }
    HILOGI("CreateAsyncWork asyncCallback created");
    // add custom deleter for destructing in JS thread.
    std::shared_ptr<TaiheAsyncWork> taiheAsyncWork(
        new TaiheAsyncWork(vm, env, asyncWork, asyncCallback, needCallback, haUtils),
        [env](TaiheAsyncWork *ptr) {
            if (ptr) {
                HILOGI("CreateAsyncWork delete ptr");
                delete ptr;
            }
        });
    HILOGI("CreateAsyncWork leave");
    return taiheAsyncWork;
}

void TaiheAsyncWork::Info::Execute(void)
{
    HILOGI("Execute enter");
    if (taiheAsyncWork == nullptr) {
        HILOGE("taiheAsyncWork is nullptr");
        errCode = BT_ERR_INTERNAL_ERROR;
        object = nullptr;
        return;
    }
    auto ret = taiheAsyncWork->func_();
    errCode = ret.errCode;
    object = ret.object;
    HILOGI("Execute leave");
}

void TaiheAsyncWork::Info::Complete(void)
{
    HILOGD("needCallback: %{public}d, errCode: %{public}d", needCallback, errCode);
    if (taiheAsyncWork == nullptr) {
        HILOGE("taiheAsyncWork is nullptr");
        return;
    }

    // need wait callback
    if (needCallback && (errCode == BT_NO_ERROR)) {
        if (taiheAsyncWork->triggered_) {
            HILOGE("TaiheAsyncWork is triggered, Callback is earlier than Complete in thread scheduling");
            return;
        }
        // start timer to avoid the callback is lost.
        std::weak_ptr<TaiheAsyncWork> asyncWorkWptr = taiheAsyncWork;
        auto func = [asyncWorkWptr]() {
            auto asyncWorkSptr = asyncWorkWptr.lock();
            if (asyncWorkSptr == nullptr) {
                HILOGE("asyncWorkSptr is nullptr");
                return;
            }
            asyncWorkSptr->TimeoutCallback();
        };
        TaiheTimer::GetInstance()->Register(func, taiheAsyncWork->timerId_);
        return;
    }

    if (object == nullptr) {
        HILOGD("taihe native object is nullptr");
        object = std::make_shared<TaiheNativeUndefined>();
    }

    if (taiheAsyncWork->taiheAsyncCallback_) {
        taiheAsyncWork->triggered_ = true;
        auto haUtils = taiheAsyncWork->GetHaUtilsPtr();
        if (haUtils) {
            haUtils->WriteErrCode(errCode);
        }

        auto work = taiheAsyncWork;
        auto code = errCode;
        auto obj = object;
        std::thread workerThread([work, code, obj]() {
            work->taiheAsyncCallback_->CallFunction(code, obj);
        });
        workerThread.join();
    }
}

void TaiheAsyncWork::Run(void)
{
    HILOGI("Run enter");
    TaiheAsyncWork::Info *info = new TaiheAsyncWork::Info();
    info->needCallback = needCallback_.load();
    info->taiheAsyncWork = shared_from_this();

    info->Execute();
    info->Complete();
    delete info;
    HILOGI("Run leave");
}

std::shared_ptr<TaiheHaEventUtils> TaiheAsyncWork::GetHaUtilsPtr(void) const
{
    return haUtils_;
}

void TaiheAsyncWork::TimeoutCallback(void)
{
    HILOGI("enter");
    CallFunction(BT_ERR_TIMEOUT, nullptr);
}

void TaiheAsyncWork::CallFunction(int errCode, std::shared_ptr<TaiheNativeObject> object)
{
    if (!needCallback_.load()) {
        HILOGE("Unsupported in no needCallback mode");
        if (haUtils_) {
            haUtils_->WriteErrCode(BT_ERR_INTERNAL_ERROR);
        }
        return;
    }

    HILOGI("enter");
    auto nativeObj = object;
    if (nativeObj == nullptr) {
        HILOGD("taihe native object is nullptr");
        nativeObj = std::make_shared<TaiheNativeUndefined>();
    }
    // Check timer triggered & remove timer if supported
    TaiheTimer::GetInstance()->Unregister(timerId_);

    triggered_ = true;
    taiheAsyncCallback_->CallFunction(errCode, nativeObj);
}

ani_object TaiheAsyncWork::GetRet(void)
{
    if (!taiheAsyncCallback_) {
        HILOGI("taiheAsyncCallback_ is nullptr");
        return reinterpret_cast<ani_object>(TaiheGetUndefinedRet(env_));
    }
    return taiheAsyncCallback_->GetRet();
}

void AsyncWorkCallFunction(TaiheAsyncWorkMap &map, TaiheAsyncType type, std::shared_ptr<TaiheNativeObject> nativeObject,
    int status)
{
    HILOGD("type: %{public}d", type);
    auto asyncWork = map.Get(type);
    if (!asyncWork) {
        HILOGD("async work(%{public}d) is nullptr", type);
        return;
    }
    map.Erase(type);

    asyncWork->CallFunction(status, nativeObject);
}

bool TaiheAsyncWorkMap::TryPush(TaiheAsyncType type, std::shared_ptr<TaiheAsyncWork> asyncWork)
{
    HILOGI("TryPush enter");
    if (!asyncWork) {
        HILOGE("asyncWork is nullptr");
        return false;
    }
    HILOGI("TryPush asyncWork");
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
    HILOGI("TryPush map_");
    map_[type] = std::move(asyncWork);
    HILOGI("TryPush leave");
    return true;
}

void TaiheAsyncWorkMap::Erase(TaiheAsyncType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    map_.erase(type);
}

std::shared_ptr<TaiheAsyncWork> TaiheAsyncWorkMap::Get(TaiheAsyncType type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(type);
    return it != map_.end() ? it->second : nullptr;
}
}  // namespace Bluetooth
}  // namespace OHOS