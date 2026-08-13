/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef GATT_SERVICE_BASE_H
#define GATT_SERVICE_BASE_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <cstddef>

#include "bt_def.h"
#include "log.h"
#include "gatt_api.h"

namespace OHOS {
namespace bluetooth {
using GattValue = std::shared_ptr<std::unique_ptr<uint8_t[]>>;

#define WPTR_CBACK(cbWptr, func, ...)               \
do {                                                \
    auto cbSptr = (cbWptr).lock();                  \
    if (cbSptr) {                                   \
        cbSptr->func(__VA_ARGS__);                  \
    } else {                                        \
        HILOGE("callback is nullptr");              \
    }                                               \
} while (0)

constexpr int GATT_TRANSPORT_INVALID = BT_TRANSPORT_INVALID;
constexpr int GATT_TRANSPORT_UNKNOWN = BT_TRANSPORT_UNKNOWN;
constexpr int GATT_TRANSPORT_AUTO = BT_TRANSPORT_AUTO;
constexpr int GATT_TRANSPORT_BR_EDR = BT_TRANSPORT_BR_EDR;
constexpr int GATT_TRANSPORT_LE = BT_TRANSPORT_LE;

enum class GattProperties : uint8_t {
    BROADCAST = 0x01,
    READ = 0x02,
    WRITE_WITHOUT_RESPONSE = 0x04,
    WRITE = 0x08,
    NOTIFY = 0x10,
    INDICATE = 0x20,
    AUTHENTICATED_SIGNED_WRITES = 0x40,
    EXTENDED_PROPERTIES = 0x80
};

constexpr uint16_t GATT_HANDLE_MIN = 0x0001;
constexpr uint16_t GATT_HANDLE_MAX = 0xFFFF;

// Client Characteristic Configuration
constexpr uint16_t UUID_CLIENT_CHARACTERISTIC_CONFIGURATION = 0x2902;
constexpr uint8_t CCC_PAYLOAD_SIZE = 2;
constexpr uint8_t CCC_NOTIFICATION_MASK = 0x01;
constexpr uint8_t CCC_INDICATION_MASK = 0x02;

class GattServiceBase {
public:
    // static const uint8_t MAXIMUM_NUMBER_APPLICATION;

    GattServiceBase() : runningState_(false)
    {}
    virtual ~GattServiceBase() = default;

    bool InRunningState(void)
    {
        return runningState_.load();
    }
    void Start(void)
    {
        runningState_ = true;
    }
    void Stop(void)
    {
        runningState_ = false;
    }

    static GattValue MoveToGattValue(std::unique_ptr<uint8_t[]> &value);
    static GattValue BuildGattValue(const uint8_t *value, size_t length);

    // BlueH (first) <-> bluedroid (second)
    static const std::vector<std::pair<int, int>> g_gattStatusCode;
    static int GattStatusFromBluedroid(int status);
    static int GattStatusToBluedroid(int status);

    // BlueH (first) <-> bluedroid (second)
    static const std::vector<std::pair<int, int>> g_gattTransport;
    static int GattTransportFromBluedroid(int transport);
    static int GattTransportToBluedroid(int transport);
    
    static void ConvertDisconnectReason(int &convertReason, int reason, std::string &reasonMessage);

private:
    std::atomic_bool runningState_;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // !GATT_SERVICE_BASE_H
