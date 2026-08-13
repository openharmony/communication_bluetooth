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
#ifndef LOG_TAG
#define LOG_TAG "bt_server_pan"
#endif

#include "bluetooth_pan_server.h"

#include "bluetooth_errorcode.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_utils_server.h"
#include "interface_profile.h"
#include "interface_profile_pan.h"
#include "i_bluetooth_host_observer.h"
#include "remote_observer_list.h"
#include "hilog/log.h"
#include "ipc_skeleton.h"
#include "safe_map.h"
#include "hitrace_meter.h"
#include "permission_manager.h"
#include "common_util.h"
#include "bluetooth_observer_application_container.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothPanServer::impl {
    impl();
    ~impl();

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;
    class BluetoothPanCallback;
    std::unique_ptr<BluetoothPanCallback> observerImp_ = nullptr;

    RemoteObserverList<IBluetoothPanObserver> observers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;
    IProfilePan *panService_ = nullptr;

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
        HITRACE_METER(BT_TRACE_TAG);
        switch (state) {
            case BTSystemState::ON:
                pimpl_->panService_ = pimpl_->GetServicePtr();
                if (pimpl_->panService_ != nullptr) {
                    pimpl_->panService_->RegisterObserver(
                        *reinterpret_cast<IPanObserver *>(pimpl_->observerImp_.get()));
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

class BluetoothPanServer::impl::BluetoothPanCallback : public IPanObserver {
public:
    BluetoothPanCallback(BluetoothPanServer::impl *pimpl) : pimpl_(pimpl) {}
    ~BluetoothPanCallback() override = default;

    void OnConnectionStateChanged(const bluetooth::RawAddress &device, int state, int cause, int role) override
    {
        HILOGI("addr:%{public}s, state:%{public}d, cause:%{public}d", GET_ENCRYPT_ADDR(device), state, cause);
        observers_->ForEach([this, device, state, cause, role](sptr<IBluetoothPanObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            if (PermissionManager::IsNativeCaller(tokenId) ||
                PermissionManager::GetApiVersion(tokenId) >= API_VERSION_10) {
                CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            }
            bluetooth::RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr, state, cause, role);
            }
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothPanObserver> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothPanObserver> *observers_ = nullptr;
    BluetoothPanServer::impl *pimpl_ = nullptr;
};

BluetoothPanServer::impl::impl()
{
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothPanServer::impl::~impl()
{
}

BluetoothPanServer::BluetoothPanServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_ = std::make_unique<impl::BluetoothPanCallback>(pimpl.get());
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
    CHECK_AND_RETURN_LOG_RET(observer, BT_ERR_INTERNAL_ERROR, "observer is null");
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ or appContainer_ too much");
        return BT_ERR_INTERNAL_ERROR;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
    return BT_NO_ERROR;
}

ErrCode BluetoothPanServer::DeregisterObserver(const sptr<IBluetoothPanObserver> observer)
{
    CHECK_AND_RETURN_LOG_RET(observer, BT_ERR_INTERNAL_ERROR, "observer is null");
    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
    pimpl->panService_->DeregisterObserver(*pimpl->observerImp_.get());
    return BT_NO_ERROR;
}

int32_t BluetoothPanServer::GetDevicesByStates(const std::vector<int32_t> &states,
    std::vector<BluetoothRawAddress>& result)
{
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->panService_, BT_ERR_INTERNAL_ERROR, "not init");
    std::vector<bluetooth::RawAddress> serviceDeviceList = pimpl->panService_->GetDevicesByStates(states);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(serviceDeviceList, result);
    return BT_NO_ERROR;
}

int32_t BluetoothPanServer::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->panService_, BT_ERR_INTERNAL_ERROR, "not init");
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(RawAddress(device));
    state = pimpl->panService_->GetDeviceState(realAddr);
    HILOGI("addr:%{public}s, res:%{public}d", GET_ENCRYPT_ADDR(device), state);
    return BT_NO_ERROR;
}

int32_t BluetoothPanServer::Connect(const BluetoothRawAddress &device)
{
    HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->panService_, BT_ERR_INTERNAL_ERROR, "not init");
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(RawAddress(device));
    return pimpl->panService_->Connect(realAddr);
}

int32_t BluetoothPanServer::Disconnect(const BluetoothRawAddress &device)
{
    HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->panService_, BT_ERR_INTERNAL_ERROR, "not init");
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(RawAddress(device));
    return pimpl->panService_->Disconnect(realAddr);
}

int32_t BluetoothPanServer::SetTethering(const bool enable)
{
    HILOGI("enable:%{public}d", enable);
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->panService_, BT_ERR_INTERNAL_ERROR, "not init");
    return pimpl->panService_->SetTethering(enable);
}

int32_t BluetoothPanServer::IsTetheringOn(bool& result)
{
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->panService_, BT_ERR_INTERNAL_ERROR, "not init");
    result = pimpl->panService_->IsTetheringOn();
    HILOGI("IsTetheringOn:%{public}d", result);
    return BT_NO_ERROR;
}

int32_t BluetoothPanServer::SetConnectStrategy(const BluetoothRawAddress &device, int strategy)
{
    HILOGI("addr: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device.GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->panService_, BT_ERR_INTERNAL_ERROR, "not init");
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(RawAddress(device));
    return pimpl->panService_->SetConnectStrategy(realAddr, strategy);
}

int32_t BluetoothPanServer::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(device.GetAddress()), BT_ERR_INVALID_PARAM, "addr is invalid.");
    CHECK_AND_RETURN_LOG_RET(pimpl && pimpl->panService_, BT_ERR_INTERNAL_ERROR, "not init");
    RawAddress realAddr = BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(RawAddress(device));
    strategy = pimpl->panService_->GetConnectStrategy(realAddr);
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
