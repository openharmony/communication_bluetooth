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

#include "bluetooth_errorcode.h"
#include "bluetooth_pan.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_pan_observer_stub.h"
#include "i_bluetooth_pan.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
class PanInnerObserver : public BluetoothPanObserverStub {
public:
    explicit PanInnerObserver(BluetoothObserverList<PanObserver> &observers) : observers_(observers)
    {
        HILOGI("enter");
    }
    ~PanInnerObserver() override
    {
        HILOGI("enter");
    }

    ErrCode OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        HILOGI("enter, device: %{public}s, state: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), state);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 1);
        observers_.ForEach([remoteDevice, state](std::shared_ptr<PanObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state);
        });
        return NO_ERROR;
    }

private:
    BluetoothObserverList<PanObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PanInnerObserver);
};

struct Pan::impl {
    impl();
    ~impl();

    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice>& result)
    {
        HILOGI("enter");
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

    int32_t Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGI("device: %{public}s", GET_ENCRYPT_ADDR(device));
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

    void RegisterObserver(std::shared_ptr<PanObserver> observer)
    {
        HILOGI("enter");
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<PanObserver> observer)
    {
        HILOGI("enter");
        observers_.Deregister(observer);
    }

    int32_t SetTethering(bool value)
    {
        HILOGI("enter");
        if (!proxy_) {
            HILOGE("proxy_ is nullptr.");
            return BT_ERR_SERVICE_DISCONNECTED;
        }
        if (!IS_BT_ENABLED()) {
            HILOGE("bluetooth is off.");
            return BT_ERR_INVALID_STATE;
        }
        int32_t ret = proxy_->SetTethering(value);
        HILOGI("fwk ret:%{public}d", ret);
        return ret;
    }

    int32_t IsTetheringOn(bool &value)
    {
        HILOGI("enter");
        if (!proxy_) {
            HILOGE("proxy_ is nullptr.");
            return BT_ERR_SERVICE_DISCONNECTED;
        }
        if (!IS_BT_ENABLED()) {
            HILOGE("bluetooth is off.");
            return BT_ERR_INVALID_STATE;
        }
        return proxy_->IsTetheringOn(value);
    }

private:
    BluetoothObserverList<PanObserver> observers_;
    sptr<PanInnerObserver> innerObserver_;
    sptr<IBluetoothPan> proxy_;
    class PanDeathRecipient;
    sptr<PanDeathRecipient> deathRecipient_;
};

class Pan::impl::PanDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    PanDeathRecipient(Pan::impl &impl) : impl_(impl)
    {};
    ~PanDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PanDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("starts");
        impl_.proxy_->AsObject()->RemoveDeathRecipient(impl_.deathRecipient_);
        impl_.proxy_ = nullptr;
    }

private:
    Pan::impl &impl_;
};

Pan::impl::impl()
{
    HILOGI("enter");
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        HILOGE("failed: no hostRemote");
        return;
    }
    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_PAN_SERVER);

    if (!remote) {
        HILOGE("failed: no remote");
        return;
    }
    HILOGI("remote obtained");

    innerObserver_ = new PanInnerObserver(observers_);

    proxy_ = iface_cast<IBluetoothPan>(remote);
    proxy_->RegisterObserver(innerObserver_);
    deathRecipient_ = new PanDeathRecipient(*this);
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

Pan::impl::~impl()
{
    HILOGI("enter");
    if (proxy_ != nullptr) {
        proxy_->DeregisterObserver(innerObserver_);
    }
    proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
}

Pan::Pan()
{
    pimpl = std::make_unique<impl>();
}

Pan::~Pan()
{}

Pan *Pan::GetProfile()
{
    static Pan instance;
    return &instance;
}

int32_t Pan::GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result)
{
    return pimpl->GetDevicesByStates(states, result);
}

int32_t Pan::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    return pimpl->GetDeviceState(device, state);
}

int32_t Pan::Disconnect(const BluetoothRemoteDevice &device)
{
    return pimpl->Disconnect(device);
}

void Pan::RegisterObserver(PanObserver *observer)
{
    std::shared_ptr<PanObserver> observerPtr(observer, [](PanObserver *) {});
    return pimpl->RegisterObserver(observerPtr);
}

void Pan::DeregisterObserver(PanObserver *observer)
{
    std::shared_ptr<PanObserver> observerPtr(observer, [](PanObserver *) {});
    return pimpl->DeregisterObserver(observerPtr);
}

int32_t Pan::SetTethering(bool value)
{
    return pimpl->SetTethering(value);
}

int32_t Pan::IsTetheringOn(bool &value)
{
    return pimpl->IsTetheringOn(value);
}
}  // namespace Bluetooth
}  // namespace OHOS