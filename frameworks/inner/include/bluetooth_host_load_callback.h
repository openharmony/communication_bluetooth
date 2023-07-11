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

#ifndef BLUETOOTH_HOST_LOAD_CALLBACK_H
#define BLUETOOTH_HOST_LOAD_CALLBACK_H

#include <cstdint>
#include <string>

#include "refbase.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHostLoadCallBack : public SystemAbilityLoadCallbackStub {
public:
    /**
     * @brief After the Bluetooth switch is turned on, the system interface will be called to start the Bluetooth
     * process, and the interface callback will be received after success.
     *
     * @param systemAbilityId bluetooth systemAbilityId(1130).
     * @param remoteObject bluetooth host proxy.
     */
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
    /**
     * @brief After the Bluetooth switch is turned on, the system interface will be called to start the Bluetooth
     * process, and the interface callback will be received after failure.
     *
     * @param systemAbilityId bluetooth systemAbilityId(1130).
     */
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};
}  // namespace Bluetooth
}  // namespace OHOS

#endif  // BLUETOOTH_HOST_LOAD_CALLBACK_H