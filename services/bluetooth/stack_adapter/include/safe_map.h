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
 * Stub of the removed stack layer safe_map.h, a mutex-guarded map used by
 * the service layer.
 */

#ifndef SAFE_MAP_H
#define SAFE_MAP_H

#include <map>
#include <mutex>

template <typename K, typename V>
class SafeMap {
public:
    SafeMap() = default;
    ~SafeMap() = default;

    SafeMap(const SafeMap &) = delete;
    SafeMap &operator=(const SafeMap &) = delete;

    void EnsureInsert(const K &key, const V &value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_[key] = value;
    }

    void Insert(const K &key, const V &value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_[key] = value;
    }

    void Erase(const K &key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.erase(key);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        map_.clear();
    }

    size_t Size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.size();
    }

    bool IsEmpty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.empty();
    }

    bool Contains(const K &key) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.find(key) != map_.end();
    }

    bool Find(const K &key, V &value) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it == map_.end()) {
            return false;
        }
        value = it->second;
        return true;
    }

    bool Get(const K &key, V &value) const
    {
        return Find(key, value);
    }

    template <typename F>
    void Iterate(F func)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &it : map_) {
            func(it.first, it.second);
        }
    }

    V &operator[](const K &key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_[key];
    }

private:
    mutable std::mutex mutex_;
    std::map<K, V> map_;
};

#endif  // SAFE_MAP_H
