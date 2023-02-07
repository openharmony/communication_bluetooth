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
        HILOGI("hid conn state, device: %{public}s, state: %{public}s",
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
    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice>& result)
    {
        HILOGI("Enter!");
        if (!proxy_) {
            HILOGE("proxy_ is nullptr.");
            return BT_ERR_SERVICE_DISCONNECTED;
        }
        if (!IS_BT_ENABLED()) {
            HILOGE("bluetooth is off.");
            return BT_ERR_INVALID_STATE;
        }

        std::vector<BluetoothRawAddress> rawDevices;
        std::vector<int32_t> tmpStates;
        for (int32_t state : states) {
            tmpStates.push_back((int32_t)state);
        }

        int32_t ret = proxy_->GetDevicesByStates(tmpStates, rawDevices);
        if (ret != BT_SUCCESS) {
            HILOGE("inner error.");
            return ret;
        }

        for (BluetoothRawAddress rawDevice : rawDevices) {
            BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), 1);
            result.push_back(remoteDevice);
        }

        return BT_SUCCESS;
    }

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }
        if (!IS_BT_ENABLED()) {
            HILOGE("bluetooth is off.");
            return BT_ERR_INVALID_STATE;
        }
        return proxy_->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
    }

    int32_t Connect(const BluetoothRemoteDevice &device)
    {
        if (proxy_ == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }
        if (!IS_BT_ENABLED()) {
            HILOGE("bluetooth is off.");
            return BT_ERR_INVALID_STATE;
        }

        int cod = 0;
        int32_t err = device.GetDeviceClass(cod);
        if (err != BT_SUCCESS) {
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
        if (!IS_BT_ENABLED()) {
            HILOGE("bluetooth is off.");
            return BT_ERR_INVALID_STATE;
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
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->HidHostVCUnplug(device, id, size, type, result);
        }
    }

    void HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGI("Enter!");
        int result;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->HidHostSendData(device, id, size, type, result);
        }
    }

    void HidHostSetReport(std::string device, uint8_t type, uint16_t size, uint8_t report)
    {
        HILOGI("Enter!");
        int result;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
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

private:
    BluetoothObserverList<HidHostObserver> observers_;
    sptr<HidHostInnerObserver> innerObserver_;
    sptr<IBluetoothHidHost> proxy_;
    class HidHostDeathRecipient;
    sptr<HidHostDeathRecipient> deathRecipient_;
    void GetHidProxy()
    {
        sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            HILOGE("error: no samgr");
            return;
        }
        sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);
        if (hostRemote == nullptr) {
            HILOGE("failed: no hostRemote");
            return;
        }

        sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
        if (hostProxy == nullptr) {
            HILOGE("error: host no proxy");
            return;
        }

        sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_HID_HOST_SERVER);
        if (remote == nullptr) {
            HILOGE("failed: no remote");
            return;
        }

        proxy_ = iface_cast<IBluetoothHidHost>(remote);
        if (proxy_ == nullptr) {
            HILOGE("error: no proxy");
            return;
        }
    }
};

class HidHost::impl::HidHostDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    HidHostDeathRecipient(HidHost::impl &impl) : impl_(impl)
    {};
    ~HidHostDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HidHostDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("starts");
        impl_.proxy_->AsObject()->RemoveDeathRecipient(impl_.deathRecipient_);
        impl_.proxy_ = nullptr;

        // Re-obtain the proxy and register the observer.
        sleep(GET_PROXY_SLEEP_SEC);
        impl_.GetHidProxy();
        if (impl_.proxy_ == nullptr) {
            HILOGE("proxy reset failed");
            return;
        }
        if (impl_.innerObserver_ == nullptr || impl_.deathRecipient_ == nullptr) {
            HILOGE("innerObserver_ or deathRecipient_ is null");
            return;
        }
        impl_.proxy_->RegisterObserver(impl_.innerObserver_);
        impl_.proxy_->AsObject()->AddDeathRecipient(impl_.deathRecipient_);
    }

private:
    HidHost::impl &impl_;
};

HidHost::impl::impl()
{
    HILOGI("Enter!");
    GetHidProxy();
    if (proxy_ == nullptr) {
        HILOGI("get proxy_ failed");
        return;
    }

    innerObserver_ = new HidHostInnerObserver(observers_);
    if (innerObserver_ == nullptr) {
        HILOGE("innerObserver_ is null");
        return;
    }
    proxy_->RegisterObserver(innerObserver_);

    deathRecipient_ = new HidHostDeathRecipient(*this);
    if (deathRecipient_ == nullptr) {
        HILOGE("deathRecipient_ is null");
        return;
    }
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

HidHost::impl::~impl()
{
    HILOGI("Enter!");
    if (proxy_ != nullptr) {
        proxy_->DeregisterObserver(innerObserver_);
    }
    proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
}

HidHost::HidHost()
{
    pimpl = std::make_unique<impl>();
}

HidHost::~HidHost()
{}

HidHost *HidHost::GetProfile()
{
    static HidHost instance;
    return &instance;
}

int32_t HidHost::GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result)
{
    return pimpl->GetDevicesByStates(states, result);
}

int32_t HidHost::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    return pimpl->GetDeviceState(device, state);
}

int32_t HidHost::Connect(const BluetoothRemoteDevice &device)
{
    return pimpl->Connect(device);
}

int32_t HidHost::Disconnect(const BluetoothRemoteDevice &device)
{
    return pimpl->Disconnect(device);
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
    return pimpl->HidHostVCUnplug(device, id, size, type);
}

void HidHost::HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    return pimpl->HidHostSendData(device, id, size, type);
}

void HidHost::HidHostSetReport(std::string device, uint8_t type, uint16_t size, uint8_t report)
{
    return pimpl->HidHostSetReport(device, type, size, report);
}

void HidHost::HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    return pimpl->HidHostGetReport(device, id, size, type);
}
}  // namespace Bluetooth
}  // namespace OHOS