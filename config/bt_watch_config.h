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
#ifndef BT_WATCH_CONFIG_H
#define BT_WATCH_CONFIG_H

#define BTA_HFP_VERSION HFP_VERSION_1_8
#define BLUETOOTH_L2CAP_NEW_SCHED_ENABLED BT_FALSE
#define BLUETOOTH_HID_DEVICE_VERSION 0x101
#define BLUETOOTH_MAX_DEFAULT_CONNECTIONS_NUM 1
#define BLUETOOTH_A2DP_CODEC_PRIORITY_LDAC BTAV_A2DP_CODEC_PRIORITY_DISABLED
#define BLUETOOTH_A2DP_CODEC_L2HCST_PRIORITY BTAV_A2DP_CODEC_PRIORITY_DISABLED
#define BLUETOOTH_A2DP_CODEC_L2HCV2_PRIORITY BTAV_A2DP_CODEC_PRIORITY_DISABLED
#define BLUETOOTH_GATT_SERVER_SYNC_TIMEOUT 4 // 4s
#define BLUETOOTH_BLE_CREATE_ADV_TIMEOUT 4 // 4s
#define BLUETOOTH_A2DP_OFFLOAD_AAC_ENABLE
#define DEFAULT_STARTUP_TIMEOUT_MS 6000
#define A2DP_SBC_MAX_BITPOOL 35
#define HW_TWS_ECHO_VER_MAJOR_DEFAULT 0x00
#define HW_TWS_ECHO_VER_MINOR_DEFAULT 0x00
#define BTA_DM_PM_SNIFF_HD_ACTIVE_IDX BTA_DM_PM_ACTIVE
#define HCI_SET_HIGH_POWER_MODE_DEFAULT (0x004E | HCI_GRP_VENDOR_SPECIFIC)
#define BLUETOOTH_GATT_CLIENT_TIMEOUT 4 // 4s
#define HIGH_PRIORITY_MIN_INTERVAL "HighPriorityMaxInterval"
#endif // BT_WATCH_CONFIG_H
