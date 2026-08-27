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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_av_sync_manager"
#endif

#include "av_sync_manager.h"
#include "av_sync_common.h"
#include "bluetooth_hw_interface.h"
#include "common_util.h"
#include "log.h"
#include "json_utils.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
AvSyncManager::AvSyncManager()
{}

void AvSyncManager::SaveHiechoAvLatency(const RawAddress &rawAddr, int32_t avLatency)
{
    HILOGI("addr: %{public}s, avLatency: %{public}d", GET_ENCRYPT_ADDR(rawAddr), avLatency);
    hiechoAvLatencyMap_.EnsureInsert(rawAddr.GetAddress(), avLatency);
}

void AvSyncManager::ReadLatencyConfig()
{
    if (isAlreadyLoad_) {
        HILOGI("av_sync, is already load");
        return;
    }
    HILOGI("isAlreadyLoad: %{public}d", isAlreadyLoad_);
    DoInA2dpThread([this]() {
        // read bt_av_latency_config.json
        cJSON* avLatencyList = JsonUtil::ReadJsonFile(AV_LATENCY_CONFIG_PATH);
        if (avLatencyList == nullptr) {
            HILOGE("av_sync, ReadJsonFile failed");
            avSyncConfigInfo_.ClearLatencyInfo();
            return;
        }
        bool result = ParseLatencyConfig(avLatencyList);
        cJSON_Delete(avLatencyList);
        if (!result) {
            HILOGE("av_sync, ParseLatencyConfig failed");
            // 清除configInfo
            avSyncConfigInfo_.ClearLatencyInfo();
            return;
        }
        isAlreadyLoad_ = true;
    });
}

bool AvSyncManager::ParseLatencyConfig(cJSON* avLatencyList)
{
    if (!cJSON_IsArray(avLatencyList) || cJSON_GetArraySize(avLatencyList) == 0) {
        HILOGE("av_sync, avLatencyList is empty");
        return false;
    }
    cJSON* firstElement = cJSON_GetArrayItem(avLatencyList, 0);
    cJSON* latencyBaseList = cJSON_GetObjectItemCaseSensitive(firstElement, TAG_LATENCY_BASE);
    if (latencyBaseList == nullptr || !cJSON_IsArray(latencyBaseList) || cJSON_GetArraySize(latencyBaseList) == 0) {
        HILOGE("av_sync, latencyBaseList is empty");
        return false;
    }
    HILOGI("av_sync, latencyBaseList size: %{public}d", cJSON_GetArraySize(latencyBaseList));
    for (int i = 0; i < cJSON_GetArraySize(latencyBaseList); i++) {
        cJSON* item = cJSON_GetArrayItem(latencyBaseList, i);
        if (item == nullptr) {
            HILOGE("av_sync, latencyBaseList item is null");
            return false;
        }
        int32_t latencyMs = JsonUtil::GetInt32(item, TAG_LATENCY_MS, INVALID_LATENCY);
        if (latencyMs == INVALID_LATENCY) {
            HILOGE("av_sync, latencyMs is invalid");
            continue;
        }
        std::string deviceName = JsonUtil::GetString(item, TAG_DEVICE_NAME, "");
        if (!deviceName.empty()) {
            avSyncConfigInfo_.AddLatencyBaseByDeviceName(deviceName, latencyMs);
            continue;
        }
        std::string modelId = JsonUtil::GetString(item, TAG_MODEL_ID, "");
        if (!modelId.empty()) {
            avSyncConfigInfo_.AddLatencyBaseByModelId(modelId, latencyMs);
            continue;
        }
        std::string newModelId = JsonUtil::GetString(item, TAG_NEW_MODEL_ID, "");
        if (!newModelId.empty()) {
            avSyncConfigInfo_.AddLatencyBaseByNewModelId(newModelId, latencyMs);
            continue;
        }
    }
    return true;
}

void AvSyncManager::UpdateBtAudioLatency(const RawAddress &rawAddr)
{
    int32_t latency = GetAvSyncLatency(rawAddr);
    if ((latency > INVALID_LATENCY) && (latency < MAX_AV_LATENCY)) {
        // bluedroid latency unit is 0.1ms
        latency *= LATENCY_UNIT;
    } else {
        latency = DEFAULT_AV_LATENCY;
    }

    auto bthwif = BluetoothHwInterface::GetInstance()->GetBtHwInterface();
    if (bthwif == nullptr) {
        HILOGE("Failed to get hw bluetooth interface");
        return;
    }
    STACK::RawAddress dev = ServiceUtil::AddrToStack(rawAddr);
    bthwif->hwUpdateBtAudioLatency(dev, latency);
    HILOGI("av_sync, %{public}s update latency %{public}d", GET_ENCRYPT_ADDR(rawAddr), latency);
}

int32_t AvSyncManager::GetAvSyncLatency(const RawAddress &rawAddr)
{
    int32_t avLatency = INVALID_LATENCY;
    if (hiechoAvLatencyMap_.Find(rawAddr.GetAddress(), avLatency) && avLatency != INVALID_LATENCY) {
        HILOGI("av_sync, getAvSyncLatency from hiecho, avLatency: %{public}d", avLatency);
        return avLatency;
    }
    avLatency = avSyncConfigInfo_.GetLatencyBase(rawAddr);
    if (avLatency != INVALID_LATENCY) {
        HILOGI("av_sync, getAvSyncLatency from base, avLatency: %{public}d", avLatency);
        return avLatency;
    }
    return INVALID_LATENCY;
}

void AvSyncManager::ClearLatencyInfoByAddr(const RawAddress &rawAddr)
{
    hiechoAvLatencyMap_.Erase(rawAddr.GetAddress());
}

void AvSyncManager::ClearLatencyInfo()
{
    // 清除configInfo
    avSyncConfigInfo_.ClearLatencyInfo();
    hiechoAvLatencyMap_.Clear();
    isAlreadyLoad_ = false;
}
}  // namespace bluetooth
}  // namespace OHOS
