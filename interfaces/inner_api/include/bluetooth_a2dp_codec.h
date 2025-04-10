/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

/**
 * @addtogroup Bluetooth
 * @{
 *
 * @brief A2dp codec information and status define.
 *
 * @since 6
 */

#ifndef BLUETOOTH_A2DP_CODEC_H
#define BLUETOOTH_A2DP_CODEC_H

#include <vector>
#include "bluetooth_def.h"
#include "bluetooth_types.h"

namespace OHOS {
namespace Bluetooth {
/**
 * @brief A2dp codec configuration information of a2dp source
 *
 * @since 6.0
 */
struct A2dpCodecInfo {
    // Codec priority
    uint32_t codecPriority;

    // Codec type
    uint8_t codecType;

    // Codec sample
    uint32_t sampleRate;

    // Codec bits per sample
    uint8_t bitsPerSample;

    // Codec channel mode
    uint8_t channelMode;

    // Codec specific value1
    uint64_t codecSpecific1;

    // Codec specific value2
    uint64_t codecSpecific2;

    // Codec specific value3
    uint64_t codecSpecific3;

    // Codec specific value4
    uint64_t codecSpecific4;
};

/**
 * @brief The codec configuration and capability information of a2dp source
 *
 * @since 6.0
 */
struct A2dpCodecStatus {
    // current codec information
    A2dpCodecInfo codecInfo;

    // local codec information
    std::vector<A2dpCodecInfo> codecInfoLocalCap;

    // Local device and peer confirmed codec information
    std::vector<A2dpCodecInfo> codecInfoConfirmedCap;
};

/**
 * @brief A2dp offload Codec configuration information of a2dp source
 *
 * @since 6.0
 */
struct A2dpOffloadCodecInfo {
    uint16_t mediaPacketHeader = 0;
    uint8_t mPt = 0;
    uint32_t ssrc = 0;
    uint8_t boundaryFlag = 0;
    uint8_t broadcastFlag = 0;
    uint32_t codecType = 0;
    uint16_t maxLatency = 0;
    uint16_t scmsTEnable = 0;
    uint32_t sampleRate = 0;
    uint32_t encodedAudioBitrate = 0;
    uint8_t bitsPerSample = 0;
    uint8_t chMode = 0;
    uint16_t aclHdl = 0;
    uint16_t l2cRcid = 0;
    uint16_t mtu = 0;
    uint8_t codecSpecific0 = 0;
    uint8_t codecSpecific1 = 0;
    uint8_t codecSpecific2 = 0;
    uint8_t codecSpecific3 = 0;
    uint8_t codecSpecific4 = 0;
    uint8_t codecSpecific5 = 0;
    uint8_t codecSpecific6 = 0;
    uint8_t codecSpecific7 = 0;
};

class BluetoothA2dpOffloadCodecStatus;

struct A2dpOffloadCodecStatus {
    A2dpOffloadCodecInfo offloadInfo;
    A2dpOffloadCodecStatus() {};
    A2dpOffloadCodecStatus(const BluetoothA2dpOffloadCodecStatus &status);
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // BLUETOOTH_A2DP_CODEC_H