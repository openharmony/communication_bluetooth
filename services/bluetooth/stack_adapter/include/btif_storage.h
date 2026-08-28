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
 * Stub of the removed stack layer btif storage glue (btif_storage.h).
 */

#ifndef BTIF_STORAGE_H
#define BTIF_STORAGE_H

#include <cstdint>

#include "bt_types.h"

inline int BtifStorageGetRemoteAddrType(const RawAddress *remoteBdAddr, uint8_t *addrType)
{
    if (addrType != nullptr) {
        *addrType = BLE_ADDR_PUBLIC;
    }
    return 0;
}

#endif  // BTIF_STORAGE_H
