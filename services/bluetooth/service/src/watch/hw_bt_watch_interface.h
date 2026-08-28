/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_HW_BT_WATCH_INTERFACE_H
#define BLUETOOTH_HW_BT_WATCH_INTERFACE_H

#include <cstddef>
#include <cstdint>

#include "hardware/bluetooth.h"
#include "types/raw_address.h"

#ifndef STACK
#define STACK
#endif

namespace OHOS {
namespace bluetooth {

/* Watch callbacks dispatched from the closed-source HAL. */
typedef struct {
    size_t size;
    void (*disconnect_rssi_cb)(const STACK::RawAddress *bdAddr, int rssi);
    void (*sais_connection_state_cb)(const STACK::RawAddress *bdAddr,
                                     bool isSaisConnected);
    void (*a2dp_ofld_play_state_cb)(uint8_t playState);
    void (*sais_server_added_cb)(void);
    void (*le_sais_enable_cb)(const STACK::RawAddress *bdAddr);
    void (*br_hid_ctrl_cb)(uint8_t act);
    void (*link_loss_notify_cb)(void);
} BtWatchCallbacks;

/* Closed-source watch interface. */
class BtHwWatchInterface {
public:
    virtual ~BtHwWatchInterface() = default;

    virtual BtStackStatus init(BtWatchCallbacks *callbacks) = 0;
    virtual void updateReconnectState(const STACK::RawAddress &device,
                                      bool isBtOn) = 0;
    virtual void updateDevice(const STACK::RawAddress &device,
                              bool isSaisDevice) = 0;
    virtual void sendIsSecondPhonePair(bool isSecondaryPair) = 0;
    virtual void sendHfpState2Hisi(const STACK::RawAddress &device,
                                   bool isBtOn) = 0;
    virtual bool isPhoneMajorClass(const STACK::RawAddress &device) = 0;
    virtual void sendScreenOn2Mcu() = 0;
    virtual void sendTbsr(const STACK::RawAddress &device, uint16_t mode) = 0;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_HW_BT_WATCH_INTERFACE_H
