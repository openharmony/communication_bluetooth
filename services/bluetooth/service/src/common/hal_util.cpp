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

#include "hal_util.h"

/* Stub implementation of the Bluetooth HAL library loader. The actual
 * implementation lives in the closed-source vendor HAL; this placeholder
 * returns a static, zero-initialized bt_interface_t so that the migrated
 * bluetooth service code can link. */
int hal_util_load_bt_library(const bt_interface_t **btInterface)
{
    static bt_interface_t bluetoothInterface = {};
    if (btInterface != nullptr) {
        *btInterface = &bluetoothInterface;
    }
    return 0;
}
