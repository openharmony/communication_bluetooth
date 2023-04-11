/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include <list>
#include <memory>
#include <mutex>

#include "bluetooth_def.h"
#include "i_bluetooth_avrcp_tg.h"
#include "bluetooth_avrcp_tg_observer_stub.h"
#include "bluetooth_host.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "iservice_registry.h"
#include "raw_address.h"
#include "system_ability_definition.h"
#include "bluetooth_avrcp_tg.h"

namespace OHOS {
namespace Bluetooth {
struct AvrcpTarget::impl {
public:
    impl()
    {
        return;
    }

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

    std::mutex observerMutex_;
    sptr<IBluetoothAvrcpTg> proxy_;
    BluetoothObserverList<AvrcpTarget::IObserver> observers_;
};

AvrcpTarget *AvrcpTarget::GetProfile(void)
{
    HILOGI("enter");
    return nullptr;
}

/******************************************************************
 * REGISTER / UNREGISTER OBSERVER                                 *
 ******************************************************************/

void AvrcpTarget::RegisterObserver(AvrcpTarget::IObserver *observer)
{
    return;
}

void AvrcpTarget::UnregisterObserver(AvrcpTarget::IObserver *observer)
{
    return;
}

/******************************************************************
 * CONNECTION                                                     *
 ******************************************************************/

void AvrcpTarget::SetActiveDevice(const BluetoothRemoteDevice &device)
{
    return;
}

std::vector<BluetoothRemoteDevice> AvrcpTarget::GetConnectedDevices(void)
{
    HILOGI("enter");

    std::vector<BluetoothRemoteDevice> devices;
    return devices;
}

std::vector<BluetoothRemoteDevice> AvrcpTarget::GetDevicesByStates(std::vector<int> states)
{
    HILOGI("enter");

    std::vector<BluetoothRemoteDevice> devices;
    return devices;
}

int AvrcpTarget::GetDeviceState(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    int32_t result = static_cast<int32_t>(BTConnectState::DISCONNECTED);
    return static_cast<int>(result);
}

bool AvrcpTarget::Connect(const BluetoothRemoteDevice &device)
{
    return false;
}

bool AvrcpTarget::Disconnect(const BluetoothRemoteDevice &device)
{
    return false;
}

/******************************************************************
 * NOTIFICATION                                                   *
 ******************************************************************/

void AvrcpTarget::NotifyPlaybackStatusChanged(uint8_t playStatus, uint32_t playbackPos)
{
    return;
}

void AvrcpTarget::NotifyTrackChanged(uint64_t uid, uint32_t playbackPos)
{
    return;
}

void AvrcpTarget::NotifyTrackReachedEnd(uint32_t playbackPos)
{
    return;
}

void AvrcpTarget::NotifyTrackReachedStart(uint32_t playbackPos)
{
    return;
}

void AvrcpTarget::NotifyPlaybackPosChanged(uint32_t playbackPos)
{
    return;
}

void AvrcpTarget::NotifyPlayerAppSettingChanged(
    const std::vector<uint8_t> &attributes, const std::vector<uint8_t> &values)
{
    return;
}

void AvrcpTarget::NotifyNowPlayingContentChanged(void)
{
    return;
}

void AvrcpTarget::NotifyAvailablePlayersChanged(void)
{
    return;
}

void AvrcpTarget::NotifyAddressedPlayerChanged(uint16_t playerId, uint16_t uidCounter)
{
    return;
}

void AvrcpTarget::NotifyUidChanged(uint16_t uidCounter)
{
    return;
}

void AvrcpTarget::NotifyVolumeChanged(uint8_t volume)
{
    return;
}

AvrcpTarget::AvrcpTarget(void)
{
    HILOGI("enter");

    pimpl = std::make_unique<impl>();
}

AvrcpTarget::~AvrcpTarget(void)
{
    return;
}
} // namespace Bluetooth
} // namespace OHOS
