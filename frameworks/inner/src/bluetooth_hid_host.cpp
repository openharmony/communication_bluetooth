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
        HILOGD("Enter!");
    }
    ~HidHostInnerObserver() override
    {
        HILOGD("Enter!");
    }

    ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        HILOGI("addr:%{public}s, state:%{public}d", GET_ENCRYPT_ADDR(remoteDevice), state);
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
    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states)
    {
        HILOGD("Enter!");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            std::vector<BluetoothRawAddress> rawDevices;
            std::vector<int32_t> tmpStates;
            for (int state : states) {
                tmpStates.push_back((int32_t)state);
            }

            proxy_->GetDevicesByStates(tmpStates, rawDevices);
            for (BluetoothRawAddress rawDevice : rawDevices) {
                BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), 0);
                remoteDevices.push_back(remoteDevice);
            }
        }
        return remoteDevices;
    }

    int GetDeviceState(const BluetoothRemoteDevice &device)
    {
        HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            int state;
            proxy_->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
            HILOGI("addr:%{public}s, state:%{public}d", GET_ENCRYPT_ADDR(device), state);
            return state;
        }
        HILOGI("failed and fw return DISCONNECTED, addr:%{public}s", GET_ENCRYPT_ADDR(device));
        return (int)BTConnectState::DISCONNECTED;
    }

    bool Connect(const BluetoothRemoteDevice &device)
    {
        HILOGI("start, addr:%{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ != nullptr && IS_BT_ENABLED() && !BluetoothHost::GetDefaultHost().IsBtDiscovering() &&
            device.IsValidBluetoothRemoteDevice()) {
                bool isOk;
                proxy_->Connect(BluetoothRawAddress(device.GetDeviceAddr()), isOk);
                HILOGI("end, result:%{public}d", isOk);
                return isOk;
        }
        HILOGE("end, fw return false!");
        return false;
    }

    bool Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGI("start, addr:%{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            bool isOk;
            proxy_->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()), isOk);
            HILOGI("end, result:%{public}d", isOk);
            return isOk;
        }
        HILOGE("fw return false!");
        return false;
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
        HILOGD("Enter!");
        int result;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->HidHostVCUnplug(device, id, size, type, result);
        }
    }

    void HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGD("Enter!");
        int result;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->HidHostSendData(device, id, size, type, result);
        }
    }

    void HidHostSetReport(std::string device, uint8_t type, uint16_t size, uint8_t report)
    {
        HILOGD("Enter!");
        int result;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->HidHostSetReport(device, type, size, report, result);
        }
    }

    void HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        HILOGD("Enter!");
        int result;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->HidHostGetReport(device, id, size, type, result);
        }
    }

private:
    BluetoothObserverList<HidHostObserver> observers_;
    HidHostInnerObserver innerObserver_ {HidHostInnerObserver(observers_)};
    sptr<IBluetoothHidHost> proxy_;
    class HidHostDeathRecipient;
    sptr<HidHostDeathRecipient> deathRecipient_;
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
    }

private:
    HidHost::impl &impl_;
};

HidHost::impl::impl()
{
    HILOGD("Enter!");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        HILOGE("failed: no hostRemote");
        return;
    }
    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_HID_HOST_SERVER);

    if (!remote) {
        HILOGE("failed: no remote");
        return;
    }
    HILOGI("remote obtained");

    proxy_ = iface_cast<IBluetoothHidHost>(remote);
    proxy_->RegisterObserver(&innerObserver_);
    deathRecipient_ = new HidHostDeathRecipient(*this);
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

HidHost::impl::~impl()
{
    HILOGD("Enter!");
    if (proxy_ != nullptr) {
        proxy_->DeregisterObserver(&innerObserver_);
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

std::vector<BluetoothRemoteDevice> HidHost::GetDevicesByStates(std::vector<int> states)
{
    return pimpl->GetDevicesByStates(states);
}

int HidHost::GetDeviceState(const BluetoothRemoteDevice &device)
{
    return pimpl->GetDeviceState(device);
}

bool HidHost::Connect(const BluetoothRemoteDevice &device)
{
    return pimpl->Connect(device);
}

bool HidHost::Disconnect(const BluetoothRemoteDevice &device)
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