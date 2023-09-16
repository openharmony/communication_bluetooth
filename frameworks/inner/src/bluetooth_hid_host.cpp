/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "bluetooth_hid_host.h"
#include <unistd.h>
#include "bluetooth_device.h"
#include "bluetooth_host.h"
#include "bluetooth_load_system_ability.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_hid_host_observer_stub.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_hid_host.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
std::mutex g_hidProxyMutex;
class HidHostInnerObserver : public BluetoothHidHostObserverStub {
public:
    explicit HidHostInnerObserver(BluetoothObserverList<HidHostObserver> &observers) : observers_(observers)
    {
        HILOGI("Enter!");
    }
    ~HidHostInnerObserver() override
    {
        HILOGI("Enter!");
    }

    ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        HILOGD("hid conn state, device: %{public}s, state: %{public}s",
            GetEncryptAddr((device).GetAddress()).c_str(), GetProfileConnStateName(state).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state](std::shared_ptr<HidHostObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state);
        });
        return NO_ERROR;
    }

private:
    BluetoothObserverList<HidHostObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HidHostInnerObserver);
};

struct HidHost::impl {
    impl();
    ~impl();
    bool InitHidHostProxy(void);

    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice>& result)
    {
        HILOGI("Enter!");
        std::vector<BluetoothRawAddress> rawDevices;
        std::vector<int32_t> tmpStates;
        for (int32_t state : states) {
            tmpStates.push_back((int32_t)state);
        }

        if (!proxy_) {
            HILOGE("proxy_ is nullptr.");
            return BT_ERR_SERVICE_DISCONNECTED;
        }
        int32_t ret = proxy_->GetDevicesByStates(tmpStates, rawDevices);
        if (ret != BT_NO_ERROR) {
            HILOGE("inner error.");
            return ret;
        }

        for (BluetoothRawAddress rawDevice : rawDevices) {
            BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), 1);
            result.push_back(remoteDevice);
        }

        return BT_NO_ERROR;
    }

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        return proxy_->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
    }

    int32_t Connect(const BluetoothRemoteDevice &device)
    {
        if (proxy_ == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        int cod = 0;
        int32_t err = device.GetDeviceClass(cod);
        if (err != BT_NO_ERROR) {
            HILOGE("GetDeviceClass Failed.");
            return false;
        }
        BluetoothDeviceClass devClass = BluetoothDeviceClass(cod);
        if (!devClass.IsProfileSupported(BluetoothDevice::PROFILE_HID)) {
            HILOGE("hid connect failed. The remote device does not support HID service.");
            return BT_ERR_PROFILE_DISABLED;
        }
        cod = devClass.GetClassOfDevice();
        HILOGI("hid connect remote device: %{public}s, cod: %{public}d", GET_ENCRYPT_ADDR(device), cod);
        return proxy_->Connect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    int32_t Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGI("hid disconnect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        return proxy_->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    void RegisterObserver(std::shared_ptr<HidHostObserver> observer)
    {
        HILOGI("Enter!");
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<HidHostObserver> observer)
    {
        HILOGI("Enter!");
        observers_.Deregister(observer);
    }

    void HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGI("Enter!");
        int result;
        if (proxy_ != nullptr) {
            proxy_->HidHostVCUnplug(device, id, size, type, result);
        }
    }

    void HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGI("Enter!");
        int result;
        if (proxy_ != nullptr) {
            proxy_->HidHostSendData(device, id, size, type, result);
        }
    }

    void HidHostSetReport(std::string device, uint8_t type, uint16_t size, uint8_t report)
    {
        HILOGI("Enter!");
        int result;
        if (proxy_ != nullptr) {
            proxy_->HidHostSetReport(device, type, size, report, result);
        }
    }

    void HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGI("Enter!");
        int result;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->HidHostGetReport(device, id, size, type, result);
        }
    }

    int SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
    {
        HILOGI("enter");
        return proxy_->SetConnectStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
    }

    int GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
    {
        HILOGI("enter");
        return proxy_->GetConnectStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
    }

    sptr<IBluetoothHidHost> proxy_;
