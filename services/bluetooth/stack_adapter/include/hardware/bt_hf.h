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
 * Stub of the removed stack layer HFP HAL interface (hardware/bt_hf.h).
 */

#ifndef HARDWARE_BT_HF_H
#define HARDWARE_BT_HF_H

#include <cstddef>

#include "bt_types.h"
#include "../bt_hf.h"

struct BthfHalInterface {
    size_t size;
    BtStackStatus (*init)(BthfCallbacks *callbacks);
    void (*cleanup)(void);
    BtStackStatus (*connect)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*disconnect)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*audioConnect)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*audioDisconnect)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*startVoiceRecognition)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*stopVoiceRecognition)(const OHOS::bluetooth::RawAddress *bdAddr);
    BtStackStatus (*volumeControl)(const OHOS::bluetooth::RawAddress *bdAddr,
                                  BthfVolumeType type, int volume);
};

#endif  // HARDWARE_BT_HF_H
