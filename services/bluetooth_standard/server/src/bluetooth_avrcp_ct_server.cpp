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
#include <mutex>
#include "bluetooth_avrcp_ct_server.h"
#include "bluetooth_log.h"

#include "interface_adapter_manager.h"
#include "interface_profile.h"
#include "interface_profile_avrcp_ct.h"
#include "interface_profile_manager.h"
#include "remote_observer_list.h"


namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;

struct BluetoothAvrcpCtServer::impl {
public:
    class SysStsObserverImpl : public ISystemStateObserver {
    public:
        explicit SysStsObserverImpl(BluetoothAvrcpCtServer::impl *impl) : impl_(impl)
        {}
        virtual ~SysStsObserverImpl()
        {}

        void OnSystemStateChange(const BTSystemState state) override
        {
            impl_->OnSystemStateChange(state);
        }

    private:
        BluetoothAvrcpCtServer::impl *impl_;
    };
    class ObserverImpl : public IProfileAvrcpCt::IObserver {
    public:
        explicit ObserverImpl(BluetoothAvrcpCtServer::impl *impl) : impl_(impl)
        {}
        virtual ~ObserverImpl()
        {}

        void OnConnectionStateChanged(const RawAddress &rawAddr, int state) override
        {
            impl_->OnConnectionStateChanged(rawAddr, state);
        }

        void OnPressButton(const RawAddress &rawAddr, uint8_t button, int result) override
        {
            impl_->OnPressButton(rawAddr, button, result);
        }

        void OnReleaseButton(const RawAddress &rawAddr, uint8_t button, int result) override
        {
            impl_->OnReleaseButton(rawAddr, button, result);
        }

        void OnSetAddressedPlayer(const RawAddress &rawAddr, int result, int detail) override
        {
            // The interface is not exposed
        }

        void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
            const std::vector<std::string> &folderNames, int result, int detail) override
        {
            impl_->OnSetBrowsedPlayer(rawAddr, uidCounter, numOfItems, folderNames, result, detail);
        }

        void OnGetCapabilities(const RawAddress &rawAddr, const std::vector<uint32_t> &companies,
            const std::vector<uint8_t> &events, int result) override
        {
            impl_->OnGetCapabilities(rawAddr, companies, events, result);
        }

        void OnGetPlayerAppSettingAttributes(
            const RawAddress &rawAddr, std::vector<uint8_t> attributes, int result) override
        {
            impl_->OnGetPlayerAppSettingAttributes(rawAddr, attributes, result);
        }

        void OnGetPlayerAppSettingValues(
            const RawAddress &rawAddr, uint8_t attribute, const std::vector<uint8_t> &values, int result) override
        {
            impl_->OnGetPlayerAppSettingValues(rawAddr, attribute, values, result);
        }

