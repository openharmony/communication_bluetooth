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
#ifndef LOG_TAG
#define LOG_TAG "bt_fwk_profile_manager"
#endif

#include "bluetooth_profile_manager.h"

#include <atomic>
#include <mutex>

#include "i_bluetooth_host.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bluetooth_no_destructor.h"
#include "ohos_bt_gatt.h"

namespace OHOS {
namespace Bluetooth {

BluetoothProfileManager::BluetoothProfileManager()
{
    bluetoothSystemAbility_ = new BluetoothSystemAbility();
    SubScribeBluetoothSystemAbility();
}

BluetoothProfileManager::~BluetoothProfileManager()
{
    UnSubScribeBluetoothSystemAbility();
}

BluetoothProfileManager &BluetoothProfileManager::GetInstance()
{
    static BluetoothNoDestructor<BluetoothProfileManager> instance;
    return *instance;
}

void BluetoothProfileManager::SubScribeBluetoothSystemAbility()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG(samgrProxy != nullptr, "[BLUETOOTH_PROFILE_MANAGER] failed to get samgrProxy");
    int32_t ret = samgrProxy->SubscribeSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, bluetoothSystemAbility_);
    CHECK_AND_RETURN_LOG(ret == ERR_OK,
        "[BLUETOOTH_PROFILE_MANAGER] subscribe systemAbilityId: bluetooth service failed!");
}

void BluetoothProfileManager::UnSubScribeBluetoothSystemAbility()
{
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG(samgrProxy != nullptr, "[BLUETOOTH_PROFILE_MANAGER] failed to get samgrProxy");
    int32_t ret = samgrProxy->UnSubscribeSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, bluetoothSystemAbility_);
    CHECK_AND_RETURN_LOG(ret == ERR_OK,
        "[BLUETOOTH_PROFILE_MANAGER] Unsubscribe systemAbilityId: bluetooth service failed!");
}

sptr<IRemoteObject> BluetoothProfileManager::GetHostRemote()
{
    sptr<IRemoteObject> value = nullptr;
    if (profileRemoteMap_.Find(BLUETOOTH_HOST, value)) {
        return value;
    }
    std::lock_guard<std::mutex> lock(needCheckBluetoothServiceOnMutex_);
    if (!isNeedCheckBluetoothServiceOn_.load()) {
        HILOGD("Bluetooth service is not start");
        return value;
    }
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_LOG_RET(samgrProxy != nullptr, nullptr, "samgrProxy is nullptr");
    auto object = samgrProxy->CheckSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (object == nullptr) {
        HILOGE("object is nullptr");
        isNeedCheckBluetoothServiceOn_ = false;
        return nullptr;
    }
    CHECK_AND_RETURN_LOG_RET(object != nullptr, nullptr, "object is nullptr");
    return object;
}

sptr<IRemoteObject> BluetoothProfileManager::GetProfileRemote(const std::string &objectName)
{
    std::lock_guard<std::mutex> lock(getProfileRemoteMutex_);
    sptr<IRemoteObject> remote = nullptr;
    if (profileRemoteMap_.Find(objectName, remote)) {
        return remote;
    } // SafeMap
    auto hostRemote = GetHostRemote();
    if (hostRemote == nullptr) {
        HILOGD("hostRemote is nullptr");
        return nullptr;
    }
    if (objectName == BLUETOOTH_HOST) {
        remote = hostRemote;
    } else {
        sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
        CHECK_AND_RETURN_LOG_RET(hostProxy != nullptr, nullptr, "hostProxy is nullptr");
        if (objectName == BLE_ADVERTISER_SERVER || objectName == BLE_CENTRAL_MANAGER_SERVER) {
            remote = hostProxy->GetBleRemote(objectName);
        } else {
            remote = hostProxy->GetProfile(objectName);
        }
    }
    if (remote == nullptr) {
        HILOGD("remote is nullptr");
        return nullptr;
    }
    profileRemoteMap_.Insert(objectName, remote);
    return remote;
}

void BluetoothProfileManager::NotifyBluetoothStateChange(int32_t transport, int32_t status)
{
    if (transport == ADAPTER_BLE && status == STATE_TURN_OFF) {
        profileIdFuncMap_.Iterate([this](const int32_t id, ProfileIdProperty &property) {
            // true if *this stores a callcack function target.flase otherwise
            if (property.functions.bluetoothTurnOffFunc) {
                property.functions.bluetoothTurnOffFunc();
            }
        });
    }
    if (transport == ADAPTER_BLE && status == STATE_TURN_ON) {
        HILOGD("Clear global variables, return to initial state");
        ClearGlobalResource();
        profileIdFuncMap_.Iterate([this](const int32_t id, ProfileIdProperty &property) {
            if (property.functions.bleTurnOnFunc) {
                auto remote = GetProfileRemote(property.objectName);
                property.functions.bleTurnOnFunc(remote);
            }
        });
    }
    return;
}

