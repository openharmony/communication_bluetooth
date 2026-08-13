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

#ifndef DIALOG_PAIR_H
#define DIALOG_PAIR_H

#include "raw_address.h"

namespace OHOS {
namespace bluetooth {
class DialogPair {
public:
    /**
     * @brief Request bluetooth pair dialog.
     *
     * @param device device.
     * @param reqType reqType for pair
     * @param number pinCode
     * @return Returns <b>true</b> if the operation is successful;
     *         returns <b>false</b> if the operation fails.
     * @since 6
     */
    static bool RequestBluetoothPairDialog(const RawAddress &device, int reqType, int number);

    /**
     * @brief Build command string to send
     *
     * @param device device.
     * @param reqType reqType for pair
     * @param number pinCode
     * @return Returns string to send
     * @since 6
     */
    static std::string BuildStartCommand(const RawAddress &device, int reqType, int number);
};
} // namespace bluetooth
} // namespace OHOS
#endif // DIALOG_PAIR_H