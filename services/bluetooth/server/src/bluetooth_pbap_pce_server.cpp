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

#include "bluetooth_pbap_pce_server.h"
#include "i_bluetooth_pbap_pce.h"
#include "interface_adapter_manager.h"
#include "interface_profile_manager.h"
#include "interface_profile_pbap_pce.h"
#include "bluetooth_log.h"
#include "remote_observer_list.h"
#include "bluetooth_def.h"

namespace OHOS {
namespace Bluetooth {
class IPbapPullPhoneBookParamAdapter {
public:
    IPbapPullPhoneBookParamAdapter() = default;
    IPbapPullPhoneBookParamAdapter(const BluetoothIPbapPullPhoneBookParam &other)
    {
        data_.SetName(other.GetName());
        data_.SetPropertySelector(other.GetPropertySelector());
        data_.SetFormat(other.GetFormat());
        data_.SetMaxListCount(other.GetMaxListCount());
        data_.SetListStartOffset(other.GetListStartOffset());
        data_.SetResetNewMissedCalls(other.GetResetNewMissedCalls());
        data_.SetvCardSelector(other.GetvCardSelector());
        data_.SetvCardSelectorOp(other.GetvCardSelectorOp());
        data_.SetSpecifiedBitset(other.GetSpecifiedBitset());
    }
    virtual ~IPbapPullPhoneBookParamAdapter() = default;
    operator bluetooth::IPbapPullPhoneBookParam() const
    {
        return data_;
    }

private:
    bluetooth::IPbapPullPhoneBookParam data_;
};

class IPbapPullvCardListingParamAdapter {
public:
    IPbapPullvCardListingParamAdapter() = default;
    IPbapPullvCardListingParamAdapter(const BluetoothIPbapPullvCardListingParam &other)
    {
        data_.SetName(other.GetName());
        data_.SetOrder(other.GetOrder());
        data_.SetSearchValue(other.GetSearchValue());
        data_.SetSearchProperty(other.GetSearchProperty());
        data_.SetMaxListCount(other.GetMaxListCount());
        data_.SetListStartOffset(other.GetListStartOffset());
        data_.SetResetNewMissedCalls(other.GetResetNewMissedCalls());
        data_.SetvCardSelector(other.GetvCardSelector());
        data_.SetvCardSelectorOp(other.GetvCardSelectorOp());
        data_.SetSpecifiedBitset(other.GetSpecifiedBitset());
    }
    virtual ~IPbapPullvCardListingParamAdapter() = default;
    operator bluetooth::IPbapPullvCardListingParam() const
    {
        return data_;
    }

private:
    bluetooth::IPbapPullvCardListingParam data_;
};  

class IPbapPullvCardEntryParamAdapter {
public:
    IPbapPullvCardEntryParamAdapter() = default;
    IPbapPullvCardEntryParamAdapter(const BluetoothIPbapPullvCardEntryParam &other)
    {
        data_.SetName(other.GetName());
        data_.SetPropertySelector(other.GetPropertySelector());
        data_.SetFormat(other.GetFormat());
        data_.SetSpecifiedBitset(other.GetSpecifiedBitset());
    }
    virtual ~IPbapPullvCardEntryParamAdapter() = default;
    operator bluetooth::IPbapPullvCardEntryParam() const
    {
        return data_;
    }

private:
    bluetooth::IPbapPullvCardEntryParam data_;
};

class BluetoothIPbapPhoneBookDataAdapter {
public:
    BluetoothIPbapPhoneBookDataAdapter() = default;
    BluetoothIPbapPhoneBookDataAdapter(const bluetooth::IPbapPhoneBookData &other)
    {
        data_.phoneBookSize_ = other.phoneBookSize_;
        data_.primaryFolderVersion_ = other.primaryFolderVersion_;
        data_.secondaryFolderVersion_ = other.secondaryFolderVersion_;
        data_.databaseIdentifier_ = other.databaseIdentifier_;
        data_.deviceAddr_ = other.deviceAddr_;
        data_.vcardPath_ = other.vcardPath_;
        data_.vcardFileName_ = other.vcardFileName_;
        data_.resultLoaded_ = other.resultLoaded_;
        data_.result_ = other.result_;
    }
    virtual ~BluetoothIPbapPhoneBookDataAdapter() = default;
    operator BluetoothIPbapPhoneBookData() const
    {
        return data_;
    }

private:
    BluetoothIPbapPhoneBookData data_;
};

class PbapPceObserver : public bluetooth::IPbapPceObserver {
public:
    PbapPceObserver() = default;
    ~PbapPceObserver() = default;
    void OnServiceConnectionStateChanged(const bluetooth::RawAddress &remoteAddr, int state) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([remoteAddr, state](IBluetoothPbapPceObserver *observer) {
            observer->OnServiceConnectionStateChanged(remoteAddr, state);
        });
    }
    void OnServicePasswordRequired(
        const bluetooth::RawAddress &remoteAddr, const std::vector<uint8_t> &description, uint8_t charset, bool fullAccess) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([remoteAddr, description, charset, fullAccess](IBluetoothPbapPceObserver *observer) {
            observer->OnServicePasswordRequired(remoteAddr, description, charset, fullAccess);
        });
    }
    void OnActionCompleted(const bluetooth::RawAddress &remoteAddr, int respCode,
        int actionType, const bluetooth::IPbapPhoneBookData &result) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([remoteAddr, respCode, actionType, result](IBluetoothPbapPceObserver *observer) {
            observer->OnActionCompleted(remoteAddr, respCode, actionType, BluetoothIPbapPhoneBookDataAdapter(result));
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothPbapPceObserver> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothPbapPceObserver> *observers_{nullptr};
};

struct BluetoothPbapPceServer::impl {
    impl();
    RemoteObserverList<IBluetoothPbapPceObserver> observers_;
    std::unique_ptr<PbapPceObserver> observerImp_ = {std::make_unique<PbapPceObserver>()};
    bluetooth::IProfilePbapPce *pbapPceService_{nullptr};
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_{nullptr};
};

class BluetoothPbapPceServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    SystemStateObserver(BluetoothPbapPceServer::impl *impl) : impl_(impl) {};
    ~SystemStateObserver() = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        switch (state) {
            case bluetooth::BTSystemState::ON: {
                bluetooth::IProfileManager *serviceManager = bluetooth::IProfileManager::GetInstance();
                if (serviceManager != nullptr) {
                    bluetooth::IProfile *profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PCE);
                    if (profileService) {
                        impl_->pbapPceService_ = (bluetooth::IProfilePbapPce *)profileService;
                        impl_->pbapPceService_->RegisterObserver(*(impl_->observerImp_));  // re regist observer
                    } else {
                        HILOGI("can't find PbapPceService from ProfileServiceManager!");
                    }
                } else {
                    HILOGI("can't find ProfileServiceManager!");
                }
            } break;
            case bluetooth::BTSystemState::OFF:
                impl_->pbapPceService_ = nullptr;
                break;
            default:
                break;
        }
    }

