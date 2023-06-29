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

#ifndef BLUETOOTH_OBJECT_MAP_H
#define BLUETOOTH_OBJECT_MAP_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>

constexpr uint32_t MAX_MAP_SIZE = 100;
template <typename T, int maxSize = MAX_MAP_SIZE>
class BluetoothObjectMap final {
public:
    BluetoothObjectMap() = default;
    ~BluetoothObjectMap()
    {
        std::lock_guard<std::mutex> lock(lock_);
        objectsMap.clear();
    }

    int AddObject(T object)
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (objectIncrease > maxSize) {
            objectIncrease = 1;
        }
        objectsMap.insert(std::make_pair(objectIncrease, object));
        return objectIncrease++;
    }

    void RemoveObject(int objectId)
    {
        std::lock_guard<std::mutex> lock(lock_);
        objectsMap.erase(objectId);
    }

    void RemoveAllObject(void)
    {
        std::lock_guard<std::mutex> lock(lock_);
        objectsMap.clear();
    }

    T GetObject(int objectId)
    {
        std::lock_guard<std::mutex> lock(lock_);
        auto iter = objectsMap.find(objectId);
        if (iter == objectsMap.end()) {
            return nullptr;
        }
        return iter->second;
    }

    T GetObject()
    {
        std::lock_guard<std::mutex> lock(lock_);
        for (int i = 1; i < maxSize; i++) {
            auto iter = objectsMap.find(i);
            if (iter != objectsMap.end()) {
                return iter->second;
            }
        }
        return nullptr;
    }

    int AddLimitedObject(T object)
    {
        std::lock_guard<std::mutex> lock(lock_);
        int i;
        for (i = 1; i < maxSize; i++) {
            if (objectsMap.find(i) == objectsMap.end()) {
                break;
            }
        }
        if (i == maxSize) {
            return -1;
        }
        objectsMap.insert(std::make_pair(i, object));
        return i;
    }

private:
    std::mutex lock_;
    std::map<int, T> objectsMap;
    int objectIncrease = 1;

    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothObjectMap);
};

#endif  // BLUETOOTH_OBJECT_MAP_H