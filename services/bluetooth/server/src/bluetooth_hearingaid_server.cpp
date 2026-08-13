/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_server_hearingaid"
#endif

#ifdef STUB_FUNC
#undef STUB_FUNC
#endif
#define STUB_FUNC(code, perm) IBluetoothHearingAidIpcCode::code, perm
#define ACCESS_MANAGE_PERM CHECK_PERM(true, {}, MULTI_PERM(ACCESS_BLUETOOTH, MANAGE_BLUETOOTH))

#include "bluetooth_hearingaid_server.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "common_util.h"

#include "interface_profile_hearing_aid.h"
#include "bluetooth_utils_server.h"

#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "interface_profile_manager.h"
#include "interface_adapter_manager.h"
#include "remote_observer_list.h"
#include "bluetooth_device_manager.h"

namespace OHOS {
namespace Bluetooth {

struct BluetoothHearingAidServer::impl {
    impl();
    ~impl();

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    class HearingAidObserver;
    std::unique_ptr<HearingAidObserver> observerImp_ = nullptr;

    RemoteObserverList<IBluetoothHearingAidObserver> observers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;
    bluetooth::IProfileHearingAid *hearingAidService_ = nullptr;
};

class BluetoothHearingAidServer::impl::SystemStateObserver : public bluetooth::ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothHearingAidServer::impl *pimpl) : pimpl_(pimpl) {};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const bluetooth::BTSystemState state) override
    {
        bluetooth::IProfileManager *serviceMgr = bluetooth::IProfileManager::GetInstance();
        if (!pimpl_) {
            HILOGI("failed: pimpl_ is null");
            return;
        }

        switch (state) {
            case bluetooth::BTSystemState::ON:
                if (serviceMgr != nullptr) {
                    pimpl_->hearingAidService_ = (bluetooth::IProfileHearingAid *)serviceMgr->GetProfileService(
                        bluetooth::PROFILE_NAME_HEARING_AID);
                    if (pimpl_->hearingAidService_ != nullptr) {
                        pimpl_->hearingAidService_->RegisterObserver(
                            *reinterpret_cast<bluetooth::IHearingAidObserver *>(pimpl_->observerImp_.get()));
                    }
                }
                break;
            case bluetooth::BTSystemState::OFF:
                pimpl_->hearingAidService_ = nullptr;
                break;
            default:
                break;
        }
    }

private:
    BluetoothHearingAidServer::impl *pimpl_ = nullptr;
};

class BluetoothHearingAidServer::impl::HearingAidObserver : public bluetooth::IHearingAidObserver {
public:
    explicit HearingAidObserver(BluetoothHearingAidServer::impl *pimpl) : pimpl_(pimpl) {};
    ~HearingAidObserver() override = default;

    void OnConnectionStateChanged(const bluetooth::RawAddress &remoteAddr, int state, int cause) override
    {
        HILOGI("addr: %{public}s, state: %{public}d, cause: %{public}d", GET_ENCRYPT_ADDR(remoteAddr), state, cause);
        observers_->ForEach([this, remoteAddr, state, cause](sptr<IBluetoothHearingAidObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            bluetooth::RawAddress randomAddr;
            if (bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                tokenId, remoteAddr, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr.GetAddress(), static_cast<ConnectionState>(state), cause);
            }
        });
    }

    void OnActiveDeviceChanged(const bluetooth::RawAddress &remoteAddr, const std::string& name) override
    {
        HILOGI("active addr: %{public}s", GET_ENCRYPT_ADDR(remoteAddr));
        observers_->ForEach([this, remoteAddr, name](sptr<IBluetoothHearingAidObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            HILOGI("OnActiveDeviceChanged enter real");
            bluetooth::RawAddress randomAddr;
            if (bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                tokenId, remoteAddr, randomAddr)) {
                observer->OnActiveDeviceChanged(randomAddr.GetAddress(), name);
            }
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothHearingAidObserver> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothHearingAidObserver> *observers_ = nullptr;
    BluetoothHearingAidServer::impl *pimpl_ = nullptr;
};

BluetoothHearingAidServer::impl::impl()
{
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothHearingAidServer::impl::~impl()
{
}

BluetoothHearingAidServer::BluetoothHearingAidServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->systemStateObserver_ = std::make_unique<impl::SystemStateObserver>(pimpl.get());
    pimpl->observerImp_ = std::make_unique<impl::HearingAidObserver>(pimpl.get());
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    bluetooth::IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));

    bluetooth::IProfileManager *serviceMgr = bluetooth::IProfileManager::GetInstance();
    if (serviceMgr != nullptr) {
        pimpl->hearingAidService_ =
            (bluetooth::IProfileHearingAid *)serviceMgr->GetProfileService(bluetooth::PROFILE_NAME_HEARING_AID);
        if (pimpl->hearingAidService_ != nullptr) {
            pimpl->hearingAidService_->RegisterObserver(*(pimpl->observerImp_.get()));
        }
    }
}

