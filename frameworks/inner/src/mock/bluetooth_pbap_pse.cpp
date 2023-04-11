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
#include <mutex>
#include <string>
#include "bluetooth_pbap_pse_observer_stub.h"
#include "i_bluetooth_pbap_pse.h"
#include "bluetooth_pbap_server.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "iservice_registry.h"
#include "raw_address.h"
#include "system_ability_definition.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
struct PbapServer::impl {
    impl();

    ~impl()
    {}

    void RegisterObserver(std::shared_ptr<PbapObserver> &observer)
    {
        return;
    }

    void DeregisterObserver(std::shared_ptr<PbapObserver> &observer)
    {
        return;
    }

    int GetDeviceState(const BluetoothRemoteDevice &device)
    {
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }

    std::vector<BluetoothRemoteDevice> GetDevicesByStates(const std::vector<int> &states)
    {
        HILOGI("enter");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        return remoteDevices;
    }

    std::vector<BluetoothRemoteDevice> GetConnectedDevices()
    {
        HILOGI("enter");
        std::vector<int> states {static_cast<int>(BTConnectState::CONNECTED)};
        return GetDevicesByStates(states);
    }

    bool Disconnect(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool SetConnectionStrategy(const BluetoothRemoteDevice &device, int strategy)
    {
        return false;
    }

    int GetConnectionStrategy(const BluetoothRemoteDevice &device) const
    {
        return static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN);
    }

    void GrantPermission(const BluetoothRemoteDevice &device, bool allow, bool save)
    {
        return;
    }
    int SetDevicePassword(const BluetoothRemoteDevice &device, const std::string &password, std::string userId)
    {
        return (int)RET_NO_SUPPORT;
    }

private:
    std::mutex mutex_;
    BluetoothObserverList<PbapObserver> observers_;
};

PbapServer::impl::impl()
{
    return;
}

PbapServer *PbapServer::GetProfile()
{
    return nullptr;
}

PbapServer::PbapServer()
{
    pimpl = std::make_unique<impl>();
}

PbapServer::~PbapServer()
{}

void PbapServer::RegisterObserver(PbapObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<PbapObserver> pointer(observer, [](PbapObserver *) {});
    return pimpl->RegisterObserver(pointer);
}

void PbapServer::DeregisterObserver(PbapObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<PbapObserver> pointer(observer, [](PbapObserver *) {});
    return pimpl->DeregisterObserver(pointer);
}

int PbapServer::GetDeviceState(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->GetDeviceState(device);
}

std::vector<BluetoothRemoteDevice> PbapServer::GetDevicesByStates(const std::vector<int> &states)
{
    HILOGI("enter");
    return pimpl->GetDevicesByStates(states);
}

std::vector<BluetoothRemoteDevice> PbapServer::GetConnectedDevices()
{
    HILOGI("enter");
    return pimpl->GetConnectedDevices();
}

bool PbapServer::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->Disconnect(device);
}

bool PbapServer::SetConnectionStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("enter, device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    return pimpl->SetConnectionStrategy(device, strategy);
}

int PbapServer::GetConnectionStrategy(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->GetConnectionStrategy(device);
}

void PbapServer::GrantPermission(const BluetoothRemoteDevice &device, bool allow, bool save)
{
    HILOGI("enter, device: %{public}s, allow: %{public}d, save: %{public}d", GET_ENCRYPT_ADDR(device), allow, save);
    pimpl->GrantPermission(device, allow, save);
}

int PbapServer::SetDevicePassword(const BluetoothRemoteDevice &device, const std::string &password, std::string userId)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->SetDevicePassword(device, password, userId);
}
}  // namespace Bluetooth
}  // namespace OHOS