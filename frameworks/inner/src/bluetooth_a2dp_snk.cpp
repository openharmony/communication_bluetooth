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

#include "bluetooth_a2dp_snk.h"
#include "bluetooth_a2dp_sink_proxy.h"
#include "bluetooth_a2dp_sink_observer_stub.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_observer_list.h"
#include "raw_address.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"

#include "bluetooth_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;

struct A2dpSink::impl {
    impl();
    ~impl();

    BluetoothObserverList<A2dpSinkObserver> observers_;
    sptr<IBluetoothA2dpSink> proxy_ = nullptr;
    class BluetoothA2dpSinkObserverImp;
    sptr<BluetoothA2dpSinkObserverImp> observerImp_ = nullptr;
    class BluetoothA2dpSinkDeathRecipient;
    sptr<BluetoothA2dpSinkDeathRecipient> deathRecipient_ = nullptr;

private:
    void GetProxy();
};

class A2dpSink::impl::BluetoothA2dpSinkObserverImp : public BluetoothA2dpSinkObserverStub {
public:
    BluetoothA2dpSinkObserverImp(A2dpSink::impl &a2dpSink) : a2dpSink_(a2dpSink)
    {};
    ~BluetoothA2dpSinkObserverImp() override
    {};

    void Register(std::shared_ptr<A2dpSinkObserver> &observer)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        a2dpSink_.observers_.Register(observer);
    }

    void Deregister(std::shared_ptr<A2dpSinkObserver> &observer)
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        a2dpSink_.observers_.Deregister(observer);
    }

    void OnConnectionStateChanged(const RawAddress &device, int state) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        a2dpSink_.observers_.ForEach([device, state](std::shared_ptr<A2dpSinkObserver> observer) {
            observer->OnConnectionStateChanged(BluetoothRemoteDevice(device.GetAddress(), 0), state);
        });
    }

private:
    A2dpSink::impl &a2dpSink_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSinkObserverImp);
};

class A2dpSink::impl::BluetoothA2dpSinkDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    BluetoothA2dpSinkDeathRecipient(A2dpSink::impl &a2dpSinkDeath) : a2dpSinkDeath_(a2dpSinkDeath)
    {};
    ~BluetoothA2dpSinkDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSinkDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("A2dpSink::impl::BluetoothA2dpSinkDeathRecipient::OnRemoteDied starts");
        a2dpSinkDeath_.proxy_->AsObject()->RemoveDeathRecipient(a2dpSinkDeath_.deathRecipient_);
        a2dpSinkDeath_.proxy_ = nullptr;
    }

private:
    A2dpSink::impl &a2dpSinkDeath_;
};

A2dpSink::impl::impl()
{
    HILOGI("A2dpSink::impl::impl start");
    GetProxy();
    if (proxy_ == nullptr) {
        HILOGI("A2dpSink::get proxy_ failed");
        return;
    }

    deathRecipient_ = new BluetoothA2dpSinkDeathRecipient(*this);
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);

    observerImp_ = new (std::nothrow) BluetoothA2dpSinkObserverImp(*this);
    if (observerImp_ == nullptr) {
        HILOGI("A2dpSink::get observerImp_ failed");
        return;
    }
    proxy_->RegisterObserver(observerImp_);
};

A2dpSink::impl::~impl()
{
    HILOGD("A2dpSink::impl::~impl  start");
    if (proxy_ != nullptr) {
        proxy_->DeregisterObserver(observerImp_);
        proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
}

void A2dpSink::impl::GetProxy()
{
    HILOGI("A2dpSink::impl::GetProxy start");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        HILOGE("A2dpSink::impl::GetProxy error: no samgr");
        return;
    }

    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
    if (!hostRemote) {
        HILOGE("A2dpSink::impl:GetProxy failed: no hostRemote");
        return;
    }

    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    if (!hostProxy) {
        HILOGE("A2dpSink::impl::GetProxy error: host no proxy");
        return;
    }

    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_A2DP_SINK);
    if (!remote) {
        HILOGE("A2dpSink::impl::GetProxy error: no remote");
        return;
    }

    proxy_ = iface_cast<IBluetoothA2dpSink>(remote);
    if (!proxy_) {
        HILOGE("A2dpSink::impl::GetProxy error: no proxy");
        return;
    }
}

A2dpSink::A2dpSink()
{
    pimpl = std::make_unique<impl>();
    if (!pimpl) {
        HILOGE("A2dpSink::A2dpSink fails: no pimpl");
    }
}

A2dpSink::~A2dpSink()
{
    HILOGI("A2dpSink::~A2dpSink start");
}

