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

#ifndef BLUETOOTH_HW_INTEROP_H
#define BLUETOOTH_HW_INTEROP_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {

/* Huawei interoperability feature bits. Values are opaque placeholders. */
constexpr uint16_t INTEROP_CHANGE_PLAY_STATE_PLAY_TO_PAUSE = 0x0001;
constexpr uint16_t INTEROP_DISABLE_DELAY_SEND_CALL_STATE = 0x0002;
constexpr uint16_t INTEROP_DISABLE_INBAND_RING = 0x0003;
constexpr uint16_t INTEROP_DELAY_CELLULAR_CREATE_SCO = 0x0004;
constexpr uint16_t INTEROP_DELAY_CONNECT_VOIP_SCO = 0x0005;
constexpr uint16_t INTEROP_DELAY_CIEV = 0x0006;
constexpr uint16_t INTEROP_DELAY_CALL_STATE = 0x0007;
constexpr uint16_t INTEROP_NOT_DROP_PLAY = 0x0008;
constexpr uint16_t INTEROP_DISABLE_AUTO_CONNECT_DEVICES = 0x0009;

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_HW_INTEROP_H
