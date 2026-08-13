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
#define LOG_TAG "bt_server_map_mse"
#endif

#include "bluetooth_map_mse_server.h"

#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "i_bluetooth_map_mse.h"
#include "interface_adapter_manager.h"
#include "interface_profile_manager.h"
#include "interface_profile_map_mse.h"
#include "ipc_skeleton.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
struct BluetoothMapMseServer::impl {
    impl();
    RemoteObserverList<IBluetoothMapMseObserver> observers_;

    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_{nullptr};

    class MapMseServiceObserver;
    std::unique_ptr<MapMseServiceObserver> serviceObserverImp_{nullptr};

    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;

    bluetooth::IProfileMapMse* mapMseService_{nullptr};
};

class BluetoothMapMseServer::impl::MapMseServiceObserver : public bluetooth::IMapMseObserver {
public:
    explicit MapMseServiceObserver(BluetoothMapMseServer::impl *pimpl) : pimpl_(pimpl) {};
    ~MapMseServiceObserver() override = default;
    void OnConnectionStateChanged(const bluetooth::RawAddress& device, int state, int cause) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr:%{public}s, state:%{public}d, cause:%{public}d", GET_ENCRYPT_ADDR(device), state, cause);
        observers_->ForEach([this, device, state, cause](IBluetoothMapMseObserver* observer) {
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

    void SetObserver(RemoteObserverList<IBluetoothMapMseObserver>* observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothMapMseObserver>* observers_;
    BluetoothMapMseServer::impl *pimpl_ = nullptr;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(MapMseServiceObserver);
};

class BluetoothMapMseServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothMapMseServer::impl* impl) : impl_(impl) {};
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
                CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find MapMseService");
                impl_->mapMseService_ = static_cast<bluetooth::IProfileMapMse*>(profileService);
                impl_->mapMseService_->RegisterObserver(*impl_->serviceObserverImp_);
                break;
            }
            case bluetooth::BTSystemState::OFF:
                impl_->mapMseService_ = nullptr;
                break;
            default:
                break;
            }
    }

private:
    BluetoothMapMseServer::impl* impl_;
};

BluetoothMapMseServer::impl::impl()
{
    systemStateObserver_ = std::make_unique<SystemStateObserver>(this);
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*systemStateObserver_);
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothMapMseServer::BluetoothMapMseServer()
{
    HILOGI("BluetoothMapMseServer!");
    pimpl = std::make_unique<impl>();
    pimpl->serviceObserverImp_ = std::make_unique<impl::MapMseServiceObserver>(pimpl.get());
    pimpl->serviceObserverImp_->SetObserver(&(pimpl->observers_));

    bluetooth::IProfileManager* serviceManager = bluetooth::IProfileManager::GetInstance();
    CHECK_AND_RETURN_LOG((serviceManager != nullptr), "can't find ProfileServiceManager");

    bluetooth::IProfile* profileService = serviceManager->GetProfileService(bluetooth::PROFILE_NAME_MAP_MSE);
    CHECK_AND_RETURN_LOG((profileService != nullptr), "can't find Service");

    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");
    pimpl->mapMseService_ = (bluetooth::IProfileMapMse*)profileService;
    pimpl->mapMseService_->RegisterObserver(*pimpl->serviceObserverImp_);
}

BluetoothMapMseServer::~BluetoothMapMseServer() {}

int BluetoothMapMseServer::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->mapMseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or mapMseService_ is null");

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    state = pimpl->mapMseService_->GetDeviceState(realAddr);
    HILOGD("state : %{public}d", state);
    return BT_NO_ERROR;
}

int BluetoothMapMseServer::GetDevicesByStates(
    const std::vector<int32_t> &tmpStates, std::vector<BluetoothRawAddress> &rawDevices)
{
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->mapMseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or mapMseService_ is null");

    auto devices = pimpl->mapMseService_->GetDevicesByStates(tmpStates);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(devices, rawDevices);
    return BT_NO_ERROR;
}

int BluetoothMapMseServer::Disconnect(const BluetoothRawAddress& device)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->mapMseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or mapMseService_ is null");

    return pimpl->mapMseService_->Disconnect(realAddr);
}

int BluetoothMapMseServer::SetConnectionStrategy(const BluetoothRawAddress& device, int32_t strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), strategy);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->mapMseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or mapMseService_ is null");

    return pimpl->mapMseService_->SetConnectionStrategy(realAddr, strategy);
}

int BluetoothMapMseServer::GetConnectionStrategy(const BluetoothRawAddress& device, int32_t &strategy)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->mapMseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or mapMseService_ is null");

    strategy = pimpl->mapMseService_->GetConnectionStrategy(realAddr);
    return BT_NO_ERROR;
}

int BluetoothMapMseServer::SetMessageAccessAuthorization(const BluetoothRawAddress& device,
    int32_t accessAuthorization)
{
    HILOGI("device: %{public}s, accessAuthorization: %{public}d",
        GetEncryptAddr((device).GetAddress()).c_str(), accessAuthorization);
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->mapMseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or mapMseService_ is null");

    return pimpl->mapMseService_->SetMessageAccessAuthorization(realAddr, accessAuthorization);
}

int BluetoothMapMseServer::GetMessageAccessAuthorization(const BluetoothRawAddress& device,
    int32_t &accessAuthorization)
{
    HILOGI("device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET((pimpl != nullptr && pimpl->mapMseService_ != nullptr),
        BT_ERR_INTERNAL_ERROR, "pimpl or mapMseService_ is null");

    return pimpl->mapMseService_->GetMessageAccessAuthorization(realAddr, accessAuthorization);
}

void BluetoothMapMseServer::RegisterObserver(const sptr<IBluetoothMapMseObserver>& observer)
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

void BluetoothMapMseServer::DeregisterObserver(const sptr<IBluetoothMapMseObserver>& observer)
{
    CHECK_AND_RETURN_LOG((observer != nullptr), "observer is null");
    CHECK_AND_RETURN_LOG((pimpl != nullptr), "pimpl is null");

    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
}

}  // namespace Bluetooth
}  // namespace OHOS