void A2dpSink::RegisterObserver(A2dpSinkObserver *observer)
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);
    std::shared_ptr<A2dpSinkObserver> pointer(observer, [](A2dpSinkObserver *) {});
    pimpl->observers_.Register(pointer);
}

void A2dpSink::DeregisterObserver(A2dpSinkObserver *observer)
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);
    std::shared_ptr<A2dpSinkObserver> pointer(observer, [](A2dpSinkObserver *) {});
    pimpl->observers_.Deregister(pointer);
}

int A2dpSink::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("[A2dpSink] input parameter error.");
        return RET_BAD_STATUS;
    }

    int ret = RET_BAD_STATUS;
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        ret = pimpl->proxy_->GetDeviceState(RawAddress(device.GetDeviceAddr()));
    } else {
        HILOGI("[A2dpSink] proxy or bt disable.");
    }

    return ret;
}

std::vector<BluetoothRemoteDevice> A2dpSink::GetDevicesByStates(std::vector<int> states) const
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);
    std::vector<BluetoothRemoteDevice> devices;
    std::vector<BluetoothRawAddress> devicesRaw;

    for (int state : states) {
        if ((static_cast<int>(BTConnectState::CONNECTED) != state) &&
            (static_cast<int>(BTConnectState::CONNECTING) != state) &&
            (static_cast<int>(BTConnectState::DISCONNECTING) != state) &&
            (static_cast<int>(BTConnectState::DISCONNECTED) != state)) {
            HILOGI("[A2dpSink] input parameter error.");
            return devices;
        }
    }

    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        pimpl->proxy_->GetDevicesByStates(states);
    }

    for (RawAddress it : devicesRaw) {
        BluetoothRemoteDevice remoteDevice(it.GetAddress(), 0);
        devices.push_back(remoteDevice);
    }

    return devices;
}

int A2dpSink::GetPlayingState(const BluetoothRemoteDevice &device) const
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("[A2dpSink] input parameter error.");
        return RET_BAD_STATUS;
    }

    int ret = RET_BAD_STATUS;
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        ret = pimpl->proxy_->GetPlayingState(RawAddress(device.GetDeviceAddr()));
    } else {
        HILOGI("[A2dpSink] proxy or bt disable.");
    }

    return ret;
}

bool A2dpSink::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("[A2dpSink] input parameter error.");
        return false;
    }

    int ret = RET_NO_ERROR;
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        ret = pimpl->proxy_->Connect(RawAddress(device.GetDeviceAddr()));
    } else {
        HILOGI("[A2dpSink] proxy or bt disable.");
        return false;
    }
    return (RET_NO_ERROR == ret);
}

bool A2dpSink::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("[A2dpSink] input parameter error.");
        return false;
    }

    int ret = RET_NO_ERROR;
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        ret = pimpl->proxy_->Disconnect(RawAddress(device.GetDeviceAddr()));
    } else {
        HILOGI("[A2dpSink] proxy or bt disable.");
        return false;
    }
    return (RET_NO_ERROR == ret);
}

A2dpSink *A2dpSink::GetProfile()
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);
    static A2dpSink service;
    return &service;
}

bool A2dpSink::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("[A2dpSink] %s", __func__);

    if ((!device.IsValidBluetoothRemoteDevice()) || (((int)BTStrategyType::CONNECTION_ALLOWED != strategy) &&
                                                        ((int)BTStrategyType::CONNECTION_FORBIDDEN != strategy))) {
        HILOGI("[A2dpSink] input parameter error.");
        return false;
    }

    int ret = RET_NO_ERROR;
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        ret = pimpl->proxy_->SetConnectStrategy(RawAddress(device.GetDeviceAddr()), strategy);
    } else {
        HILOGI("[A2dpSink] proxy or bt disable.");
        return false;
    }

    return (RET_NO_ERROR == ret);
}

int A2dpSink::GetConnectStrategy(const BluetoothRemoteDevice &device) const
{
    HILOGI("[A2dpSink] %{public}s\n", __func__);

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("[A2dpSink] input parameter error.");
        return RET_BAD_PARAM;
    }

    int ret = RET_NO_ERROR;
    if (pimpl->proxy_ != nullptr && IS_BT_ENABLED()) {
        ret = pimpl->proxy_->GetConnectStrategy(RawAddress(device.GetDeviceAddr()));
    } else {
        HILOGI("[A2dpSink] proxy or bt disable.");
        return false;
    }
    return ret;
}
}  // namespace Bluetooth
}  // namespace OHOS