/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_server_pbap_pce"
#endif

#include "bluetooth_pbap_pce_server.h"

#include "ipc_skeleton.h"

#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "interface_adapter_manager.h"
#include "i_bluetooth_pbap_pce.h"
#include "interface_profile_manager.h"
#include "interface_profile_pbap_pce.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"
#include "bt_def.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

struct BluetoothPbapPceServer::impl {
    impl();
    RemoteObserverList<IBluetoothPbapPceObserver> observers_;

    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_{nullptr};

    class PbapPceObserver;
    std::unique_ptr<PbapPceObserver> observerImp_{nullptr};

    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;

    bluetooth::IProfilePbapPce* pbapPceService_{nullptr};
};

class BluetoothPbapPceServer::impl::PbapPceObserver : public bluetooth::IPbapPceObserver {
public:
    explicit PbapPceObserver(BluetoothPbapPceServer::impl *pimpl) : pimpl_(pimpl) {};
    ~PbapPceObserver() override = default;
    void OnConnectionStateChanged(const bluetooth::RawAddress& device, int state, int cause) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr:%{public}s, state:%{public}d, cause:%{public}d", GET_ENCRYPT_ADDR(device), state, cause);
        observers_->ForEach([this, device, state, cause](IBluetoothPbapPceObserver* observer) {
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

    void OnActionCompleted(const bluetooth::RawAddress &remoteAddr, int respCode, int actionType) override
    {
        HILOGI("respCode: %{public}d, actionType: %{public}d", respCode, actionType);
        observers_->ForEach([remoteAddr, respCode, actionType](IBluetoothPbapPceObserver *observer) {
            observer->OnActionCompleted(remoteAddr, respCode, actionType);
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothPbapPceObserver>* observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothPbapPceObserver>* observers_;
    BluetoothPbapPceServer::impl *pimpl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(PbapPceObserver);
};

class BluetoothPbapPceServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothPbapPceServer::impl* impl) : impl_(impl) {};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        HILOGI("OnSystemStateChange state=%{public}d", static_cast<int>(state));
        switch (state) {
            case bluetooth::BTSystemState::ON: {
                bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
                CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");
                bluetooth::IProfile* profileService =
                    serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PCE);
                CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find PbapPceService");
                impl_->pbapPceService_ = static_cast<bluetooth::IProfilePbapPce*>(profileService);
                impl_->pbapPceService_->RegisterObserver(*impl_->observerImp_);
                break;
            }
            case bluetooth::BTSystemState::OFF:
                impl_->pbapPceService_ = nullptr;
                break;
            default:
                break;
            }
    }

private:
    BluetoothPbapPceServer::impl* impl_;
};

BluetoothPbapPceServer::impl::impl()
{
    systemStateObserver_ = std::make_unique<SystemStateObserver>(this);
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothPbapPceServer::BluetoothPbapPceServer()
{
    HILOGI("BluetoothPbapPceServer!");
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_ = std::make_unique<impl::PbapPceObserver>(pimpl.get());
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));

    bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");

    bluetooth::IProfile* profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_PBAP_PCE);
    CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find PbapPceService");

    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    pimpl->pbapPceService_ = (bluetooth::IProfilePbapPce*)profileService;
    pimpl->pbapPceService_->RegisterObserver(*pimpl->observerImp_);
}

BluetoothPbapPceServer::~BluetoothPbapPceServer() {}

int32_t BluetoothPbapPceServer::GetPhoneBookSyncState(const BluetoothRawAddress &device)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPceService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPceService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->pbapPceService_->GetPhoneBookSyncState(realAddr);
}

int BluetoothPbapPceServer::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPceService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPceService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    state = pimpl->pbapPceService_->GetDeviceState(realAddr);
    HILOGD("state : %{public}d", state);
    return BT_NO_ERROR;
}

int BluetoothPbapPceServer::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress> &rawDevices)
{
    HILOGI("Enter!");
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPceService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPceService_ is null");

    auto devices = pimpl->pbapPceService_->GetDevicesByStates(states);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(devices, rawDevices);
    return BT_NO_ERROR;
}

int BluetoothPbapPceServer::Connect(const BluetoothRawAddress& device)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPceService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPceService_ is null");

    return pimpl->pbapPceService_->Connect(realAddr);
}

int BluetoothPbapPceServer::Disconnect(const BluetoothRawAddress& device)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPceService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPceService_ is null");

    return pimpl->pbapPceService_->Disconnect(realAddr);
}

int BluetoothPbapPceServer::SetConnectionStrategy(const BluetoothRawAddress& device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), strategy);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPceService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPceService_ is null");

    return pimpl->pbapPceService_->SetConnectionStrategy(realAddr, strategy);
}

int BluetoothPbapPceServer::GetConnectionStrategy(const BluetoothRawAddress& device, int32_t &strategy)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->pbapPceService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or pbapPceService_ is null");

    strategy = pimpl->pbapPceService_->GetConnectionStrategy(realAddr);
    return BT_NO_ERROR;
}

void BluetoothPbapPceServer::RegisterObserver(const sptr<IBluetoothPbapPceObserver>& observer)
{
    HILOGI("Enter!");
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ or appContainer_ too much");
        return;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
}

void BluetoothPbapPceServer::DeregisterObserver(const sptr<IBluetoothPbapPceObserver>& observer)
{
    HILOGI("Enter!");
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");

    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
}

}  // namespace Bluetooth
}  // namespace OHOS