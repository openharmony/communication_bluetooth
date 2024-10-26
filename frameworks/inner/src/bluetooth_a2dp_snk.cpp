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
#ifndef LOG_TAG
#define LOG_TAG "bt_fwk_a2dp_snk"
#endif

#include "bluetooth_a2dp_snk.h"
#include <cstdint>
#include "bluetooth_a2dp_sink_observer_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_profile_manager.h"
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
    BluetoothObserverList<A2dpSinkObserver> observers_;
    class BluetoothA2dpSinkObserverImp;
    sptr<BluetoothA2dpSinkObserverImp> observerImp_ = nullptr;
    int32_t profileRegisterId = 0;
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

    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {
        HILOGD("device: %{public}s, state: %{public}d, cause: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), state, cause);
        a2dpSink_.observers_.ForEach([device, state, cause](std::shared_ptr<A2dpSinkObserver> observer) {
            observer->OnConnectionStateChanged(BluetoothRemoteDevice(device.GetAddress(), 0), state, cause);
        });
    }

private:
    A2dpSink::impl &a2dpSink_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSinkObserverImp);
};

A2dpSink::impl::impl()
{
    observerImp_ = new (std::nothrow) BluetoothA2dpSinkObserverImp(*this);
    CHECK_AND_RETURN_LOG(observerImp_ != nullptr, "observerImp_ is nullptr");
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_A2DP_SINK,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothA2dpSink> proxy = iface_cast<IBluetoothA2dpSink>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(observerImp_);
    });
};

A2dpSink::impl::~impl()
{
    HILOGD("start");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(observerImp_);
}

A2dpSink::A2dpSink()
{
    pimpl = std::make_unique<impl>();
    if (!pimpl) {
        HILOGE("fails: no pimpl");
    }
}

A2dpSink::~A2dpSink()
{
    HILOGD("start");
}

void A2dpSink::RegisterObserver(std::shared_ptr<A2dpSinkObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->observers_.Register(observer);
}

void A2dpSink::DeregisterObserver(std::shared_ptr<A2dpSinkObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->observers_.Deregister(observer);
}

int A2dpSink::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }
    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "A2dpSink proxy is nullptr");
    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return proxy->GetDeviceState(RawAddress(device.GetDeviceAddr()));
}

std::vector<BluetoothRemoteDevice> A2dpSink::GetDevicesByStates(std::vector<int> states) const
{
    HILOGI("enter");

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }

    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::vector<BluetoothRemoteDevice>(),
        "A2dpSink proxy is nullptr");

    std::vector<int32_t> convertStates;
    for (auto state : states) {
        convertStates.push_back(static_cast<int32_t>(state));
    }
    std::vector<BluetoothRemoteDevice> devices;
    std::vector<RawAddress> rawAddrs = proxy->GetDevicesByStates(convertStates);
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

    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "A2dpSink proxy is nullptr");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    int ret = RET_BAD_STATUS;
    proxy->GetPlayingState(RawAddress(device.GetDeviceAddr()), ret);
    return ret;
}

int A2dpSink::GetPlayingState(const BluetoothRemoteDevice &device, int &state) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "A2dpSink proxy is nullptr");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return proxy->GetPlayingState(RawAddress(device.GetDeviceAddr()), state);
}

bool A2dpSink::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "A2dpSink proxy is nullptr");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return false;
    }

    int ret = proxy->Connect(RawAddress(device.GetDeviceAddr()));
    return (ret == RET_NO_ERROR);
}

bool A2dpSink::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "A2dpSink proxy is nullptr");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return false;
    }

    int ret = proxy->Disconnect(RawAddress(device.GetDeviceAddr()));
    return (ret == RET_NO_ERROR);
}

A2dpSink *A2dpSink::GetProfile()
{
    HILOGI("enter");
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<A2dpSink> service;
    return service.get();
#else
    static A2dpSink service;
    return &service;
#endif
}

bool A2dpSink::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("enter, device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }
    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "A2dpSink proxy is nullptr");

    if ((!device.IsValidBluetoothRemoteDevice()) ||
        ((strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) &&
            (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)))) {
        HILOGE("input parameter error.");
        return false;
    }

    int ret = proxy->SetConnectStrategy(RawAddress(device.GetDeviceAddr()), strategy);
    return (ret == RET_NO_ERROR);
}

int A2dpSink::GetConnectStrategy(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }
    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "A2dpSink proxy is nullptr");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    int ret = proxy->GetConnectStrategy(RawAddress(device.GetDeviceAddr()));
    return ret;
}

bool A2dpSink::SendDelay(const BluetoothRemoteDevice &device, uint16_t delayValue)
{
    HILOGI("enter, device: %{public}s, delayValue: %{public}d", GET_ENCRYPT_ADDR(device), delayValue);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }
    sptr<IBluetoothA2dpSink> proxy = GetRemoteProxy<IBluetoothA2dpSink>(PROFILE_A2DP_SINK);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "A2dpSink proxy is nullptr");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return false;
    }

    int ret = proxy->SendDelay(RawAddress(device.GetDeviceAddr()), (int32_t)delayValue);
    return (ret == RET_NO_ERROR);
}
} // namespace Bluetooth
} // namespace OHOS