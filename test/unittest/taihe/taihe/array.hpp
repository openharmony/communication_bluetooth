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

#ifndef TAIHE_ARRAY_HPP_
#define TAIHE_ARRAY_HPP_

#include <cstddef>
#include <iterator>

namespace taihe {

template<typename T>
class array {
public:
    T *data_ = nullptr;
    size_t size_ = 0;

    T *begin() { return data_; }
    T *end() { return data_ + size_; }
    const T *begin() const { return data_; }
    const T *end() const { return data_ + size_; }
    size_t size() const { return size_; }
    T &operator[](size_t idx) { return data_[idx]; }
    const T &operator[](size_t idx) const { return data_[idx]; }
};

} // namespace taihe

#endif // TAIHE_ARRAY_HPP_