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
#ifndef LOG_TAG
#define LOG_TAG "bt_server_hid_host"
#endif

#include "bluetooth_hid_host_server.h"

#include "ipc_skeleton.h"

#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "hilog/log.h"
#include "interface_profile.h"
#include "interface_profile_hid_host.h"
#include "interface_adapter_manager.h"
#include "interface_profile_manager.h"
#include "i_bluetooth_host_observer.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothHidHostServer::impl {
    impl();
    ~impl();

    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    class BluetoothHidHostCallback;
    std::unique_ptr<BluetoothHidHostCallback> observerImp_ = nullptr;

    RemoteObserverList<IBluetoothHidHostObserver> observers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;
    IProfileHidHost *hidHostService_ = nullptr;

    IProfileHidHost *GetServicePtr()
    {
        if (IProfileManager::GetInstance() == nullptr) {
            return nullptr;
        }
        return static_cast<IProfileHidHost *>(
            IProfileManager::GetInstance()->GetProfileService(PROFILE_NAME_HID_HOST));
    }
};

class BluetoothHidHostServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothHidHostServer::impl *pimpl) : pimpl_(pimpl) {};
    void OnSystemStateChange(const BTSystemState state) override
    {
        HILOGI("BTSystemState:%{public}d", state);
        switch (state) {
            case BTSystemState::ON:
                pimpl_->hidHostService_ = pimpl_->GetServicePtr();
                if (pimpl_->hidHostService_ != nullptr) {
                    pimpl_->hidHostService_->RegisterObserver(
                        *reinterpret_cast<bluetooth::IHidHostObserver *>(pimpl_->observerImp_.get()));
                }
                break;
            case BTSystemState::OFF:
                pimpl_->hidHostService_ = nullptr;
                break;
            default:
                break;
        }
    };

private:
    BluetoothHidHostServer::impl *pimpl_ = nullptr;
};

class BluetoothHidHostServer::impl::BluetoothHidHostCallback : public bluetooth::IHidHostObserver {
public:
    explicit BluetoothHidHostCallback(BluetoothHidHostServer::impl *pimpl) : pimpl_(pimpl) {};
    ~BluetoothHidHostCallback() override = default;

    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        // Reference "BTConnectState"
        HILOGI("addr:%{public}s, state:%{public}d, cause:%{public}d", GET_ENCRYPT_ADDR(device), state, cause);
        observers_->ForEach([this, device, state, cause](sptr<IBluetoothHidHostObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            if (PermissionManager::IsNativeCaller(tokenId) ||
                PermissionManager::GetApiVersion(tokenId) >= API_VERSION_10) {
                CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            }
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr, state, cause);
            }
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothHidHostObserver> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothHidHostObserver> *observers_;
    BluetoothHidHostServer::impl *pimpl_ = nullptr;
};

BluetoothHidHostServer::impl::impl()
{
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothHidHostServer::impl::~impl()
{
}

BluetoothHidHostServer::BluetoothHidHostServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_ = std::make_unique<impl::BluetoothHidHostCallback>(pimpl.get());
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    pimpl->systemStateObserver_ = std::make_unique<impl::SystemStateObserver>(pimpl.get());
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));

    pimpl->hidHostService_ = pimpl->GetServicePtr();
    if (pimpl->hidHostService_ != nullptr) {
        pimpl->hidHostService_->RegisterObserver(*pimpl->observerImp_.get());
    }
}

BluetoothHidHostServer::~BluetoothHidHostServer()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_));
    if (pimpl->hidHostService_ != nullptr) {
        pimpl->hidHostService_->DeregisterObserver(*pimpl->observerImp_.get());
    }
}

ErrCode BluetoothHidHostServer::RegisterObserver(const sptr<IBluetoothHidHostObserver> observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return ERR_INVALID_VALUE;
    }
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return ERR_NO_INIT;
    }
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ or appContainer_ too much");
        return ERR_INVALID_VALUE;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
    return ERR_OK;
}

ErrCode BluetoothHidHostServer::DeregisterObserver(const sptr<IBluetoothHidHostObserver> observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return ERR_INVALID_VALUE;
    }
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return ERR_NO_INIT;
    }
    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
    return ERR_OK;
}

