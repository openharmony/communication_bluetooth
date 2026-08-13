/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_chr_pkgname_manager"
#endif
 
#include "bt_chr_pkgname_manager.h"
#include "bt_chr_business_event.h"
#include <string>
#include "safe_map.h"
#include "bluetooth_log.h"
#include "bt_chr_base.h"
 
namespace OHOS {
namespace bluetooth {

constexpr uint8_t MAX_RENDER_RUNNING_APPCOUNT = 5; /* Maximum running media app count */
 
BtChrPkgNameManager& BtChrPkgNameManager::GetInstance()
{
    static BtChrPkgNameManager instance;
    return instance;
}

void BtChrPkgNameManager::UpdateRenderPkgName(const std::string &bundleName, const bool isRendererRunning,
    const BtChrBusinessType businessType, uint32_t streamId)
{
    if (bundleName.empty() || businessType >= BUSINESS_TYPE_MAX_VALUE) {
        HILOGI("invalid bundleName or invalid businessType=%{public}d", businessType);
        return;
    }
    BtChrPkgNameInfo pkgNameInfo = {};
    pkgNameInfo.pkgName = bundleName;
    pkgNameInfo.businessType = businessType;

    std::lock_guard<std::mutex> lock(renderPkgNameSetMutex_);
    auto it = std::find_if(renderPkgNameSet_.begin(), renderPkgNameSet_.end(), [&](const BtChrPkgNameInfo &info) {
        return info.pkgName == bundleName && info.businessType == businessType;
    });
    if (it != renderPkgNameSet_.end()) {
        pkgNameInfo.streamIds = it->streamIds;
        // Remove the old pkgNameInfo
        DeleteRenderPkgName(*it);
        // Update the streamIds
        if (isRendererRunning) {
            pkgNameInfo.streamIds.insert(streamId);
        } else {
            pkgNameInfo.streamIds.erase(streamId);
        }
        // Reinsert the updated pkgNameInfo
        if (!pkgNameInfo.streamIds.empty()) {
            SetRenderPkgName(pkgNameInfo);
        }
    } else if (isRendererRunning) {
        pkgNameInfo.streamIds.insert(streamId);
        if (renderPkgNameSet_.empty()) {
            // record first start Pkgname used by UE event
            BtChrBusinessEvent::GetInstance().SavePkgName(bundleName);
        }
        SetRenderPkgName(pkgNameInfo);
    }
}

void BtChrPkgNameManager::SetRenderPkgName(const BtChrPkgNameInfo& pkgNameInfo)
{
    if (renderPkgNameSet_.size() < MAX_RENDER_RUNNING_APPCOUNT) {
        renderPkgNameSet_.emplace(pkgNameInfo);
    }
}
 
void BtChrPkgNameManager::DeleteRenderPkgName(const BtChrPkgNameInfo& pkgNameInfo)
{
    renderPkgNameSet_.erase(pkgNameInfo);
}
 
std::string BtChrPkgNameManager::GetRenderPkgName(const BtChrBusinessType businessType)
{
    std::string renderPkgNames = "";
    std::lock_guard<std::mutex> lock(renderPkgNameSetMutex_);
    for (const auto& str : renderPkgNameSet_) {
        if (str.businessType == businessType) {
            if (!renderPkgNames.empty()) {
                renderPkgNames += ",";
            }
            renderPkgNames += str.pkgName;
        }
    }
    return renderPkgNames;
}
 
}  // namespace bluetooth
}  // namespace OHOS