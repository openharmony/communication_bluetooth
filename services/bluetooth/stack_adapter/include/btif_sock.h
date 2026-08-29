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

/* Socket option flags of the removed stack layer (bluedroid
 * system/include/hardware/bt_sock.h). */
#define BTSOCK_FLAG_ENCRYPT 1
#define BTSOCK_FLAG_AUTH (1 << 1)
#define BTSOCK_FLAG_NO_SDP (1 << 2)
#define BTSOCK_FLAG_AUTH_MITM (1 << 3)
#define BTSOCK_FLAG_AUTH_16_DIGIT (1 << 4)
#define BTSOCK_FLAG_LE_COC (1 << 5)

enum BtsockType {
    BTSOCK_RFCOMM = 1,
    BTSOCK_SCO = 2,
    BTSOCK_L2CAP = 3,
    BTSOCK_L2CAP_LE = 4,
};

enum BtsockOpt {
    BTSOCK_OPT_AUTH = 0,
    BTSOCK_OPT_ENCRYPT,
};

/* Socket interface consumed by the service layer (socket_service.cpp); the
 * signatures follow the legacy bluedroid layout, which the removed stack
 * library implements (7-parameter listen/connect). */
struct BtsockInterface {
    size_t size;
    BtStackStatus (*listen)(BtsockType type, const char *serviceName,
        const OHOS::bluetooth::Uuid *serviceUuid, int channel, int *sockFd, int flags,
        int callingUid);
    BtStackStatus (*connect)(const OHOS::bluetooth::RawAddress *bdAddr, BtsockType type,
        const OHOS::bluetooth::Uuid *uuid, int channel, int *sockFd, int flags,
        int callingUid);
    BtStackStatus (*close)(int fd);
    BtStackStatus (*disconnect)(const OHOS::bluetooth::RawAddress *bdAddr, BtsockType type, int channel,
        int flags, int callingUid);
};

inline void BtifSockCleanup(void)
{
}

#endif  // BTIF_SOCK_H
