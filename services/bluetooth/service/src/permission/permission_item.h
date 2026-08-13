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

#ifndef PERMISSION_ITEM_H
#define PERMISSION_ITEM_H

#include <map>
#include <set>
#include <string>

namespace OHOS {
namespace Bluetooth {
// API version.
constexpr int32_t API_VERSION_INVALID = -1;
constexpr int32_t API_VERSION_9 = 9;
constexpr int32_t API_VERSION_10 = 10;
constexpr int32_t API_VERSION_12 = 12;
class PermissionItem {
public:
    explicit PermissionItem(bool systemHapNeeded, const std::set<std::string> permissionsApi9,
        const std::set<std::string> permissionsApi10);
    ~PermissionItem();
    bool SystemHapNeeded();
    std::set<std::string> GetPermissions(int apiVer);
private:
    bool systemHapNeeded_ = false;
    std::set<std::string> permissionsApi9_ {};
    std::set<std::string> permissionsApi10_ {};
};
}  // namespace bluetooth
}  // namespace OHOS
#endif