/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_DIALOG_H
#define BLUETOOTH_DIALOG_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include "json_utils.h"
#include "log.h"
#include "btcommon/timer_manager.h"
#include "bluetooth_ability_connection.h"
#include "raw_address.h"

namespace OHOS {
namespace bluetooth {
enum DialogType : uint8_t {
    PBAP_AUTH_DIALOG = 0x00,
    MAP_AUTH_DIALOG,
    AUTO_PLAY_AUTH_DIALOG,
};

struct DialogInfo {
    std::string address;
    DialogType type;
    uint32_t timeOutMs;
    std::shared_ptr<utility::Timer> timer;
};

class BluetoothDialog {
public:
    /**
     * @brief Request pbap auth dialog.
     *
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     */
    static bool RequestAuthDialog(DialogInfo &dialog);

    static bool DismissCurAndShowNext();

    /**
     * @brief Set param for dialog
     *
     * @param commandStr Str for dialog.
     * @param bundleName bundleName for dialog.
     * @param abilityName abilityName for dialog.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    static bool DialogConnectExtension(const std::string commandStr,
        const std::string bundleName, const std::string abilityName);

    /**
     * @brief Build bluetooth dialog.
     *
     * @param want want to send.
     * @param commandStr Str for connection.
     * @param bundleName bundleName for dialog.
     * @param abilityName abilityName for dialog.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    static bool DialogConnectExtensionAbility(const AAFwk::Want &want, const std::string commandStr,
        const std::string bundleName, const std::string abilityName);
private:
    static std::string BuildStartCommand(const std::string &address);

    static bool DisplayDialog(DialogInfo &dialog);
};
} // namespace bluetooth
} // namespace OHOS
#endif // BLUETOOTH_DIALOG_H