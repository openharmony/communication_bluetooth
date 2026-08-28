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

typedef struct {
    size_t size;
    bt_status_t (*init)(bthf_callbacks_t *callbacks);
    void (*cleanup)(void);
    bt_status_t (*connect)(const RawAddress *bd_addr);
    bt_status_t (*disconnect)(const RawAddress *bd_addr);
    bt_status_t (*audio_connect)(const RawAddress *bd_addr);
    bt_status_t (*audio_disconnect)(const RawAddress *bd_addr);
    bt_status_t (*start_voice_recognition)(const RawAddress *bd_addr);
    bt_status_t (*stop_voice_recognition)(const RawAddress *bd_addr);
    bt_status_t (*volume_control)(const RawAddress *bd_addr,
                                  bthf_volume_type_t type, int volume);
} bthf_hal_interface_t;

#endif  // HARDWARE_BT_HF_H
