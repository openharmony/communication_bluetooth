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

#include "bluetooth_pan_server.h"
#include "bluetooth_log.h"
#include "interface_profile.h"
#include "interface_profile_pan.h"
#include "i_bluetooth_host_observer.h"
#include "remote_observer_list.h"
#include "hilog/log.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothPanCallback : public bluetooth::IPanObserver {
public:
    BluetoothPanCallback() = default;
    ~BluetoothPanCallback() override = default;

    void OnConnectionStateChanged(const RawAddress &device, int state) override
    {
        HILOGI("BluetoothPanCallback::OnConnectionStateChanged start.");
        observers_->ForEach([device, state](sptr<IBluetoothPanObserver> observer) {
            observer->OnConnectionStateChanged(device, state);
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothPanObserver> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothPanObserver> *observers_;
};

struct BluetoothPanServer::impl {
    impl();
    ~impl();

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    RemoteObserverList<IBluetoothPanObserver> observers_;
    std::unique_ptr<BluetoothPanCallback> observerImp_ = std::make_unique<BluetoothPanCallback>();
    IProfilePan *panService_ = nullptr;
    std::vector<sptr<IBluetoothPanObserver>> advCallBack_;

    IProfilePan *GetServicePtr()
    {
        if (IProfileManager::GetInstance() == nullptr) {
            return nullptr;
        }
        return static_cast<IProfilePan *>(
            IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_PAN));
    }
};

class BluetoothPanServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothPanServer::impl *pimpl) : pimpl_(pimpl) {}
    void OnSystemStateChange(const BTSystemState state) override
    {
        switch (state) {
            case BTSystemState::ON:
                pimpl_->panService_ = pimpl_->GetServicePtr();
                if (pimpl_->panService_ != nullptr) {
                    pimpl_->panService_->RegisterObserver(*pimpl_->observerImp_.get());
                }
                break;
            case BTSystemState::OFF:
                pimpl_->panService_ = nullptr;
                break;
            default:
                break;
        }
    };

private:
    BluetoothPanServer::impl *pimpl_ = nullptr;
};

BluetoothPanServer::impl::impl()
{
    HILOGI("BluetoothPanServer::impl::impl() starts");
}

BluetoothPanServer::impl::~impl()
{
    HILOGI("BluetoothPanServer::impl::~impl() starts");
}

BluetoothPanServer::BluetoothPanServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    pimpl->systemStateObserver_ = std::make_unique<impl::SystemStateObserver>(pimpl.get());
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));

    pimpl->panService_ = pimpl->GetServicePtr();
    if (pimpl->panService_ != nullptr) {
        pimpl->panService_->RegisterObserver(*pimpl->observerImp_.get());
    }
}

BluetoothPanServer::~BluetoothPanServer()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_));
    if (pimpl->panService_ != nullptr) {
        pimpl->panService_->DeregisterObserver(*pimpl->observerImp_.get());
    }
}

ErrCode BluetoothPanServer::RegisterObserver(const sptr<IBluetoothPanObserver> observer)
{
    HILOGI("BluetoothPanServer::RegisterObserver");

    if (observer == nullptr) {
        HILOGE("BluetoothPanServer::RegisterObserver observer is null");
        return ERR_INVALID_VALUE;
    }
    if (pimpl == nullptr) {
        HILOGE("BluetoothPanServer::RegisterObserver pimpl is null");
        return ERR_NO_INIT;
    }

    pimpl->observers_.Register(observer);
    pimpl->advCallBack_.push_back(observer);
    return ERR_OK;
}

ErrCode BluetoothPanServer::DeregisterObserver(const sptr<IBluetoothPanObserver> observer)
{
    HILOGI("BluetoothPanServer::DeregisterObserver");
    if (observer == nullptr) {
        HILOGE("BluetoothPanServer::DeregisterObserver observer is null");
        return ERR_INVALID_VALUE;
    }
    if (pimpl == nullptr) {
        HILOGE("BluetoothPanServer::DeregisterObserver pimpl is null");
        return ERR_NO_INIT;
    }
    for (auto iter = pimpl->advCallBack_.begin(); iter != pimpl->advCallBack_.end(); ++iter) {
        if ((*iter)->AsObject() == observer->AsObject()) {
            if (pimpl != nullptr) {
                pimpl->observers_.Deregister(*iter);
                pimpl->advCallBack_.erase(iter);
                break;
            }
        }
    }
    pimpl->panService_->DeregisterObserver(*pimpl->observerImp_.get());
    return ERR_OK;
}

ErrCode BluetoothPanServer::GetDevicesByStates(
    const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress>& result)
{
    HILOGI("Bluetooth Pan Server GetDevicesByStates Triggered!");
    if (pimpl == nullptr || pimpl->panService_ == nullptr) {
        HILOGI("BluetoothPanServer: GetDevicesByStates not init.");
        return ERR_NO_INIT;
    }

    std::vector<bluetooth::RawAddress> serviceDeviceList = pimpl->panService_->GetDevicesByStates(states);
    for (auto &device : serviceDeviceList) {
        BluetoothRawAddress bluetoothDevice(device.GetAddress());
        result.push_back(bluetoothDevice);
    }
    return ERR_OK;
}

ErrCode BluetoothPanServer::GetDeviceState(const BluetoothRawAddress &device,
    int& result)
{
    HILOGI("Bluetooth Pan Server GetDeviceState Triggered!");
    if (pimpl == nullptr || pimpl->panService_ == nullptr) {
        HILOGI("BluetoothPanServer: GetDevicesByStates not init.");
        return ERR_NO_INIT;
    }
    result = pimpl->panService_->GetDeviceState(device);
    return ERR_OK;
}

ErrCode BluetoothPanServer::Disconnect(
    const BluetoothRawAddress &device,
    bool& result)
{
    HILOGI("Bluetooth Pan Server Disconnect Triggered!");
    if (pimpl == nullptr || pimpl->panService_ == nullptr) {
        HILOGI("BluetoothPanServer: Disconnect not init.");
        return ERR_NO_INIT;
    }
    result = pimpl->panService_->Disconnect(device);
    return ERR_OK;
}

ErrCode BluetoothPanServer::SetTethering(
    const bool enable,
    bool& result)
{
    HILOGI("Bluetooth Pan Server SetTethering Triggered!");
    if (pimpl == nullptr || pimpl->panService_ == nullptr) {
        HILOGI("BluetoothPanServer: SetTethering not init.");
        return ERR_NO_INIT;
    }
    result = pimpl->panService_->SetTethering(enable);
    return ERR_OK;
}

ErrCode BluetoothPanServer::IsTetheringOn(
    bool& result)
{
        HILOGI("Bluetooth Pan Server IsTetheringOn Triggered!");
    if (pimpl == nullptr || pimpl->panService_ == nullptr) {
        HILOGI("BluetoothPanServer: IsTetheringOn not init.");
        return ERR_NO_INIT;
    }
    result = pimpl->panService_->IsTetheringOn();
    return ERR_OK;
}
}  // namespace Bluetooth
}  // namespace OHOS
