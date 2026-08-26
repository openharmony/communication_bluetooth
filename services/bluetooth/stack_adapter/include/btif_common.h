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
 * Stub of the removed stack layer btif common glue (btif_common.h).
 */

#ifndef BTIF_COMMON_H
#define BTIF_COMMON_H

#include "bt_types.h"

typedef enum {
    BTIF_CTRL_STATE_ON = 0,
    BTIF_CTRL_STATE_OFF,
} btif_ctrl_state_t;

inline int btif_get_address_type(const RawAddress *bd_addr, int *addr_type)
{
    if (addr_type != nullptr) {
        *addr_type = BLE_ADDR_PUBLIC;
    }
    return 0;
}

#endif  // BTIF_COMMON_H
