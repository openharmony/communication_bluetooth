/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef BLE_DEFS_H
#define BLE_DEFS_H

#include "bt_def.h"

namespace OHOS {
namespace bluetooth {

/// Advertising channel mask
typedef enum {
    ADV_CHNL_37 = 0x01,
    ADV_CHNL_38 = 0x02,
    ADV_CHNL_39 = 0x04,
    ADV_CHNL_ALL = 0x07,
} BLE_ADV_CHANNEL;

/// Advertising status
typedef enum {
    ADVERTISE_NOT_STARTED = -1,
    ADVERTISE_SUCCESS = 0,
    ADVERTISE_FAILED_DATA_TOO_LARGE,
    ADVERTISE_FAILED_TOO_MANY_ADVERTISERS,
    ADVERTISE_FAILED_ALREADY_STARTED,
    ADVERTISE_FAILED_INTERNAL_ERROR,
    ADVERTISE_FAILED_FEATURE_UNSUPPORTED
} ADVERTISE_STATUS;

/// Scan status
typedef enum {
    SCAN_NOT_STARTED = -1,
    SCAN_SUCCESS = 0,
    SCAN_FAILED_ALREADY_STARTED,
    SCAN_FAILED_APPLICATION_REGISTRATION_FAILED,
    SCAN_FAILED_INTERNAL_ERROR,
    SCAN_FAILED_FEATURE_UNSUPPORTED,
    SCAN_FAILED_OUT_OF_HARDWARE_RESOURCES
} SCAN_STATUS;

// Advertiser state
typedef enum {
    ADV_STATE_ON_FOUND = 0,
    ADV_STATE_ON_LOST = 1
} ADVERTISER_STATE;

typedef struct {
    uint8_t scanId;
    uint8_t advertiserState;
    uint8_t addrType;
    int8_t rssi;
} TrackAdvBaseInfo;

constexpr uint8_t BTM_PHY_LE_1M = 0x01;
constexpr uint8_t BTM_PHY_LE_2M = 0x02;
constexpr uint8_t BTM_PHY_LE_CODED = 0x04;
constexpr int BLE_MIN_ADV_LENGTH = 31;
constexpr int32_t BLE_SCAN_INVALID_ID = 0;
}  // namespace bluetooth
}  // namespace OHOS

#endif