BluetoothHearingAidServer::~BluetoothHearingAidServer()
{
    bluetooth::IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_));
    if (pimpl->hearingAidService_ != nullptr) {
        pimpl->hearingAidService_->DeregisterObserver(*(pimpl->observerImp_.get()));
    }
}


const std::map<IBluetoothHearingAidIpcCode, std::shared_ptr<PermissionItem>>
    BluetoothHearingAidServer::permissionCheckMap_ = {
    {STUB_FUNC(COMMAND_IS_LOCAL_DEVICE_SUPPORT_HEARING_AID, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_CONNECT, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_DISCONNECT, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_SET_VOLUME, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_GET_HEARING_AID_DEVICE_INFO, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_GET_CONNECTED_DEVICES, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_GET_ACTIVE_DEVICE, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_GET_CONNECTION_STATE, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_SET_CONNECTION_STRATEGY, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_GET_CONNECTION_STRATEGY, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_REGISTER_OBSERVER, ACCESS_MANAGE_PERM)},
    {STUB_FUNC(COMMAND_DE_REGISTER_OBSERVER, ACCESS_MANAGE_PERM)},
};

ErrCode BluetoothHearingAidServer::IsLocalDeviceSupportHearingAid(bool& support)
{
    HILOGI("BluetoothHearingAidServer::IsLocalDeviceSupportHearingAid.");
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    support = pimpl->hearingAidService_->IsLe2MPhySupported();
    return BT_NO_ERROR;
}

ErrCode BluetoothHearingAidServer::Connect(const std::string& address)
{
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(address), BT_ERR_INVALID_PARAM, "addr is invalid.");
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }

    bluetooth::RawAddress realAddr;
    if (!bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(
            bluetooth::RawAddress(address), realAddr)) {
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    int32_t result = pimpl->hearingAidService_->Connect(realAddr);
    if (result != BT_NO_ERROR) {
        result = BT_OV_ERR_INTERNAL_ERROR;
    }
    return result;
}

ErrCode BluetoothHearingAidServer::Disconnect(const std::string& address)
{
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(address), BT_ERR_INVALID_PARAM, "addr is invalid.");
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }

    bluetooth::RawAddress realAddr;
    if (!bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(
            bluetooth::RawAddress(address), realAddr)) {
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    return pimpl->hearingAidService_->Disconnect(realAddr);
}

ErrCode BluetoothHearingAidServer::SetVolume(const std::string& address, int64_t volume)
{
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(address), BT_ERR_INVALID_PARAM, "addr is invalid.");
    CHECK_AND_RETURN_LOG_RET((volume >= VOLUME_MIN && volume <= VOLUME_MAX), BT_ERR_INVALID_PARAM, "volume invalid.");
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    bluetooth::RawAddress realAddr;
    if (!bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(
            bluetooth::RawAddress(address), realAddr)) {
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    return pimpl->hearingAidService_->SetVolumeByAddr(realAddr, volume);
}

ErrCode BluetoothHearingAidServer::GetHearingAidDeviceInfo(const std::string& address,
    DeviceSide& side, DeviceMode& mode, int64_t& id)
{
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(address), BT_ERR_INVALID_PARAM, "addr is invalid.");
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    bluetooth::RawAddress realAddr;
    if (!bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(
            bluetooth::RawAddress(address), realAddr)) {
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    int deviceSide = 0;
    int deviceMode = 0;
    int result =
        pimpl->hearingAidService_->GetHearingAidDeviceInfo(realAddr, deviceSide, deviceMode, id);
    if (result != BT_NO_ERROR) {
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    side = static_cast<DeviceSide>(deviceSide);
    mode = static_cast<DeviceMode>(deviceMode);
    return result;
}

ErrCode BluetoothHearingAidServer::GetConnectedDevices(std::vector<std::string>& connectedDevices)
{
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    std::list<bluetooth::RawAddress> resultList = pimpl->hearingAidService_->GetConnectDevices();
    auto resultVec = std::vector<bluetooth::RawAddress> (resultList.begin(), resultList.end());
    std::vector<bluetooth::RawAddress> rawAddrs;
    bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(resultVec, rawAddrs);
    for (auto device = rawAddrs.begin(); device != rawAddrs.end(); device ++) {
        connectedDevices.emplace_back((*device).GetAddress());
    }
    return BT_NO_ERROR;
}

ErrCode BluetoothHearingAidServer::GetActiveDevice(std::string& address, std::string& name)
{
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }

    bluetooth::RawAddress addr;
    bluetooth::RawAddress result;
    pimpl->hearingAidService_->GetActiveDevice(addr, name);
    bluetooth::BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(addr, result);
    address = result.GetAddress();
    return BT_NO_ERROR;
}