        void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
            const std::vector<uint8_t> &values, int result) override
        {
            impl_->OnGetPlayerAppSettingCurrentValue(rawAddr, attributes, values, result);
        }

        void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, int result) override
        {
            impl_->OnSetPlayerAppSettingCurrentValue(rawAddr, result);
        }

        void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
            const std::vector<std::string> &attributeName, int result) override
        {
            impl_->OnGetPlayerAppSettingAttributeText(rawAddr, attributes, attributeName, result);
        }

        void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr, const std::vector<uint8_t> &values,
            const std::vector<std::string> &valueName, int result) override
        {
            impl_->OnGetPlayerAppSettingValueText(rawAddr, values, valueName, result);
        }

        void OnGetElementAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
            const std::vector<std::string> &valueName, int result) override
        {
            impl_->OnGetElementAttributes(rawAddr, attributes, valueName, result);
        }

        void OnGetPlayStatus(const RawAddress &rawAddr, uint32_t songLength, uint32_t songPosition, uint8_t playStatus,
            int result) override
        {
            impl_->OnGetPlayStatus(rawAddr, songLength, songPosition, playStatus, result);
        }

        void OnPlayItem(const RawAddress &rawAddr, int status, int result) override
        {
            impl_->OnPlayItem(rawAddr, status, result);
        }

        void OnAddToNowPlaying(const RawAddress &rawAddr, int status, int detail) override
        {
            // The interface is not exposed
        }

        void OnChangePath(const RawAddress &rawAddr, uint32_t numOfItems, int result, int detail) override
        {
            // The interface is not exposed
        }

        void OnGetMediaPlayers(const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMpItem> &items,
            int result, int detail) override
        {
            impl_->OnGetMediaPlayers(rawAddr, uidCounter, items, result, detail);
        }

        void OnGetFolderItems(const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMeItem> &items,
            int result, int detail) override
        {
            impl_->OnGetFolderItems(rawAddr, uidCounter, items, result, detail);
        }

        void OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
            const std::vector<std::string> &values, int result, int detail) override
        {
            impl_->OnGetItemAttributes(rawAddr, attributes, values, result, detail);
        }

        void OnGetTotalNumberOfItems(
            const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems, int result, int detail) override
        {
            impl_->OnGetTotalNumberOfItems(rawAddr, uidCounter, numOfItems, result, detail);
        }

        void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, int result) override
        {
            impl_->OnSetAbsoluteVolume(rawAddr, volume, result);
        }

        void OnPlaybackStatusChanged(const RawAddress &rawAddr, uint8_t playStatus, int result) override
        {
            impl_->OnPlaybackStatusChanged(rawAddr, playStatus, result);
        }

        void OnTrackChanged(const RawAddress &rawAddr, uint64_t uid, int result) override
        {
            impl_->OnTrackChanged(rawAddr, uid, result);
        }

        void OnTrackReachedEnd(const RawAddress &rawAddr, int result) override
        {
            impl_->OnTrackReachedEnd(rawAddr, result);
        }

        void OnTrackReachedStart(const RawAddress &rawAddr, int result) override
        {
            impl_->OnTrackReachedStart(rawAddr, result);
        }

        void OnPlaybackPosChanged(const RawAddress &rawAddr, uint32_t playbackPos, int result) override
        {
            impl_->OnPlaybackPosChanged(rawAddr, playbackPos, result);
        }

        void OnPlayerAppSettingChanged(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
            const std::vector<uint8_t> &values, int result) override
        {
            impl_->OnPlayerAppSettingChanged(rawAddr, attributes, values, result);
        }

        void OnNowPlayingContentChanged(const RawAddress &rawAddr, int result) override
        {
            impl_->OnNowPlayingContentChanged(rawAddr, result);
        }

        void OnAvailablePlayersChanged(const RawAddress &rawAddr, int result) override
        {
            impl_->OnAvailablePlayersChanged(rawAddr, result);
        }

        void OnAddressedPlayerChanged(
            const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, int result) override
        {
            impl_->OnAddressedPlayerChanged(rawAddr, playerId, uidCounter, result);
        }

        void OnUidChanged(const RawAddress &rawAddr, uint16_t uidCounter, int result) override
        {
            impl_->OnUidChanged(rawAddr, uidCounter, result);
        }

        void OnVolumeChanged(const RawAddress &rawAddr, uint8_t volume, int result) override
        {
            impl_->OnVolumeChanged(rawAddr, volume, result);
        }

    private:
        BluetoothAvrcpCtServer::impl *impl_;
    };

    impl()
    {
        HILOGD("%{public}s start.", __func__);

        auto svManager = IProfileManager::GetInstance();
        service_ = static_cast<IProfileAvrcpCt *>(svManager->GetProfileService(PROFILE_NAME_AVRCP_CT));
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
        service_ = static_cast<IProfileAvrcpCt *>(svManager->GetProfileService(PROFILE_NAME_AVRCP_CT));
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
        service_ = static_cast<IProfileAvrcpCt *>(servManager->GetProfileService(PROFILE_NAME_AVRCP_CT));

        return (service_ != nullptr && service_->IsEnabled());
    }

    void OnSystemStateChange(const BTSystemState state)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(serviceMutex_);

        switch (state) {
            case BTSystemState::ON: {
                auto svManager = IProfileManager::GetInstance();
                service_ = static_cast<IProfileAvrcpCt *>(svManager->GetProfileService(PROFILE_NAME_AVRCP_CT));
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

        observers_.ForEach([rawAddr, state](IBluetoothAvrcpCtObserver *observer) {
            observer->OnConnectionStateChanged(
                rawAddr, static_cast<int32_t>(state));
        });
    }

    void OnPressButton(const RawAddress &rawAddr, uint8_t button, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, button, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnPressButton(rawAddr,
                static_cast<int32_t>(button),
                static_cast<int32_t>(result));
        });
    }

    void OnReleaseButton(const RawAddress &rawAddr, uint8_t button, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, button, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnReleaseButton(rawAddr,
                static_cast<int32_t>(button),
                static_cast<int32_t>(result));
        });
    }

    void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
        const std::vector<std::string> &folderNames, int result, int detail)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach(
            [rawAddr, uidCounter, numOfItems, folderNames, result, detail](IBluetoothAvrcpCtObserver *observer) {
                observer->OnSetBrowsedPlayer(rawAddr,
                    static_cast<int32_t>(uidCounter),
                    static_cast<int32_t>(numOfItems),
                    folderNames,
                    static_cast<int32_t>(result),
                    static_cast<int32_t>(detail));
            });
    }

    void OnGetCapabilities(const RawAddress &rawAddr, const std::vector<uint32_t> &companies,
        const std::vector<uint8_t> &events, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, companies, events, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetCapabilities(rawAddr,
                companies,
                events,
                static_cast<int32_t>(result));
        });
    }

    void OnGetPlayerAppSettingAttributes(const RawAddress &rawAddr, std::vector<uint8_t> attributes, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, attributes, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetPlayerAppSettingAttributes(
                rawAddr, attributes, static_cast<int32_t>(result));
        });
    }

    void OnGetPlayerAppSettingValues(
        const RawAddress &rawAddr, uint8_t attribute, const std::vector<uint8_t> values, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, attribute, values, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetPlayerAppSettingValues(
                rawAddr, static_cast<int32_t>(attribute), values, static_cast<int32_t>(result));
        });
    }

    void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, attributes, values, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetPlayerAppSettingCurrentValue(rawAddr,
                attributes,
                values,
                static_cast<int32_t>(result));
        });
    }

    void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnSetPlayerAppSettingCurrentValue(
                rawAddr, static_cast<int32_t>(result));
        });
    }

    void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<std::string> &attributeName, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, attributes, attributeName, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetPlayerAppSettingAttributeText(rawAddr,
                attributes,
                attributeName,
                static_cast<int32_t>(result));
        });
    }

    void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr, const std::vector<uint8_t> &values,
        const std::vector<std::string> &valueName, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, values, valueName, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetPlayerAppSettingValueText(
                rawAddr, values, valueName, static_cast<int32_t>(result));
        });
    }

    void OnGetElementAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &valueName, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, attributes, valueName, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetElementAttributes(rawAddr,
                attributes,
                valueName,
                static_cast<int32_t>(result));
        });
    }

    void OnGetPlayStatus(
        const RawAddress &rawAddr, uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach(
            [rawAddr, songLength, songPosition, playStatus, result](IBluetoothAvrcpCtObserver *observer) {
                observer->OnGetPlayStatus(rawAddr,
                    songLength,
                    songPosition,
                    static_cast<int32_t>(playStatus),
                    static_cast<int32_t>(result));
            });
    }

    void OnPlayItem(const RawAddress &rawAddr, int result, int detail)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, result, detail](IBluetoothAvrcpCtObserver *observer) {
            observer->OnPlayItem(rawAddr,
                static_cast<int32_t>(result),
                static_cast<int32_t>(detail));
        });
    }

    void OnGetMediaPlayers(
        const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMpItem> &items, int result, int detail)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, uidCounter, items, result, detail](IBluetoothAvrcpCtObserver *observer) {
            std::vector<BluetoothAvrcpMpItem> myItems;
            for (size_t i = 0; i < items.size(); i++) {
                myItems.push_back(BluetoothAvrcpMpItem(items.at(i)));
            }
            observer->OnGetMediaPlayers(rawAddr,
                static_cast<int32_t>(uidCounter),
                myItems,
                static_cast<int32_t>(result),
                static_cast<int32_t>(detail));
        });
    }

    void OnGetFolderItems(
        const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMeItem> &items, int result, int detail)
    {
        HILOGD("%{public}s start items.size = %{public}zu.", __func__, items.size());

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, uidCounter, items, result, detail](IBluetoothAvrcpCtObserver *observer) {
            std::vector<BluetoothAvrcpMeItem> myItems;
            for (size_t i = 0; i < items.size(); i++) {
                myItems.push_back(BluetoothAvrcpMeItem(items.at(i)));
            }
            observer->OnGetFolderItems(rawAddr,
                static_cast<int32_t>(uidCounter),
                myItems,
                static_cast<int32_t>(result),
                static_cast<int32_t>(detail));
        });
    }

    void OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &values, int result, int detail)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, attributes, values, result, detail](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetItemAttributes(rawAddr,
                attributes,
                values,
                static_cast<int32_t>(result),
                static_cast<int32_t>(detail));
        });
    }

    void OnGetTotalNumberOfItems(
        const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems, int result, int detail)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, uidCounter, numOfItems, result, detail](IBluetoothAvrcpCtObserver *observer) {
            observer->OnGetTotalNumberOfItems(rawAddr,
                static_cast<int32_t>(uidCounter),
                numOfItems,
                static_cast<int32_t>(result),
                static_cast<int32_t>(detail));
        });
    }

    void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, volume, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnSetAbsoluteVolume(rawAddr,
                static_cast<int32_t>(volume),
                static_cast<int32_t>(result));
        });
    }

    void OnPlaybackStatusChanged(const RawAddress &rawAddr, uint8_t playStatus, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, playStatus, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnPlaybackStatusChanged(rawAddr,
                static_cast<int32_t>(playStatus),
                static_cast<int32_t>(result));
        });
    }

    void OnTrackChanged(const RawAddress &rawAddr, uint64_t uid, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, uid, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnTrackChanged(rawAddr,
                uid,
                static_cast<int32_t>(result));
        });
    }

    void OnTrackReachedEnd(const RawAddress &rawAddr, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnTrackReachedEnd(
                rawAddr, static_cast<int32_t>(result));
        });
    }

    void OnTrackReachedStart(const RawAddress &rawAddr, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnTrackReachedStart(
                rawAddr, static_cast<int32_t>(result));
        });
    }

    void OnPlaybackPosChanged(const RawAddress &rawAddr, uint32_t playbackPos, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, playbackPos, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnPlaybackPosChanged(rawAddr,
                playbackPos,
                static_cast<int32_t>(result));
        });
    }

    void OnPlayerAppSettingChanged(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, attributes, values, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnPlayerAppSettingChanged(rawAddr,
                attributes,
                values,
                static_cast<int32_t>(result));
        });
    }

    void OnNowPlayingContentChanged(const RawAddress &rawAddr, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnNowPlayingContentChanged(
                rawAddr, static_cast<int32_t>(result));
        });
    }

    void OnAvailablePlayersChanged(const RawAddress &rawAddr, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnAvailablePlayersChanged(
                rawAddr, static_cast<int32_t>(result));
        });
    }

    void OnAddressedPlayerChanged(const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, playerId, uidCounter, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnAddressedPlayerChanged(rawAddr,
                static_cast<int32_t>(playerId),
                static_cast<int32_t>(uidCounter),
                static_cast<int32_t>(result));
        });
    }

    void OnUidChanged(const RawAddress &rawAddr, uint16_t uidCounter, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, uidCounter, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnUidChanged(rawAddr,
                static_cast<int32_t>(uidCounter),
                static_cast<int32_t>(result));
        });
    }

    void OnVolumeChanged(const RawAddress &rawAddr, uint8_t volume, int result)
    {
        HILOGD("%{public}s start.", __func__);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([rawAddr, volume, result](IBluetoothAvrcpCtObserver *observer) {
            observer->OnVolumeChanged(rawAddr,
                static_cast<int32_t>(volume),
                static_cast<int32_t>(result));
        });
    }

    std::mutex serviceMutex_;
    IProfileAvrcpCt *service_;

    std::mutex observerMutex_;
    RemoteObserverList<IBluetoothAvrcpCtObserver> observers_;

    std::unique_ptr<ObserverImpl> observer_;
    std::unique_ptr<SysStsObserverImpl> sysObserver_;
};

