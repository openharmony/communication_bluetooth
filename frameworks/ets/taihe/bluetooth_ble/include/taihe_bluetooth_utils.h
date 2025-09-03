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

#ifndef TAIHE_BLUETOOTH_UTILS_H_
#define TAIHE_BLUETOOTH_UTILS_H_

#include "bluetooth_types.h"
#include "taihe/runtime.hpp"
#include "taihe/array.hpp"

namespace OHOS {
namespace Bluetooth {

enum class ScanReportType {
    ON_FOUND = 1, // the found of advertisement packet
    ON_LOST = 2 // the lost of advertisement packet
};

enum class SensitivityMode {
    SENSITIVITY_MODE_HIGH = 1,  //  high sensitivity mode
    SENSITIVITY_MODE_LOW = 2    //  low sensitivity mode
};

enum ani_status {
    ani_ok = 0,
    ani_invalid_arg,
};

#ifndef ANI_BT_CALL_RETURN
#define ANI_BT_CALL_RETURN(func)                                          \
    do {                                                                   \
        ani_status ret = (func);                                          \
        if (ret != ani_ok) {                                              \
            HILOGE("api call function failed. ret:%{public}d", ret);      \
            return ret;                                                    \
        }                                                                  \
    } while (0)
#endif

bool IsValidAddress(std::string bdaddr);
bool ParseUuidParams(const std::string &uuid, UUID &outUuid);
template<typename T>
void ParseArrayBufferParams(const taihe::array<T>& data, std::vector<T> &outParam)
{
    for (const auto item : data) {
        outParam.push_back(item);
    }
}
} // namespace Bluetooth
} // namespace OHOS
#endif // TAIHE_BLUETOOTH_UTILS_H_