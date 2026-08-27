/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef SERVICE_UTIL_H
#define SERVICE_UTIL_H

// openharmony
#include "bt_uuid.h"
#include "raw_address.h"
// Bluedroid
#include "types/bt_transport.h"
#include "types/raw_address.h"

#include <algorithm>

#include "bt_def.h"
#include "bt_types.h"
#include "gatt_api.h"
#include "common_util.h"

namespace OHOS {
namespace bluetooth {
#define STACK

class ServiceUtil final {
public:
    // UUID
    static std::vector<std::string> UuidsToUuidStrs(const std::vector<Uuid> &uuids)
    {
        std::vector<std::string> uuidStrs;
        std::transform(uuids.begin(), uuids.end(), std::back_inserter(uuidStrs), [](const Uuid uuid) {
            return uuid.ToString();
        });
        return uuidStrs;
    }

    // RawAddress
    static RawAddress AddrFromStack(const STACK::RawAddress &addr)
    {
        std::string tmp = addr.ToString();
        // BlueH need upper address, but bluedroid use lower address
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
        return RawAddress(tmp);
    }

    static STACK::RawAddress AddrToStack(const RawAddress &addr)
    {
        STACK::RawAddress dst;
        if (!STACK::RawAddress::FromString(addr.GetAddress(), dst)) {
            return STACK::RawAddress::kEmpty;
        }
        return dst;
    }

    // Address type
    static int AddrTypeFromStack(uint8_t type)
    {
        int ret = BLE_BT_DEVICE_TYPE_UNKNOWN;
        switch (type) {
            case BLE_ADDR_PUBLIC:
            case BLE_ADDR_PUBLIC_ID:
                ret = BLE_ADDR_TYPE_PUBLIC;
                break;
            case BLE_ADDR_RANDOM:
            case BLE_ADDR_RANDOM_ID:
                ret = BLE_ADDR_TYPE_RANDOM;
                break;
            case BLE_ADDR_ANONYMOUS:
                ret = BLE_ADDR_TYPE_ANONYMOUS;
                break;
            default:
                ret = BLE_BT_DEVICE_TYPE_UNKNOWN;
                break;
        }
        return ret;
    }
    // Transport
    static int TransportToStack(const int32_t transport)
    {
        int ret = BT_TRANSPORT_INVALID;
        switch (transport) {
            case BT_TRANSPORT_BREDR:
                ret = BT_TRANSPORT_BR_EDR;
                break;
            case BT_TRANSPORT_BLE:
                ret = BT_TRANSPORT_LE;
                break;
            default:
                ret = BT_TRANSPORT_INVALID;
                break;
        }
        return ret;
    }

    static int TransportFromStack(const tBT_TRANSPORT transport)
    {
        int ret = BT_TRANSPORT_NONE;
        switch (transport) {
            case BT_TRANSPORT_BR_EDR:
                ret = BT_TRANSPORT_BREDR;
                break;
            case BT_TRANSPORT_LE:
                ret = BT_TRANSPORT_BLE;
                break;
            default:
                ret = BT_TRANSPORT_NONE;
                break;
        }
        return ret;
    }

    // stack transport type to service link type.
    static int LinkTypeFromStack(tBT_TRANSPORT transport)
    {
        switch (transport) {
            case BT_TRANSPORT_BR_EDR:
                return LINK_TYPE_BREDR;
            case BT_TRANSPORT_LE:
                return LINK_TYPE_LE;
            default:
                return LINK_TYPE_UNKNOWN;
        }
    }
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // SERVICE_UTIL_H
