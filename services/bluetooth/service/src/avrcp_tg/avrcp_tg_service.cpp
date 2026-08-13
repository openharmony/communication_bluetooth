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

/**
 * @file avrcp_tg_service.cpp
 *
 * @brief Defines the class of the AVRCP TG service, including attributes and methods.
 */
#ifndef LOG_TAG
#define LOG_TAG "bt_service_avrcp_tg"
#endif

#include "avrcp_tg_service.h"

#include <functional>
#include "adapter_config.h"
#include "class_creator.h"
#include "log.h"
#include "common_util.h"
#include "profile_service_manager.h"
#include "stub/media_service.h"
#include "btcommon/timer_manager.h"
#include "avrcp_tg_service_manager.h"
#include "thread_util.h"

namespace OHOS {
namespace bluetooth {
void AvrcpTgService::ObserverImpl::OnConnectionStateChanged(const std::string &addr, int state)
{
    HILOGI("addr[%{public}s], state[%{public}d]", GetEncryptAddr(addr).c_str(), state);
}

void AvrcpTgService::ObserverImpl::OnPressButton(const std::string &addr, uint8_t button)
{
    HILOGI("addr[%{public}s], button[%{public}x]", GetEncryptAddr(addr).c_str(), button);
}

void AvrcpTgService::ObserverImpl::OnReleaseButton(const std::string &addr, uint8_t button)
{
    HILOGI("addr[%{public}s], button[%{public}x]", GetEncryptAddr(addr).c_str(), button);
}

void AvrcpTgService::ObserverImpl::OnSetAddressedPlayer(const std::string &addr, uint8_t label, int status) {}

void AvrcpTgService::ObserverImpl::OnSetBrowsedPlayer(const std::string &addr, uint16_t uidCounter, uint32_t numOfItems,
    const std::vector<std::string> &folderNames, uint8_t label, int status) {}

void AvrcpTgService::ObserverImpl::OnGetPlayerAppSettingAttributes(
    const std::string &addr, const std::deque<uint8_t> &attributes, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetPlayerAppSettingValues(
    const std::string &addr, const std::deque<uint8_t> &values, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetPlayerAppSettingCurrentValue(const std::string &addr,
    const std::deque<uint8_t> &attributes, const std::deque<uint8_t> &values, uint8_t label, uint8_t context) {}

void AvrcpTgService::ObserverImpl::OnSetPlayerAppSettingCurrentValue(const std::string &addr, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetPlayerAppSettingAttributeText(const std::string &addr,
    const std::vector<uint8_t> &attributes, const std::vector<std::string> &attrStr, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetPlayerAppSettingValueText(const std::string &addr,
    const std::vector<uint8_t> &values, const std::vector<std::string> &valueStr, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetElementAttributes(const std::string &addr,
    const std::vector<uint32_t> &attribtues, const std::vector<std::string> &values, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetPlayStatus(const std::string &addr, uint32_t songLength, uint32_t songPosition,
    uint8_t playStatus, uint8_t label, uint8_t context) {}

void AvrcpTgService::ObserverImpl::OnPlayItem(const std::string &addr, uint8_t label, int status) {}
void AvrcpTgService::ObserverImpl::OnAddToNowPlaying(const std::string &addr, uint8_t label, int status) {}

void AvrcpTgService::ObserverImpl::OnChangePath(const std::string &addr, uint32_t numOfItems,
    uint8_t label, int status) {}

void AvrcpTgService::ObserverImpl::OnGetMediaPlayers(const std::string &addr, uint16_t uidCounter,
    const std::vector<stub::MediaService::MediaPlayer> &items, uint8_t label, int status) {}

void AvrcpTgService::ObserverImpl::OnGetFolderItems(const std::string &addr, uint16_t uidCounter,
    const std::vector<stub::MediaService::MediaItem> &items, uint8_t label, int status) {}

void AvrcpTgService::ObserverImpl::OnGetItemAttributes(const std::string &addr, const std::vector<uint32_t> &attributes,
    const std::vector<std::string> &values, uint8_t label, int status) {}

void AvrcpTgService::ObserverImpl::OnGetTotalNumberOfItems(
    const std::string &addr, uint16_t uidCounter, uint32_t numOfItems, uint8_t label, int status) {}

void AvrcpTgService::ObserverImpl::OnSetAbsoluteVolume(const std::string &addr, uint8_t volume, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetSelectedTrack(const std::string &addr, uint64_t uid, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetAddressedPlayer(
    const std::string &addr, uint16_t playerId, uint16_t uidCounter, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetUidCounter(const std::string &addr, uint16_t uidCounter, uint8_t label) {}

void AvrcpTgService::ObserverImpl::OnGetCurrentAbsoluteVolume(const std::string &addr,
    uint8_t volume, uint8_t label) {}

IProfileAvrcpTg *AvrcpTgService::ObserverImpl::GetService(void)
{
    auto svManager = IProfileManager::GetInstance();

    return static_cast<IProfileAvrcpTg *>(svManager->GetProfileService(PROFILE_NAME_AVRCP_TG));
}

AvrcpTgService::AvrcpTgService() : utility::Context(PROFILE_NAME_AVRCP_TG, "1.6.2") {}

AvrcpTgService::~AvrcpTgService() {}

utility::Context *AvrcpTgService::GetContext()
{
    return this;
}

/******************************************************************
 * REGISTER / UNREGISTER OBSERVER                                 *
 ******************************************************************/

void AvrcpTgService::RegisterObserver(IObserver *observer)
{
}

void AvrcpTgService::UnregisterObserver(void)
{
}

/******************************************************************
 * ENABLE / DISABLE                                               *
 ******************************************************************/

void AvrcpTgService::Enable(void)
{
    DoInAvrcpTgThread([this]() { EnableNative(); });
}

void AvrcpTgService::EnableNative(void)
{
    AvrcpServiceManager::GetInstance().Init();
    GetContext()->OnEnable(PROFILE_NAME_AVRCP_TG, IsEnabled());
}

void AvrcpTgService::Disable(void)
{
    DoInAvrcpTgThread([this]() { DisableNative(); });
}

void AvrcpTgService::DisableNative(void)
{
    AvrcpServiceManager::GetInstance().DeInit();
    GetContext()->OnDisable(PROFILE_NAME_AVRCP_TG, true);
}

bool AvrcpTgService::IsEnabled(void)
{
    return true;
}

/******************************************************************
 * CONNECTION                                                     *
 ******************************************************************/

void AvrcpTgService::SetActiveDevice(const RawAddress &rawAddr) {}

std::vector<RawAddress> AvrcpTgService::GetConnectedDevices(void)
{
    std::vector<RawAddress> result;

    return result;
}

std::vector<bluetooth::RawAddress> AvrcpTgService::GetDevicesByStates(const std::vector<int> &states)
{
    std::vector<bluetooth::RawAddress> result;

    return result;
}

int AvrcpTgService::GetDeviceState(const RawAddress &rawAddr)
{
    HILOGI("rawAddr[%{public}s]", GetEncryptAddr(rawAddr.GetAddress()).c_str());

    int result = static_cast<int>(BTConnectState::DISCONNECTED);

    HILOGI("result[%{public}d]", result);

    return result;
}

int AvrcpTgService::GetMaxConnectNum(void)
{
    int result = 0;

    return result;
}

int AvrcpTgService::Connect(const RawAddress &rawAddr)
{
    HILOGI("rawAddr[%{public}s]", GetEncryptAddr(rawAddr.GetAddress()).c_str());

    int result = RET_BAD_STATUS;
    return result;
}

int AvrcpTgService::Disconnect(const RawAddress &rawAddr)
{
    HILOGI("rawAddr[%{public}s]", GetEncryptAddr(rawAddr.GetAddress()).c_str());

    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpTgService::GetConnectState(void)
{
    int result = PROFILE_STATE_DISCONNECTED;

    return result;
}

void AvrcpTgService::OnSetAddressedPlayer(const RawAddress &rawAddr, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), label, status);
}

void AvrcpTgService::OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
    const std::vector<std::string> &folderNames, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], uidCounter: %{public}d, label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), uidCounter, label, status);
}

/******************************************************************
 * PLAYER APPLICATION SETTINGS                                    *
 ******************************************************************/

void AvrcpTgService::OnGetPlayerAppSettingAttributes(
    const RawAddress &rawAddr, const std::deque<uint8_t> &attributes, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::OnGetPlayerAppSettingValues(
    const RawAddress &rawAddr, const std::deque<uint8_t> &values, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::deque<uint8_t> &attributes,
    const std::deque<uint8_t> &values, uint8_t label, uint8_t context)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr,
    const std::vector<uint8_t> &attributes, const std::vector<std::string> &attrStr, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::OnGetPlayerAppSettingValueText(const RawAddress &rawAddr, const std::vector<uint8_t> &values,
    const std::vector<std::string> &valueStr, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

/******************************************************************
 * MEDIA INFORMATION PDUS                                         *
 ******************************************************************/

void AvrcpTgService::OnGetElementAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attribtues,
    const std::vector<std::string> &values, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

/******************************************************************
 * PLAY                                                           *
 ******************************************************************/

void AvrcpTgService::OnGetPlayStatus(const RawAddress &rawAddr, uint32_t songLength, uint32_t songPosition,
    uint8_t playStatus, uint8_t label, uint8_t context)
{
    HILOGI("rawAddr[%{public}s], songLength: %{public}u, songPosition: %{public}u, label: %{public}d, "
        "context: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), songLength, songPosition, label, context);
}

void AvrcpTgService::OnPlayItem(const RawAddress &rawAddr, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), label, status);
}

void AvrcpTgService::OnAddToNowPlaying(const RawAddress &rawAddr, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), label, status);
}

/******************************************************************
 * OPERATE THE VIRTUAL FILE SYSTEM                                *
 ******************************************************************/
void AvrcpTgService::OnChangePath(const RawAddress &rawAddr, uint32_t numOfItems, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], numOfItems: %{public}u, label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), numOfItems, label, status);
}

void AvrcpTgService::OnGetMediaPlayers(
    const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMpItem> &items, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], uidCounter: %{public}d, label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), uidCounter, label, status);
}

