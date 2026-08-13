/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "bt_server_avrcp_tg"
#endif

#include <mutex>
#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_avrcp_tg_server.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "interface_adapter_manager.h"
#include "interface_profile.h"
#include "interface_profile_avrcp_tg.h"
#include "interface_profile_manager.h"
#include "ipc_skeleton.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

struct BluetoothAvrcpTgServer::impl {
public:
    class SysStsObserverImpl : public ISystemStateObserver {
    public:
        explicit SysStsObserverImpl(BluetoothAvrcpTgServer::impl *impl) : impl_(impl)
        {}
        ~SysStsObserverImpl() override
        {}

        void OnSystemStateChange(const BTSystemState state) override
        {
            impl_->OnSystemStateChange(state);
        }

    private:
        BluetoothAvrcpTgServer::impl *impl_;
    };
    class ObserverImpl : public IProfileAvrcpTg::IObserver {
    public:
        explicit ObserverImpl(BluetoothAvrcpTgServer::impl *impl) : impl_(impl)
        {}
        ~ObserverImpl() override
        {}

        void OnConnectionStateChanged(const RawAddress &rawAddr, int state) override
        {
            int cause = static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE);
            if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
                cause = static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FAIL_INTERNAL);
            }
            impl_->OnConnectionStateChanged(rawAddr, state, cause);
        }

    private:
        BluetoothAvrcpTgServer::impl *impl_;
    };

    impl()
    {
        auto svManager = IProfileManager::GetInstance();
        service_ = static_cast<IProfileAvrcpTg *>(svManager->GetProfileService(PROFILE_NAME_AVRCP_TG));
        if (service_ != nullptr) {
            observer_ = std::make_unique<ObserverImpl>(this);
            service_->RegisterObserver(observer_.get());
        }

        appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
        appContainer_->Init();
        sysObserver_ = std::make_unique<SysStsObserverImpl>(this);
        IAdapterManager::GetInstance()->RegisterSystemStateObserver(*sysObserver_);
    }

    ~impl()
    {
        auto svManager = IProfileManager::GetInstance();
        service_ = static_cast<IProfileAvrcpTg *>(svManager->GetProfileService(PROFILE_NAME_AVRCP_TG));
        if (service_ != nullptr) {
            service_->UnregisterObserver();
            observer_ = nullptr;
            service_ = nullptr;
        }

        IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*sysObserver_);
        sysObserver_ = nullptr;
    }

    bool IsEnabled()
    {
        auto servManager = IProfileManager::GetInstance();
        service_ = static_cast<IProfileAvrcpTg *>(servManager->GetProfileService(PROFILE_NAME_AVRCP_TG));

        return (service_ != nullptr && service_->IsEnabled());
    }

    void OnSystemStateChange(const BTSystemState state)
    {
        std::lock_guard<std::mutex> lock(serviceMutex_);

        switch (state) {
            case BTSystemState::ON: {
                auto svManager = IProfileManager::GetInstance();
                service_ = static_cast<IProfileAvrcpTg *>(svManager->GetProfileService(PROFILE_NAME_AVRCP_TG));
                if (service_ != nullptr) {
                    observer_ = std::make_unique<ObserverImpl>(this);
                    service_->RegisterObserver(observer_.get());
                }
                break;
            }
            case BTSystemState::OFF:
                /// FALL THROUGH
            default:
                if (service_ != nullptr) {
                    service_->UnregisterObserver();
                    observer_ = nullptr;
                    service_ = nullptr;
                }
                break;
        }
    }

    void OnConnectionStateChanged(const RawAddress &rawAddr, int state, int cause)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("res: %{public}s, state: %{public}d, cause: %{public}d.", GET_ENCRYPT_AVRCP_ADDR(rawAddr), state, cause);
        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, state, cause](IBluetoothAvrcpTgObserver *observer) {
            uint64_t tokenId = this->appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr, static_cast<int32_t>(state), cause);
            }
        });
    }

    std::mutex serviceMutex_;
    IProfileAvrcpTg *service_;

    std::mutex observerMutex_;
    RemoteObserverList<IBluetoothAvrcpTgObserver> observers_;

    std::unique_ptr<ObserverImpl> observer_;
    std::unique_ptr<SysStsObserverImpl> sysObserver_;

    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;
};

