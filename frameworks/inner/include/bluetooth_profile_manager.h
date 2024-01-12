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

#ifndef BLUETOOTH_DIALOG_H
#define BLUETOOTH_DIALOG_H

#include <string>
#include "safe_map.h"
#include "singleton.h"

#include "i_bluetooth_host.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "iremote_broker.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"
#include <mutex>

namespace OHOS {
namespace Bluetooth {
const std::string BLUETOOTH_HOST = "BluetoothHost";
struct ProfileIdProperty {
    std::function<void (sptr<IRemoteObject>)> func {};
    std::function<void(void)> bluetoothTurnOffFunc {};
    std::string objectName = "";
};
class BluetoothProfileManager {
DECLARE_DELAYED_SINGLETON(BluetoothProfileManager);

public:
    /**
     * @brief Get the Remote of the Profile
     *
     * @param objectName the objectName of profile
     *
     * @return Returns the Remote of the Profile.
     */
    sptr<IRemoteObject> GetProfileRemote(const std::string &objectName);
    /**
     * @brief register function for profile to get proxy when profile is init
     *
     * @param objectName the objectName of profile
     * @param func the function for profile to register
     *
     * @return Returns the id of the Profile.
     */
    int32_t RegisterFunc(const std::string &objectName, std::function<void (sptr<IRemoteObject>)> func);
    /**
     * @brief register function for profile to get proxy when profile is init
     *
     * @param objectName the objectName of profile
     * @param func the function for profile to register
     * @param bluetoothTurnOffFunc the function for profile to register
     *
     * @return Returns the id of the Profile.
     */
    int32_t RegisterFunc(const std::string &objectName, std::function<void (sptr<IRemoteObject>)> func,
        std::function<void(void)> bluetoothTurnOffFunc);
    /**
     * @brief Deregister function for profile, ensure that there is a deregister after register
     *
     * @param id the id of profile
     */
    void DeregisterFunc(int32_t id);
    /**
     * @brief Notify Bluetooth State Change
     */
    void NotifyBluetoothStateChange(int32_t transport, int32_t status);

private:
    class BluetoothSystemAbility : public SystemAbilityStatusChangeStub {
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    };

    void SubScribeBluetoothSystemAbility();
    void UnSubScribeBluetoothSystemAbility();
    void RunFuncWhenBluetoothServiceStarted();
    sptr<IRemoteObject> GetHostRemote();
    void GetValidId();

    SafeMap<std::string, sptr<IRemoteObject>> profileRemoteMap_;
    SafeMap<int32_t, ProfileIdProperty> profileIdFuncMap_;
    std::atomic_bool isBluetoothServiceOn_ = false;
    sptr<BluetoothSystemAbility> bluetoothSystemAbility_ = nullptr;
    sptr<IRemoteObject> GetProfileRemoteInner(const std::string &objectName);
    std::mutex mutex_;
};
template <typename T>
sptr<T> GetRemoteProxy(const std::string &objectName)
{
    return iface_cast<T>(DelayedSingleton<BluetoothProfileManager>::GetInstance()->GetProfileRemote(objectName));
};
} // namespace bluetooth
} // namespace OHOS
#endif // BLUETOOTH_PROFILE_MANAGER_H