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

#ifndef AV_SYNC_COMMON_H
#define AV_SYNC_COMMON_H

namespace OHOS {
namespace bluetooth {
constexpr const char* AV_LATENCY_CONFIG_PATH = "/system/etc/bluetooth/bt_av_latency_config.json";

// latency_base tag in json
constexpr const char* TAG_LATENCY_BASE = "latency_base";

// device_name tag in json
constexpr const char* TAG_DEVICE_NAME = "device_name";

// model_id tag in json
constexpr const char* TAG_MODEL_ID = "model_id";

// new_model_id tag in json
constexpr const char* TAG_NEW_MODEL_ID = "new_model_id";

// latency_ms tag in json
constexpr const char* TAG_LATENCY_MS = "latency_ms";

// latency unit, ms to 0.1ms
constexpr int32_t LATENCY_UNIT = 10;

constexpr int32_t INVALID_LATENCY = -1;

// default latency, unit: 0.1ms
constexpr int32_t DEFAULT_AV_LATENCY = 2500;

// max latency, unit: ms
constexpr int32_t MAX_AV_LATENCY = 1000;
}  // namespace bluetooth
}  // namespace OHOS

#endif
