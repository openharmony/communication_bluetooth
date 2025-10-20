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

#include <cstdint>
#include <string>
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {

#ifndef TAIHE_BT_ASSERT_RETURN
#define TAIHE_BT_ASSERT_RETURN(cond, errCode, retObj)     \
do {                                                      \
    if (!(cond)) {                                        \
        HandleSyncErr((errCode));                         \
        HILOGE("bluetoothManager taihe assert failed.");  \
        return (retObj);                                  \
    }                                                     \
} while (0)
#endif

#ifndef TAIHE_BT_ASSERT_RETURN_VOID
#define TAIHE_BT_ASSERT_RETURN_VOID(cond, errCode)        \
do {                                                      \
    if (!(cond)) {                                        \
        HandleSyncErr((errCode));                         \
        HILOGE("bluetooth taihe assert failed.");         \
        return;                                           \
    }                                                     \
} while (0)
#endif

std::string GetTaiheErrMsg(const int32_t errCode);
void HandleSyncErr(int32_t errCode);
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_ERROR_H_