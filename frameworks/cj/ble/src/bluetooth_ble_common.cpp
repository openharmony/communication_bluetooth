/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bluetooth_ble_common.h"

#include <regex>

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
char *MallocCString(const std::string &origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto length = origin.length() + 1;
    char *res = static_cast<char *>(malloc(sizeof(char) * length));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), length);
}

CArrString Convert2CArrString(std::vector<std::string> &tids)
{
    CArrString res{0};
    if (tids.empty()) {
        return res;
    }

    size_t size = tids.size();
    if (size == 0 || size > std::numeric_limits<size_t>::max() / sizeof(char *)) {
        return res;
    }
    res.head = static_cast<char **>(malloc(sizeof(char *) * size));
    if (res.head == nullptr) {
        return res;
    }

    size_t i = 0;
    for (; i < size; ++i) {
        res.head[i] = MallocCString(tids[i]);
    }
    res.size = static_cast<int64_t>(i);

    return res;
}

CArrUI8 Convert2CArrUI8(std::vector<uint8_t> vec)
{
    CArrUI8 res{0};
    if (vec.empty()) {
        return res;
    }
    size_t size = vec.size();
    if (size == 0 || size > std::numeric_limits<size_t>::max() / sizeof(uint8_t)) {
        return res;
    }
    res.head = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * vec.size()));
    if (res.head == nullptr) {
        return res;
    }
    size_t i = 0;
    for (; i < vec.size(); i++) {
        res.head[i] = vec[i];
    }
    res.size = static_cast<int64_t>(i);
    return res;
}

} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS