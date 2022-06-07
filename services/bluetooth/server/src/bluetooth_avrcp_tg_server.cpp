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
#include <mutex>
#include "bluetooth_def.h"
#include "bluetooth_avrcp_tg_server.h"
#include "bluetooth_log.h"

#include "interface_adapter_manager.h"
#include "interface_profile.h"
#include "interface_profile_avrcp_tg.h"
#include "interface_profile_manager.h"
#include "remote_observer_list.h"
#include "permission_utils.h"


namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;

struct BluetoothAvrcpTgServer::impl {
public:
    class SysStsObserverImpl : public ISystemStateObserver {
    public:
        explicit SysStsObserverImpl(BluetoothAvrcpTgServer::impl *impl) : impl_(impl)
        {}
        virtual ~SysStsObserverImpl()
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
        virtual ~ObserverImpl()
        {}

        void OnConnectionStateChanged(const RawAddress &rawAddr, int state) override
        {
            impl_->OnConnectionStateChanged(rawAddr, state);
        }

    private:
        BluetoothAvrcpTgServer::impl *impl_;
    };

    impl()
    {
        HILOGD("%{public}s start.", __func__);

        auto svManager = IProfileManager::GetInstance();
        service_ = static_cast<IProfileAvrcpTg *>(svManager->GetProfileService(PROFILE_NAME_AVRCP_TG));
        if (service_ != nullptr) {
            observer_ = std::make_unique<ObserverImpl>(this);
            service_->RegisterObserver(observer_.get());
        }

        sysObserver_ = std::make_unique<SysStsObserverImpl>(this);
        IAdapterManager::GetInstance()->RegisterSystemStateObserver(*sysObserver_);
    }

    ~impl()
    {
        HILOGD("%{public}s start.", __func__);

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
        HILOGD("%{public}s start.", __func__);

        auto servManager = IProfileManager::GetInstance();
        service_ = static_cast<IProfileAvrcpTg *>(servManager->GetProfileService(PROFILE_NAME_AVRCP_TG));

        return (service_ != nullptr && service_->IsEnabled());
    }

    void OnSystemStateChange(const BTSystemState state)
    {
        HILOGD("%{public}s start.", __func__);

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

    void OnConnectionStateChanged(const RawAddress &rawAddr, int state)
    {
        HILOGD("%{public}s start state:%{public}d.", __func__, state);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, state](IBluetoothAvrcpTgObserver *observer) {
            observer->OnConnectionStateChanged(rawAddr, static_cast<int32_t>(state));
        });
    }

    std::mutex serviceMutex_;
    IProfileAvrcpTg *service_;

    std::mutex observerMutex_;
    RemoteObserverList<IBluetoothAvrcpTgObserver> observers_;

    std::unique_ptr<ObserverImpl> observer_;
    std::unique_ptr<SysStsObserverImpl> sysObserver_;
};

BluetoothAvrcpTgServer::BluetoothAvrcpTgServer()
{
    HILOGD("%{public}s start.", __func__);

    pimpl = std::make_unique<impl>();
}

BluetoothAvrcpTgServer::~BluetoothAvrcpTgServer()
{
    pimpl = nullptr;
}

void BluetoothAvrcpTgServer::RegisterObserver(const sptr<IBluetoothAvrcpTgObserver> &observer)
{
    HILOGD("%{public}s start.", __func__);

    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);

    if (observer == nullptr) {
        HILOGD("RegisterObserver called with NULL binder. Ignoring.");
        return ;
    }
    pimpl->observers_.Register(observer);
    HILOGD("%{public}s end.", __func__);

    return ;
}

void BluetoothAvrcpTgServer::UnregisterObserver(const sptr<IBluetoothAvrcpTgObserver> &observer)
{
    HILOGD("%{public}s start.", __func__);

    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);

    if (observer == nullptr) {
        HILOGD("UnregisterObserver called with NULL binder. Ignoring.");
        return;
    }
    pimpl->observers_.Deregister(observer);
    HILOGD("%{public}s end.", __func__);

    return;
}

void BluetoothAvrcpTgServer::SetActiveDevice(const BluetoothRawAddress &addr)
{
    HILOGD("%{public}s start.", __func__);

    if (pimpl->IsEnabled()) {
        pimpl->service_->SetActiveDevice(addr);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);
}

