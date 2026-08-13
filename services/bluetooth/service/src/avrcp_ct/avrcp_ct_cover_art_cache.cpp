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
#define LOG_TAG "avrcp_ct_cover_art_cache"
#endif

#include "avrcp_ct_cover_art_cache.h"
#include "log.h"

namespace OHOS {
namespace bluetooth {

AvrcpCtCoverArtCache::AvrcpCtCoverArtCache() {}

void AvrcpCtCoverArtCache::AddImage(const std::string &deviceAddr, const std::string &imageUuid,
                                    const std::vector<uint8_t> &imageData)
{
    if (deviceAddr.empty() || imageUuid.empty() || imageData.empty()) {
        HILOGE("Invalid parameters for AddImage");
        return;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto &deviceCache = cache_[deviceAddr];
    deviceCache[imageUuid] = imageData;
    HILOGD("Added cover art for device: %{public}s, uuid: %{public}s, size: %{public}zu", deviceAddr.c_str(),
           imageUuid.c_str(), imageData.size());
}

bool AvrcpCtCoverArtCache::GetImage(const std::string &deviceAddr, const std::string &imageUuid,
                                     std::vector<uint8_t> &imageData) const
{
    if (deviceAddr.empty() || imageUuid.empty()) {
        HILOGE("Invalid parameters for GetImage");
        return false;
    }
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto deviceIt = cache_.find(deviceAddr);
    if (deviceIt == cache_.end()) {
        HILOGD("No cache for device: %{public}s", deviceAddr.c_str());
        return false;
    }

    auto &deviceCache = deviceIt->second;
    auto imageIt = deviceCache.find(imageUuid);
    if (imageIt == deviceCache.end()) {
        HILOGD("No cover art for uuid: %{public}s", imageUuid.c_str());
        return false;
    }

    imageData.assign(imageIt->second.begin(), imageIt->second.end());
    return true;
}

bool AvrcpCtCoverArtCache::HasImage(const std::string &deviceAddr, const std::string &imageUuid) const
{
    if (deviceAddr.empty() || imageUuid.empty()) {
        HILOGE("Invalid parameters for HasImage");
        return false;
    }
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto deviceIt = cache_.find(deviceAddr);
    if (deviceIt == cache_.end()) {
        return false;
    }

    auto &deviceCache = deviceIt->second;
    return deviceCache.find(imageUuid) != deviceCache.end();
}

void AvrcpCtCoverArtCache::RemoveImage(const std::string &deviceAddr, const std::string &imageUuid)
{
    if (deviceAddr.empty() || imageUuid.empty()) {
        HILOGE("Invalid parameters for RemoveImage");
        return;
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto deviceIt = cache_.find(deviceAddr);
    if (deviceIt == cache_.end()) {
        return;
    }

    auto &deviceCache = deviceIt->second;
    auto it = deviceCache.find(imageUuid);
    if (it != deviceCache.end()) {
        deviceCache.erase(it);
        HILOGD("Removed cover art for device: %{public}s, uuid: %{public}s", deviceAddr.c_str(), imageUuid.c_str());
    }

    if (deviceCache.empty()) {
        cache_.erase(deviceIt);
    }
}

void AvrcpCtCoverArtCache::ClearDevice(const std::string &deviceAddr)
{
    if (deviceAddr.empty()) {
        HILOGE("Invalid parameters for ClearDevice");
        return;
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (cache_.find(deviceAddr) != cache_.end()) {
        cache_.erase(deviceAddr);
        HILOGD("Cleared cover art cache for device: %{public}s", deviceAddr.c_str());
    }
}

void AvrcpCtCoverArtCache::ClearAll()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    cache_.clear();
    HILOGD("Cleared all cover art cache");
}

size_t AvrcpCtCoverArtCache::GetDeviceCacheSize(const std::string &deviceAddr) const
{
    if (deviceAddr.empty()) {
        HILOGE("Invalid parameters for GetDeviceCacheSize");
        return 0;
    }
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto deviceIt = cache_.find(deviceAddr);
    if (deviceIt == cache_.end()) {
        return 0;
    }
    return deviceIt->second.size();
}

size_t AvrcpCtCoverArtCache::GetTotalCacheSize() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    size_t total = 0;
    for (const auto &devicePair : cache_) {
        total += devicePair.second.size();
    }
    return total;
}

}  // namespace bluetooth
}  // namespace OHOS