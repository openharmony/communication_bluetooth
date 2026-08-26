/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

/*
 * Stub of the removed stack layer EDM whitelist manager
 * (common/bluetooth_edm_whitelist_manager.h).
 */

#ifndef BLUETOOTH_EDM_WHITELIST_MANAGER_H
#define BLUETOOTH_EDM_WHITELIST_MANAGER_H

#include <string>

namespace OHOS {
namespace bluetooth {
class BluetoothEdmWhitelistManager {
public:
    static BluetoothEdmWhitelistManager &GetInstance()
    {
        static BluetoothEdmWhitelistManager instance;
        return instance;
    }

    bool IsWhiteListAddress(const std::string &address) const
    {
        (void)address;
        return false;
    }

private:
    BluetoothEdmWhitelistManager() = default;
    ~BluetoothEdmWhitelistManager() = default;
};
}  // namespace bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_EDM_WHITELIST_MANAGER_H