int32_t BluetoothAvrcpTgServer::Connect(const BluetoothRawAddress &addr)
{
    HILOGD("%{public}s start.", __func__);

    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->Connect(addr);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpTgServer::Disconnect(const BluetoothRawAddress &addr)
{
    HILOGD("%{public}s start.", __func__);

    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->Disconnect(addr);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

std::vector<BluetoothRawAddress> BluetoothAvrcpTgServer::GetConnectedDevices()
{
    HILOGD("%{public}s start.", __func__);

    std::vector<BluetoothRawAddress> results;
    
    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return results;
    }

    std::vector<RawAddress> devices;
    devices = pimpl->service_->GetConnectedDevices();
    for (auto device : devices) {
        BluetoothRawAddress rawAddr = BluetoothRawAddress(device);
        results.emplace_back(rawAddr);
        
    }
    HILOGD("%{public}s end.", __func__);

    return results;
}

std::vector<BluetoothRawAddress> BluetoothAvrcpTgServer::GetDevicesByStates(const std::vector<int32_t> &states)
{
    HILOGD("%{public}s start.", __func__);

    std::vector<BluetoothRawAddress> results;

    if (PermissionUtils::VerifyUseBluetoothPermission() == PERMISSION_DENIED) {
        HILOGE("GetDevicesByStates() false, check permission failed");
        return results;
    }
    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return results;
    }

    std::vector<RawAddress> devices;
    std::vector<int> convertStates;
    for (auto state : states) {
        HILOGD("%{public}s: state = %{public}d", __func__, state);
        convertStates.push_back(static_cast<int>(state));
    }
        
    devices = pimpl->service_->GetDevicesByStates(convertStates);
    for (auto device : devices) {
        BluetoothRawAddress rawAddr = BluetoothRawAddress(device);
        results.emplace_back(rawAddr);
    }
    HILOGD("%{public}s end.", __func__);

    return results;
}

int32_t BluetoothAvrcpTgServer::GetDeviceState(const BluetoothRawAddress &addr)
{
    HILOGD("%{public}s start.", __func__);
    if (PermissionUtils::VerifyUseBluetoothPermission() == PERMISSION_DENIED) {
        HILOGE("GetDeviceState() false, check permission failed");
        return BT_FAILURE;
    }
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetDeviceState(addr);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

void BluetoothAvrcpTgServer::NotifyPlaybackStatusChanged(int32_t playStatus, int32_t playbackPos)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: playStatus = %{public}d, playbackPos = %{public}d", __func__, playStatus, playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyPlaybackStatusChanged(static_cast<uint8_t>(playStatus),
                                                 static_cast<uint32_t>(playbackPos));
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyTrackChanged(int64_t uid, int32_t playbackPos)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: uid = %{public}jd, playbackPos = %{public}d", __func__, uid, playbackPos);
    
    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyTrackChanged(static_cast<uint64_t>(uid), static_cast<uint32_t>(playbackPos));
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyTrackReachedEnd(int32_t playbackPos)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: playbackPos = %{public}d", __func__, playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyTrackReachedEnd(static_cast<uint32_t>(playbackPos));
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyTrackReachedStart(int32_t playbackPos)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: playbackPos = %{public}d", __func__, playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyTrackReachedStart(static_cast<uint32_t>(playbackPos));
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyPlaybackPosChanged(int32_t playbackPos)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: playbackPos = %{public}d", __func__, playbackPos);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyPlaybackPosChanged(static_cast<uint32_t>(playbackPos));
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyPlayerAppSettingChanged(const std::vector<int32_t> &attributes,
    const std::vector<int32_t> &values)
{
    HILOGD("%{public}s start.", __func__);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    std::deque<uint8_t> attrs;
    std::deque<uint8_t> vals;

    for (auto attribute : attributes) {
        HILOGD("%{public}s: attributes = %{public}d", __func__, attribute);
        attrs.push_back(attribute);
    }
    for (auto value : values) {
        HILOGD("%{public}s: values = %{public}d", __func__, value);
        vals.push_back(value);
    }

    pimpl->service_->NotifyPlayerAppSettingChanged(attrs, vals);
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyNowPlayingContentChanged()
{
    HILOGD("%{public}s start.", __func__);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyNowPlayingContentChanged();
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyAvailablePlayersChanged()
{
    HILOGD("%{public}s start.", __func__);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyAvailablePlayersChanged();
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyAddressedPlayerChanged(int32_t playerId, int32_t uidCounter)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: playerId = %{public}d, uidCounter = %{public}d", __func__, playerId, uidCounter);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyAddressedPlayerChanged(static_cast<uint32_t>(playerId), static_cast<uint32_t>(uidCounter));
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyUidChanged(int32_t uidCounter)
{
    HILOGD("%{public}s start.", __func__);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyUidChanged(static_cast<uint32_t>(uidCounter));
    HILOGD("%{public}s end.", __func__);
}

void BluetoothAvrcpTgServer::NotifyVolumeChanged(int32_t volume)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: volume = %{public}d", __func__, volume);

    if (!pimpl->IsEnabled()) {
        HILOGE("%{public}s: service is null or disable ", __func__);
        return;
    }

    pimpl->service_->NotifyVolumeChanged(static_cast<uint8_t>(volume));
    HILOGD("%{public}s end.", __func__);
}

}  // namespace binder

}  // namespace ipc