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
 * Stub of the removed stack layer HID device profile interface (bt_hd.h).
 */

#ifndef BT_HD_H
#define BT_HD_H

#include <cstddef>

#include "bt_types.h"

typedef enum {
    BTHD_CONN_STATE_DISCONNECTED = 0,
    BTHD_CONN_STATE_CONNECTING,
    BTHD_CONN_STATE_CONNECTED,
    BTHD_CONN_STATE_DISCONNECTING,
} bthd_connection_state_t;

typedef struct {
    size_t size;
    int (*init)(...);
    void (*cleanup)(...);
    int (*connect)(...);
    int (*disconnect)(...);
    int (*virtual_unplug)(...);
} bthd_interface_t;

#endif  // BT_HD_H
