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
#include <mutex>
#include "bluetooth_map_mse_observer_stub.h"
#include "i_bluetooth_map_mse.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_map_mse.h"

#include "bt_def.h"
#include "bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_observer_list.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "i_bluetooth_host.h"

namespace OHOS {
namespace Bluetooth {

struct MapServer::impl {
    impl();
    ~impl()
    {
        return;
    }

    std::mutex mutex_;
    sptr<IBluetoothMapMse> proxy_;
    BluetoothObserverList<MapServerObserver> observers_;
};

MapServer::impl::impl()
{
    return;
}

MapServer::MapServer() : pimpl(nullptr)
{
    return;
}

MapServer::~MapServer()
{}

MapServer *MapServer::GetProfile()
{
    return nullptr;
}

void MapServer::RegisterObserver(MapServerObserver &observer)
{
    return;
}

void MapServer::DeregisterObserver(MapServerObserver &observer)
{
    return;
}

int MapServer::GetState() const
{
    return RET_NO_ERROR;
}

bool MapServer::Disconnect(const BluetoothRemoteDevice &device)
{
    return false;
}

bool MapServer::IsConnected(const BluetoothRemoteDevice &device)
{
    return false;
}

std::vector<BluetoothRemoteDevice> MapServer::GetConnectedDevices() const
{
    HILOGI("enter");
    std::vector<BluetoothRemoteDevice> btDeviceList;
    return btDeviceList;
}

std::vector<BluetoothRemoteDevice> MapServer::GetDevicesByStates(std::vector<int> states) const
{
    HILOGI("enter");
    std::vector<BluetoothRemoteDevice> btDeviceList;
    return btDeviceList;
}

int MapServer::GetConnectionState(const BluetoothRemoteDevice &device) const
{
    return (int)BTConnectState::DISCONNECTED;
}

bool MapServer::SetConnectionStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    return false;
}

int MapServer::GetConnectionStrategy(const BluetoothRemoteDevice &device) const
{
    return (int)BTStrategyType::CONNECTION_FORBIDDEN;
}

void MapServer::GrantPermission(const BluetoothRemoteDevice &device, bool allow, bool save)
{
    return;
}
}  // namespace Bluetooth
}  // namespace OHOS