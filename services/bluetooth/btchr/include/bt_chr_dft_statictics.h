/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef BT_CHR_DFT_STATISTICS_H
#define BT_CHR_DFT_STATISTICS_H
#include "bt_chr_base.h"

namespace OHOS {
namespace bluetooth {
// CHR_BT_STATISTICS_ATOMIC_EVENT event type
enum BtStatisticsAtomicEventType {
    EVENT_TYPE_CODEC_STATISTICS = 1,
    EVENT_TYPE_BITRATE_STATISTICS = 2,
    EVENT_TYPE_A2DP_CALL_STATISTICS = 3,
    EVENT_TYPE_OFFLOAD_STATISTICS = 4,
    EVENT_TYPE_AUTOCONNECT_STATISTICS = 5,
    EVENT_TYPE_TRANSACTION_STATISTICS = 6,
};

enum BtDurationStatisticsType {
    DURATION_STAT_TYPE_A2DP = 1,
    DURATION_STAT_TYPE_CALL = 2,
    DURATION_STAT_TYPE_OFFLOAD = 3,
};

struct BleAdvReachingMaxRecord {
    BleAdvReachingMaxRecord(int32_t advNum, int32_t maxHoldingTime)
        : advNum(advNum), maxHoldingTime(maxHoldingTime) {}

    int32_t advNum;
    int32_t maxHoldingTime;
};

class BtChrDftStatictics {
public:
    BtChrDftStatictics();
    static std::shared_ptr<BtChrDftStatictics> GetInstance();
    void WriteCommonStatictics(uint8_t eventType, const std::vector<BtChrEventParam>& params);
    void WriteCodecStatictics(const uint8_t codedType);
    void WriteAutoConnectStatictics(const uint8_t result);
    void WriteDurationStatictics(uint8_t durationStatType, bool isStart);
    uint32_t GetDurationInMinutes(uint32_t durationInSeconds);
    /* CHR_BLE_ADV_RESOURCE_REACHING_MAX begin */
    void BleAdvReachingMaxStatisticsEvent();
    void SetBleAdvReachingMaxRecord(const std::string &packageName, int32_t advNum, uint32_t maxHoldingTime);
    /* CHR_BLE_ADV_RESOURCE_REACHING_MAX end */
private:
    static std::shared_ptr<BtChrDftStatictics> instance_;

    std::string GetEventValueJsonStr(const std::vector<BtChrEventParam>& params);
    std::string GetBleAdvReachingMaxRecordsJsonStr();
};

}  // namespace bluetooth
}  // namespace OHOS
#endif /* BT_CHR_DFT_STATISTICS_H */
