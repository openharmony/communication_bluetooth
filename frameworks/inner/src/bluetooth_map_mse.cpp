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

#include <list>
#include <mutex>
#include <string>
#include "bluetooth_map_mse_observer_stub.h"
#include "bluetooth_map_mse_proxy.h"
#include "bluetooth_map_mse.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "iservice_registry.h"
#include "raw_address.h"
#include "system_ability_definition.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothMapMseObserverImp : public BluetoothMapMseObserverStub {
public:
    explicit BluetoothMapMseObserverImp(BluetoothObserverList<MapMseObserver> &observers)
        : observers_(observers)
    {}
    ~BluetoothMapMseObserverImp() override
    {}

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {
        HILOGI("enter, device: %{public}s, state: %{public}s, cause: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), GetProfileConnStateName(state).c_str(), cause);
        observers_.ForEach([device, state, cause](std::shared_ptr<MapMseObserver> observer) {
            BluetoothRemoteDevice dev(device.GetAddress(), 0);
            observer->OnConnectionStateChanged(dev, state, cause);
        });
    }

private:
    BluetoothObserverList<MapMseObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothMapMseObserverImp);
};

struct MapMse::impl {
    impl();
    ~impl();
    void RegisterObserver(std::shared_ptr<MapMseObserver> &observer);
    void DeregisterObserver(std::shared_ptr<MapMseObserver> &observer);
    int32_t profileRegisterId = 0;
private:
    BluetoothObserverList<MapMseObserver> observers_;
    sptr<BluetoothMapMseObserverImp> serviceObserverImp_ = nullptr;
};

MapMse::impl::impl()
{
    serviceObserverImp_ = new (std::nothrow) BluetoothMapMseObserverImp(observers_);
    CHECK_AND_RETURN_LOG(serviceObserverImp_ != nullptr, "serviceObserverImp_ is nullptr");
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_MAP_MSE,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothMapMse> proxy = iface_cast<IBluetoothMapMse>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(serviceObserverImp_);
    });
};

MapMse::impl::~impl()
{
    HILOGD("enter");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothMapMse> proxy = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(serviceObserverImp_);
}

void MapMse::impl::RegisterObserver(std::shared_ptr<MapMseObserver> &observer)
{
    HILOGI("enter");
    if (observer) {
        observers_.Register(observer);
    }
}

void MapMse::impl::DeregisterObserver(std::shared_ptr<MapMseObserver> &observer)
{
    HILOGI("enter");
    if (observer) {
        observers_.Deregister(observer);
    }
}

MapMse *MapMse::GetProfile()
{
    static MapMse instance;
    return &instance;
}

MapMse::MapMse()
{
    pimpl = std::make_unique<impl>();
}

MapMse::~MapMse()
{
    HILOGI("enter");
}

void MapMse::RegisterObserver(std::shared_ptr<MapMseObserver> observer)
{
    HILOGI("enter");
    pimpl->RegisterObserver(observer);
}

void MapMse::DeregisterObserver(std::shared_ptr<MapMseObserver> observer)
{
    HILOGI("enter");
    pimpl->DeregisterObserver(observer);
}

int32_t MapMse::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothMapMse> proxy = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
}

int32_t MapMse::GetDevicesByStates(const std::vector<int32_t> &states, std::vector<BluetoothRemoteDevice> &result) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothMapMse> proxy = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");

    std::vector<BluetoothRawAddress> rawAddress {};
    int32_t ret = proxy->GetDevicesByStates(states, rawAddress);
    CHECK_AND_RETURN_LOG_RET((ret == BT_NO_ERROR), ret, "inner error");

    for (BluetoothRawAddress rawAddr : rawAddress) {
        BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
        result.push_back(device);
    }
    return BT_NO_ERROR;
}

int32_t MapMse::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothMapMse> proxy = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
}

int32_t MapMse::SetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothMapMse> proxy = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");
    CHECK_AND_RETURN_LOG_RET(CheckConnectionStrategyInvalid(strategy), BT_ERR_INVALID_PARAM, "strategy param error");

    return proxy->SetConnectionStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
}

int32_t MapMse::GetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t &strategy) const
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothMapMse> proxy = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetConnectionStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
}

int32_t MapMse::SetMessageAccessAuthorization(const BluetoothRemoteDevice &device, int32_t accessAuthorization)
{
    HILOGI("device: %{public}s, accessAuthorization: %{public}d", GET_ENCRYPT_ADDR(device), accessAuthorization);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothMapMse> proxy = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");
    CHECK_AND_RETURN_LOG_RET(CheckAccessAuthorizationInvalid(accessAuthorization),
        BT_ERR_INVALID_PARAM, "metaData param error");

    return proxy->SetMessageAccessAuthorization(
        BluetoothRawAddress(device.GetDeviceAddr()), accessAuthorization);
}

int32_t MapMse::GetMessageAccessAuthorization(const BluetoothRemoteDevice &device, int32_t &accessAuthorization) const
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothMapMse> proxy = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetMessageAccessAuthorization(
        BluetoothRawAddress(device.GetDeviceAddr()), accessAuthorization);
}

} // namespace Bluetooth
} // namespace OHOS