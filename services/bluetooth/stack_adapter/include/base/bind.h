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
 * Stub of the removed stack layer base bind (base/bind.h).
 * Implements base::Bind for member functions, free functions and lambdas
 * with a leading list of bound arguments; the remaining arguments become
 * the signature of the returned base::Callback.
 */

#ifndef BASE_BIND_H
#define BASE_BIND_H

#include <tuple>
#include <type_traits>
#include <utility>

#include "base/callback.h"

namespace base {
namespace detail {

template <typename T>
struct CallableTraits : CallableTraits<decltype(&std::remove_reference_t<T>::operator())> {};

template <typename R, typename C, typename... Args>
struct CallableTraits<R (C::*)(Args...)> {
    using Signature = R(Args...);
};

template <typename R, typename C, typename... Args>
struct CallableTraits<R (C::*)(Args...) const> {
    using Signature = R(Args...);
};

template <typename R, typename C, typename... Args>
struct CallableTraits<R (C::*)(Args...) noexcept> {
    using Signature = R(Args...);
};

template <typename R, typename C, typename... Args>
struct CallableTraits<R (C::*)(Args...) const noexcept> {
    using Signature = R(Args...);
};

template <typename R, typename... Args>
struct CallableTraits<R (*)(Args...)> {
    using Signature = R(Args...);
};

template <typename R, typename... Args>
struct CallableTraits<R (*)(Args...) noexcept> {
    using Signature = R(Args...);
};

template <typename R, typename... Args>
struct CallableTraits<R (&)(Args...)> : CallableTraits<R (*)(Args...)> {};

template <typename Signature>
struct SignatureSize;

template <typename R, typename... Args>
struct SignatureSize<R(Args...)> {
    using ArgsTuple = std::tuple<Args...>;
};

/*
 * Binds a free function / static member / lambda together with leading bound
 * arguments. The first signature args minus the bound ones form the returned
 * callback signature.
 */
template <typename Signature>
struct CallableBinder;

template <typename R, typename... MArgs>
struct CallableBinder<R(MArgs...)> {
    template <typename F, typename... BArgs, size_t... I>
    static Callback<R(std::tuple_element_t<sizeof...(BArgs) + I, std::tuple<MArgs...>>...)>
    Make(F f, std::tuple<BArgs...> bound, std::index_sequence<I...>)
    {
        return Callback<R(std::tuple_element_t<sizeof...(BArgs) + I, std::tuple<MArgs...>>...)>(
            [f = std::move(f), bound = std::move(bound)](
                std::tuple_element_t<sizeof...(BArgs) + I, std::tuple<MArgs...>>... args) mutable -> R {
                return std::apply(
                    [&f, &args...](auto &&...bargs) -> R {
                        return f(std::forward<decltype(bargs)>(bargs)..., args...);
                    },
                    bound);
            });
    }
};

/*
 * Binds a member function together with base::Unretained(this) and leading
 * bound arguments. The first bound argument is the object pointer, the
 * remaining bound arguments map to the leading member function parameters.
 */
template <typename Signature>
struct MemberBinder;

template <typename R, typename C, typename... MArgs>
struct MemberBinder<R (C::*)(MArgs...)> {
    template <typename... BArgs, size_t... I>
    static Callback<R(std::tuple_element_t<sizeof...(BArgs) - 1 + I, std::tuple<MArgs...>>...)>
    Make(R (C::*f)(MArgs...), std::tuple<BArgs...> bound, std::index_sequence<I...>)
    {
        return Callback<R(std::tuple_element_t<sizeof...(BArgs) - 1 + I, std::tuple<MArgs...>>...)>(
            [f, bound = std::move(bound)](
                std::tuple_element_t<sizeof...(BArgs) - 1 + I, std::tuple<MArgs...>>... args) mutable -> R {
                if constexpr (sizeof...(BArgs) == 1) {
                    return std::apply(
                        [f, &args...](auto &&obj) -> R {
                            return (obj->*f)(args...);
                        },
                        bound);
                } else {
                    return std::apply(
                        [f, &args...](auto &&...bargs) -> R {
                            auto b = std::forward_as_tuple(bargs...);
                            return (std::get<0>(b)->*f)(std::get<I + 1>(b)..., args...);
                        },
                        bound);
                }
            });
    }
};

template <typename R, typename C, typename... MArgs>
struct MemberBinder<R (C::*)(MArgs...) const> {
    template <typename... BArgs, size_t... I>
    static Callback<R(std::tuple_element_t<sizeof...(BArgs) - 1 + I, std::tuple<MArgs...>>...)>
    Make(R (C::*f)(MArgs...) const, std::tuple<BArgs...> bound, std::index_sequence<I...>)
    {
        return Callback<R(std::tuple_element_t<sizeof...(BArgs) - 1 + I, std::tuple<MArgs...>>...)>(
            [f, bound = std::move(bound)](
                std::tuple_element_t<sizeof...(BArgs) - 1 + I, std::tuple<MArgs...>>... args) mutable -> R {
                if constexpr (sizeof...(BArgs) == 1) {
                    return std::apply(
                        [f, &args...](auto &&obj) -> R {
                            return (obj->*f)(args...);
                        },
                        bound);
                } else {
                    return std::apply(
                        [f, &args...](auto &&...bargs) -> R {
                            auto b = std::forward_as_tuple(bargs...);
                            return (std::get<0>(b)->*f)(std::get<I + 1>(b)..., args...);
                        },
                        bound);
                }
            });
    }
};

template <typename Signature, typename F, typename... BArgs, size_t... I>
inline auto MakeCallable(F f, std::tuple<BArgs...> bound, std::index_sequence<I...>)
{
    return CallableBinder<Signature>::Make(std::move(f), std::move(bound),
        std::index_sequence<I...>());
}

}  // namespace detail

/*
 * Binds a member function. The first bound argument must be the object
 * pointer (typically base::Unretained(this)).
 */
template <typename R, typename C, typename... MArgs, typename... BArgs>
auto Bind(R (C::*f)(MArgs...), BArgs &&...bound)
{
    static_assert(sizeof...(BArgs) >= 1 && sizeof...(BArgs) <= sizeof...(MArgs) + 1,
        "base::Bind: bound argument count out of range");
    constexpr size_t kUnbound = sizeof...(MArgs) - (sizeof...(BArgs) - 1);
    return detail::MemberBinder<R (C::*)(MArgs...)>::Make(
        f, std::make_tuple(std::forward<BArgs>(bound)...), std::make_index_sequence<kUnbound>());
}

template <typename R, typename C, typename... MArgs, typename... BArgs>
auto Bind(R (C::*f)(MArgs...) const, BArgs &&...bound)
{
    static_assert(sizeof...(BArgs) >= 1 && sizeof...(BArgs) <= sizeof...(MArgs) + 1,
        "base::Bind: bound argument count out of range");
    constexpr size_t kUnbound = sizeof...(MArgs) - (sizeof...(BArgs) - 1);
    return detail::MemberBinder<R (C::*)(MArgs...) const>::Make(
        f, std::make_tuple(std::forward<BArgs>(bound)...), std::make_index_sequence<kUnbound>());
}

/*
 * Binds a free function, a static member function or a (captureless) lambda.
 */
template <typename F, typename... BArgs,
    typename = std::enable_if_t<!std::is_member_function_pointer_v<std::decay_t<F>>>>
auto Bind(F &&f, BArgs &&...bound)
{
    using Traits = detail::CallableTraits<std::decay_t<F>>;
    using Signature = typename Traits::Signature;
    constexpr size_t kNumArgs = std::tuple_size_v<typename detail::SignatureSize<Signature>::ArgsTuple>;
    static_assert(sizeof...(BArgs) <= kNumArgs, "base::Bind: too many bound arguments");
    constexpr size_t kUnbound = kNumArgs - sizeof...(BArgs);
    return detail::MakeCallable<Signature>(
        std::forward<F>(f), std::make_tuple(std::forward<BArgs>(bound)...), std::make_index_sequence<kUnbound>());
}

}  // namespace base

#endif  // BASE_BIND_H
