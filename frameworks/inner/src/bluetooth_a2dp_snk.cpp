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
#include <cstdint>
#include "bluetooth_a2dp_sink_observer_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_load_system_ability.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_types.h"
#include "bluetooth_utils.h"
#include "functional"
#include "i_bluetooth_a2dp_sink.h"
#include "i_bluetooth_a2dp_sink_observer.h"
#include "i_bluetooth_host.h"
#include "if_system_ability_manager.h"
#include "iosfwd"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "list"
#include "memory"
#include "new"
#include "raw_address.h"
#include "refbase.h"
#include "string"
#include "system_ability_definition.h"
#include "vector"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
std::mutex g_a2dpSnkProxyMutex;
struct A2dpSink::impl {
    impl();
    ~impl();
    bool InitA2dpSinkProxy(void);

    BluetoothObserverList<A2dpSinkObserver> observers_;
    sptr<IBluetoothA2dpSink> proxy_ = nullptr;
    class BluetoothA2dpSinkObserverImp;
    sptr<BluetoothA2dpSinkObserverImp> observerImp_ = nullptr;
    class BluetoothA2dpSinkDeathRecipient;
    sptr<BluetoothA2dpSinkDeathRecipient> deathRecipient_ = nullptr;
};

class A2dpSink::impl::BluetoothA2dpSinkObserverImp : public BluetoothA2dpSinkObserverStub {
public:
    explicit BluetoothA2dpSinkObserverImp(A2dpSink::impl &a2dpSink) : a2dpSink_(a2dpSink)
    {};
    ~BluetoothA2dpSinkObserverImp() override
    {};

    void Register(std::shared_ptr<A2dpSinkObserver> &observer)
    {
        HILOGI("enter");
        a2dpSink_.observers_.Register(observer);
    }

    void Deregister(std::shared_ptr<A2dpSinkObserver> &observer)
    {
        HILOGI("enter");
        a2dpSink_.observers_.Deregister(observer);
    }

    void OnConnectionStateChanged(const RawAddress &device, int state) override
    {
        HILOGD("device: %{public}s, state: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), state);
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
    explicit BluetoothA2dpSinkDeathRecipient(A2dpSink::impl &a2dpSinkDeath) : a2dpSinkDeath_(a2dpSinkDeath)
    {};
    ~BluetoothA2dpSinkDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSinkDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("enter");
        std::lock_guard<std::mutex> lock(g_a2dpSnkProxyMutex);
        if (!a2dpSinkDeath_.proxy_) {
            return;
        }
        a2dpSinkDeath_.proxy_ = nullptr;
    }

private:
    A2dpSink::impl &a2dpSinkDeath_;
};

A2dpSink::impl::impl()
{
    HILOGI("start");
    if (proxy_) {
        return;
    }
    BluetootLoadSystemAbility::GetInstance().RegisterNotifyMsg(PROFILE_ID_A2DP_SINK);
    if (!BluetootLoadSystemAbility::GetInstance().HasSubscribedBluetoothSystemAbility()) {
        BluetootLoadSystemAbility::GetInstance().SubScribeBluetoothSystemAbility();
        return;
    }
    InitA2dpSinkProxy();
};

A2dpSink::impl::~impl()
{
    HILOGI("start");
    if (proxy_ != nullptr) {
        proxy_->DeregisterObserver(observerImp_);
        proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
}

bool A2dpSink::impl::InitA2dpSinkProxy(void)
{
    std::lock_guard<std::mutex> lock(g_a2dpSnkProxyMutex);
    if (proxy_) {
        return true;
    }
    HILOGI("Enter!");
    proxy_ = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    if (!proxy_) {
        HILOGE("get A2dpSink proxy_ failed");
        return false;
    }

    deathRecipient_ = new BluetoothA2dpSinkDeathRecipient(*this);
    if (deathRecipient_ != nullptr) {
        proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
    }

    observerImp_ = new (std::nothrow) BluetoothA2dpSinkObserverImp(*this);
    if (observerImp_ != nullptr) {
        proxy_->RegisterObserver(observerImp_);
    }
    return true;
}

A2dpSink::A2dpSink()
{
    pimpl = std::make_unique<impl>();
    if (!pimpl) {
        HILOGE("fails: no pimpl");
    }
}

void A2dpSink::Init()
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    if (!pimpl->InitA2dpSinkProxy()) {
        HILOGE("A2dpSink proxy is nullptr");
        return;
    }
}

A2dpSink::~A2dpSink()
{
    HILOGI("start");
}

void A2dpSink::RegisterObserver(A2dpSinkObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<A2dpSinkObserver> pointer(observer, [](A2dpSinkObserver *) {});
    pimpl->observers_.Register(pointer);
}

void A2dpSink::DeregisterObserver(A2dpSinkObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<A2dpSinkObserver> pointer(observer, [](A2dpSinkObserver *) {});
    pimpl->observers_.Deregister(pointer);
}

int A2dpSink::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return pimpl->proxy_->GetDeviceState(RawAddress(device.GetDeviceAddr()));
}

