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
 * Stub of the removed stack layer HID host profile interface (bt_hh.h).
 */

#ifndef BT_HH_H
#define BT_HH_H

#include <cstddef>

#include "bt_types.h"

typedef enum {
    BTHH_CONN_STATE_DISCONNECTED = 0,
    BTHH_CONN_STATE_CONNECTING,
    BTHH_CONN_STATE_CONNECTED,
    BTHH_CONN_STATE_DISCONNECTING,
} bthh_connection_state_t;

typedef enum {
    BTHH_ERR_NONE = 0,
    BTHH_ERR_DEV_NOT_CONNECTED,
} bthh_status_t;

typedef enum {
    BTHH_PROTOCOL_MODE_REPORT = 0,
    BTHH_PROTOCOL_MODE_BOOT,
} bthh_protocol_mode_t;

typedef struct {
    size_t size;
    int (*connect)(...);
    int (*disconnect)(...);
    int (*virtual_unplug)(...);
    int (*set_protocol_mode)(...);
} bthh_interface_t;

#endif  // BT_HH_H
