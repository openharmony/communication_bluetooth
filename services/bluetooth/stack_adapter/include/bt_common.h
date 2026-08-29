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
 * Stub of the removed stack layer common glue (bt_common.h).
 */

#ifndef BT_COMMON_H
#define BT_COMMON_H

#include "bt_types.h"

enum BtCtrlState {
    BT_CTRL_STATE_ON = 0,
    BT_CTRL_STATE_OFF,
};

inline int BtGetAddressType(const OHOS::bluetooth::RawAddress &bdAddr, uint8_t *addrType)
{
    (void)bdAddr;
    if (addrType != nullptr) {
        *addrType = BLE_ADDR_PUBLIC;
    }
    return 0;
}

#endif  // BT_COMMON_H
