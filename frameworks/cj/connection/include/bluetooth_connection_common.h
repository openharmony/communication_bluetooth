/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BLUETOOTH_CONNECTION_COMMON_H
#define BLUETOOTH_CONNECTION_COMMON_H

#include "cj_common_ffi.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothConnection {
enum class BondState { BOND_STATE_INVALID = 0, BOND_STATE_BONDING = 1, BOND_STATE_BONDED = 2 };

bool IsValidAddress(std::string bdaddr);
std::string GetFormatPinCode(uint32_t pinType, uint32_t pinCode);
char* MallocCString(const std::string& origin);
CArrString Convert2CArrString(std::vector<std::string> &tids);
void DealPairStatus(const int &status, int &bondStatus);

} // namespace BluetoothConnection
} // namespace CJSystemapi
} // namespace OHOS

#endif // BLUETOOTH_CONNECTION_COMMON_H