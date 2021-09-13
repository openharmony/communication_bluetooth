/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef PROFILE_LIST_H
#define PROFILE_LIST_H

#include "base_def.h"
#include <string>
#include <vector>

namespace bluetooth {

template <typename T>
class ProfilesList final {
public:
    ProfilesList() = default;
    ~ProfilesList()
    {}

    T &Get(const BTTransport transport, std::string name)
    {
        return profiles_[transport][name];
    }

    void Clear()
    {
        profiles_.clear();
    }

    std::map<std::string, T> *GetProfiles(const BTTransport transport)
    {
        auto it = profiles_.find(transport);
        if (it != profiles_.end()) {  // find
            return &(profiles_[transport]);
        } else {
            return nullptr;
        }
    }

    bool contains(const BTTransport transport, const std::string &name)
    {
        auto its = profiles_.find(transport);
        if (its != profiles_.end()) {
            auto it = profiles_[transport].find(name);
            if (it != profiles_[transport].end()) {
                return true;
            }
        }
        return false;
    }

    bool contains(const std::string &name)
    {
        for (auto &its : profiles_) {
            auto it = profiles_[its.first].find(name);
            if (it != profiles_[its.first].end()) {
                return true;
            }
        }
        return false;
    }

    bool Find(const BTTransport transport, const std::string &name, T *data)
    {
        auto its = profiles_.find(transport);
        if (its != profiles_.end()) {
            auto it = profiles_[transport].find(name);
            if (it != profiles_[transport].end()) {
                *data = profiles_[transport][name];
                return true;
            }
        }
        return false;
    }

    bool Find(const std::string &name, T *data)
    {
        for (auto its = profiles_.begin(); its != profiles_.end(); its++) {
            auto it = profiles_[its->first].find(name);
            if (it != profiles_[its->first].end()) {
                *data = profiles_[its->first][name];
                return true;
            }
        }
        return false;
    }

    bool IsEmpty(const BTTransport transport)
    {
        auto it = profiles_.find(transport);
        if (it != profiles_.end()) {  // find
            return profiles_[transport].empty();
        } else {
            return true;
        }
    }

    int Size(const BTTransport transport)
    {
        auto it = profiles_.find(transport);
        if (it != profiles_.end()) {  // find
            return profiles_[transport].size();
        } else {
            return 0;
        }
    }

private:
    std::map<BTTransport, std::map<std::string, T>> profiles_;
};

#define FOR_EACH_LIST(it, profileList, transport) for (auto &it : *(profileList.GetProfiles(transport)))

};  // namespace bluetooth

#endif  // PROFILE_CONFIG_H