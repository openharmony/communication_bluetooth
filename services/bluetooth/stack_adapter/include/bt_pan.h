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

enum BtpanConnectionState {
    PAN_STATE_DISCONNECTED = 0,
    PAN_STATE_CONNECTED,
};

enum BtpanControlState {
    PAN_CONTROL_DISCONNECTED = 0,
    PAN_CONTROL_CONNECTED,
};

typedef void (*BtpanConnectionStateCallback)(BtpanConnectionState state,
    BtStackStatus error, const OHOS::bluetooth::RawAddress *bdAddr, int localRole, int remoteRole);
typedef void (*BtpanControlStateCallback)(BtpanControlState state, int localRole,
    BtStackStatus error, const char *ifname);

struct BtpanCallbacks {
    size_t size;
    BtpanControlStateCallback controlStateCb;
    BtpanConnectionStateCallback connectionStateCb;
};

struct BtpanInterface {
    size_t size;
    BtStackStatus (*init)(const BtpanCallbacks *callbacks);
    BtStackStatus (*enable)(int localRole);
    int (*getLocalRole)(void);
    BtStackStatus (*connect)(const OHOS::bluetooth::RawAddress *bdAddr, int localRole, int remoteRole);
    BtStackStatus (*disconnect)(const OHOS::bluetooth::RawAddress *bdAddr);
    void (*cleanup)(void);
};

#endif  // BT_PAN_H
