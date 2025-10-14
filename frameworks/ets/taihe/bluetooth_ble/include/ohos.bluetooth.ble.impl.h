/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_BLUETOOTH_BLE_IMPL_H
#define TAIHE_BLUETOOTH_BLE_IMPL_H

#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "taihe_bluetooth_ble_central_manager_callback.h"
#include "taihe_bluetooth_gatt_client_callback.h"
#include "taihe/common.h"
#include "taihe/common.hpp"
#include "taihe/runtime.hpp"
#include "taihe/string.hpp"

namespace OHOS {
namespace Bluetooth {

ani_object StartAdvertisingAsyncPromise([[maybe_unused]] ani_env *env, ani_object advertisingParams);
ani_object StartAdvertisingAsyncCallback([[maybe_unused]] ani_env *env, ani_object advertisingParams,
                                         [[maybe_unused]] ani_object object);
ani_object DisableAdvertisingAsyncPromise([[maybe_unused]] ani_env *env, ani_object advertisingDisableParams);
ani_object DisableAdvertisingAsyncCallback([[maybe_unused]] ani_env *env, ani_object advertisingDisableParams,
                                           [[maybe_unused]] ani_object object);
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_BLE_IMPL_H