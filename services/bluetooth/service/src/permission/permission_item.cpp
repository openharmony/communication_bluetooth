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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_permission_item"
#endif

#include "permission_item.h"
#include "log.h"

namespace OHOS {
namespace Bluetooth {

PermissionItem::PermissionItem(bool systemHapNeeded, const std::set<std::string> permissionsApi9,
    const std::set<std::string> permissionsApi10)
{
    systemHapNeeded_ = systemHapNeeded;
    permissionsApi9_ = permissionsApi9;
    permissionsApi10_ = permissionsApi10;
}

PermissionItem::~PermissionItem()
{}

bool PermissionItem::SystemHapNeeded()
{
    return systemHapNeeded_;
}

std::set<std::string> PermissionItem::GetPermissions(int apiVer)
{
    if (apiVer != API_VERSION_9 && apiVer != API_VERSION_10) {
        HILOGE("[PERMISSION] invalid input: apiVer{%{public}d}", apiVer);
        return {};
    }
    return (apiVer == API_VERSION_9) ? permissionsApi9_ : permissionsApi10_;
}

}  // namespace bluetooth
}  // namespace OHOS
