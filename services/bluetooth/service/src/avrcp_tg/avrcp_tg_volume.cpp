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
#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_tg_volume"
#endif

#include "avrcp_tg_volume.h"

#include <cmath>

#include "bluetooth_common_event_helper.h"
#include "gavdp/a2dp_service.h"
#include "ipc_skeleton.h"
#include "common_util.h"
#include "interface_adapter_classic.h"
#include "interface_adapter_manager.h"
#include "permission_manager.h"
#include "preferences_manager.h"
#include "bt_def.h"
#include "remote_device_properties.h"
#include "refuse_play_helper.h"
#include "stack_adapter.h"
#include "bt_chr_business_event.h"
#include "bt_chr_dft_exception.h"
#include "bt_chr_ue_manager.h"
#include "bluetooth_audio_framework_adapter.h"
#include "hfp_ag_system_interface.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
using namespace OHOS::Bluetooth;
const int HW_BT_PROPERTY_ABS_VOLUM_KEY = 0x211;
OHOS::bluetooth::RawAddress GetA2dpActiveDevice()
{
    A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
    if (a2dpService == nullptr) {
        return RawAddress(INVALID_MAC_ADDRESS);
    }
    auto device = a2dpService->GetActiveSinkDevice();
    OHOS::bluetooth::RawAddress rawAddr = device;

    return rawAddr;
}

void AvrcpVolumeInterfaceImpl::Init()
{
    musicMaxVolumeLevel_ = BluetoothAudioFrameworkAdapter::BtGetMaxVolume(BtVolumeType::VOLUME_MUSIC,
        BtDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP);
}

void AvrcpVolumeInterfaceImpl::Cleanup()
{}

void AvrcpVolumeInterfaceImpl::DeviceConnected(const OHOS::bluetooth::RawAddress &bdaddr)
{
    RemoteDeviceProperties::GetInstance()->GetRemoteDeviceProperty(
        bdaddr, static_cast<BtPropertyType>(HW_BT_PROPERTY_ABS_VOLUM_KEY));
    PreferencesManager::Save(bdaddr.GetAddress(), false, PreferencesManagerType::ABS_VOLUME_SWITCH);
    SwitchAbsVolumeDevice(bdaddr, false);
    RefusePlayHelper::GetInstance()->SetLastAvrcpConnectTime(bdaddr.GetAddress());
    BluetoothHelper::BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(bdaddr.GetAddress(),
        static_cast<int32_t>(BTConnectState::CONNECTED));
    DoInA2dpThread([bdaddr]() {
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(a2dpService, "a2dp service is nullptr");
        a2dpService->ProcessAvrcpDynamicLoad(
            static_cast<int>(BTConnectState::CONNECTED), bdaddr, "avrcp");
    });
}

void AvrcpVolumeInterfaceImpl::DeviceConnected(const OHOS::bluetooth::RawAddress &bdaddr, VolumeChangedCb cb)
{
    HILOGI("support absolute volume %{public}s.", GET_ENCRYPT_STR_ADDR(bdaddr.GetAddress()));
    RemoteDeviceProperties::GetInstance()->GetRemoteDeviceProperty(
        bdaddr, static_cast<BtPropertyType>(HW_BT_PROPERTY_ABS_VOLUM_KEY));
    {
        std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
        volumeCallbackMap_.insert_or_assign(bdaddr, cb);
    }

    PreferencesManager::Save(bdaddr.GetAddress(), true, PreferencesManagerType::ABS_VOLUME_SWITCH);
    SwitchAbsVolumeDevice(bdaddr, false);
    RefusePlayHelper::GetInstance()->SetLastAvrcpConnectTime(bdaddr.GetAddress());
    BluetoothHelper::BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(bdaddr.GetAddress(),
        static_cast<int32_t>(BTConnectState::CONNECTED));
    DoInA2dpThread([bdaddr]() {
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(a2dpService, "a2dp service is nullptr");
        a2dpService->ProcessAvrcpDynamicLoad(
            static_cast<int>(BTConnectState::CONNECTED), bdaddr, "avrcp");
    });
}

void AvrcpVolumeInterfaceImpl::DeviceEmplaceSetVolume(const OHOS::bluetooth::RawAddress &bdaddr, VolumeChangedCb cb)
{
    std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
    volumeCallbackMap_.emplace(bdaddr, cb);
}

