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

#include "bluetooth_audio_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_host.h"
#include "i_bluetooth_audio_manager.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothAudioManager::impl {
    impl();
    int EnableWearDetection(const std::string &deviceId);
    int DisableWearDetection(const std::string &deviceId);
    int GetWearDetectionState(const std::string &deviceId, int32_t &ability);
    int IsWearDetectionSupported(const BluetoothRemoteDevice &device, bool &isSupported);
    int SendDeviceSelection(const BluetoothRemoteDevice &device, int useA2dp, int useHfp, int userSelection);
};

BluetoothAudioManager::impl::impl()
{}

BluetoothAudioManager::BluetoothAudioManager():pimpl(std::make_unique<impl>())
{}

int BluetoothAudioManager::impl::EnableWearDetection(const std::string &deviceId)
{
    sptr<IBluetoothAudioManager> proxy = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");
    return proxy->EnableWearDetection(deviceId);
}

int BluetoothAudioManager::impl::DisableWearDetection(const std::string &deviceId)
{
    sptr<IBluetoothAudioManager> proxy = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");
    return proxy->DisableWearDetection(deviceId);
}

int BluetoothAudioManager::impl::GetWearDetectionState(const std::string &deviceId, int32_t &ability)
{
    sptr<IBluetoothAudioManager> proxy = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");
    return proxy->GetWearDetectionState(deviceId, ability);
}

int BluetoothAudioManager::impl::IsWearDetectionSupported(const BluetoothRemoteDevice &device, bool &isSupported)
{
    sptr<IBluetoothAudioManager> proxy = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");
    return proxy->IsWearDetectionSupported(BluetoothRawAddress(device.GetDeviceAddr()), isSupported);
}

int BluetoothAudioManager::impl::SendDeviceSelection(const BluetoothRemoteDevice &device, int useA2dp,
    int useHfp, int userSelection)
{
    sptr<IBluetoothAudioManager> proxy = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");
    return proxy->SendDeviceSelection(BluetoothRawAddress(device.GetDeviceAddr()), useA2dp, useHfp, userSelection);
}

int BluetoothAudioManager::EnableWearDetection(const std::string &deviceId)
{
    sptr<IBluetoothAudioManager> proxy = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");
    return pimpl->EnableWearDetection(deviceId);
}

int BluetoothAudioManager::DisableWearDetection(const std::string &deviceId)
{
    sptr<IBluetoothAudioManager> proxy = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");
    return pimpl->DisableWearDetection(deviceId);
}

int BluetoothAudioManager::GetWearDetectionState(const std::string &deviceId, int32_t &ability)
{
    sptr<IBluetoothAudioManager> proxy = GetRemoteProxy<IBluetoothAudioManager>(PROFILE_AUDIO_MANAGER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");
    return pimpl->GetWearDetectionState(deviceId, ability);
}

int BluetoothAudioManager::IsWearDetectionSupported(const BluetoothRemoteDevice &device, bool &isSupported)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return BT_ERR_INVALID_STATE;
    }
    return pimpl->IsWearDetectionSupported(device, isSupported);
}

int BluetoothAudioManager::SendDeviceSelection(const BluetoothRemoteDevice &device,
    int useA2dp, int useHfp, int userSelection) const
{
    HILOGI("enter, device: %{public}s, useA2dp: %{public}d, useHfp: %{public}d, userSelection:%{public}d",
        GET_ENCRYPT_ADDR(device), useA2dp, useHfp, userSelection);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return BT_ERR_INVALID_STATE;
    }
    return pimpl->SendDeviceSelection(device, useA2dp, useHfp, userSelection);
}

BluetoothAudioManager &BluetoothAudioManager::GetInstance()
{
    static BluetoothAudioManager instance;
    return instance;
}

}  // namespace Bluetooth
}  // namespace OHOS
