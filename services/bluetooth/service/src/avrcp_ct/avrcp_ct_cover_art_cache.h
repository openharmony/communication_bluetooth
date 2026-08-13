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

#ifndef AVRCP_CT_COVER_ART_CACHE_H
#define AVRCP_CT_COVER_ART_CACHE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

namespace OHOS {
namespace bluetooth {

class AvrcpCtCoverArtCache {
public:
    AvrcpCtCoverArtCache();
    ~AvrcpCtCoverArtCache() = default;

    void AddImage(const std::string &deviceAddr, const std::string &imageUuid, const std::vector<uint8_t> &imageData);

    bool GetImage(const std::string &deviceAddr, const std::string &imageUuid, std::vector<uint8_t> &imageData) const;

    bool HasImage(const std::string &deviceAddr, const std::string &imageUuid) const;

    void RemoveImage(const std::string &deviceAddr, const std::string &imageUuid);

    void ClearDevice(const std::string &deviceAddr);

    void ClearAll();

    size_t GetDeviceCacheSize(const std::string &deviceAddr) const;

    size_t GetTotalCacheSize() const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<uint8_t>>> cache_;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // AVRCP_CT_COVER_ART_CACHE_H