BluetoothAvrcpCtServer::BluetoothAvrcpCtServer()
{
    HILOGD("%{public}s start.", __func__);

    pimpl = std::make_unique<impl>();
}

BluetoothAvrcpCtServer::~BluetoothAvrcpCtServer()
{
    pimpl = nullptr;
}

void BluetoothAvrcpCtServer::RegisterObserver(const sptr<IBluetoothAvrcpCtObserver> &observer)
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

void BluetoothAvrcpCtServer::UnregisterObserver(const sptr<IBluetoothAvrcpCtObserver> &observer)
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

std::vector<RawAddress> BluetoothAvrcpCtServer::GetConnectedDevices()
{
    HILOGD("%{public}s start.", __func__);
    std::vector<RawAddress> devices {};
    if (pimpl->IsEnabled()) {
        devices = pimpl->service_->GetConnectedDevices();
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return devices;
}

std::vector<RawAddress> BluetoothAvrcpCtServer::GetDevicesByStates(const std::vector<int32_t> &states)
{
    HILOGD("%{public}s start.", __func__);
    std::vector<RawAddress> devices {};
    if (pimpl->IsEnabled()) {
        std::vector<int> convertStates;
        for (auto state : states) {
            convertStates.push_back(static_cast<int>(state));
        }
        
        devices = pimpl->service_->GetDevicesByStates(convertStates);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);
    return devices;
}

int32_t BluetoothAvrcpCtServer::GetDeviceState(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetDeviceState(BluetoothRawAddress(device));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::Connect(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->Connect(BluetoothRawAddress(device));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::Disconnect(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->Disconnect(BluetoothRawAddress(device));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }

    HILOGD("%{public}s end.", __func__);
    return result;
}

int32_t BluetoothAvrcpCtServer::PressButton(const RawAddress &device, int32_t button)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: button = %{public}d", __func__, button);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->PressButton(BluetoothRawAddress(device), static_cast<uint8_t>(button));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::ReleaseButton(const RawAddress &device, int32_t button)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: button = %{public}d", __func__, button);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->ReleaseButton(BluetoothRawAddress(device), static_cast<uint8_t>(button));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetUnitInfo(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetUnitInfo(BluetoothRawAddress(device));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetSubUnitInfo(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetSubUnitInfo(BluetoothRawAddress(device));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetSupportedCompanies(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetSupportedCompanies(BluetoothRawAddress(device));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetSupportedEvents(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetSupportedEvents(device);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingAttributes(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetPlayerAppSettingAttributes(BluetoothRawAddress(device));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingValues(const RawAddress &device, int32_t attribute)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: attribute = %{public}d", __func__, attribute);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetPlayerAppSettingValues(BluetoothRawAddress(device), static_cast<uint8_t>(attribute));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingCurrentValue(const RawAddress &device,
        const std::vector<int32_t> &attributes)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;
    std::vector<uint8_t> myAttributes;

    for (auto attribute : attributes) {
        HILOGD("%{public}s: attribute = %{public}d", __func__, attribute);
        myAttributes.push_back(static_cast<uint8_t>(attribute));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetPlayerAppSettingCurrentValue(BluetoothRawAddress(device), myAttributes);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::SetPlayerAppSettingCurrentValue(const RawAddress &device,
        const std::vector<int32_t> &attributes, const std::vector<int32_t> &values)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;
    std::vector<uint8_t> myAttributes;
    std::vector<uint8_t> myValues;
    for (auto attribute : attributes) {
        HILOGD("%{public}s: attribute = %{public}d", __func__, attribute);
        myAttributes.push_back(static_cast<uint8_t>(attribute));
    }
    for (auto value : values) {
        HILOGD("%{public}s: value = %{public}d", __func__, value);
        myValues.push_back(static_cast<uint8_t>(value));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->SetPlayerAppSettingCurrentValue(BluetoothRawAddress(device), myAttributes, myValues);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingAttributeText(const RawAddress &device,
        const std::vector<int32_t> &attributes)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;
    if (pimpl->IsEnabled()) {
        std::vector<uint8_t> attrs;
        for (auto attribute : attributes) {
            HILOGD("%{public}s: attribute = %{public}d", __func__, attribute);
            attrs.push_back(static_cast<uint8_t>(attribute));
        }

        result = pimpl->service_->GetPlayerAppSettingAttributeText(BluetoothRawAddress(device), attrs);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingValueText(const RawAddress &device, int32_t attributes,
        const std::vector<int32_t> &values)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: button = %{public}d", __func__, attributes);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        std::vector<uint8_t> myValues;
        for (auto value : values) {
            HILOGD("%{public}s: value = %{public}d", __func__, value);
            myValues.push_back(static_cast<uint8_t>(value));
        }

        result =
            pimpl->service_->GetPlayerAppSettingValueText(BluetoothRawAddress(device), static_cast<uint8_t>(attributes), myValues);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetElementAttributes(const RawAddress &device,
        const std::vector<int32_t> &attributes) 
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        std::vector<uint32_t> attrs;
        for (auto attribute : attributes) {
            HILOGD("%{public}s: attribute = %{public}d", __func__, attribute);
            attrs.push_back(static_cast<uint32_t>(attribute));
        }

        result =
            pimpl->service_->GetElementAttributes(BluetoothRawAddress(device), AVRC_ELEMENT_ATTRIBUTES_IDENTIFIER_PLAYING, attrs);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayStatus(const RawAddress &device)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetPlayStatus(BluetoothRawAddress(device));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::PlayItem(const RawAddress &device, int32_t scope, int64_t uid, int32_t uidCounter) 
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: scope = %{public}d, uid = %{public}jd, uidCounter = %{public}d", __func__, scope, uid, uidCounter);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->PlayItem(
            BluetoothRawAddress(device), static_cast<uint8_t>(scope),
            static_cast<uint64_t>(uid), static_cast<uint16_t>(uidCounter));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetFolderItems(const RawAddress &device, int32_t startItem, int32_t endItem,
        const std::vector<int32_t> &attributes)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: startItem = %{public}d, endItem = %{public}d", __func__, startItem, endItem);
    int32_t result = 0;
    if (pimpl->IsEnabled()) {
        std::vector<uint32_t> attrs;
        for (auto attribute : attributes) {
            HILOGD("%{public}s: attribute = %{public}d,", __func__, attribute);
            attrs.push_back(static_cast<uint32_t>(attribute));
        }
        result = pimpl->service_->GetFolderItems(BluetoothRawAddress(device),
            AVRC_MEDIA_SCOPE_NOW_PLAYING,
            static_cast<uint32_t>(startItem),
            static_cast<uint32_t>(endItem),
            attrs);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetTotalNumberOfItems(const RawAddress &device, int32_t scope)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: scope = %{public}d,", __func__, scope);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetTotalNumberOfItems(BluetoothRawAddress(device), static_cast<uint8_t>(scope));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::SetAbsoluteVolume(const RawAddress &device, int32_t volume)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: volume = %{public}d,", __func__, volume);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->SetAbsoluteVolume(BluetoothRawAddress(device), static_cast<uint8_t>(volume));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::EnableNotification(const RawAddress &device,
        const std::vector<int32_t> &events, int32_t interval)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: interval = %{public}d,", __func__, interval);
    int32_t result = 0;

    std::vector<uint8_t> myEvents;
    for (auto event : events) {
        HILOGD("%{public}s: event = %{public}d,", __func__, event);
        myEvents.push_back(static_cast<uint8_t>(event));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->EnableNotification(BluetoothRawAddress(device), myEvents, static_cast<uint32_t>(interval));
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::DisableNotification(const RawAddress &device, const std::vector<int32_t> &events)
{
    HILOGD("%{public}s start.", __func__);
    int32_t result = 0;

    std::vector<uint8_t> myEvents;
    for (auto event : events) {
        HILOGD("%{public}s: event = %{public}d,", __func__, event);
        myEvents.push_back(static_cast<uint8_t>(event));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->DisableNotification(BluetoothRawAddress(device), myEvents);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetItemAttributes(const RawAddress &device, int64_t uid, int32_t uidCounter,
        const std::vector<int32_t> &attributes) 
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: uid = %{public}jd, uidCounter = %{public}d", __func__, uid, uidCounter);
    int32_t result = 0;

    std::vector<uint32_t> myAttribtues;
    for (auto attribue : attributes) {
        HILOGD("%{public}s: attribue = %{public}d", __func__, attribue);
        myAttribtues.push_back(static_cast<uint32_t>(attribue));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetItemAttributes(
            BluetoothRawAddress(device), AVRC_MEDIA_SCOPE_NOW_PLAYING, (uint64_t)uid, (uint16_t)uidCounter, myAttribtues);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::SetBrowsedPlayer(const RawAddress &device, int32_t playerId)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: playerId = %{public}d", __func__, playerId);
    int32_t result = 0;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->SetBrowsedPlayer(BluetoothRawAddress(device), (uint16_t)playerId);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

int32_t BluetoothAvrcpCtServer::GetMeidaPlayerList(const RawAddress &device, int32_t startItem, int32_t endItem)
{
    HILOGD("%{public}s start.", __func__);
    HILOGD("%{public}s: startItem = %{public}d, endItem = %{public}d", __func__, startItem, endItem);
    int32_t result = 0;
    std::vector<uint32_t> attrs;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetFolderItems(BluetoothRawAddress(device),
            AVRC_MEDIA_SCOPE_PLAYER_LIST,
            static_cast<uint32_t>(startItem),
            static_cast<uint32_t>(endItem),
            attrs);
    } else {
        HILOGE("%{public}s: service is null or disable ", __func__);
    }
    HILOGD("%{public}s end.", __func__);

    return result;
}

}  // namespace binder

}  // namespace ipc
