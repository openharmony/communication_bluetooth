/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef BT_BATTERY_INFO_PARCEL_H
#define BT_BATTERY_INFO_PARCEL_H

#include <string>

namespace OHOS {
namespace bluetooth {

/**
 * @brief remote device battery information.
 *
 * @since 12
 */
class BatteryInfo {
public:
    BatteryInfo() {}
    ~BatteryInfo() {}

    void ResetInfo()
    {
        batteryLevel_ = -1;
        leftEarBatteryLevel_ = -1;
        leftEarChargeState_ = 0;
        rightEarBatteryLevel_ = -1;
        rightEarChargeState_ = 0;
        boxBatteryLevel_ = -1;
        boxChargeState_ = 0;
    }

public:
    int32_t batteryLevel_ = -1;
    int32_t leftEarBatteryLevel_ = -1;
    int32_t leftEarChargeState_ = 0;
    int32_t rightEarBatteryLevel_ = -1;
    int32_t rightEarChargeState_ = 0;
    int32_t boxBatteryLevel_ = -1;
    int32_t boxChargeState_ = 0;
};

class BatteryDetailInfo {
public:
    BatteryDetailInfo() {}
    ~BatteryDetailInfo() {}

    void ResetInfo()
    {
        batteryObtainType_ = 0;
        batteryInfo_.ResetInfo();
    }
public:
    uint8_t batteryObtainType_ = 0; // 0 not exist,1 AT,2 ECHO,3 ALL
    BatteryInfo batteryInfo_;
};
}
}

#endif //BT_BATTERY_INFO_PARCEL_H