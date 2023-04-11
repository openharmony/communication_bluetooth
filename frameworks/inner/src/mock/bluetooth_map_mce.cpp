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
#include <codecvt>
#include <string>
#include <mutex>
#include <memory>

#include "bluetooth_map_mce_observer_stub.h"
#include "i_bluetooth_map_mce.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_map_mce.h"

#include "bt_def.h"
#include "bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_observer_list.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "i_bluetooth_host.h"
using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
/**
 * @brief make observer for framework
 */

struct MapClient::impl {
    impl();
    ~impl()
    {
        return;
    }
    sptr<IBluetoothMapMce> proxy_;
    std::mutex mutex_;
    BluetoothObserverList<MapClientObserver> mapClientObserverList_;
};

MapClient::impl::impl()
{
    return;
}

MapClient *MapClient::GetProfile()
{
    HILOGI("enter");
    return nullptr;
}

MapClient::MapClient() : pimpl(nullptr)
{
    HILOGI("excute");
    pimpl = std::make_unique<impl>();
}

MapClient::~MapClient()
{
}

void MapClient::RegisterObserver(MapClientObserver &observer)
{
    return;
}

void MapClient::DeregisterObserver(MapClientObserver &observer)
{
    return;
}

bool MapClient::Connect(const BluetoothRemoteDevice &device)
{
    return false;
}

bool MapClient::Disconnect(const BluetoothRemoteDevice &device)
{
    return false;
}

bool MapClient::IsConnected(const BluetoothRemoteDevice &device)
{
    return false;
}

std::vector<BluetoothRemoteDevice> MapClient::GetConnectedDevices() const
{
    HILOGI("enter");
    std::vector<BluetoothRemoteDevice> btDeviceList;
    return btDeviceList;
}

std::vector<BluetoothRemoteDevice> MapClient::GetDevicesByStates(const std::vector<int> &statesList) const
{
    HILOGI("enter");
    std::vector<BluetoothRemoteDevice> btDeviceList;
    return btDeviceList;
}

int MapClient::GetConnectionState(const BluetoothRemoteDevice &device) const
{
    return (int)BTConnectState::DISCONNECTED;
}

bool MapClient::SetConnectionStrategy(const BluetoothRemoteDevice &device, const int strategy)
{
    return false;
}

int MapClient::GetConnectionStrategy(const BluetoothRemoteDevice &device) const
{
    return (int)BTStrategyType::CONNECTION_FORBIDDEN;
}

int MapClient::GetUnreadMessages(const BluetoothRemoteDevice &device, MapMessageType type, uint8_t max)
{
    return RET_BAD_STATUS;
}

int MapClient::GetSupportedFeatures(const BluetoothRemoteDevice &device) const
{
    return RET_BAD_STATUS;
}

int MapClient::SendMessage(const BluetoothRemoteDevice &device, const MapSendMessageParameters &msg)
{
    return RET_BAD_STATUS;
}

int MapClient::SetNotificationFilter(const BluetoothRemoteDevice &device, const int mask)
{
    return RET_BAD_PARAM;
}

int MapClient::GetMessagesListing(const BluetoothRemoteDevice &device, const GetMessagesListingParameters &para) const
{
    return RET_BAD_PARAM;
}

int MapClient::GetMessage(const BluetoothRemoteDevice &device, MapMessageType type, const std::u16string &msgHandle,
    const GetMessageParameters &para) const
{
    return RET_BAD_PARAM;
}

int MapClient::UpdateInbox(const BluetoothRemoteDevice &device, MapMessageType type)
{
    return RET_BAD_PARAM;
}

int MapClient::GetConversationListing(
    const BluetoothRemoteDevice &device, const GetConversationListingParameters &para) const
{
    return RET_BAD_PARAM;
}

int MapClient::SetMessageStatus(
    const BluetoothRemoteDevice &device, MapMessageType type, const MapSetMessageStatus &msgStatus)
{
    return RET_BAD_PARAM;
}

int MapClient::SetOwnerStatus(const BluetoothRemoteDevice &device, const SetOwnerStatusParameters &para)
{
    return RET_BAD_PARAM;
}

int MapClient::GetOwnerStatus(const BluetoothRemoteDevice &device, const std::string &conversationId) const
{
    return RET_BAD_PARAM;
}

MapMasInstanceInfoList MapClient::GetMasInstanceInfo(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    MapMasInstanceInfoList frameworkMasInfoList;
    return frameworkMasInfoList;
}
}  // namespace Bluetooth
}  // namespace OHOS
