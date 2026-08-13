/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef SAFE_VECTOR_H
#define SAFE_VECTOR_H

#include <vector>
#include <mutex>

namespace utility {

/**
 * @file safe_queue.h
 *
 * @brief The file contains interfaces of thread-safe queues in c_utils.
 *
 * The file contains thread-safe abstract class, the SafeQueue
 * and SafeStack that override the virtual methods of the abstract class.
 */
template <typename T>
class SafeVectorInner {
public:
    SafeVectorInner() {}

    virtual ~SafeVectorInner()
    {
        if (!vector_.empty()) {
            vector_.clear();
        }
    }

    bool IsExist(const T& object)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto iter = vector_.begin(); iter != vector_.end(); iter++) {
            if (*iter == object) {
                return true;
            }
        }
        return false;
    }

    void Erase(const T& object)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (vector_.empty()) {
            return;
        }
        auto itor = std::find(vector_.begin(), vector_.end(), object);
        if (itor != vector_.end()) {
            vector_.erase(itor);
        }
        return;
    }

    void EraseIf(const std::function<bool(T &)> &itorFunc)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = vector_.begin(); it != vector_.end();) {
            if (itorFunc(*it)) {
                it = vector_.erase(it);
            } else {
                it++;
            }
        }
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return vector_.empty();
    }

    void Push(const T& object)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return DoPush(object);
    }

    void PushNoDuplicate(const T& object)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto itor = std::find(vector_.begin(), vector_.end(), object);
        if (itor != vector_.end()) {
            vector_.erase(itor);
        }
        return DoPush(object);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!vector_.empty()) {
            vector_.clear();
        }
        return;
    }

    int Size()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return vector_.size();
    }

    std::vector<T> GetVector()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return vector_;
    }

    bool Front(T& value)
    {
        bool ret = false;
        std::lock_guard<std::mutex> lock(mutex_);
        if (vector_.empty()) {
            return ret;
        }
        value = vector_.front();
        return true;
    }

    void ForEach(const std::function<void(T &)> &itorFunc)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &it : vector_) {
            itorFunc(it);
        }
    }

protected:
    virtual void DoPush(const T& pt) = 0;

    std::vector<T> vector_;
    std::mutex mutex_;
};

/**
 * @brief Thread-safe Vector.
 *
 * Overrides the DoPush methods of abstract classes to implement
 * the push functionality of the "SafeVector".
 */
template <typename T>
class SafeVector : public SafeVectorInner<T> {
protected:
    using SafeVectorInner<T>::vector_;
    using SafeVectorInner<T>::mutex_;

    void DoPush(const T& pt) override
    {
        vector_.push_back(pt);
    }
};
}  // namespace utility

#endif  // SAFE_VECTOR_H
