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

#ifndef AV_SYNC_MANAGER_H
#define AV_SYNC_MANAGER_H

#include <map>
#include "cJSON.h"
#include "safe_map.h"
#include "raw_address.h"
#include "av_sync_config_info.h"

namespace OHOS {
namespace bluetooth {

/**
 * @brief This class implements the av sync manager.
 */
class AvSyncManager {
public:
    explicit AvSyncManager();
    ~AvSyncManager() = default;

    void SaveHiechoAvLatency(const RawAddress &rawAddr, int32_t avLatency);
    void ReadLatencyConfig();
    void UpdateBtAudioLatency(const RawAddress &rawAddr);
    void ClearLatencyInfoByAddr(const RawAddress &rawAddr);
    void ClearLatencyInfo();
private:
    bool ParseLatencyConfig(cJSON* avLatencyList);
    int32_t GetAvSyncLatency(const RawAddress &rawAddr);

private:
    bool isAlreadyLoad_ {false};
    SafeMap<std::string, int32_t> hiechoAvLatencyMap_;
    AvSyncConfigInfo avSyncConfigInfo_ {};
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
