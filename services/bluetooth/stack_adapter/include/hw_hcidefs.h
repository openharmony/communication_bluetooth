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

#ifndef BLUETOOTH_HW_HCIDEFS_H
#define BLUETOOTH_HW_HCIDEFS_H

#include <cstdint>

#include "raw_address.h"

#ifndef STACK
#define STACK
#endif

namespace OHOS {
namespace bluetooth {

/* Class of Device masks and HID major/minor combinations. */
constexpr uint32_t COD_MASK = 0x1FFC;
constexpr uint32_t COD_HID_POINTING = 0x0580;
constexpr uint32_t COD_HID_KEYBOARD = 0x0540;

/* Vendor-specific HCI command payloads */
#define HCI_FILTER_MOUSE_OR_KEYBOARD_OPCODE 0xFC4A

struct ForceSleepFilterHidData {
    bool enabled;
    uint8_t transport;
    OHOS::bluetooth::RawAddress rawAddr;
};

union UnionCmdToHisi {
    ForceSleepFilterHidData hidData;
    uint8_t raw[16];
};

struct SpecificCmdToHisi {
    UnionCmdToHisi cmd;
    uint16_t opcode;
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_HW_HCIDEFS_H
