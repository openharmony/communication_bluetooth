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
 * Stub of the removed stack layer AVRCP profile interface (bt_rc.h).
 */

#ifndef BT_RC_H
#define BT_RC_H

#include <cstddef>

#include "bt_types.h"

typedef struct {
    size_t size;
    int (*init)(...);
    void (*cleanup)(...);
    int (*send_pass_through_cmd)(...);
    int (*get_playback_state_cmd)(...);
    int (*get_current_metadata_cmd)(...);
    int (*register_abs_vol_rsp)(...);
    int (*set_volume_rsp)(...);
    int (*set_player_app_setting_cmd)(...);
} btrc_ctrl_interface_t;

#endif  // BT_RC_H
