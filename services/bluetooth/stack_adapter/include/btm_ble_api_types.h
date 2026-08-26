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

typedef struct {
    uint8_t bda[6];
    tBLE_ADDR_TYPE addr_type;
} tBLE_BD_ADDR;

typedef struct {
    tBLE_BD_ADDR bd_addr;
    uint8_t adv_flag;
    int8_t rssi;
} tBLE_DEV_INFO;

typedef enum {
    BLE_CONN_MODE_NONE = 0,
    BLE_CONN_MODE_ACTIVE,
    BLE_CONN_MODE_PASSIVE,
} tBLE_CONN_MODE;

typedef enum {
    BLE_HCI_LE_ADV_IND = 0x00,
    BLE_HCI_LE_ADV_DIRECT_IND = 0x01,
    BLE_HCI_LE_ADV_SCAN_IND = 0x02,
    BLE_HCI_LE_ADV_NONCONN_IND = 0x03,
} tBLE_ADV_TYPE;

#endif  // BTM_BLE_API_TYPES_H
