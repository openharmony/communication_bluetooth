/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <cstddef>
#include "cJSON.h"

namespace OHOS {
namespace bluetooth {
class JsonUtil {
public:

    static int32_t GetInt32(const cJSON* value, const std::string& key, int32_t defaultValue);
    static std::string GetString(const cJSON* value, const std::string& key, const std::string& defaultValue);
    static std::string GetSimpleFormatString(const cJSON* value);
    static cJSON* ReadJsonFile(const std::string &path);
    static cJSON* ReadJsonContents(const std::string &jsonContents);
};
}  // namespace bluetooth
}  // namespace OHOS
#endif // JSON_UTILS_H
