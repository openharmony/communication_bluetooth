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
#define LOG_TAG "bt_service_avrcp_tg_manager"
#endif

#include "avrcp_tg_service_manager.h"
#include "adapter_manager.h"
#include "avrcp_tg_avsession_media_loader.h"
#include "log.h"
#include "control_intercept_plugin.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace bluetooth {
static AvrcpVolumeInterfaceImpl g_volumeInterface;
AvrcpServiceManager &AvrcpServiceManager::GetInstance()
{
    static AvrcpServiceManager instance;
    return instance;
}
void AvrcpServiceManager::Init()
{
#ifdef BT_MCU_PROXY_ENABLE
	return;
#endif
    g_volumeInterface.Init();
    bt_interface_t *bluetoothInterface = AdapterManager::GetInstance()->getBluetoothInterface();
    if (bluetoothInterface == nullptr) {
        HILOGE("bluetoothInterface in nullptr");
        return;
    }
    avrcpServiceInterface_ = bluetoothInterface->get_avrcp_service();
    if (avrcpServiceInterface_) {
        avrcpServiceInterface_->Init(mediaLoader_.get(), &g_volumeInterface);
    }
    DoInBipThread([this]() {
        if (avrcpServiceInterface_ && mediaLoader_) {
            mediaLoader_->StartSocketListener();
            int32_t psm = mediaLoader_->GetBipPsm();
            if (psm > 0) {
                avrcpServiceInterface_->RegisterBipServer(psm);
            }
        }
    });
}
void AvrcpServiceManager::DeInit()
{
    g_volumeInterface.Cleanup();
    DoInBipThread([this]() {
        this->DisableBipService();
        if (avrcpServiceInterface_) {
            avrcpServiceInterface_->Cleanup();
        }
        avrcpServiceInterface_ = nullptr;
    });
}
void AvrcpServiceManager::Connect(const RawAddress &rawAddr)
{
    ControlInterceptMessage msg {
        .addr = rawAddr.GetAddress(),
        .pid = IPCSkeleton::GetCallingPid(),
        .uid = IPCSkeleton::GetCallingUid(),
    };
    CHECK_AND_RETURN_LOG(ControlInterceptIsAllowedAvrcpConn(msg), "Restricted by control intercept");
    if (avrcpServiceInterface_) {
        avrcpServiceInterface_->ConnectDevice(ServiceUtil::AddrToBluedroid(rawAddr));
    }
}
void AvrcpServiceManager::DisConnect(const RawAddress &rawAddr)
{
    if (avrcpServiceInterface_) {
        avrcpServiceInterface_->DisconnectDevice(ServiceUtil::AddrToBluedroid(rawAddr));
    }
}
void AvrcpServiceManager::SwitchAbsVolumeDevice(const RawAddress &rawAddr)
{
    g_volumeInterface.SwitchAbsVolumeDevice(ServiceUtil::AddrToBluedroid(rawAddr));
}
void AvrcpServiceManager::SetDeviceAbsVolumeAbility(const RawAddress &rawAddr, int32_t ability)
{
    g_volumeInterface.SetDeviceAbsVolumeAbility(ServiceUtil::AddrToBluedroid(rawAddr), ability);
}
void AvrcpServiceManager::SetDeviceAbsoluteVolume(const RawAddress &rawAddr, int32_t volumeLevel)
{
    g_volumeInterface.SetDeviceAbsoluteVolume(ServiceUtil::AddrToBluedroid(rawAddr), volumeLevel);
}
int32_t AvrcpServiceManager::GetDeviceAbsVolumeAbility(const RawAddress &rawAddr)
{
    return g_volumeInterface.GetDeviceAbsVolumeAbility(ServiceUtil::AddrToBluedroid(rawAddr));
}

void AvrcpServiceManager::NotifyAudioVolumeEvent(int32_t streamType, int32_t volume)
{
    return g_volumeInterface.NotifyAudioVolumeEvent(streamType, volume);
}

void AvrcpServiceManager::SetDeviceAbsVolumeProperty(const BLUEDROID::RawAddress &rawAddr, int32_t ability)
{
    return g_volumeInterface.SetDeviceAbsVolumeProperty(rawAddr, ability);
}

void AvrcpServiceManager::SetActiveDevice(const RawAddress &rawAddr)
{
    if (avrcpServiceInterface_ != nullptr && mediaLoader_ != nullptr) {
        HILOGI("set avrcp active device: %{public}s", GetEncryptAddr(rawAddr.GetAddress()).c_str());
        mediaLoader_->SetActiveDevice(ServiceUtil::AddrToBluedroid(rawAddr));
    } else {
        HILOGE("avrcp service is nullptr");
    }
}

void AvrcpServiceManager::DisableBipService()
{
    if (mediaLoader_ != nullptr) {
        mediaLoader_->DisableBipService();
    } else {
        HILOGE("avrcp service is nullptr");
    }
}
}  // namespace bluetooth
}  // namespace OHOS