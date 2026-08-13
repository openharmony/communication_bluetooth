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

#ifndef BLUETOOTH_ABILITY_CONNECTION_H
#define BLUETOOTH_ABILITY_CONNECTION_H

#include <cstdint>
#include <string>

#include "ability_connection.h"

namespace OHOS {
namespace bluetooth {
class BluetoothAbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    explicit BluetoothAbilityConnection(const std::string commandStr,
        const std::string bundleName, const std::string abilityName)
    {
        commandStr_ = commandStr;
        bundleName_ = bundleName;
        abilityName_ = abilityName;
    }

    virtual ~BluetoothAbilityConnection() = default;
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;

private:
    std::string commandStr_;
    std::string bundleName_;
    std::string abilityName_;
};
} // namespace bluetooth
} // namespace OHOS
#endif // BLUETOOTH_ABILITY_CONNECTION_H