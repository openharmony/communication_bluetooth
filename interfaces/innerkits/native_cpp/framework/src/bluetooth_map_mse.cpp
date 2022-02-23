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
#include "bluetooth_map_mse_proxy.h"
#include "bluetooth_raw_address.h"
#include "bluetooth_map_mse.h"

#include "bt_def.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_observer_list.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "i_bluetooth_host.h"

namespace OHOS {
namespace Bluetooth {
class MseServiceObserver : public BluetoothMapMseObserverStub {
public:
    MseServiceObserver(BluetoothObserverList<MapServerObserver> *observers) : observers_(observers)
    {
        HILOGI("%{public}s", __func__);
    }

    ~MseServiceObserver() override = default;

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), bluetooth::BTTransport::ADAPTER_BREDR);
        observers_->ForEach([remoteDevice, state](std::shared_ptr<MapServerObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state);
        });
    }

    void OnPermission(const BluetoothRawAddress &device) override
    {
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), bluetooth::BTTransport::ADAPTER_BREDR);
        observers_->ForEach([remoteDevice](std::shared_ptr<MapServerObserver> observer) {
            observer->OnPermission(remoteDevice);
        });
    }

private:
    BluetoothObserverList<MapServerObserver> *observers_;
};

struct MapServer::impl {
    impl();
    ~impl()
    {
        if (proxy_ != nullptr) {
            proxy_->DeregisterObserver(serviceObserver_);
            proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        }
    }

    std::mutex mutex_;
    sptr<IBluetoothMapMse> proxy_;
    class BluetoothMapMseDeathRecipient;
    sptr<BluetoothMapMseDeathRecipient> deathRecipient_ = nullptr;
    BluetoothObserverList<MapServerObserver> observers_;
    sptr<MseServiceObserver> serviceObserver_ = new MseServiceObserver(&observers_);
};

class MapServer::impl::BluetoothMapMseDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    BluetoothMapMseDeathRecipient(MapServer::impl &MapMse) : MapMse_(MapMse) {};
    ~BluetoothMapMseDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothMapMseDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("MapServer::impl::BluetoothMapMseDeathRecipient::OnRemoteDied starts");
        MapMse_.proxy_->AsObject()->RemoveDeathRecipient(MapMse_.deathRecipient_);
        MapMse_.proxy_ = nullptr;
    }

private:
    MapServer::impl &MapMse_;
};

MapServer::impl::impl()
{
    HILOGI("MapServer::impl::impl starts");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        HILOGE("MapServer::impl:impl() failed: no hostRemote");
        return;
    }
    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_MAP_MSE);

    if (!remote) {
        HILOGE("MapServer::impl:impl() failed: no remote");
        return;
    }
    HILOGI("MapServer::impl:impl() remote obtained");

    proxy_ = iface_cast<IBluetoothMapMse>(remote);

    deathRecipient_ = new BluetoothMapMseDeathRecipient(*this);
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
    if (proxy_ != nullptr) {
        proxy_->RegisterObserver(serviceObserver_);
    }
}

MapServer::MapServer() : pimpl(nullptr)
{
    HILOGI("%{public}s :excute", __func__);
    pimpl = std::make_unique<impl>();
}

MapServer::~MapServer()
{}

MapServer *MapServer::GetProfile()
{
    static MapServer instance;
    return &instance;
}

void MapServer::RegisterObserver(MapServerObserver &observer)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::shared_ptr<MapServerObserver> pointer(&observer, [](MapServerObserver *) {});
    pimpl->observers_.Register(pointer);
}

void MapServer::DeregisterObserver(MapServerObserver &observer)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::shared_ptr<MapServerObserver> pointer(&observer, [](MapServerObserver *) {});
    pimpl->observers_.Deregister(pointer);
}

int MapServer::GetState() const
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    int32_t ret = RET_NO_ERROR;
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        pimpl->proxy_->GetState(ret);
    }
    return ret;
}

bool MapServer::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("%{public}s :BluetoothRemoteDevice erro", __func__);
        return false;
    }
    int32_t ret = RET_NO_ERROR;
    BluetoothRawAddress rawAddress(device.GetDeviceAddr());
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        pimpl->proxy_->Disconnect(rawAddress, ret);
    }
    return ret == RET_NO_ERROR;
}

bool MapServer::IsConnected(const BluetoothRemoteDevice &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("%{public}s :BluetoothRemoteDevice erro", __func__);
        return false;
    }
    bool ret = false;
    BluetoothRawAddress rawAddress(device.GetDeviceAddr());
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        pimpl->proxy_->IsConnected(rawAddress, ret);
    }
    return ret;
}

std::vector<BluetoothRemoteDevice> MapServer::GetConnectedDevices() const
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::vector<BluetoothRemoteDevice> btDeviceList;
    std::vector<BluetoothRawAddress> btDevice;
    if ((pimpl->proxy_ != nullptr) && IS_BT_ENABLED()) {
        pimpl->proxy_->GetConnectedDevices(btDevice);
        for (auto it = btDevice.begin(); it != btDevice.end(); it++) {
            btDeviceList.push_back(BluetoothRemoteDevice(it->GetAddress(), 0));
        }
    }
    return btDeviceList;
}

std::vector<BluetoothRemoteDevice> MapServer::GetDevicesByStates(std::vector<int> states) const
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::vector<BluetoothRemoteDevice> btDeviceList;
    std::vector<BluetoothRawAddress> btDevice;
    if ((pimpl->proxy_ != nullptr) && IS_BT_ENABLED()) {
        pimpl->proxy_->GetDevicesByStates(states, btDevice);
        for (auto it = btDevice.begin(); it != btDevice.end(); it++) {
            btDeviceList.push_back(BluetoothRemoteDevice(it->GetAddress(), 0));
        }
    }
    return btDeviceList;
}

int MapServer::GetConnectionState(const BluetoothRemoteDevice &device) const
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("%{public}s :BluetoothRemoteDevice erro", __func__);
        return (int)BTConnectState::DISCONNECTED;
    }
    int32_t ret = RET_NO_ERROR;
    BluetoothRawAddress rawAddress(device.GetDeviceAddr());
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        pimpl->proxy_->GetConnectionState(rawAddress, ret);
    }
    return ret;
}

bool MapServer::SetConnectionStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("%{public}s :BluetoothRemoteDevice erro", __func__);
        return false;
    }
    bool ret = false;
    BluetoothRawAddress rawAddress(device.GetDeviceAddr());
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        pimpl->proxy_->SetConnectionStrategy(rawAddress, strategy, ret);
    }
    return ret;
}

int MapServer::GetConnectionStrategy(const BluetoothRemoteDevice &device) const
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("%{public}s :BluetoothRemoteDevice erro", __func__);
        return (int)BTStrategyType::CONNECTION_FORBIDDEN;
    }
    int32_t ret = (int)BTStrategyType::CONNECTION_FORBIDDEN;
    BluetoothRawAddress rawAddress(device.GetDeviceAddr());
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        pimpl->proxy_->GetConnectionStrategy(rawAddress, ret);
    }
    return ret;
}

void MapServer::GrantPermission(const BluetoothRemoteDevice &device, bool allow, bool save)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("%{public}s :BluetoothRemoteDevice erro", __func__);
        return;
    }
    BluetoothRawAddress rawAddress(device.GetDeviceAddr());
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        pimpl->proxy_->GrantPermission(rawAddress, allow, save);
    }
}
}  // namespace Bluetooth
}  // namespace OHOS