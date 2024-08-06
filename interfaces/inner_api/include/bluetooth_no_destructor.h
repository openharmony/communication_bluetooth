/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef NO_DESTRUCTOR_H
#define NO_DESTRUCTOR_H

#include <new>
#include <utility>

namespace OHOS {
namespace Bluetooth {
// Only used in Singleton pattern, for example: static BluetoothNoDestructor<SingletonDemo> instance;
template <typename T>
class BluetoothNoDestructor {
public:
    template <typename... Args>
    explicit BluetoothNoDestructor(Args&&... args)
    {
        new (buff_) T(std::forward<Args>(args)...);
    }

    explicit BluetoothNoDestructor(const T& x) { new (buff_) T(x); }
    explicit BluetoothNoDestructor(T&& x) { new (buff_) T(std::move(x)); }

    ~BluetoothNoDestructor() = default;

    BluetoothNoDestructor(const BluetoothNoDestructor&) = delete;
    BluetoothNoDestructor& operator=(const BluetoothNoDestructor&) = delete;

    const T& operator*() const { return *get(); }
    T& operator*() { return *get(); }

    const T* get() const { return reinterpret_cast<const T*>(buff_); }
    T* get() { return reinterpret_cast<T*>(buff_); }

private:
    alignas(T) char buff_[sizeof(T)];
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NO_DESTRUCTOR_H