private:
    BluetoothObserverList<HidHostObserver> observers_;
    sptr<HidHostInnerObserver> innerObserver_;    
    class HidHostDeathRecipient;
    sptr<HidHostDeathRecipient> deathRecipient_;
};

class HidHost::impl::HidHostDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit HidHostDeathRecipient(HidHost::impl &impl) : impl_(impl)
    {};
    ~HidHostDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HidHostDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("starts");
        std::lock_guard<std::mutex> lock(g_hidProxyMutex);
        if (!impl_.proxy_) {
            return;
        }
        impl_.proxy_ = nullptr;
    }

private:
    HidHost::impl &impl_;
};

HidHost::impl::impl()
{
    if (proxy_) {
        return;
    }
    BluetootLoadSystemAbility::GetInstance().RegisterNotifyMsg(PROFILE_ID_HID_HOST);
    if (!BluetootLoadSystemAbility::GetInstance().HasSubscribedBluetoothSystemAbility()) {
        BluetootLoadSystemAbility::GetInstance().SubScribeBluetoothSystemAbility();
        return;
    }
    InitHidHostProxy();
}

HidHost::impl::~impl()
{
    HILOGI("Enter!");
    if (proxy_ != nullptr) {
        proxy_->DeregisterObserver(innerObserver_);
        proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
}

bool HidHost::impl::InitHidHostProxy(void)
{
    HILOGI("enter");
    std::lock_guard<std::mutex> lock(g_hidProxyMutex);
    if (proxy_) {
        return true;
    }
    proxy_ = GetRemoteProxy<IBluetoothHidHost>(PROFILE_HID_HOST_SERVER);
    if (!proxy_) {
        HILOGE("get HidHost proxy fail");
        return false;
    }

    innerObserver_ = new HidHostInnerObserver(observers_);
    if (innerObserver_ != nullptr) {
        proxy_->RegisterObserver(innerObserver_);
    }

    deathRecipient_ = new HidHostDeathRecipient(*this);
    if (deathRecipient_ != nullptr) {
        proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
    }
    return true;
}

HidHost::HidHost()
{
    pimpl = std::make_unique<impl>();
}

HidHost::~HidHost() {}

void HidHost::Init()
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    if (!pimpl->InitHidHostProxy()) {
        HILOGE("HidHost proxy is nullptr");
        return;
    }
}

HidHost *HidHost::GetProfile()
{
    static HidHost instance;
    return &instance;
}

int32_t HidHost::GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->GetDevicesByStates(states, result);
}

int32_t HidHost::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->GetDeviceState(device, state);
}

int32_t HidHost::Connect(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->Connect(device);
}

int32_t HidHost::Disconnect(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->Disconnect(device);
}

int HidHost::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("enter, device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    if ((!device.IsValidBluetoothRemoteDevice()) || (
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) &&
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)))) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->SetConnectStrategy(device, strategy);
}

int HidHost::GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->GetConnectStrategy(device, strategy);
}

void HidHost::RegisterObserver(HidHostObserver *observer)
{
    std::shared_ptr<HidHostObserver> observerPtr(observer, [](HidHostObserver *) {});
    return pimpl->RegisterObserver(observerPtr);
}

void HidHost::DeregisterObserver(HidHostObserver *observer)
{
    std::shared_ptr<HidHostObserver> observerPtr(observer, [](HidHostObserver *) {});
    return pimpl->DeregisterObserver(observerPtr);
}

void HidHost::HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return;
    }

    return pimpl->HidHostVCUnplug(device, id, size, type);
}

void HidHost::HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return;
    }

    return pimpl->HidHostSendData(device, id, size, type);
}

void HidHost::HidHostSetReport(std::string device, uint8_t type, uint16_t size, uint8_t report)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return;
    }

    return pimpl->HidHostSetReport(device, type, size, report);
}

void HidHost::HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or hidHost proxy is nullptr");
        return;
    }

    return pimpl->HidHostGetReport(device, id, size, type);
}
} // namespace Bluetooth
} // namespace OHOS