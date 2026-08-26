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
 * Stub of the removed stack layer btif socket glue (btif_sock.h).
 */

#ifndef BTIF_SOCK_H
#define BTIF_SOCK_H

#include <cstddef>

#include "bt_types.h"

typedef enum {
    BTSOCK_RFCOMM = 1,
    BTSOCK_SCO = 2,
    BTSOCK_L2CAP = 3,
    BTSOCK_L2CAP_LE = 4,
} btsock_type_t;

typedef enum {
    BTSOCK_OPT_AUTH = 0,
    BTSOCK_OPT_ENCRYPT,
} btsock_opt_t;

typedef struct {
    size_t size;
    int (*listen)(...);
    int (*connect)(...);
    int (*close)(...);
    int (*disconnect)(...);
} btsock_interface_t;

inline void btif_sock_cleanup(void)
{
}

#endif  // BTIF_SOCK_H
