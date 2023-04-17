/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <deque>
#include <list>
#include <mutex>

#include "bluetooth_avrcp_ct.h"

#include "bluetooth_avrcp_ct_stub.h"
#include "bluetooth_avrcp_ct_observer_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "i_bluetooth_avrcp_ct.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

AvrcpCtResponse::AvrcpCtResponse(uint8_t type, int resp) : type_(type), resp_(resp)
{
    HILOGI("enter");
}

AvrcpCtResponse::~AvrcpCtResponse()
{
    HILOGI("enter");
}

struct AvrcpController::impl {
public:

    impl();
    ~impl()
    {
        HILOGI("enter");
    }

    bool IsEnabled(void)
    {
        return false;
    }

    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state)
    {
        return;
    }

    void OnPressButton(const BluetoothRemoteDevice &device, uint8_t button, int result)
    {
        return;
    }

    void OnReleaseButton(const BluetoothRemoteDevice &device, uint8_t button, int result)
    {
        return;
    }

    void OnSetBrowsedPlayer(const BluetoothRemoteDevice &device, uint16_t uidCounter, uint32_t numOfItems,
        const std::vector<std::string> &folderNames, int result, int detail)
    {
        return;
    }

    void OnGetCapabilities(const BluetoothRemoteDevice &device, const std::vector<uint32_t> &companies,
        const std::vector<uint8_t> &events, int result)
    {
        return;
    }

    void OnGetPlayerAppSettingAttributes(
        const BluetoothRemoteDevice &device, std::vector<uint8_t> attributes, int result)
    {
        return;
    }

    void OnGetPlayerAppSettingValues(
        const BluetoothRemoteDevice &device, uint8_t attribute, std::vector<uint8_t> values, int result)
    {
        return;
    }

    void OnGetPlayerAppSettingCurrentValue(
        const BluetoothRemoteDevice &device, std::vector<uint8_t> attributes, std::vector<uint8_t> values, int result)
    {
        return;
    }

    void OnSetPlayerAppSettingCurrentValue(const BluetoothRemoteDevice &device, int result)
    {
        return;
    }

    void OnGetPlayerAppSettingAttributeText(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes,
        const std::vector<std::string> &valueName, int result)
    {
        return;
    }

    void OnGetPlayerAppSettingValueText(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &values,
        const std::vector<std::string> &valueName, int result)
    {
        return;
    }

    void OnGetElementAttributes(const BluetoothRemoteDevice &device, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &valueName, int result)
    {
        return;
    }

    void OnGetPlayStatus(
        const BluetoothRemoteDevice &device, uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result)
    {
        return;
    }

    void OnPlayItem(const BluetoothRemoteDevice &device, int result, int detail)
    {
        return;
    }

    void OnGetMediaPlayers(const BluetoothRemoteDevice &device, uint16_t uidCounter,
        const std::vector<AvrcMpItem> &items, int result, int detail)
    {
        return;
    }

    void OnGetFolderItems(const BluetoothRemoteDevice &device, uint16_t uidCounter,
        const std::vector<AvrcMeItem> &items, int result, int detail)
    {
        return;
    }

    void OnGetItemAttributes(const BluetoothRemoteDevice &device, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &values, int result, int detail)
    {
        return;
    }

    void OnGetTotalNumberOfItems(
        const BluetoothRemoteDevice &device, uint16_t uidCounter, uint32_t numOfItems, int result, int detail)
    {
        return;
    }

    void OnSetAbsoluteVolume(const BluetoothRemoteDevice &device, uint16_t volume, int result)
    {
        return;
    }

    void OnPlaybackStatusChanged(const BluetoothRemoteDevice &device, uint8_t playStatus, int result)
    {
        return;
    }

    void OnTrackChanged(const BluetoothRemoteDevice &device, uint64_t uid, int result)
    {
        return;
    }

    void OnTrackReachedEnd(const BluetoothRemoteDevice &device, int result)
    {
        return;
    }

    void OnTrackReachedStart(const BluetoothRemoteDevice &device, int result)
    {
        return;
    }

    void OnPlaybackPosChanged(const BluetoothRemoteDevice &device, uint32_t playbackPos, int result)
    {
        return;
    }

    void OnPlayerAppSettingChanged(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result)
    {
        return;
    }

    void OnNowPlayingContentChanged(const BluetoothRemoteDevice &device, int result)
    {
        return;
    }

    void OnAvailablePlayersChanged(const BluetoothRemoteDevice &device, int result)
    {
        return;
    }

    void OnAddressedPlayerChanged(
        const BluetoothRemoteDevice &device, uint16_t playerId, uint16_t uidCounter, int result)
    {
        return;
    }

    void OnUidChanged(const BluetoothRemoteDevice &device, uint16_t uidCounter, int result)
    {
        return;
    }

    void OnVolumeChanged(const BluetoothRemoteDevice &device, uint8_t volume, int result)
    {
        return;
    }

    std::mutex observerMutex_;
    BluetoothObserverList<AvrcpController::IObserver> observers_;
    sptr<IBluetoothAvrcpCt> proxy_ = nullptr;
};