void BluetoothProfileManager::RunFuncWhenBluetoothServiceStarted()
{
    profileIdFuncMap_.Iterate([this](const int32_t id, ProfileIdProperty &property) {
        auto remote = GetProfileRemote(property.objectName);
        if (property.functions.bluetoothLoadedfunc) {
            property.functions.bluetoothLoadedfunc(remote);
        }
    });
}

void BluetoothProfileManager::BluetoothSystemAbility::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    HILOGI("systemAbilityId:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
        case BLUETOOTH_HOST_SYS_ABILITY_ID: {
            BluetoothProfileManager::GetInstance().isBluetoothServiceOn_ = true;
            {
                std::lock_guard<std::mutex> lock(
                    BluetoothProfileManager::GetInstance().needCheckBluetoothServiceOnMutex_);
                BluetoothProfileManager::GetInstance().isNeedCheckBluetoothServiceOn_ = true;
            }
            BluetoothHost::GetDefaultHost().LoadSystemAbilitySuccess(nullptr);
            BluetoothProfileManager::GetInstance().RunFuncWhenBluetoothServiceStarted();
            break;
        }
        default:
            HILOGE("unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
    return;
}

void BluetoothProfileManager::BluetoothSystemAbility::OnRemoveSystemAbility(int32_t systemAbilityId,
    const std::string &deviceId)
{
    HILOGI("systemAbilityId:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
        case BLUETOOTH_HOST_SYS_ABILITY_ID: {
            HILOGD("Clear global variables first");
            ClearGlobalResource();
            BluetoothProfileManager::GetInstance().profileRemoteMap_.Clear();
            BluetoothProfileManager::GetInstance().isBluetoothServiceOn_ = false;
            {
                std::lock_guard<std::mutex> lock(
                    BluetoothProfileManager::GetInstance().needCheckBluetoothServiceOnMutex_);
                BluetoothProfileManager::GetInstance().isNeedCheckBluetoothServiceOn_ = true;
            }
            BluetoothHost::GetDefaultHost().OnRemoveBluetoothSystemAbility();
            break;
        }
        default:
            HILOGE("unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
    return;
}

int32_t BluetoothProfileManager::GetValidId()
{
    std::lock_guard<std::mutex> lock(idMutex_);
    registerValidId_++;
    return registerValidId_;
}

int32_t BluetoothProfileManager::RegisterFunc(const std::string &objectName,
    std::function<void (sptr<IRemoteObject>)> func)
{
    int32_t id = GetValidId();
    ProfileIdProperty value;
    ProfileIdProperty idProperties;
    idProperties.objectName = objectName;
    idProperties.functions.bluetoothLoadedfunc = func;
    HILOGD("objectname: %{public}s, id: %{public}d", objectName.c_str(), id);
    profileIdFuncMap_.Insert(id, idProperties);
    if (isBluetoothServiceOn_) {
        sptr<IRemoteObject> remote = GetProfileRemote(objectName);
        CHECK_AND_RETURN_LOG_RET(remote != nullptr, id, "remote is nullptr"); // 蓝牙已开启，但getremote失败。
        func(remote);
    }
    return id;
}

int32_t BluetoothProfileManager::RegisterFunc(const std::string &objectName, ProfileFunctions profileFunctions)
{
    int32_t id = GetValidId();
    ProfileIdProperty value;
    ProfileIdProperty idProperties;
    idProperties.objectName = objectName;
    idProperties.functions = profileFunctions;
    HILOGI("objectname: %{public}s, id: %{public}d", objectName.c_str(), id);
    profileIdFuncMap_.Insert(id, idProperties);
    if (isBluetoothServiceOn_) {
        sptr<IRemoteObject> remote = GetProfileRemote(objectName);
        CHECK_AND_RETURN_LOG_RET(remote != nullptr, id, "remote is nullptr"); // 蓝牙已开启，但getremote失败。
        if (profileFunctions.bluetoothLoadedfunc) {
            profileFunctions.bluetoothLoadedfunc(remote);
        }
        if (profileFunctions.bleTurnOnFunc && IS_BLE_ENABLED()) {
            profileFunctions.bleTurnOnFunc(remote);
        }
    }
    return id;
}

void BluetoothProfileManager::DeregisterFunc(int32_t id)
{
    HILOGI("id: %{public}d", id);
    ProfileIdProperty value;
    CHECK_AND_RETURN_LOG(profileIdFuncMap_.Find(id, value), "id is not exist");
    profileIdFuncMap_.Erase(id);
}

bool BluetoothProfileManager::IsBluetoothServiceOn()
{
    return isBluetoothServiceOn_.load();
}
} // namespace bluetooth
} // namespace OHOS