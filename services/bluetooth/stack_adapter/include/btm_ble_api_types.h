/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer BLE API types (btm_ble_api_types.h).
 */

#ifndef BTM_BLE_API_TYPES_H
#define BTM_BLE_API_TYPES_H

#include <cstdint>

#include "bt_types.h"

struct BleBdAddr {
    uint8_t bda[6];
    BleAddrType addrType;
};

struct BleDevInfo {
    BleBdAddr bdAddr;
    uint8_t advFlag;
    int8_t rssi;
};

enum BleConnMode {
    BLE_CONN_MODE_NONE = 0,
    BLE_CONN_MODE_ACTIVE,
    BLE_CONN_MODE_PASSIVE,
};

enum BleAdvType {
    BLE_HCI_LE_ADV_IND = 0x00,
    BLE_HCI_LE_ADV_DIRECT_IND = 0x01,
    BLE_HCI_LE_ADV_SCAN_IND = 0x02,
    BLE_HCI_LE_ADV_NONCONN_IND = 0x03,
};

/* BTM command success status; the service layer compares the uint8_t status
 * reported by the scan-filter callbacks against it. */
#define BTM_SUCCESS 0

/* Scan filter content types of the removed stack layer (btm_ble_api_types.h),
 * used as the ApcfCommand.type values of the scan filter. */
#define BTM_BLE_PF_ADDR_FILTER 0
#define BTM_BLE_PF_SRVC_DATA 1
#define BTM_BLE_PF_SRVC_UUID 2
#define BTM_BLE_PF_SRVC_SOL_UUID 3
#define BTM_BLE_PF_LOCAL_NAME 4
#define BTM_BLE_PF_MANU_DATA 5
#define BTM_BLE_PF_SRVC_DATA_PATTERN 6

/* HCI error code returned for a repeated (duplicate) scan filter entry. */
#define HCI_ERR_REPEATED_ATTEMPTS 0x17

#endif  // BTM_BLE_API_TYPES_H
