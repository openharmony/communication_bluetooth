/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_BT_GATT_UTILS_H
#define OHOS_BT_GATT_UTILS_H

#include "string"

#ifdef __cplusplus
extern "C" {
#endif

namespace OHOS {
namespace Bluetooth {

#define ADV_ADDR_TIME_THRESHOLD (15 * 60 * 1000) // 15 mins

/*
 * The method is only available for {@link BleStartAdvWithAddr}.
 * Because there cannot be duplicate adv addresses within one hour,
 * this method will delete adv addresses after one hour.
 */
void RemoveTimeoutAdvAddr();

/*
 * This method is only available for {@link BleStartAdvWithAddr}.
 * Duplicate addresses within 15 minutes are allowed to be broadcast,
 * and duplicate addresses after 15 minutes are not allowed to be broadcast.
 * There is no limit on non-duplicate addresses.
 */
bool CanStartAdv(const std::string& addrStr);

}  // namespace Bluetooth
}  // namespace OHOS
#ifdef __cplusplus
}
#endif
#endif