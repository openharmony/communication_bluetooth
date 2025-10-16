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

#ifndef TAIHE_TOOL_BLUETOOTH_UTILS_H
#define TAIHE_TOOL_BLUETOOTH_UTILS_H

#include "ohos.bluetooth.ble.impl.hpp"
#include "ohos.bluetooth.ble.proj.hpp"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {
::ohos::bluetooth::ble::AdvertisingParams TaiheParseAdvertisingParams(ani_env *env, ani_object ani_obj);
::ohos::bluetooth::ble::AdvertisingDisableParams TaiheParseAdvertisingDisableParams(ani_env *env, ani_object ani_obj);
::ohos::bluetooth::ble::AdvertisingEnableParams TaiheParseAdvertisingEnableParams(ani_env* env, ani_object ani_obj);
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // TAIHE_TOOL_BLUETOOTH_UTILS_H