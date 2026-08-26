/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer base callback (base/callback.h).
 * Provides base::Callback with the AOSP-like Run()/is_null() interface used
 * by the service layer.
 */

#ifndef BASE_CALLBACK_H
#define BASE_CALLBACK_H

#include <functional>
#include <type_traits>
#include <utility>

namespace base {

template <typename Signature>
class Callback;

template <typename R, typename... Args>
class Callback<R(Args...)> {
public:
    Callback() = default;
    Callback(const Callback &) = default;
    Callback(Callback &&) = default;
    Callback &operator=(const Callback &) = default;
    Callback &operator=(Callback &&) = default;

    template <typename F, typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, Callback>>>
    Callback(F &&f) : fn_(std::forward<F>(f)) {}

    bool is_null() const
    {
        return !fn_;
    }

    bool IsNull() const
    {
        return is_null();
    }

    explicit operator bool() const
    {
        return static_cast<bool>(fn_);
    }

    void Reset()
    {
        fn_ = nullptr;
    }

    R Run(Args... args) const
    {
        return fn_(std::forward<Args>(args)...);
    }

    R operator()(Args... args) const
    {
        return fn_(std::forward<Args>(args)...);
    }

private:
    std::function<R(Args...)> fn_;
};

template <typename T>
inline T *Unretained(T *obj)
{
    return obj;
}

}  // namespace base

#endif  // BASE_CALLBACK_H