AvrcpController::impl::impl()
{
    return;
}

AvrcpController *AvrcpController::GetProfile(void)
{
    HILOGI("enter");
    return nullptr;
}

/******************************************************************
 * REGISTER / UNREGISTER OBSERVER                                 *
 ******************************************************************/

void AvrcpController::RegisterObserver(IObserver *observer)
{
    return;
}

void AvrcpController::UnregisterObserver(IObserver *observer)
{
    return;
}

/******************************************************************
 * CONNECTION                                                     *
 ******************************************************************/

std::vector<BluetoothRemoteDevice> AvrcpController::GetConnectedDevices(void)
{
    HILOGI("enter");

    std::vector<BluetoothRemoteDevice> devices;
    return devices;
}

std::vector<BluetoothRemoteDevice> AvrcpController::GetDevicesByStates(const std::vector<int> &states)
{
    HILOGI("enter");

    std::vector<BluetoothRemoteDevice> devices;
    return devices;
}

int AvrcpController::GetDeviceState(const BluetoothRemoteDevice &device)
{
    return static_cast<int>(BTConnectState::DISCONNECTED);
}

bool AvrcpController::Connect(const BluetoothRemoteDevice &device)
{
    return false;
}

bool AvrcpController::Disconnect(const BluetoothRemoteDevice &device)
{
    return false;
}

/******************************************************************
 * BUTTON OPERATION                                               *
 ******************************************************************/

int AvrcpController::PressButton(const BluetoothRemoteDevice &device, uint8_t button)
{
    return RET_BAD_STATUS;
}

int AvrcpController::ReleaseButton(const BluetoothRemoteDevice &device, uint8_t button)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * TEMP UNIT INFO / SUB UNIT INFO                                 *
 ******************************************************************/

