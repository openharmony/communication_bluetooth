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
 * Stub of the removed stack layer PAN profile interface (bt_pan.h).
 * Callback and interface table layouts mirror bluedroid
 * system/include/hardware/bt_pan.h.
 */

#ifndef BT_PAN_H
#define BT_PAN_H

#include <cstddef>

#include "bt_types.h"

typedef enum {
    PAN_STATE_DISCONNECTED = 0,
    PAN_STATE_CONNECTED,
} btpan_connection_state_t;

typedef enum {
    PAN_CONTROL_DISCONNECTED = 0,
    PAN_CONTROL_CONNECTED,
} btpan_control_state_t;

typedef void (*btpan_connection_state_callback)(btpan_connection_state_t state,
    bt_status_t error, const RawAddress *bd_addr, int local_role, int remote_role);
typedef void (*btpan_control_state_callback)(btpan_control_state_t state, int local_role,
    bt_status_t error, const char *ifname);

typedef struct {
    size_t size;
    btpan_control_state_callback control_state_cb;
    btpan_connection_state_callback connection_state_cb;
} btpan_callbacks_t;

typedef struct {
    size_t size;
    bt_status_t (*init)(const btpan_callbacks_t *callbacks);
    bt_status_t (*enable)(int local_role);
    int (*get_local_role)(void);
    bt_status_t (*connect)(const RawAddress *bd_addr, int local_role, int remote_role);
    bt_status_t (*disconnect)(const RawAddress *bd_addr);
    void (*cleanup)(void);
} btpan_interface_t;

#endif  // BT_PAN_H
