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
#include "bluetooth_pbap_pse_observer_stub.h"
#include "bluetooth_pbap_pse_proxy.h"
#include "bluetooth_pbap_pse.h"
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
enum class BTShareType : int32_t {
    SHARE_NAME_AND_PHONE_NUMBER = 0,
    SHARE_ALL = 1,
    SHARE_NOTHING = 2,
};

class BluetoothPbapPseObserverImp : public BluetoothPbapPseObserverStub {
public:
    explicit BluetoothPbapPseObserverImp(BluetoothObserverList<PbapPseObserver> &observers)
        : observers_(observers)
    {}
    ~BluetoothPbapPseObserverImp() override
    {}

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {
        observers_.ForEach([device, state, cause](std::shared_ptr<PbapPseObserver> observer) {
            BluetoothRemoteDevice dev(device.GetAddress(), 0);
            observer->OnConnectionStateChanged(dev, state, cause);
        });
    }

private:
    BluetoothObserverList<PbapPseObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothPbapPseObserverImp);
};

struct PbapPse::impl {
    impl();
    ~impl();
    void RegisterObserver(std::shared_ptr<PbapPseObserver> &observer);
    void DeregisterObserver(std::shared_ptr<PbapPseObserver> &observer);
    std::mutex pbapPseProxyMutex_;
    int32_t profileRegisterId = 0;
private:
    BluetoothObserverList<PbapPseObserver> observers_;
    sptr<BluetoothPbapPseObserverImp> serviceObserverImp_ = nullptr;
};

PbapPse::impl::impl()
{
    serviceObserverImp_ = new BluetoothPbapPseObserverImp(observers_);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_PBAP_PSE,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothPbapPse> proxy = iface_cast<IBluetoothPbapPse>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(serviceObserverImp_);
    });
}

PbapPse::impl::~impl()
{
    HILOGI("enter");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(serviceObserverImp_);
}

void PbapPse::impl::RegisterObserver(std::shared_ptr<PbapPseObserver> &observer)
{
    HILOGI("enter");
    if (observer) {
        observers_.Register(observer);
    }
}

void PbapPse::impl::DeregisterObserver(std::shared_ptr<PbapPseObserver> &observer)
{
    HILOGI("enter");
    if (observer) {
        observers_.Deregister(observer);
    }
}

PbapPse *PbapPse::GetProfile()
{
    static PbapPse instance;
    return &instance;
}

PbapPse::PbapPse()
{
    pimpl = std::make_unique<impl>();
}

PbapPse::~PbapPse()
{
    HILOGI("enter");
}

void PbapPse::RegisterObserver(std::shared_ptr<PbapPseObserver> observer)
{
    HILOGI("enter");
    pimpl->RegisterObserver(observer);
}

void PbapPse::DeregisterObserver(std::shared_ptr<PbapPseObserver> observer)
{
    HILOGI("enter");
    pimpl->DeregisterObserver(observer);
}

int32_t PbapPse::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
}

int32_t PbapPse::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRemoteDevice> &result) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
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

int32_t PbapPse::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
}

int32_t PbapPse::SetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");
    CHECK_AND_RETURN_LOG_RET(CheckConnectionStrategyInvalid(strategy), BT_ERR_INVALID_PARAM, "strategy param error");

    return proxy->SetConnectionStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
}

int32_t PbapPse::GetConnectionStrategy(const BluetoothRemoteDevice &device, int32_t &strategy) const
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetConnectionStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
}

bool CheckShareTypeInvalid(int32_t shareType)
{
    if (shareType == static_cast<int32_t>(BTShareType::SHARE_NAME_AND_PHONE_NUMBER) ||
        shareType == static_cast<int32_t>(BTShareType::SHARE_ALL) ||
        shareType == static_cast<int32_t>(BTShareType::SHARE_NOTHING)) {
        return true;
    }
    return false;
}

int32_t PbapPse::SetShareType(const BluetoothRemoteDevice &device, int32_t shareType)
{
    HILOGI("device: %{public}s, shareType: %{public}d", GET_ENCRYPT_ADDR(device), shareType);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");
    CHECK_AND_RETURN_LOG_RET(CheckShareTypeInvalid(shareType), BT_ERR_INVALID_PARAM, "shareType param error");

    return proxy->SetShareType(BluetoothRawAddress(device.GetDeviceAddr()), shareType);
}

int32_t PbapPse::GetShareType(const BluetoothRemoteDevice &device, int32_t &shareType) const
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetShareType(BluetoothRawAddress(device.GetDeviceAddr()), shareType);
}

int32_t PbapPse::SetPhoneBookAccessAuthorization(const BluetoothRemoteDevice &device, int32_t accessAuthorization)
{
    HILOGI("device: %{public}s, accessAuthorization: %{public}d",
        GET_ENCRYPT_ADDR(device), accessAuthorization);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");
    CHECK_AND_RETURN_LOG_RET(CheckAccessAuthorizationInvalid(accessAuthorization),
        BT_ERR_INVALID_PARAM, "accessAuthorization param error");

    return proxy->SetPhoneBookAccessAuthorization(BluetoothRawAddress(device.GetDeviceAddr()),
        accessAuthorization);
}

int32_t PbapPse::GetPhoneBookAccessAuthorization(const BluetoothRemoteDevice &device,
    int32_t &accessAuthorization) const
{
    HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off");
    sptr<IBluetoothPbapPse> proxy = GetRemoteProxy<IBluetoothPbapPse>(PROFILE_PBAP_PSE);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "proxy is nullptr");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device param error");

    return proxy->GetPhoneBookAccessAuthorization(BluetoothRawAddress(device.GetDeviceAddr()),
        accessAuthorization);
}

} // namespace Bluetooth
} // namespace OHOS