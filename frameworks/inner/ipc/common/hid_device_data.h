/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef HID_DEVICE_DATA_H
#define HID_DEVICE_DATA_H

#include <cstring>
#include <memory>
#include <vector>
#include "bt_def.h"
#include "cstdint"
#include "raw_address.h"
#include "type_traits"

namespace OHOS {
namespace bluetooth {
struct SdpSetting {
    SdpSetting() = default;
    SdpSetting(const std::string &name, const std::string &description,
        const std::string &provider, int32_t subclass, const std::vector<uint8_t> &sdpSettings)
        : name_(name), description_(description), provider_(provider), subclass_(subclass), sdpSettings_(sdpSettings) {}

    ~SdpSetting() = default;
    SdpSetting(const SdpSetting &src) = default;
    SdpSetting(SdpSetting &&src) = default;

    SdpSetting &operator=(const SdpSetting &src) = default;
    SdpSetting &operator=(SdpSetting &&src) = default;

    std::string name_;
    std::string description_;
    std::string provider_;
    int32_t subclass_;
    std::vector<uint8_t> sdpSettings_;
};

struct QosSetting {
    QosSetting() = default;
    QosSetting(int32_t serviceType, int32_t tokenRate, int32_t tokenBucketSize,
        int32_t peakBandwidth, int32_t latency, int32_t delayVariation)
        : serviceType_(serviceType),
          tokenRate_(tokenRate),
          tokenBucketSize_(tokenBucketSize),
          peakBandwidth_(peakBandwidth),
          latency_(latency),
          delayVariation_(delayVariation)
    {}

    ~QosSetting() = default;
    QosSetting(const QosSetting &src) = default;
    QosSetting(QosSetting &&src) = default;
    QosSetting &operator=(const QosSetting &src) = default;
    QosSetting &operator=(QosSetting &&src) = default;

    int32_t serviceType_;
    int32_t tokenRate_;
    int32_t tokenBucketSize_;
    int32_t peakBandwidth_;
    int32_t latency_;
    int32_t delayVariation_;
};
}  // namespace bluetooth
}  // namespace OHOS
#endif // HID_DEVICE_DATA_H