int AvrcpController::GetUnitInfo(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

int AvrcpController::GetSubUnitInfo(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * Capabilities                                                   *
 ******************************************************************/

int AvrcpController::GetSupportedCompanies(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

int AvrcpController::GetSupportedEvents(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * PLAYER APPLICATION SETTINGS                                     *
 ******************************************************************/

int AvrcpController::GetPlayerAppSettingAttributes(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

int AvrcpController::GetPlayerAppSettingValues(const BluetoothRemoteDevice &device, uint8_t attribute)
{
    return RET_BAD_STATUS;
}

int AvrcpController::GetPlayerAppSettingCurrentValue(
    const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes)
{
    return RET_NO_ERROR;
}

int AvrcpController::SetPlayerAppSettingCurrentValue(
    const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes, const std::vector<uint8_t> &values)
{
    return RET_NO_ERROR;
}

int AvrcpController::GetPlayerApplicationSettingAttributeText(
    const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes)
{
    return RET_BAD_STATUS;
}

int AvrcpController::GetPlayerApplicationSettingValueText(
    const BluetoothRemoteDevice &device, uint8_t attributeId, const std::vector<uint8_t> &values)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * MEDIA INFORMATION                                              *
 ******************************************************************/

int AvrcpController::GetElementAttributes(const BluetoothRemoteDevice &device, const std::vector<uint32_t> &attributes)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * PLAY                                                           *
 ******************************************************************/

int AvrcpController::GetPlayStatus(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

int AvrcpController::PlayItem(const BluetoothRemoteDevice &device, uint64_t uid, uint16_t uidCounter)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * OPERATE THE VIRTUAL FILE SYSTEM                                *
 ******************************************************************/

int AvrcpController::GetFolderItems(
    const BluetoothRemoteDevice &device, uint32_t startItem, uint32_t endItem, const std::vector<uint32_t> &attributes)
{
    return RET_NO_ERROR;
}

int AvrcpController::GetMeidaPlayerList(const BluetoothRemoteDevice &device, uint32_t startItem, uint32_t endItem)
{
    return RET_BAD_STATUS;
}

int AvrcpController::GetTotalNumberOfItems(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * ABSOLUTE VOLUME                                                *
 ******************************************************************/

int AvrcpController::SetAbsoluteVolume(const BluetoothRemoteDevice &device, uint8_t volume)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * NOTIFY                                                         *
 ******************************************************************/

int AvrcpController::EnableNotification(
    const BluetoothRemoteDevice &device, const std::vector<uint8_t> &events, uint32_t interval)
{
    return RET_BAD_STATUS;
}

int AvrcpController::DisableNotification(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &events)
{
    return RET_BAD_STATUS;
}

/******************************************************************
 * DO NOT EXPOSE THE INTERFACE                                    *
 ******************************************************************/

int AvrcpController::SetAddressedPlayer(const BluetoothRemoteDevice &device, uint16_t playerId)
{
    HILOGI("enter, device: %{public}s, playerId: %{public}d", GET_ENCRYPT_ADDR(device), playerId);

    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpController::SetBrowsedPlayer(const BluetoothRemoteDevice &device, uint16_t playerId)
{
    HILOGI("enter, device: %{public}s, playerId: %{public}d", GET_ENCRYPT_ADDR(device), playerId);

    int result = RET_BAD_STATUS;
    return result;
}

int AvrcpController::ChangePath(
    const BluetoothRemoteDevice &device, uint16_t uidCounter, uint16_t direction, uint64_t folderUid)
{
    HILOGI("enter, device: %{public}s, uidCounter: %{public}d, direction: %{public}d",
        GET_ENCRYPT_ADDR(device), uidCounter, direction);

    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpController::GetItemAttributes(
    const BluetoothRemoteDevice &device, uint64_t uid, uint16_t uidCounter, const std::vector<uint32_t> &attributes)
{
    HILOGI("enter, device: %{public}s, uidCounter: %{public}d", GET_ENCRYPT_ADDR(device), uidCounter);

    int result = RET_BAD_STATUS;
    return result;
}

int AvrcpController::RequestContinuingResponse(const BluetoothRemoteDevice &device, uint8_t pduId)
{
    HILOGI("enter, device: %{public}s, pduId: %{public}d", GET_ENCRYPT_ADDR(device), pduId);

    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpController::AbortContinuingResponse(const BluetoothRemoteDevice &device, uint8_t pduId)
{
    HILOGI("enter, device: %{public}s, pduId: %{public}d", GET_ENCRYPT_ADDR(device), pduId);

    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpController::AddToNowPlaying(const BluetoothRemoteDevice &device, uint64_t uid, uint16_t uidCounter)
{
    HILOGI("enter, device: %{public}s, uidCounter: %{public}d", GET_ENCRYPT_ADDR(device), uidCounter);

    int result = RET_BAD_STATUS;

    return result;
}

AvrcpController::AvrcpController(void)
{
    HILOGI("enter");

    pimpl = std::make_unique<AvrcpController::impl>();
}

AvrcpController::~AvrcpController(void)
{
    return;
}

}  // namespace Bluetooth
}  // namespace OHOS
