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

#ifndef TAIHE_BLUETOOTH_ERROR_H_
#define TAIHE_BLUETOOTH_ERROR_H_

#include "bluetooth_log.h"
#include "taihe/runtime.hpp"

namespace OHOS {
namespace Bluetooth {
#ifndef ANI_BT_ASSERT
#define ANI_BT_ASSERT(cond, errCode, errMsg)     \
do {                                                     \
    if (!(cond)) {                                      \
        taihe::set_business_error(errCode, errMsg);            \
        HILOGE("bluetoothManager ani assert failed.");  \
        return;                                         \
    }                                                   \
} while (0)
#endif
}  // namespace Bluetooth
}  // namespace OHOS
#endif // TAIHE_BLUETOOTH_ERROR_H_