private:
    BluetoothPbapPceServer::impl *impl_{nullptr};
};

BluetoothPbapPceServer::impl::impl()
{
    systemStateObserver_ = std::make_unique<SystemStateObserver>(this);
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
}

BluetoothPbapPceServer::BluetoothPbapPceServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    bluetooth::IProfileManager *serviceManager = bluetooth::IProfileManager::GetInstance();
    if (serviceManager != nullptr) {
        bluetooth::IProfile *profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PCE);
        if (profileService) {
            pimpl->pbapPceService_ = (bluetooth::IProfilePbapPce *)profileService;
            // register service callback
            pimpl->pbapPceService_->RegisterObserver(*(pimpl->observerImp_));
        } else {
            HILOGE("can't find PbapPceService from ProfileServiceManager!");
        }
    } else {
        HILOGE("can't find ProfileServiceManager!");
    }
}

BluetoothPbapPceServer::~BluetoothPbapPceServer()
{}

int BluetoothPbapPceServer::GetDeviceState(const BluetoothRawAddress &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->GetDeviceState(device);
    } else {
        HILOGE("BluetoothPbapPceServer::GetDeviceState: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::Connect(const BluetoothRawAddress &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->Connect(device);
    } else {
        HILOGE("BluetoothPbapPceServer::Connect: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::Disconnect(const BluetoothRawAddress &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->Disconnect(device);
    } else {
        HILOGE("BluetoothPbapPceServer::Disconnect: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::PullPhoneBook(const BluetoothRawAddress &device,
    const BluetoothIPbapPullPhoneBookParam &param)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->PullPhoneBook(device, IPbapPullPhoneBookParamAdapter(param));
    } else {
        HILOGE("BluetoothPbapPceServer::PullPhoneBook: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::SetPhoneBook(const BluetoothRawAddress &device, const std::u16string &name, int32_t flag)
{
    HILOGD("[%{public}s]: %{public}s(): Enter! flag: %{public}d", __FILE__, __FUNCTION__, flag);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->SetPhoneBook(device, name.c_str(), flag);
    } else {
        HILOGE("BluetoothPbapPceServer::SetPhoneBook: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::PullvCardListing(const BluetoothRawAddress &device,
    const BluetoothIPbapPullvCardListingParam &param)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->PullvCardListing(device, IPbapPullvCardListingParamAdapter(param));
    } else {
        HILOGE("BluetoothPbapPceServer::PullvCardListing: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::PullvCardEntry(const BluetoothRawAddress &device,
    const BluetoothIPbapPullvCardEntryParam &param)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->PullvCardEntry(device, IPbapPullvCardEntryParamAdapter(param));
    } else {
        HILOGE("BluetoothPbapPceServer::PullvCardEntry: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

bool BluetoothPbapPceServer::IsDownloading(const BluetoothRawAddress &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->IsDownloading(device);
    } else {
        HILOGE("BluetoothPbapPceServer::IsDownloading: pimpl->pbapPceService_ null");
        return false;
    }
}

int BluetoothPbapPceServer::AbortDownloading(const BluetoothRawAddress &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->AbortDownloading(device);
    } else {
        HILOGE("BluetoothPbapPceServer::AbortDownloading: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::SetDevicePassword(const BluetoothRawAddress &device,
    const std::string &password, const std::string &userId)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        std::string pwdTmp = password;
        std::string usrIdTmp = userId;
        return pimpl->pbapPceService_->SetDevicePassword(device, pwdTmp, usrIdTmp);
    } else {
        HILOGE("BluetoothPbapPceServer::SetDevicePassword: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::SetConnectionStrategy(const BluetoothRawAddress &device, int32_t strategy)
{
    HILOGD("[%{public}s]: %{public}s(): Enter! strategy: %{public}d" , __FILE__, __FUNCTION__, strategy);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->SetConnectionStrategy(device, strategy);
    } else {
        HILOGE("BluetoothPbapPceServer::SetConnectionStrategy: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

int BluetoothPbapPceServer::GetConnectionStrategy(const BluetoothRawAddress &device)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_) {
        return pimpl->pbapPceService_->GetConnectionStrategy(device);
    } else {
        HILOGE("BluetoothPbapPceServer::GetConnectionStrategy: pimpl->pbapPceService_ null");
        return RET_BAD_STATUS;
    }
}

void BluetoothPbapPceServer::GetDevicesByStates(const ::std::vector<int32_t> states,
    std::vector<BluetoothRawAddress> &rawDevices)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->pbapPceService_ != nullptr) {
        std::vector<bluetooth::RawAddress> serviceDeviceList = pimpl->pbapPceService_->GetDevicesByStates(states);
        for (auto &device : serviceDeviceList) {
            BluetoothRawAddress bluetoothDevice(device.GetAddress());
            rawDevices.push_back(bluetoothDevice);
        }
        return;
    } else {
        HILOGE("BluetoothPbapPceServer::GetDevicesByStates: pimpl->pbapPceService_ null");
        return;
    }
}

::std::vector<BluetoothRawAddress> BluetoothPbapPceServer::GetConnectedDevices()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    ::std::vector<BluetoothRawAddress> bluetoothDeviceList;
    if (pimpl->pbapPceService_ != nullptr) {
        const ::std::vector<int32_t> states{static_cast<int32_t>(BTConnectState::CONNECTED)};
        std::vector<bluetooth::RawAddress> serviceDeviceList = pimpl->pbapPceService_->GetDevicesByStates(states);
        for (auto &device : serviceDeviceList) {
            BluetoothRawAddress bluetoothDevice(device.GetAddress());
            bluetoothDeviceList.push_back(bluetoothDevice);
        }
        return bluetoothDeviceList;
    } else {
        HILOGE("BluetoothPbapPceServer::GetConnectedDevices: pimpl->pbapPceService_ null");
        return bluetoothDeviceList;
    }
}

void BluetoothPbapPceServer::RegisterObserver(const sptr<IBluetoothPbapPceObserver> &observer)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!observer) {
        HILOGE("RegisterObserver called with NULL observer. Ignoring.");
        return;
    }
    pimpl->observers_.Register(observer);
}

void BluetoothPbapPceServer::DeregisterObserver(const sptr<IBluetoothPbapPceObserver> &observer)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (!observer) {
        HILOGE("DeregisterObserver called with NULL observer. Ignoring.");
        return;
    }
    pimpl->observers_.Deregister(observer);
}
}  // namespace Bluetooth
}  // namespace OHOS