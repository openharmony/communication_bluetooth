/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_RESOURCE_MANAGER_DEF_H
#define BLUETOOTH_RESOURCE_MANAGER_DEF_H

#include "bluetooth_def.h"
#include "datetime_ex.h"

namespace OHOS {
namespace bluetooth {
constexpr int ADV_2_OCTS_LENGTH = 2;
constexpr uint8_t FLAG_ADTYPE = 0x01;
constexpr uint8_t ADV_UUID_ADTYPE = 0x16;
constexpr uint8_t ADV_MANUFACTURE_ADTYPE = 0xFF;
constexpr uint16_t ADV_UUID_FDEE = 0xFDEE;
constexpr uint16_t ADV_UUID_FE35 = 0xFE35;
constexpr uint16_t ADV_MANUFACTUREID = 0x027D;
constexpr int ONE_OCTET_BITS = 8;

constexpr uint32_t BR_TRANS_CONTROL_DEFAULT_WORKING_INTERVAL_MS = 100;
constexpr uint32_t BR_TRANS_CONTROL_DEFAULT_PENDING_INTERVAL_MS = 0;
constexpr uint32_t BR_TRANS_CONTROL_DEFAULT_SLEEPING_INTERVAL_MS = 0;
constexpr uint32_t BR_TRANS_CONTROL_P10_10_100_WORKING_INTERVAL_MS = 90;
constexpr uint32_t BR_TRANS_CONTROL_P10_10_100_PENDING_INTERVAL_MS = 100;
constexpr uint32_t BR_TRANS_CONTROL_P10_10_100_SLEEPING_INTERVAL_MS = 10;
constexpr uint32_t BR_TRANS_CONTROL_P25_25_100_WORKING_INTERVAL_MS = 75;
constexpr uint32_t BR_TRANS_CONTROL_P25_25_100_PENDING_INTERVAL_MS = 100;
constexpr uint32_t BR_TRANS_CONTROL_P25_25_100_SLEEPING_INTERVAL_MS = 25;
constexpr uint32_t BR_TRANS_CONTROL_P50_50_100_WORKING_INTERVAL_MS = 50;
constexpr uint32_t BR_TRANS_CONTROL_P50_50_100_PENDING_INTERVAL_MS = 100;
constexpr uint32_t BR_TRANS_CONTROL_P50_50_100_SLEEPING_INTERVAL_MS = 50;
constexpr uint32_t BR_TRANS_CONTROL_P75_75_100_WORKING_INTERVAL_MS = 25;
constexpr uint32_t BR_TRANS_CONTROL_P75_75_100_PENDING_INTERVAL_MS = 100;
constexpr uint32_t BR_TRANS_CONTROL_P75_75_100_SLEEPING_INTERVAL_MS = 75;

constexpr int DUAL_MAC_DISABLE = 0;

constexpr int BT_TRANS_RATE_REP_SIZE = 3;

enum ScanStatus {
    STOPPED,
    STARTED,
};

enum SENSING_REPORT_TYPE {
    INVALID_VALUE_REPORT = 0,
    CHANNEL_RSSI_REPORT = 1,
    BT_TRANS_RATE_REPORT = 2,
};

struct ScanMsg {
    int scanMode_ = SCAN_MODE_LOW_POWER;
    int scanStatus_ = STOPPED;
};

enum DualMacBitMask {
    DUAL_MAC_D2D_SWITCH_BLE_ADV_SCAN = 0x1, // switch ble adv/scan/initial to mac0 for D2D
    DUAL_MAC_D2D_SWITCH_SLE_ADV_SCAN = 0x2, // switch sle adv/scan/initial to mac0 for D2D
    DUAL_MAC_D2D_SWITCH_BR_AUDIO = 0x4, // switch br audio to mac1 for D2D
    DUAL_MAC_D2D_SWITCH_BLE_AUDIO = 0x8, // switch ble audio to mac1 for D2D
    DUAL_MAC_D2D_SWITCH_SLE_AUDIO = 0x10, // switch sle audio to mac1  for D2D
    DUAL_MAC_ALL_SWITCH_BLE_ADV_SCAN = 0x20, // switch ble adv/scan/initial to mac1 for softbus business
    DUAL_MAC_ALL_BLE_MAC1 = 0x100, // ble_all default to mac1, according "multi_mac_rxpd_set" in ini_cfg
};

enum SniffModeId {
    BR_SNIFF_MODE_250_MS = 0x00,
    BR_SNIFF_MODE_125_MS = 0x01,
    BR_SNIFF_MODE_18_75_MS = 0x02,
    BR_SNIFF_MODE_31_25_MS = 0x03,
    BR_SNIFF_MODE_6_25_MS = 0x04,
    BR_SNIFF_MODE_8_75_MS = 0x06,
    BR_SNIFF_PARK_MODE = 0x07,
    BR_SNIFF_MODE_INVALID,
};

struct BrTransControlSetting {
    BrTransControlSetting() = default;
    BrTransControlSetting(uint32_t working, uint32_t pending, uint32_t sleeping)
        : workingInterval(working), pendingInterval(pending), sleepingInterval(sleeping)
    {}
    ~BrTransControlSetting() {}

    uint32_t workingInterval = 0;
    uint32_t pendingInterval = 0;
    uint32_t sleepingInterval = 0;
};

struct BleScanEnhanceModeAction {
    BleScanEnhanceModeAction() = default;
    BleScanEnhanceModeAction(bool audioDown, bool isSniff, const BrTransControlSetting &setting)
        : isAudioRateSpeedDown(audioDown), isBrLinkSniffMode(isSniff), brTransControlSetting(setting)
    {}
    ~BleScanEnhanceModeAction() {}

    bool isAudioRateSpeedDown = false;
    bool isBrLinkSniffMode = false;
    BrTransControlSetting brTransControlSetting = {};
};
}
}

#endif