BluetoothAvrcpTgServer::BluetoothAvrcpTgServer()
{
    pimpl = std::make_unique<impl>();
}

BluetoothAvrcpTgServer::~BluetoothAvrcpTgServer()
{
    pimpl = nullptr;
}

void BluetoothAvrcpTgServer::RegisterObserver(const sptr<IBluetoothAvrcpTgObserver> &observer)
{
    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);

    if (observer == nullptr) {
        HILOGE("observer is NULL.");
        return ;
    }
    if (pimpl->appContainer_->Size() > MAX_MAP_SIZE || pimpl->observers_.Size() > MAX_MAP_SIZE) {
        HILOGE("appContainer_ or observers_ too much");
        return;
    }
    pimpl->appContainer_->AddObject(observer->AsObject());
    pimpl->observers_.Register(observer);
    return ;
}

void BluetoothAvrcpTgServer::UnregisterObserver(const sptr<IBluetoothAvrcpTgObserver> &observer)
{
    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);

    if (observer == nullptr) {
        HILOGI("observer is NULL.");
        return;
    }
    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
    return;
}

void BluetoothAvrcpTgServer::SetActiveDevice(const BluetoothRawAddress &addr)
{
    HILOGI("address: %{public}s", GetEncryptAddr(addr.GetAddress()).c_str());

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return;
    }

    if (pimpl->IsEnabled()) {
        pimpl->service_->SetActiveDevice(realAddr);
    } else {
        HILOGE("service is null or disable ");
    }
}

int32_t BluetoothAvrcpTgServer::Connect(const BluetoothRawAddress &addr)
{
    HILOGI("address: %{public}s", GetEncryptAddr(addr.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->Connect(realAddr);
    } else {
        HILOGE("service is null or disable ");
    }
    return result;
}

int32_t BluetoothAvrcpTgServer::Disconnect(const BluetoothRawAddress &addr)
{
    HILOGI("address: %{public}s", GetEncryptAddr(addr.GetAddress()).c_str());

    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->Disconnect(realAddr);
    } else {
        HILOGE("service is null or disable ");
    }
    return result;
}

std::vector<BluetoothRawAddress> BluetoothAvrcpTgServer::GetConnectedDevices()
{
    std::vector<BluetoothRawAddress> results;

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return results;
    }

    std::vector<RawAddress> devices;
    devices = pimpl->service_->GetConnectedDevices();
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(devices, results);
    return results;
}

std::vector<BluetoothRawAddress> BluetoothAvrcpTgServer::GetDevicesByStates(const std::vector<int32_t> &states)
{
    std::vector<BluetoothRawAddress> results;

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return results;
    }

    std::vector<RawAddress> devices;
    std::vector<int> convertStates;
    for (auto state : states) {
        HILOGI("state: %{public}d", state);
        convertStates.push_back(static_cast<int>(state));
    }

    devices = pimpl->service_->GetDevicesByStates(convertStates);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(devices, results);

    return results;
}