ErrCode BluetoothHearingAidServer::GetConnectionState(const std::string& address, ConnectionState& state)
{
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(address), BT_ERR_INVALID_PARAM, "addr is invalid.");
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    bluetooth::RawAddress realAddr;
    if (!bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(
            bluetooth::RawAddress(address), realAddr)) {
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    bluetooth::RawAddress addr;
    state = static_cast<ConnectionState>(pimpl->hearingAidService_->GetDeviceState(realAddr));
    return BT_NO_ERROR;
}

ErrCode BluetoothHearingAidServer::SetConnectionStrategy(const std::string& address, ConnectionStrategy strategy)
{
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(address), BT_ERR_INVALID_PARAM, "addr is invalid.");
    int32_t stra = static_cast<int32_t>(strategy);
    CHECK_AND_RETURN_LOG_RET((stra >= STRATEGY_MIN && stra <= STRATEGY_MAX), BT_ERR_INVALID_PARAM, "strategy invalid");
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    bluetooth::RawAddress realAddr;
    if (!bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(
            bluetooth::RawAddress(address), realAddr)) {
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    return pimpl->hearingAidService_->SetConnectStrategy(realAddr, static_cast<int>(strategy));
}

ErrCode BluetoothHearingAidServer::GetConnectionStrategy(const std::string& address, ConnectionStrategy& strategy)
{
    CHECK_AND_RETURN_LOG_RET(bluetooth::IsValidAddr(address), BT_ERR_INVALID_PARAM, "addr is invalid.");
    if (pimpl->hearingAidService_ == nullptr) {
        HILOGE("hearingAidService_ is null");
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    bluetooth::RawAddress realAddr;
    if (!bluetooth::BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(
            bluetooth::RawAddress(address), realAddr)) {
        return BT_OV_ERR_INTERNAL_ERROR;
    }
    strategy = static_cast<ConnectionStrategy>(pimpl->hearingAidService_->GetConnectStrategy(realAddr));
    return BT_NO_ERROR;
}

ErrCode BluetoothHearingAidServer::RegisterObserver(const sptr<IBluetoothHearingAidObserver>& observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return BT_NO_ERROR;
    }
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return BT_NO_ERROR;
    }
    if (pimpl->appContainer_->Size() > MAX_MAP_SIZE || pimpl->observers_.Size() > MAX_MAP_SIZE) {
        HILOGE("appContainer_ or observers_ too much");
        return BT_NO_ERROR;
    }
    pimpl->appContainer_->AddObject(observer->AsObject());
    pimpl->observers_.Register(observer);
    if (pimpl->hearingAidService_ == nullptr) {
        return BT_NO_ERROR;
    }
    return BT_NO_ERROR;
}

ErrCode BluetoothHearingAidServer::DeRegisterObserver(const sptr<IBluetoothHearingAidObserver>& observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return BT_NO_ERROR;
    }

    if (pimpl != nullptr) {
        pimpl->observers_.Deregister(observer);
        pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHearingAidServer::CallbackEnter(uint32_t code)
{
    int32_t errCode = BT_ERR_PERMISSION_FAILED;
    auto iter = permissionCheckMap_.find(static_cast<IBluetoothHearingAidIpcCode>(code));
    if (iter == permissionCheckMap_.end()) {
        return errCode;
    }

    errCode = PermissionManager::VerifyMultiPermissions(iter->second);
    if (errCode != BT_NO_ERROR) {
        HILOGE("[PERMISSION] failed. code(%{public}d), callingName(%{public}s)",
            code, PermissionManager::GetCallingName().c_str());
    }
    return errCode;
}

int32_t BluetoothHearingAidServer::CallbackExit(uint32_t code, int32_t result)
{
    return BT_NO_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS
