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
 * Stub of the removed stack layer GATT profile glue header (bt_gatt.h).
 */

#ifndef BT_GATT_H
#define BT_GATT_H

#include <cstdint>

#include "bt_types.h"
#include "gatt_api.h"

#define BLE_GATT_MAX_ATTR_NUM 1000
#define BLE_GATT_ATTR_HANDLE_START 0x0001
#define BLE_GATT_ATTR_HANDLE_END 0xFFFF

typedef enum {
    BLE_GATT_CONNECT_OK = 0,
    BLE_GATT_CONNECT_FAIL,
    BLE_GATT_CONNECT_CANCEL,
} tBLE_GATT_CONNECT;

typedef enum {
    BLE_GATT_DISCONNECT_REASON_NORMAL = 0,
    BLE_GATT_DISCONNECT_REASON_REMOTE,
} tBLE_GATT_DISCONNECT_REASON;

typedef enum {
    BLE_GATT_SERVICE_PRIMARY = 0x01,
    BLE_GATT_SERVICE_SECONDARY = 0x02,
} tBLE_GATT_SERVICE_TYPE;

typedef enum {
    BLE_GATT_ATTR_PRIMARY_SERVICE = 0,
    BLE_GATT_ATTR_SECONDARY_SERVICE,
    BLE_GATT_ATTR_INCLUDE_SERVICE,
    BLE_GATT_ATTR_CHARACTERISTIC,
    BLE_GATT_ATTR_CHARACTERISTIC_DECLARATION,
    BLE_GATT_ATTR_DESCRIPTOR,
} tBLE_GATT_ATTR_TYPE;

typedef enum {
    BLE_GATT_PERM_READ = 0x0001,
    BLE_GATT_PERM_READ_ENCRYPTED = 0x0002,
    BLE_GATT_PERM_READ_ENCRYPTED_MITM = 0x0004,
    BLE_GATT_PERM_WRITE = 0x0010,
    BLE_GATT_PERM_WRITE_ENCRYPTED = 0x0020,
    BLE_GATT_PERM_WRITE_ENCRYPTED_MITM = 0x0040,
    BLE_GATT_PERM_WRITE_SIGNED = 0x0080,
    BLE_GATT_PERM_WRITE_SIGNED_MITM = 0x0100,
} tBLE_GATT_PERM;

typedef enum {
    BLE_GATT_CHAR_PROP_BROADCAST = 0x01,
    BLE_GATT_CHAR_PROP_READ = 0x02,
    BLE_GATT_CHAR_PROP_WRITE_NO_RSP = 0x04,
    BLE_GATT_CHAR_PROP_WRITE = 0x08,
    BLE_GATT_CHAR_PROP_NOTIFY = 0x10,
    BLE_GATT_CHAR_PROP_INDICATE = 0x20,
    BLE_GATT_CHAR_PROP_SIGNED_WRITE = 0x40,
} tBLE_GATT_CHAR_PROP;

typedef struct {
    uint8_t uuid[16];
} tBLE_GATT_UUID;

#endif  // BT_GATT_H
