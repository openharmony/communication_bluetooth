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
#define LOG_TAG "bt_service_av_sync_config"
#endif

#include "av_sync_config_info.h"
#include "av_sync_common.h"
#include "adapter_manager.h"
#include "common_util.h"
#include "log.h"
#include "remote_device_properties.h"

namespace OHOS {
namespace bluetooth {
AvSyncConfigInfo::AvSyncConfigInfo()
{}

void AvSyncConfigInfo::AddLatencyBaseByDeviceName(std::string deviceName, int32_t avLatency)
{
    if (deviceName.empty() || avLatency == INVALID_LATENCY) {
        HILOGE("av_sync, deviceName is empty or avLatency is invalid");
        return;
    }
    deviceNameLatencyBaseMap_.EnsureInsert(deviceName, avLatency);
}

void AvSyncConfigInfo::AddLatencyBaseByModelId(std::string modelId, int32_t avLatency)
{
    if (modelId.empty() || avLatency == INVALID_LATENCY) {
        HILOGE("av_sync, modelId is invalid or avLatency is invalid");
        return;
    }
    modelIdLatencyBaseMap_.EnsureInsert(modelId, avLatency);
}

void AvSyncConfigInfo::AddLatencyBaseByNewModelId(std::string newModelId, int32_t avLatency)
{
    if (newModelId.empty() || avLatency == INVALID_LATENCY) {
        HILOGE("av_sync, newModelId is empty or avLatency is invalid");
        return;
    }
    newModelIdLatencyBaseMap_.EnsureInsert(newModelId, avLatency);
}

int32_t AvSyncConfigInfo::GetLatencyBase(const RawAddress &rawAddr)
{
    std::shared_ptr<BluetoothDevice> remoteDevice = RemoteDeviceProperties::GetInstance()->FindRemoteDevice(rawAddr);
    CHECK_AND_RETURN_LOG_RET(remoteDevice != nullptr, INVALID_LATENCY, "device not exist.");
    HILOGD("av_sync, deviceNameMap size: %{public}d, modelIdMap size: %{public}d, newModelIdMap size: %{public}d",
        deviceNameLatencyBaseMap_.Size(), modelIdLatencyBaseMap_.Size(), newModelIdLatencyBaseMap_.Size());
    std::string deviceName = remoteDevice->GetRemoteName();
    int latency = GetLatencyBaseByDeviceName(deviceName);
    if (latency != INVALID_LATENCY) {
        HILOGI("av_sync, latency: %{public}d, match deviceName: %{public}s", latency,
            GET_ENCRYPT_DEVICE_NAME(deviceName));
        return latency;
    }

    auto classicAdapter = AdapterManager::GetInstance()->GetClassicAdapter();
    CHECK_AND_RETURN_LOG_RET(classicAdapter != nullptr, INVALID_LATENCY, "classicAdapter is nullptr.");
    std::string modelId = "";
    if (classicAdapter->GetRemoteDevicePropertyInfo(rawAddr, BT_PROPERTY_REMOTE_MODEL_ID, modelId)) {
        latency = GetLatencyBaseByModelId(modelId);
        if (latency != INVALID_LATENCY) {
            HILOGI("av_sync, not matched deviceName: %{public}s, match modelId: %{public}s, latency: %{public}d",
                GET_ENCRYPT_DEVICE_NAME(deviceName), modelId.c_str(), latency);
            return latency;
        }
    }
    
    std::string newModelId = "";
    if (classicAdapter->GetRemoteDevicePropertyInfo(rawAddr, BT_PROPERTY_REMOTE_NEW_MODEL_ID, newModelId)) {
        latency = GetLatencyBaseByNewModelId(newModelId);
        if (latency != INVALID_LATENCY) {
            HILOGI("av_sync,  deviceName: %{public}s, modelId: %{public}s, match newModelId: %{public}s, latency: "
                "%{public}d", GET_ENCRYPT_DEVICE_NAME(deviceName), modelId.c_str(), newModelId.c_str(), latency);
            return latency;
        }
    }
    HILOGI("av_sync, not get latency from config, deviceName: %{public}s, modelId: %{public}s, "
        "newModelId: %{public}s", GET_ENCRYPT_DEVICE_NAME(deviceName), modelId.c_str(), newModelId.c_str());
    return INVALID_LATENCY;
}

int32_t AvSyncConfigInfo::GetLatencyBaseByDeviceName(std::string deviceName)
{
    CHECK_AND_RETURN_LOG_RET(!deviceName.empty(), INVALID_LATENCY, "av_sync, deviceName is empty.");
    int32_t latency = INVALID_LATENCY;
    if (deviceNameLatencyBaseMap_.Find(deviceName, latency)) {
        return latency;
    }
    return INVALID_LATENCY;
}

int32_t AvSyncConfigInfo::GetLatencyBaseByModelId(std::string modelId)
{
    CHECK_AND_RETURN_LOG_RET(!modelId.empty(), INVALID_LATENCY, "av_sync, newModelId is empty.");
    int32_t latency = INVALID_LATENCY;
    if (modelIdLatencyBaseMap_.Find(modelId, latency)) {
        return latency;
    }
    return INVALID_LATENCY;
}

int32_t AvSyncConfigInfo::GetLatencyBaseByNewModelId(std::string newModelId)
{
    CHECK_AND_RETURN_LOG_RET(!newModelId.empty(), INVALID_LATENCY, "av_sync, newModelId is empty");
    int32_t latency = INVALID_LATENCY;
    if (newModelIdLatencyBaseMap_.Find(newModelId, latency)) {
        return latency;
    }
    return INVALID_LATENCY;
}

void AvSyncConfigInfo::ClearLatencyInfo()
{
    deviceNameLatencyBaseMap_.Clear();
    modelIdLatencyBaseMap_.Clear();
    newModelIdLatencyBaseMap_.Clear();
}
}  // namespace bluetooth
}  // namespace OHOS
