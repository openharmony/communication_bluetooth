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

#ifndef BLUETOOTH_AVRC_DEFS_H
#define BLUETOOTH_AVRC_DEFS_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {

/* AVRCP command identifiers used by the Huawei service layer. */
enum AvrcCommandId : uint8_t {
    AVRC_ID_PLAY = 0x00,
    AVRC_ID_STOP = 0x01,
    AVRC_ID_PAUSE = 0x02,
    AVRC_ID_FORWARD = 0x03,
    AVRC_ID_BACKWARD = 0x04,
    AVRC_ID_REWIND = 0x05,
    AVRC_ID_FAST_FOR = 0x06,
};

/* AVRCP player application setting values used by the Huawei service layer.
 * Value 0x00 stands for the query/status request, matching the reserved
 * value of the AVRCP player attribute value space. */
enum AvrcShuffleValue : uint8_t {
    SHUFFLE_QUERY = 0x00,
    SHUFFLE_OFF = 0x01,
    SHUFFLE_ALL_TRACKS = 0x02,
    SHUFFLE_GROUP = 0x03,
    SHUFFLE_INVALID = 0xFF,
};

enum AvrcRepeatValue : uint8_t {
    REPEAT_OFF = 0x01,
    REPEAT_SINGLE_TRACK = 0x02,
    REPEAT_ALL_TRACKS = 0x03,
    REPEAT_GROUP = 0x04,
    REPEAT_INVALID = 0xFF,
};

}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_AVRC_DEFS_H
