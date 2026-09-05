/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_fwk_pbap_pce"
#endif

#include <list>
#include <mutex>
#include <string>
#include "bluetooth_pbap_pce_observer_stub.h"
#include "bluetooth_pbap_pce_proxy.h"
#include "bluetooth_pbap_pce.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_host.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "iservice_registry.h"
#include "raw_address.h"
#include "system_ability_definition.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_log.h"
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {

class BluetoothPbapPceObserverImp : public BluetoothPbapPceObserverStub {
public:
    explicit BluetoothPbapPceObserverImp(BluetoothObserverList<PbapPceObserver> &observers)
        : observers_(observers)
    {}
    ~BluetoothPbapPceObserverImp() override
    {}

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {
        HILOGI("OnConnectionStateChanged, state: %{public}d, cause: %{public}d", state, cause);
        observers_.ForEach([device, state, cause](std::shared_ptr<PbapPceObserver> observer) {
            BluetoothRemoteDevice dev(device.GetAddress(), BTTransport::ADAPTER_BREDR);
            observer->OnConnectionStateChanged(dev, state, cause);
        });
    }

    void OnSyncStateChange(const BluetoothRawAddress &device, int32_t syncState) override
    {
        HILOGI("OnSyncStateChange, syncState: %{public}d", syncState);
        observers_.ForEach([device, syncState](std::shared_ptr<PbapPceObserver> observer) {
            BluetoothRemoteDevice dev(device.GetAddress(), BTTransport::ADAPTER_BREDR);
            observer->OnSyncStateChange(dev, syncState);
        });
    }

private:
    BluetoothObserverList<PbapPceObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothPbapPceObserverImp);
};

struct PbapPce::impl {
    impl();
    ~impl();
    void RegisterObserver(std::shared_ptr<PbapPceObserver> &observer);
    void DeregisterObserver(std::shared_ptr<PbapPceObserver> &observer);
    std::mutex pbapPceProxyMutex_;
    int32_t profileRegisterId = 0;
    // M-4: guard lambda access during destruction (DeregisterFunc does not join pending callbacks)
    std::atomic_bool destructing_{false};
private:
    BluetoothObserverList<PbapPceObserver> observers_;
    sptr<BluetoothPbapPceObserverImp> serviceObserverImp_ = nullptr;
};

PbapPce::impl::impl()
{
    serviceObserverImp_ = new BluetoothPbapPceObserverImp(observers_);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_PBAP_PCE,
        [this](sptr<IRemoteObject> remote) {
        // M-4: check destructing_ flag to prevent use-after-free during ~impl
        if (destructing_.load(std::memory_order_acquire)) {
            return;
        }
        sptr<IBluetoothPbapPce> proxy = iface_cast<IBluetoothPbapPce>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(serviceObserverImp_);
    });
}

PbapPce::impl::~impl()
{
    destructing_.store(true, std::memory_order_release);
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothPbapPce> proxy = GetRemoteProxy<IBluetoothPbapPce>(PROFILE_PBAP_PCE);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(serviceObserverImp_);
}

void PbapPce::impl::RegisterObserver(std::shared_ptr<PbapPceObserver> &observer)
{
    if (observer) {
        observers_.Register(observer);
    }
}

void PbapPce::impl::DeregisterObserver(std::shared_ptr<PbapPceObserver> &observer)
{
    if (observer) {
        observers_.Deregister(observer);
    }
}

PbapPce *PbapPce::GetProfile()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<PbapPce> instance;
    return instance.get();
#else
    static PbapPce instance;
    return &instance;
#endif
}

PbapPce::PbapPce()
{
    pimpl = std::make_unique<impl>();
}

PbapPce::~PbapPce()
{
}

void PbapPce::RegisterObserver(std::shared_ptr<PbapPceObserver> observer)
{
    pimpl->RegisterObserver(observer);
}

void PbapPce::DeregisterObserver(std::shared_ptr<PbapPceObserver> observer)
{
    pimpl->DeregisterObserver(observer);
}

int32_t PbapPce::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPce> proxy = GetRemoteProxy<IBluetoothPbapPce>(PROFILE_PBAP_PCE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
}

int32_t PbapPce::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRemoteDevice> &result) const
{
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPce> proxy = GetRemoteProxy<IBluetoothPbapPce>(PROFILE_PBAP_PCE);
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

int32_t PbapPce::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPce> proxy = GetRemoteProxy<IBluetoothPbapPce>(PROFILE_PBAP_PCE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->Connect(BluetoothRawAddress(device.GetDeviceAddr()));
}

int32_t PbapPce::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPce> proxy = GetRemoteProxy<IBluetoothPbapPce>(PROFILE_PBAP_PCE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
}

int32_t PbapPce::SetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPce> proxy = GetRemoteProxy<IBluetoothPbapPce>(PROFILE_PBAP_PCE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");
    CHECK_AND_RETURN_LOG_RET(CheckConnectionStrategyInvalid(strategy), BT_ERR_INVALID_PARAM, "strategy param error");

    return proxy->SetConnectionStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
}

int32_t PbapPce::GetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t &strategy) const
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPce> proxy = GetRemoteProxy<IBluetoothPbapPce>(PROFILE_PBAP_PCE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetConnectionStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
}

int32_t PbapPce::GetPhoneBookSyncState(const BluetoothRemoteDevice &device) const
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPce> proxy = GetRemoteProxy<IBluetoothPbapPce>(PROFILE_PBAP_PCE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetPhoneBookSyncState(BluetoothRawAddress(device.GetDeviceAddr()));
}

} // namespace Bluetooth
} // namespace OHOS