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

#include <fstream>
#include "json_utils.h"

namespace OHOS {
namespace bluetooth {
int32_t JsonUtil::GetInt32(const cJSON* value, const std::string& key, int32_t defaultValue)
{
    cJSON* target = cJSON_GetObjectItemCaseSensitive(value, key.c_str());
    if (target == nullptr || !cJSON_IsNumber(target)) {
        return defaultValue;
    }
    return target->valueint;
}

std::string JsonUtil::GetString(const cJSON* value, const std::string& key, const std::string& defaultValue)
{
    cJSON* target = cJSON_GetObjectItemCaseSensitive(value, key.c_str());
    if (target == nullptr || !cJSON_IsString(target)) {
        return defaultValue;
    }
    return target->valuestring;
}

std::string JsonUtil::GetSimpleFormatString(const cJSON* value)
{
    if (value == nullptr) {
        return "";
    }
    char* chrData = cJSON_PrintUnformatted(value);
    if (chrData == nullptr) {
        return "";
    }
    std::string result(chrData);
    cJSON_free(chrData);
    return result;
}

cJSON* JsonUtil::ReadJsonFile(const std::string &path)
{
    if (path.length() >= PATH_MAX) {
        return nullptr;
    }
    char resolved_path[PATH_MAX] = {0};
    char *result = realpath(path.c_str(), resolved_path);
    if (result == nullptr) {
        return nullptr;
    }
    std::ifstream ifs(std::string(result, strlen(result)));
    if (!ifs.is_open()) {
        return nullptr;
    }
    std::string jsonStr((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
    ifs.close();
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    return root;
}

cJSON* JsonUtil::ReadJsonContents(const std::string &jsonContents)
{
    cJSON* root = cJSON_Parse(jsonContents.c_str());
    return root;
}
}
}