int32_t BluetoothHidHostServer::GetDevicesByStates(
    const std::vector<int32_t> &states, std::vector<BluetoothRawAddress>& result)
{
#ifndef BLUETOOTH_HID_HOST_FEATURE_ENABLE
    HILOGI("hid host not support");
    return BT_NO_ERROR;
#endif
    if (pimpl == nullptr || pimpl->hidHostService_ == nullptr) {
        HILOGI("hidHostService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::vector<bluetooth::RawAddress> serviceDeviceList = pimpl->hidHostService_->GetDevicesByStates(states);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(serviceDeviceList, result);
    return BT_NO_ERROR;
}

int32_t BluetoothHidHostServer::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
    if (pimpl == nullptr || pimpl->hidHostService_ == nullptr) {
        HILOGI("hidHostService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    state = pimpl->hidHostService_->GetDeviceState(realAddr);
    HILOGD("result:%{public}d", state);
    return BT_NO_ERROR;
}

int32_t BluetoothHidHostServer::Connect(const BluetoothRawAddress &device)
{
    HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
    if (pimpl == nullptr || pimpl->hidHostService_ == nullptr) {
        HILOGE("hidHostService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->hidHostService_->Connect(realAddr);
}

int32_t BluetoothHidHostServer::Disconnect(const BluetoothRawAddress &device)
{
    HILOGI("addr:%{public}s", GET_ENCRYPT_ADDR(device));
    if (pimpl == nullptr || pimpl->hidHostService_ == nullptr) {
        HILOGI("hidHostService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->hidHostService_->Disconnect(realAddr);
}

ErrCode BluetoothHidHostServer::HidHostVCUnplug(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    if (pimpl == nullptr || pimpl->hidHostService_ == nullptr) {
        HILOGI("hidHostService_ is null");
        return ERR_NO_INIT;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    result = pimpl->hidHostService_->HidHostVCUnplug(realAddr.GetAddress(), id, size, type);
    HILOGI("result:%{public}d", result);
    return ERR_OK;
}

ErrCode BluetoothHidHostServer::HidHostSendData(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    if (pimpl == nullptr || pimpl->hidHostService_ == nullptr) {
        HILOGI("hidHostService_ is null");
        return ERR_NO_INIT;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    result = pimpl->hidHostService_->HidHostSendData(realAddr.GetAddress(), id, size, type);
    HILOGI("result:%{public}d", result);
    return ERR_OK;
}

ErrCode BluetoothHidHostServer::HidHostSetReport(std::string &device,
    uint8_t &type, std::string &report, int& result)
{
    if (pimpl == nullptr || pimpl->hidHostService_ == nullptr) {
        HILOGI("hidHostService_ is null");
        return ERR_NO_INIT;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<uint8_t> data(report.begin(), report.end());
    data.emplace(data.end(), static_cast<uint8_t>('\0'));
    result = pimpl->hidHostService_->HidHostSetReport(realAddr.GetAddress(), type, data.size(), data.data());
    return ERR_OK;
}

ErrCode BluetoothHidHostServer::HidHostGetReport(std::string &device,
    uint8_t &id, uint16_t &size, uint8_t &type, int& result)
{
    if (pimpl == nullptr || pimpl->hidHostService_ == nullptr) {
        HILOGI("hidHostService_ is null");
        return ERR_NO_INIT;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(RawAddress(device), realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    result = pimpl->hidHostService_->HidHostGetReport(realAddr.GetAddress(), id, size, type);
    HILOGI("result:%{public}d", result);
    return ERR_OK;
}

int32_t BluetoothHidHostServer::SetConnectStrategy(const BluetoothRawAddress &device, int strategy)
{
    HILOGI("addr: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int res = BT_ERR_INTERNAL_ERROR;
    if (pimpl->hidHostService_) {
        res = pimpl->hidHostService_->SetConnectStrategy(realAddr, strategy);
    }
    return res;
}

int32_t BluetoothHidHostServer::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->hidHostService_) {
        strategy = pimpl->hidHostService_->GetConnectStrategy(realAddr);
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}
}  // namespace Bluetooth
}  // namespace OHOS
