/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_OS_ACCOUNT_H
#define BLUETOOTH_OS_ACCOUNT_H

#include <cstdint>

namespace OHOS {
namespace bluetooth {

constexpr int32_t INVALID_ID = -1;
 
class BluetoothOsAccount final {
public:
    BluetoothOsAccount();
    ~BluetoothOsAccount() = default;
 
    static int32_t GetActiveOsAccountId(void);
};
} // namespace bluetooth
} // namespace OHOS
#endif // BLUETOOTH_OS_ACCOUNT