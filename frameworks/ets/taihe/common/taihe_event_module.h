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

#ifndef TAIHE_EVENT_MODULE_H
#define TAIHE_EVENT_MODULE_H

#include <shared_mutex>
#include <vector>
#include <taihe/optional.hpp>
#include <taihe/callback.hpp>
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Bluetooth {
template<typename T>
class EventModule {
public:
    void RegisterEvent(::taihe::callback_view<T> callback);
    void DeregisterEvent(::taihe::optional_view<::taihe::callback<T>> callback);

    std::vector<::taihe::optional<::taihe::callback<T>>> callbackVec_;
    std::shared_mutex lock_;
};

class TaiheUtils {
public:
    static int GetProfileConnectionState(int state);
};

template<typename T>
void EventModule<T>::RegisterEvent(::taihe::callback_view<T> callback)
{
    std::unique_lock<std::shared_mutex> guard(lock_);
    auto eventCb = ::taihe::optional<::taihe::callback<T>>{std::in_place_t{}, callback};
    if (std::find(callbackVec_.begin(), callbackVec_.end(), eventCb) != callbackVec_.end()) {
        return;
    }
    callbackVec_.emplace_back(eventCb);
}

template<typename T>
void EventModule<T>::DeregisterEvent(::taihe::optional_view<::taihe::callback<T>> callback)
{
    std::unique_lock<std::shared_mutex> guard(lock_);
    if (callback.has_value()) {
        callbackVec_.erase(
            std::remove_if(callbackVec_.begin(), callbackVec_.end(),
                [callback](const ::taihe::optional<::taihe::callback<T>>& it) {
                    return it == callback;
                }),
            callbackVec_.end());
    } else {
        callbackVec_.clear();
    }
}
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_EVENT_MODULE_H