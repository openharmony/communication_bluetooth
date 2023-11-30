/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#include "bluetooth_load_system_ability.h"
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

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        HILOGI("enter, device: %{public}s, state: %{public}s",
            GetEncryptAddr((device).GetAddress()).c_str(), GetProfileConnStateName(state).c_str());
        observers_.ForEach([device, state](std::shared_ptr<MapMseObserver> observer) {
            BluetoothRemoteDevice dev(device.GetAddress(), 0);
            observer->OnConnectionStateChanged(dev, state);
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
    bool InitMapMseProxy();

    sptr<IBluetoothMapMse> proxy_ = nullptr;
    std::mutex mapMseProxyMutex;
private:
    BluetoothObserverList<MapMseObserver> observers_;
    sptr<BluetoothMapMseObserverImp> serviceObserverImp_ = nullptr;
};

MapMse::impl::impl()
{
    if (proxy_) {
        return;
    }
    BluetootLoadSystemAbility::GetInstance()->RegisterNotifyMsg(PROFILE_ID_MAP_MSE);
    if (!BluetootLoadSystemAbility::GetInstance()->HasSubscribedBluetoothSystemAbility()) {
        BluetootLoadSystemAbility::GetInstance()->SubScribeBluetoothSystemAbility();
        return;
    }
    InitMapMseProxy();
};

MapMse::impl::~impl()
{
    HILOGD("enter");
    if (proxy_ != nullptr) {
        proxy_->DeregisterObserver(serviceObserverImp_);
    }
}

bool MapMse::impl::InitMapMseProxy(void)
{
    HILOGD("enter");
    std::lock_guard<std::mutex> lock(mapMseProxyMutex);\
    CHECK_AND_RETURN_LOG_RET((proxy_ == nullptr), true, "proxy exist");

    proxy_ = GetRemoteProxy<IBluetoothMapMse>(PROFILE_MAP_MSE);
    CHECK_AND_RETURN_LOG_RET((proxy_ != nullptr), false, "get proxy failed");

    serviceObserverImp_ = new (std::nothrow) BluetoothMapMseObserverImp(observers_);
    if (serviceObserverImp_ != nullptr) {
        proxy_->RegisterObserver(serviceObserverImp_);
    }
    return true;
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

void MapMse::Init()
{
    HILOGI("MapMse init enter");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    CHECK_AND_RETURN_LOG(pimpl->InitMapMseProxy(), "init fail");
}

void MapMse::Uinit()
{
    HILOGI("MapMse Uinit enter");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    std::lock_guard<std::mutex> lock(pimpl->mapMseProxyMutex);
    pimpl->proxy_ = nullptr;
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
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->proxy_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return pimpl->proxy_->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
}

int32_t MapMse::GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRemoteDevice> &result) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->proxy_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or proxy is nullptr");

    std::vector<BluetoothRawAddress> rawAddress {};
    int32_t ret = pimpl->proxy_->GetDevicesByStates(states, rawAddress);
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
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->proxy_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return pimpl->proxy_->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
}

int32_t MapMse::SetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->proxy_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");
    CHECK_AND_RETURN_LOG_RET(CheckConnectionStrategyInvalid(strategy), BT_ERR_INVALID_PARAM, "strategy param error");

    return pimpl->proxy_->SetConnectionStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
}

int32_t MapMse::GetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t &strategy) const
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->proxy_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return pimpl->proxy_->GetConnectionStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
}

int32_t MapMse::SetMessageAccessAuthorization(const BluetoothRemoteDevice &device, int32_t accessAuthorization)
{
    HILOGI("device: %{public}s, accessAuthorization: %{public}d",
        GET_ENCRYPT_ADDR(device), accessAuthorization);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->proxy_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");
    CHECK_AND_RETURN_LOG_RET(CheckAccessAuthorizationInvalid(accessAuthorization),
        BT_ERR_INVALID_PARAM, "metaData param error");

    return pimpl->proxy_->SetMessageAccessAuthorization(BluetoothRawAddress(device.GetDeviceAddr()), accessAuthorization);
}

int32_t MapMse::GetMessageAccessAuthorization(const BluetoothRemoteDevice &device, int32_t &accessAuthorization) const
{
    HILOGI("device: %{public}s, metaDataType: %{public}d", GET_ENCRYPT_ADDR(device), dataValue);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->proxy_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return pimpl->proxy_->GetMessageAccessAuthorization(BluetoothRawAddress(device.GetDeviceAddr()), accessAuthorization);
}

} // namespace Bluetooth
} // namespace OHOS