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

#ifndef TAIHE_BLUETOOTH_BLE_UTILS_H_
#define TAIHE_BLUETOOTH_BLE_UTILS_H_

#include "bluetooth_ble_advertiser.h"

#include "ohos.bluetooth.ble.proj.hpp"
#include "ohos.bluetooth.ble.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

namespace OHOS {
namespace Bluetooth {
constexpr int SDK_VERSION_20 = 20;

enum class ScanReportType {
    ON_FOUND = 1, // the found of advertisement packet
    ON_LOST = 2   // the lost of advertisement packet
};

enum class SensitivityMode {
    SENSITIVITY_MODE_HIGH = 1,  //  high sensitivity mode
    SENSITIVITY_MODE_LOW = 2    //  low sensitivity mode
};

int GetCurrentSdkVersion(void);
int GetSDKAdaptedStatusCode(int status);
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_BLE_UTILS_H_