std::vector<BluetoothRemoteDevice> A2dpSink::GetDevicesByStates(std::vector<int> states) const
{
    HILOGI("enter");

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return std::vector<BluetoothRemoteDevice>();
    }

    std::vector<int32_t> convertStates;
    for (auto state : states) {
        convertStates.push_back(static_cast<int32_t>(state));
    }
    std::vector<BluetoothRemoteDevice> devices;
    std::vector<RawAddress> rawAddrs = pimpl->proxy_->GetDevicesByStates(convertStates);
    for (auto rawAddr : rawAddrs) {
        BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
        devices.push_back(device);
    }
    return devices;
}

int A2dpSink::GetPlayingState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    int ret = RET_BAD_STATUS;
    pimpl->proxy_->GetPlayingState(RawAddress(device.GetDeviceAddr()), ret);
    return ret;
}

int A2dpSink::GetPlayingState(const BluetoothRemoteDevice &device, int &state) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return pimpl->proxy_->GetPlayingState(RawAddress(device.GetDeviceAddr()), state);
}

bool A2dpSink::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return false;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return false;
    }

    int ret = pimpl->proxy_->Connect(RawAddress(device.GetDeviceAddr()));
    return (ret == RET_NO_ERROR);
}

bool A2dpSink::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return false;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return false;
    }

    int ret = pimpl->proxy_->Disconnect(RawAddress(device.GetDeviceAddr()));
    return (ret == RET_NO_ERROR);
}

A2dpSink *A2dpSink::GetProfile()
{
    HILOGI("enter");
    static A2dpSink service;
    return &service;
}

bool A2dpSink::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("enter, device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return false;
    }

    if ((!device.IsValidBluetoothRemoteDevice()) ||
        ((strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) &&
            (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)))) {
        HILOGE("input parameter error.");
        return false;
    }

    int ret = pimpl->proxy_->SetConnectStrategy(RawAddress(device.GetDeviceAddr()), strategy);
    return (ret == RET_NO_ERROR);
}

int A2dpSink::GetConnectStrategy(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    int ret = pimpl->proxy_->GetConnectStrategy(RawAddress(device.GetDeviceAddr()));
    return ret;
}

bool A2dpSink::SendDelay(const BluetoothRemoteDevice &device, uint16_t delayValue)
{
    HILOGI("enter, device: %{public}s, delayValue: %{public}d", GET_ENCRYPT_ADDR(device), delayValue);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or A2dpSink proxy is nullptr");
        return false;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return false;
    }

    int ret = pimpl->proxy_->SendDelay(RawAddress(device.GetDeviceAddr()), (int32_t)delayValue);
    return (ret == RET_NO_ERROR);
}
} // namespace Bluetooth
} // namespace OHOS