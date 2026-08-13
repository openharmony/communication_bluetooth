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

#ifndef AV_SYNC_CONFIG_INFO_H
#define AV_SYNC_CONFIG_INFO_H

#include "raw_address.h"
#include "safe_map.h"

namespace OHOS {
namespace bluetooth {

/**
 * @brief This class implements the av sync config info.
 */
class AvSyncConfigInfo {
public:
    explicit AvSyncConfigInfo();
    ~AvSyncConfigInfo() = default;

    void AddLatencyBaseByDeviceName(std::string deviceName, int32_t avLatency);
    void AddLatencyBaseByModelId(std::string modelId, int32_t avLatency);
    void AddLatencyBaseByNewModelId(std::string newModelId, int32_t avLatency);
    
    int32_t GetLatencyBase(const RawAddress &rawAddr);
    void ClearLatencyInfo();
private:
    int32_t GetLatencyBaseByDeviceName(std::string deviceName);
    int32_t GetLatencyBaseByModelId(std::string modelId);
    int32_t GetLatencyBaseByNewModelId(std::string newModelId);

private:
    SafeMap<std::string, int32_t> deviceNameLatencyBaseMap_;
    SafeMap<std::string, int32_t> modelIdLatencyBaseMap_;
    SafeMap<std::string, int32_t> newModelIdLatencyBaseMap_;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif
