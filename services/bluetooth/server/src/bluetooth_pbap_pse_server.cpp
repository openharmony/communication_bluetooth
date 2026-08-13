/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_server_pbap_pse"
#endif

#include "bluetooth_pbap_pse_server.h"

#include "ipc_skeleton.h"

#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "interface_adapter_manager.h"
#include "i_bluetooth_pbap_pse.h"
#include "interface_profile_manager.h"
#include "interface_profile_pbap_pse.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
struct BluetoothPbapPseServer::impl {
    impl();
    RemoteObserverList<IBluetoothPbapPseObserver> observers_;

    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_{nullptr};

    class PbapPseObserver;
    std::unique_ptr<PbapPseObserver> observerImp_{nullptr};

    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;

    bluetooth::IProfilePbapPse* pbapPseService_{nullptr};
};

class BluetoothPbapPseServer::impl::PbapPseObserver : public bluetooth::IPbapPseObserver {
public:
    explicit PbapPseObserver(BluetoothPbapPseServer::impl *pimpl) : pimpl_(pimpl) {};
    ~PbapPseObserver() override = default;
    void OnConnectionStateChanged(const bluetooth::RawAddress& device, int state, int cause) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr:%{public}s, state:%{public}d, cause:%{public}d", GET_ENCRYPT_ADDR(device), state, cause);
        observers_->ForEach([this, device, state, cause](IBluetoothPbapPseObserver* observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr, state, cause);
            }
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothPbapPseObserver>* observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothPbapPseObserver>* observers_;
    BluetoothPbapPseServer::impl *pimpl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PbapPseObserver);
};

class BluetoothPbapPseServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothPbapPseServer::impl* impl) : impl_(impl) {};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        HILOGI("OnSystemStateChange state=%{public}d", static_cast<int>(state));
        switch (state) {
            case bluetooth::BTSystemState::ON: {
                bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
                CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");
                bluetooth::IProfile* profileService =
                    serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PSE);
                CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find PbapPseService");
                impl_->pbapPseService_ = static_cast<bluetooth::IProfilePbapPse*>(profileService);
                impl_->pbapPseService_->RegisterObserver(*impl_->observerImp_);
                break;
            }
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
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothPbapPseServer::BluetoothPbapPseServer()
{
    HILOGI("BluetoothPbapPseServer!");
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_ = std::make_unique<impl::PbapPseObserver>(pimpl.get());
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));

    bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");

    bluetooth::IProfile* profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PSE);
    CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find PbapPseService");

    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    pimpl->pbapPseService_ = (bluetooth::IProfilePbapPse*)profileService;
    pimpl->pbapPseService_->RegisterObserver(*pimpl->observerImp_);
}

BluetoothPbapPseServer::~BluetoothPbapPseServer() {}

int BluetoothPbapPseServer::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    state = pimpl->pbapPseService_->GetDeviceState(realAddr);
    HILOGD("state : %{public}d", state);
    return BT_NO_ERROR;
}

int BluetoothPbapPseServer::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress> &rawDevices)
{
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    auto devices = pimpl->pbapPseService_->GetDevicesByStates(states);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(devices, rawDevices);
    return BT_NO_ERROR;
}

int BluetoothPbapPseServer::Disconnect(const BluetoothRawAddress& device)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    return pimpl->pbapPseService_->Disconnect(realAddr);
}

int BluetoothPbapPseServer::SetConnectionStrategy(const BluetoothRawAddress& device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), strategy);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    return pimpl->pbapPseService_->SetConnectionStrategy(realAddr, strategy);
}

int BluetoothPbapPseServer::GetConnectionStrategy(const BluetoothRawAddress& device, int32_t &strategy)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    strategy = pimpl->pbapPseService_->GetConnectionStrategy(realAddr);
    return BT_NO_ERROR;
}

int BluetoothPbapPseServer::SetShareType(const BluetoothRawAddress& device, int32_t shareType)
{
    HILOGI("device: %{public}s, shareType: %{public}d",
        GetEncryptAddr((device).GetAddress()).c_str(), shareType);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    return pimpl->pbapPseService_->SetShareType(realAddr, shareType);
}

int BluetoothPbapPseServer::GetShareType(const BluetoothRawAddress& device, int32_t &shareType)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    return pimpl->pbapPseService_->GetShareType(realAddr, shareType);
}

int BluetoothPbapPseServer::SetPhoneBookAccessAuthorization(const BluetoothRawAddress& device,
    int32_t accessAuthorization)
{
    HILOGI("device: %{public}s, accessAuthorization: %{public}d",
        GetEncryptAddr((device).GetAddress()).c_str(), accessAuthorization);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    return pimpl->pbapPseService_->SetPhoneBookAccessAuthorization(realAddr, accessAuthorization);
}

int BluetoothPbapPseServer::GetPhoneBookAccessAuthorization(const BluetoothRawAddress& device,
    int32_t &accessAuthorization)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPseService_ is null");

    return pimpl->pbapPseService_->GetPhoneBookAccessAuthorization(realAddr, accessAuthorization);
}

void BluetoothPbapPseServer::RegisterObserver(const sptr<IBluetoothPbapPseObserver>& observer)
{
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ or appContainer_ too much");
        return;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
}

void BluetoothPbapPseServer::DeregisterObserver(const sptr<IBluetoothPbapPseObserver>& observer)
{
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");

    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
}

}  // namespace Bluetooth
}  // namespace OHOS