int32_t BluetoothAvrcpTgServer::GetDeviceState(const BluetoothRawAddress &addr)
{
    HILOGI("address: %{public}s", GetEncryptAddr(addr.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetDeviceState(realAddr);
    } else {
        HILOGE("service is null or disable ");
    }
    return result;
}

void BluetoothAvrcpTgServer::NotifyPlaybackStatusChanged(int32_t playStatus, int32_t playbackPos)
{
    HILOGI("playStatus: %{public}d, playbackPos: %{public}d", playStatus, playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyPlaybackStatusChanged(static_cast<uint8_t>(playStatus),
                                                 static_cast<uint32_t>(playbackPos));
}

void BluetoothAvrcpTgServer::NotifyTrackChanged(int64_t uid, int32_t playbackPos)
{
    HILOGI("uid: %{public}jd, playbackPos: %{public}d", uid, playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyTrackChanged(static_cast<uint64_t>(uid), static_cast<uint32_t>(playbackPos));
}

void BluetoothAvrcpTgServer::NotifyTrackReachedEnd(int32_t playbackPos)
{
    HILOGI("playbackPos: %{public}d", playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyTrackReachedEnd(static_cast<uint32_t>(playbackPos));
}

void BluetoothAvrcpTgServer::NotifyTrackReachedStart(int32_t playbackPos)
{
    HILOGI("playbackPos: %{public}d", playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyTrackReachedStart(static_cast<uint32_t>(playbackPos));
}

void BluetoothAvrcpTgServer::NotifyPlaybackPosChanged(int32_t playbackPos)
{
    HILOGI("playbackPos: %{public}d", playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyPlaybackPosChanged(static_cast<uint32_t>(playbackPos));
}

void BluetoothAvrcpTgServer::NotifyPlayerAppSettingChanged(const std::vector<int32_t> &attributes,
    const std::vector<int32_t> &values)
{
    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    std::deque<uint8_t> attrs;
    std::deque<uint8_t> vals;

    for (auto attribute : attributes) {
        HILOGI("attributes = %{public}d", attribute);
        attrs.push_back(attribute);
    }
    for (auto value : values) {
        HILOGI("values = %{public}d", value);
        vals.push_back(value);
    }

    pimpl->service_->NotifyPlayerAppSettingChanged(attrs, vals);
}

void BluetoothAvrcpTgServer::NotifyNowPlayingContentChanged()
{
    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyNowPlayingContentChanged();
}

void BluetoothAvrcpTgServer::NotifyAvailablePlayersChanged()
{
    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyAvailablePlayersChanged();
}

void BluetoothAvrcpTgServer::NotifyAddressedPlayerChanged(int32_t playerId, int32_t uidCounter)
{
    HILOGI("playerId: %{public}d, uidCounter: %{public}d", playerId, uidCounter);

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyAddressedPlayerChanged(static_cast<uint32_t>(playerId), static_cast<uint32_t>(uidCounter));
}

void BluetoothAvrcpTgServer::NotifyUidChanged(int32_t uidCounter)
{
    HILOGI("uidCounter: %{public}d", uidCounter);

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyUidChanged(static_cast<uint32_t>(uidCounter));
}

void BluetoothAvrcpTgServer::NotifyVolumeChanged(int32_t volume)
{
    HILOGI("volume: %{public}d", volume);

    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return;
    }

    pimpl->service_->NotifyVolumeChanged(static_cast<uint8_t>(volume));
}

int32_t BluetoothAvrcpTgServer::SetDeviceAbsoluteVolume(const BluetoothRawAddress &addr, int32_t volumeLevel)
{
    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return BT_ERR_INTERNAL_ERROR;
    }

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    pimpl->service_->SetDeviceAbsoluteVolume(realAddr, volumeLevel);
    return BT_NO_ERROR;
}
int32_t BluetoothAvrcpTgServer::GetDeviceAbsVolumeAbility(const BluetoothRawAddress &addr, int32_t &ability)
{
    ability = DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT;
    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return BT_ERR_INTERNAL_ERROR;
    }

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    ability = pimpl->service_->GetDeviceAbsVolumeAbility(realAddr);
    return BT_NO_ERROR;
}
int32_t BluetoothAvrcpTgServer::SetDeviceAbsVolumeAbility(const BluetoothRawAddress &addr, int32_t ability)
{
    if (!pimpl->IsEnabled()) {
        HILOGE("service is null or disable ");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (ability < DeviceAbsVolumeAbility::DEVICE_ABSVOL_CLOSE ||
        ability >= DeviceAbsVolumeAbility::DEVICE_ABSVOL_UNSUPPORT) {
        return BT_ERR_INVALID_PARAM;
    }

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    pimpl->service_->SetDeviceAbsVolumeAbility(realAddr, ability);
    return BT_NO_ERROR;
}

}  // namespace Bluetooth

}  // namespace OHOS