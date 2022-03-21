/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "bluetooth_pbap_pse_server.h"
#include "i_bluetooth_pbap_pse.h"
#include "interface_adapter_manager.h"
#include "interface_profile_manager.h"
#include "interface_profile_pbap_pse.h"
#include "bluetooth_log.h"
#include "remote_observer_list.h"
#include "bluetooth_def.h"

namespace OHOS {
namespace Bluetooth {
class PbapPseObserver : public bluetooth::IPbapPseObserver {
public:
    PbapPseObserver() = default;
    ~PbapPseObserver() = default;
    void OnServiceConnectionStateChanged(const bluetooth::RawAddress& remoteAddr, int state) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([remoteAddr, state](IBluetoothPbapPseObserver* observer) {
            observer->OnServiceConnectionStateChanged(remoteAddr, state);
        });
    }
    void OnServicePermission(const bluetooth::RawAddress& remoteAddr) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach(
            [remoteAddr](IBluetoothPbapPseObserver* observer) { observer->OnServicePermission(remoteAddr); });
    }
    void OnServicePasswordRequired(const bluetooth::RawAddress& remoteAddr, const std::vector<uint8_t>& description,
                                   uint8_t charset, bool fullAccess) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([remoteAddr, description, charset, fullAccess](IBluetoothPbapPseObserver* observer) {
            observer->OnServicePasswordRequired(remoteAddr, description, charset, fullAccess);
        });
    }
    void SetObserver(RemoteObserverList<IBluetoothPbapPseObserver>* observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothPbapPseObserver>* observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PbapPseObserver);
};

struct BluetoothPbapPseServer::impl {
    impl();
    RemoteObserverList<IBluetoothPbapPseObserver> observers_;
    std::unique_ptr<PbapPseObserver> observerImp_ = {std::make_unique<PbapPseObserver>()};
    bluetooth::IProfilePbapPse* pbapPseService_{nullptr};
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_{nullptr};
};

class BluetoothPbapPseServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    SystemStateObserver(BluetoothPbapPseServer::impl* impl) : impl_(impl) {};
    ~SystemStateObserver() = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        HILOGI("OnSystemStateChange: state=%{public}d",(int)state);

        switch (state) {
        case bluetooth::BTSystemState::ON: {
            bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
            if (serviceManager != nullptr) {
                bluetooth::IProfile* profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PSE);
                if (profileService) {
                    impl_->pbapPseService_ = (bluetooth::IProfilePbapPse*)profileService;
                    impl_->pbapPseService_->RegisterObserver(*impl_->observerImp_);
                } else {
                    HILOGI("can't find PbapPseService from ProfileServiceManager!");
                }
            } else {
                HILOGI("can't find ProfileServiceManager!");
            }
        } break;
        case bluetooth::BTSystemState::OFF:
            impl_->pbapPseService_ = nullptr;
            break;
        default:
            break;
        }
    }

private:
    BluetoothPbapPseServer::impl* impl_;
};

BluetoothPbapPseServer::impl::impl()
{
    systemStateObserver_ = std::make_unique<SystemStateObserver>(this);
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
}

BluetoothPbapPseServer::BluetoothPbapPseServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));

    bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
    if (serviceManager != nullptr) {
        bluetooth::IProfile* profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PSE);
        if (profileService) {
            pimpl->pbapPseService_ = (bluetooth::IProfilePbapPse*)profileService;
            pimpl->pbapPseService_->RegisterObserver(*pimpl->observerImp_);
        } else {
            HILOGI("can't find PbapPseService from ProfileServiceManager!");
        }
    } else {
        HILOGI("can't find ProfileServiceManager!");
    }
}

BluetoothPbapPseServer::~BluetoothPbapPseServer() {}

int BluetoothPbapPseServer::GetDeviceState(const BluetoothRawAddress &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPseService_) {
        return pimpl->pbapPseService_->GetDeviceState(device);
    } else {
        return -1;
    }
}

void BluetoothPbapPseServer::GetDevicesByStates(
    const ::std::vector<int32_t> tmpStates, std::vector<BluetoothRawAddress> &rawDevices)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPseService_) {
        auto devices = pimpl->pbapPseService_->GetDevicesByStates(tmpStates);
        for (auto& device : devices) {
            rawDevices.push_back(device);
        }
    }
}

int BluetoothPbapPseServer::Disconnect(const BluetoothRawAddress& device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPseService_) {
        return pimpl->pbapPseService_->Disconnect(device);
    } else {
        return -1;
    }
}

int BluetoothPbapPseServer::SetConnectionStrategy(const BluetoothRawAddress& device, int32_t strategy)
{
    HILOGD("[%{public}s]: %{public}s(): Enter! strategy: %{public}d", __FILE__, __FUNCTION__, strategy);
    std::string addString = device.GetAddress();
    bluetooth::RawAddress addr(addString);
    if (pimpl->pbapPseService_ != nullptr) {
        return pimpl->pbapPseService_->SetConnectionStrategy(addr, strategy);
    } else {
        return -1;
    }
}

int BluetoothPbapPseServer::GetConnectionStrategy(const BluetoothRawAddress& device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::string addString = device.GetAddress();
    bluetooth::RawAddress addr(addString);
    if (pimpl->pbapPseService_ != nullptr) {
        return pimpl->pbapPseService_->GetConnectionStrategy(addr);
    } else {
        return -1;
    }
}

void BluetoothPbapPseServer::GrantPermission(const BluetoothRawAddress& device, bool allow, bool save)
{
    HILOGD("[%{public}s]: %{public}s(): Enter! allow: %{public}d save: %{public}d", __FILE__, __FUNCTION__, allow, save);
    if (pimpl->pbapPseService_) {
        pimpl->pbapPseService_->GrantPermission(device, allow, save);
    }
}

int BluetoothPbapPseServer::SetDevicePassword(const BluetoothRawAddress& device, const std::string &password,
    const std::string &userId)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPseService_) {
        std::string pwdTmp = password;
        std::string usrIdTmp = userId;
        return pimpl->pbapPseService_->SetDevicePassword(device, pwdTmp, usrIdTmp);
    } else {
        return -1;
    }
}

void BluetoothPbapPseServer::RegisterObserver(const sptr<IBluetoothPbapPseObserver>& observer)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!observer) {
        HILOGE("RegisterObserver called with NULL binder. Ignoring.");
    }
    pimpl->observers_.Register(observer);
}

void BluetoothPbapPseServer::DeregisterObserver(const sptr<IBluetoothPbapPseObserver>& observer)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!observer) {
        HILOGE("DeregisterObserver called with NULL binder. Ignoring.");
    }
    pimpl->observers_.Deregister(observer);
}

std::vector<BluetoothRawAddress> BluetoothPbapPseServer::GetConnectedDevices()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    ::std::vector<int32_t> states{static_cast<int32_t>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRawAddress> rawDevices;
    GetDevicesByStates(states, rawDevices);
    
    return rawDevices;
}
}  // namespace Bluetooth
}  // namespace OHOS