void AvrcpVolumeInterfaceImpl::DeviceDisconnected(const OHOS::bluetooth::RawAddress &bdaddr)
{
    std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
    volumeCallbackMap_.erase(bdaddr);
    RefusePlayHelper::GetInstance()->ClearRecord(bdaddr.GetAddress());
    BluetoothHelper::BluetoothCommonEventHelper::PublishAvrcpConnectStateUpdateEvent(bdaddr.GetAddress(),
        static_cast<int32_t>(BTConnectState::DISCONNECTED));
    DoInA2dpThread([bdaddr]() {
        A2dpService *a2dpService = GetServiceInstance(A2DP_ROLE_SOURCE);
        CHECK_AND_RETURN_LOG(a2dpService, "a2dp service is nullptr");
        a2dpService->ProcessAvrcpDynamicLoad(
            static_cast<int>(BTConnectState::DISCONNECTED), bdaddr, "avrcp");
    });
}

void AvrcpVolumeInterfaceImpl::SetVolume(int8_t volume)
{
    int32_t deviceVolume = AvrcpToSystemVolume(volume);
    HILOGD("SetVolume avrcpVolume: %{public}d. musicMaxVolume: %{public}d. deviceVolume: %{public}d.",
        volume,
        musicMaxVolumeLevel_,
        deviceVolume);
    auto device = GetA2dpActiveDevice();
    int32_t storeVolume = PreferencesManager::Get(device.GetAddress(), GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    if (deviceVolume == storeVolume) {
        HILOGW("device %{public}s Skipping SetVolume to same as current %{public}d.",
            GET_ENCRYPT_STR_ADDR(device.GetAddress()), deviceVolume);
        return;
    }
    PreferencesManager::Save(device.GetAddress(), deviceVolume, PreferencesManagerType::ABS_VOLUME);

    std::string macAddr = device.GetAddress();
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    bool updateUi = systemInterface.IsCallIdle();
    // If in call, not update ui for music volume
    int32_t result = BluetoothAudioFrameworkAdapter::BtSetA2dpDeviceVolume(macAddr, deviceVolume, updateUi);
    BtChrBusinessEvent::GetInstance().SetVolumeParams(0, static_cast<int>(volume));
    HILOGI("device %{public}s SetVolume status %{public}d. volume: %{public}d.", GET_ENCRYPT_STR_ADDR(device.GetAddress()),
        result, volume);

    if (result != AVRCP_SUCCESS) {
        HILOGE("excute SetVolume error restore device volume.");
        PreferencesManager::Save(device.GetAddress(), storeVolume, PreferencesManagerType::ABS_VOLUME);
    }
}

void AvrcpVolumeInterfaceImpl::setVolumeOfDevice(int8_t volume, const OHOS::bluetooth::RawAddress &bdaddr)
{
    int32_t deviceVolume = AvrcpToSystemVolume(volume);
    HILOGD("SetVolume avrcpVolume: %{public}d. musicMaxVolume: %{public}d. deviceVolume: %{public}d.",
        volume,
        musicMaxVolumeLevel_,
        deviceVolume);
    int32_t storeVolume = PreferencesManager::Get(bdaddr.GetAddress(), GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    if (deviceVolume == storeVolume) {
        HILOGW("device %{public}s Skipping SetVolume to same as current %{public}d.",
            GET_ENCRYPT_STR_ADDR(bdaddr.GetAddress()), deviceVolume);
        return;
    }
    PreferencesManager::Save(bdaddr.GetAddress(), deviceVolume, PreferencesManagerType::ABS_VOLUME);

    std::string macAddr = bdaddr.GetAddress();
    const HfpAgSystemInterface &systemInterface = HfpAgSystemInterface::GetInstance();
    bool updateUi = systemInterface.IsCallIdle();
    // If in call, not update ui for music volume
    int32_t result = BluetoothAudioFrameworkAdapter::BtSetA2dpDeviceVolume(macAddr, deviceVolume, updateUi);
    BtChrBusinessEvent::GetInstance().SetVolumeParams(0, static_cast<int>(volume));
    HILOGI("device %{public}s SetA2dpDeviceVolume status %{public}d. volume: %{public}d",
        GET_ENCRYPT_STR_ADDR(bdaddr.GetAddress()), result, volume);

    if (result != AVRCP_SUCCESS) {
        HILOGE("excute SetVolume error restore device volume.");
        PreferencesManager::Save(bdaddr.GetAddress(), storeVolume, PreferencesManagerType::ABS_VOLUME);
    }
}

void AvrcpVolumeInterfaceImpl::SendVolumeChanged(const OHOS::bluetooth::RawAddress &addr, int32_t volume)
{
    if ((volume < 0) || (volume > musicMaxVolumeLevel_)) {
        HILOGE("Invalid volume %{public}d.", volume);
        return;
    }

    if (!PreferencesManager::Get(addr.GetAddress(), false, PreferencesManagerType::ABS_VOLUME_SWITCH)) {
        HILOGW("device not support abs volume %{public}s.", GET_ENCRYPT_STR_ADDR(addr.GetAddress()));
        return;
    }
    int32_t storeVolume = PreferencesManager::Get(addr.GetAddress(), GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    if (volume == storeVolume) {
        HILOGW("%{public}s Skipping update volume to same as current %{public}d.",
            GET_ENCRYPT_STR_ADDR(addr.GetAddress()), volume);
        return;
    }
    PreferencesManager::Save(addr.GetAddress(), volume, PreferencesManagerType::ABS_VOLUME);
    BtChrSetAudioVolume(addr.GetAddress(), volume);
    std::string callingName = PermissionManager::GetCallingName();
    BtChrUeManager::GetInstance()->WriteVolChangeUe(BtVolumeType::VOLUME_MUSIC, volume, callingName);

    int32_t avrcpVolume = SystemToAvrcpVolume(volume);
    BtChrBusinessEvent::GetInstance().SetVolumeParams(1, static_cast<int>(avrcpVolume));
    HILOGI("volume: %{public}d. musicMaxVolume: %{public}d. avrcpVolume: %{public}d. "
           "avrcpVolume & 0x7F: %{public}d. addr: %{public}s.",
        volume,
        musicMaxVolumeLevel_,
        avrcpVolume,
        avrcpVolume & 0x7F,
        GET_ENCRYPT_STR_ADDR(addr.GetAddress()));
    std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
    for (const auto &cb : volumeCallbackMap_) {
        cb.second(addr, avrcpVolume & 0x7F);
    }
}

// actice device changed, restore the volume saved last time.
void AvrcpVolumeInterfaceImpl::SwitchAbsVolumeDevice(const OHOS::bluetooth::RawAddress &addr, bool isNeedSetVolume)
{
    if (addr.GetAddress() != GetA2dpActiveDevice().GetAddress()) {
        HILOGW("Not current active device, skip %{public}s.", GET_ENCRYPT_STR_ADDR(addr.GetAddress()));
        return;
    }

    int32_t ret;
    auto rawAddr = addr;
    std::string macAddr = rawAddr.GetAddress();
    int32_t absVolumeAbility =
        RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(rawAddr);
    if (absVolumeAbility == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
        ret = BluetoothAudioFrameworkAdapter::BtSetDeviceAbsVolumeSupported(macAddr, false);
        int32_t volume = PreferencesManager::Get(addr.GetAddress(), GetDefaultVolume(),
            PreferencesManagerType::NON_ABS_VOLUME);
        if (isNeedSetVolume) {
            BluetoothAudioFrameworkAdapter::SetStreamVolume(STREAM_MUSIC, volume, 1);
        }
        HILOGI("device %{public}s not support abs volume status %{public}d, volume %{public}d.",
            GET_ENCRYPT_STR_ADDR(addr.GetAddress()), ret, volume);
        return;
    }

    // get remmber volume and put into effect
    int32_t storedVolume = PreferencesManager::Get(addr.GetAddress(), GetDefaultVolume(),
        PreferencesManagerType::ABS_VOLUME);
    HILOGI("device store volume %{public}d.", storedVolume);
    if (!PreferencesManager::Get(addr.GetAddress(), false, PreferencesManagerType::ABS_VOLUME_SWITCH)) {
        ret = BluetoothAudioFrameworkAdapter::BtSetDeviceAbsVolumeSupported(macAddr, false);
        HILOGI("device %{public}s abs volume switch is false, status %{public}d.",
            GET_ENCRYPT_STR_ADDR(addr.GetAddress()), ret);
        if (isNeedSetVolume) {
            BluetoothAudioFrameworkAdapter::SetStreamVolume(STREAM_MUSIC, storedVolume, 1);
        }
        return;
    }

    int32_t avrcpVolume = SystemToAvrcpVolume(storedVolume);
    {
        std::lock_guard<std::mutex> lock(volumeCallbackMapLock_);
        for (const auto &cb : volumeCallbackMap_) {
            cb.second(addr, avrcpVolume & 0x7F);
        }
    }
    ret = BluetoothAudioFrameworkAdapter::BtSetDeviceAbsVolumeSupported(macAddr, true, storedVolume);
    HILOGI("set device %{public}s support abs volume status %{public}d.", GET_ENCRYPT_STR_ADDR(addr.GetAddress()), ret);
    if (isNeedSetVolume) {
        ret = BluetoothAudioFrameworkAdapter::BtSetA2dpDeviceVolume(macAddr, storedVolume, false);
    }
    HILOGI("set device %{public}s abs volume level status %{public}d.", GET_ENCRYPT_STR_ADDR(addr.GetAddress()), ret);
}

void AvrcpVolumeInterfaceImpl::SetDeviceAbsVolumeAbility(const OHOS::bluetooth::RawAddress &rawAddr, int32_t ability)
{
    auto device = rawAddr;
    int32_t absVolumeAbility = RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(device);
    if (absVolumeAbility == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
        HILOGE("device %{public}s has no abs ability, can not enable or disbale.",
            GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()));
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE0,
            UE_COMMON_SCENE_CASE2);
        return;
    }

    SetDeviceAbsVolumeProperty(rawAddr, ability);
    HILOGI("device %{public}s, abs status = %{public}d. ", GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()), ability);
    if (ability == DeviceAbsVolumeAbility::DEVICE_ABSVOL_OPEN) {
        PreferencesManager::Save(rawAddr.GetAddress(), true, PreferencesManagerType::ABS_VOLUME_SWITCH);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE0,
            UE_COMMON_SCENE_CASE1);
    } else {
        PreferencesManager::Save(rawAddr.GetAddress(), false, PreferencesManagerType::ABS_VOLUME_SWITCH);
        BtChrUeManager::GetInstance()->WriteCommonUe(CHR_UE_BT_UI_SWITCH_CHANGE, device, UE_COMMON_SCENE_CASE0,
            UE_COMMON_SCENE_CASE0);
    }
    SwitchAbsVolumeDevice(rawAddr);
}