void AvrcpTgService::OnGetFolderItems(
    const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMeItem> &items, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], uidCounter: %{public}d, label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), uidCounter, label, status);
}

void AvrcpTgService::OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
    const std::vector<std::string> &values, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), label, status);
}

void AvrcpTgService::OnGetTotalNumberOfItems(
    const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems, uint8_t label, int status)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d, status: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), label, status);
}

/******************************************************************
 * ABSOLUTE VOLUME                                                *
 ******************************************************************/

void AvrcpTgService::OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], volume: %{public}d, label: %{public}d",
        GetEncryptAddr(rawAddr.GetAddress()).c_str(), volume, label);
}

/******************************************************************
 * NOTIFICATION                                                   *
 ******************************************************************/

void AvrcpTgService::OnGetSelectedTrack(const RawAddress &rawAddr, uint64_t uid, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::OnGetAddressedPlayer(
    const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::OnGetUidCounter(const RawAddress &rawAddr, uint16_t uidCounter, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::OnGetCurrentAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, uint8_t label)
{
    HILOGI("rawAddr[%{public}s], label: %{public}d", GetEncryptAddr(rawAddr.GetAddress()).c_str(), label);
}

void AvrcpTgService::NotifyPlaybackStatusChanged(uint8_t playStatus, uint32_t playbackPos, uint8_t label)
{
    HILOGI("playbackPos: %{public}u, label: %{public}d", playbackPos, label);
}

void AvrcpTgService::NotifyTrackChanged(uint64_t uid, uint32_t playbackPos, uint8_t label)
{
    HILOGI("playbackPos: %{public}u, label: %{public}d", playbackPos, label);
}

void AvrcpTgService::NotifyTrackReachedEnd(uint32_t playbackPos, uint8_t label)
{
    HILOGI("playbackPos: %{public}u, label: %{public}d", playbackPos, label);
}

void AvrcpTgService::NotifyTrackReachedStart(uint32_t playbackPos, uint8_t label)
{
    HILOGI("playbackPos: %{public}u, label: %{public}d", playbackPos, label);
}

void AvrcpTgService::NotifyPlaybackPosChanged(uint32_t playbackPos, uint8_t label)
{
    HILOGI("playbackPos: %{public}u, label: %{public}d", playbackPos, label);
}

void AvrcpTgService::NotifyPlayerAppSettingChanged(
    const std::deque<uint8_t> &attributes, const std::deque<uint8_t> &values, uint8_t label)
{
    HILOGI("playerId: %{public}d", label);
}

void AvrcpTgService::NotifyNowPlayingContentChanged(uint8_t label)
{
    HILOGI("playerId: %{public}d", label);
}

void AvrcpTgService::NotifyAvailablePlayersChanged(uint8_t label)
{
    HILOGI("playerId: %{public}d", label);
}

void AvrcpTgService::NotifyAddressedPlayerChanged(uint16_t playerId, uint16_t uidCounter, uint8_t label)
{
    HILOGI("playerId: %{public}d, uidCounter: %{public}d, label: %{public}d", playerId, uidCounter, label);
}

void AvrcpTgService::NotifyUidChanged(uint16_t uidCounter, uint8_t label) {}

void AvrcpTgService::NotifyVolumeChanged(uint8_t volume, uint8_t label) {}

void AvrcpTgService::OnSessionCreate(std::string sessionId)
{
    HILOGI("sessionId:%{public}s", sessionId.c_str());
}

void AvrcpTgService::OnSessionRelease(std::string sessionId)
{
    HILOGI("sessionId:%{public}s", sessionId.c_str());
}

void AvrcpTgService::OnTopSessionChange(std::string sessionId)
{
    HILOGI("sessionId:%{public}s", sessionId.c_str());
}

void AvrcpTgService::OnPlaybackStateChange(const int32_t state)
{
    HILOGI("state:%{public}d", state);
}

void AvrcpTgService::SetDeviceAbsVolumeAbility(const RawAddress &addr, int32_t ability)
{
    AvrcpServiceManager::GetInstance().SetDeviceAbsVolumeAbility(addr, ability);
}

int32_t AvrcpTgService::GetDeviceAbsVolumeAbility(const RawAddress &addr)
{
    return AvrcpServiceManager::GetInstance().GetDeviceAbsVolumeAbility(addr);
}

void AvrcpTgService::SetDeviceAbsoluteVolume(const RawAddress &addr, int32_t volumeLevel)
{
    AvrcpServiceManager::GetInstance().SetDeviceAbsoluteVolume(addr, volumeLevel);
}

void AvrcpTgService::NotifyAudioVolumeEvent(int32_t streamType, int32_t volume)
{
    AvrcpServiceManager::GetInstance().NotifyAudioVolumeEvent(streamType, volume);
}

REGISTER_CLASS_CREATOR(AvrcpTgService);
}  // namespace bluetooth
}  // namespace OHOS