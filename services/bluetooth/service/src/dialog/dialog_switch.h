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

#ifndef DIALOG_SWITCH_H
#define DIALOG_SWITCH_H

#include <string>

namespace OHOS {
namespace bluetooth {
enum DialogSwitchType {
    ENABLE_BLUETOOTH = 0,
    DISABLE_BLUETOOTH
};
class DialogSwitch {
public:
    /**
     * @brief Request bluetooth switch dialog.
     *
     * @param type enablebluetooth or disablebluetooth.
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    static bool RequestBluetoothSwitchDialog(DialogSwitchType type);

    /**
     * @brief Build command string to send
     *
     * @param type enablebluetooth or disablebluetooth.
     * @param thirdlyBundleName thirdly bundleName
     * @return Returns string to send
     * @since 6
     */
    static std::string BuildStartCommand(DialogSwitchType type, std::string thirdlyBundleName);
};
} // namespace bluetooth
} // namespace OHOS
#endif // DIALOG_SWITCH_H