int32_t AvrcpVolumeInterfaceImpl::GetDeviceAbsVolumeAbility(const OHOS::bluetooth::RawAddress &rawAddr)
{
    auto device = rawAddr;
    int32_t absVolumeAbility = RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(device);
    HILOGI("device %{public}s, abs ability = %{public}d. ", GET_ENCRYPT_STR_ADDR(rawAddr.GetAddress()), absVolumeAbility);
    return absVolumeAbility;
}

// if absolute volume support, audio set device volume by this interface
void AvrcpVolumeInterfaceImpl::SetDeviceAbsoluteVolume(const OHOS::bluetooth::RawAddress &rawAddr, int32_t volumeLevel)
{
    SendVolumeChanged(rawAddr, volumeLevel);
}

void AvrcpVolumeInterfaceImpl::NotifyAudioVolumeEvent(int32_t streamType, int32_t volume)
{
    HILOGD("streamType: %{public}d, volume: %{public}d.", streamType, volume);
    auto device = GetA2dpActiveDevice();
    CHECK_AND_RETURN_LOG(device.GetAddress() != INVALID_MAC_ADDRESS, "not found active device.");
    CHECK_AND_RETURN_LOG(BluetoothAudioFrameworkAdapter::IsAudioOutputToBluetoothA2dp(), "not output to a2dp.");
    int32_t absVolumeAbility =
        RemoteDeviceProperties::GetInstance()->GetDeviceAbsVolumeAbility(device);
    if (absVolumeAbility == DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
        HILOGE("device %{public}s has no abs ability, volume: %{public}d.",
            GET_ENCRYPT_STR_ADDR(device.GetAddress()), volume);
        PreferencesManager::Save(device.GetAddress(), volume, PreferencesManagerType::NON_ABS_VOLUME);
        return;
    }
    // 绝对音量关闭时，设置音量值
    if (PreferencesManager::Get(device.GetAddress(), false, PreferencesManagerType::ABS_VOLUME_SWITCH)) {
        HILOGE("AbsVolume is open.ignore");
        return;
    }

    PreferencesManager::Save(device.GetAddress(), volume, PreferencesManagerType::ABS_VOLUME);
}

void AvrcpVolumeInterfaceImpl::SetDeviceAbsVolumeProperty(const OHOS::bluetooth::RawAddress &rawAddr, int32_t ability)
{
    BtProperty prop;
    prop.type = static_cast<BtPropertyType>(HW_BT_PROPERTY_ABS_VOLUM_KEY);
    prop.len = sizeof(ability);
    prop.val = &ability;
    RemoteDeviceProperties::GetInstance()->SetRemoteDeviceProperty(rawAddr, prop);
}
}  // namespace bluetooth
}  // namespace OHOS
