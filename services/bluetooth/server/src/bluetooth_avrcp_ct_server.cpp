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
#define LOG_TAG "bt_server_avrcp_ct"
#endif

#include <mutex>
#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_avrcp_ct_server.h"
#include "bluetooth_log.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_utils_server.h"
#include "interface_adapter_manager.h"
#include "interface_profile.h"
#include "interface_profile_avrcp_ct.h"
#include "interface_profile_manager.h"
#include "remote_observer_list.h"
#include "bluetooth_observer_application_container.h"
#include "permission_manager.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

struct BluetoothAvrcpCtServer::impl {
public:
    class SysStsObserverImpl : public ISystemStateObserver {
    public:
        explicit SysStsObserverImpl(BluetoothAvrcpCtServer::impl *impl) : impl_(impl)
        {}
        ~SysStsObserverImpl() override
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
        ~ObserverImpl() override
        {}

        void OnConnectionStateChanged(const RawAddress &rawAddr, int state) override
        {
            int cause = static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE);
            if (state == static_cast<int>(BTConnectState::DISCONNECTED) {
                cause = static_cast<int>(ConnChangeCause::DISCONNECT_CAUSE_CONNECT_FAIL_INTERNAL);
            }
            impl_->OnConnectionStateChanged(rawAddr, state, cause);
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
        auto servManager = IProfileManager::GetInstance();
        service_ = static_cast<IProfileAvrcpCt *>(servManager->GetProfileService(PROFILE_NAME_AVRCP_CT));

        return (service_ != nullptr && service_->IsEnabled());
    }

    void OnSystemStateChange(const BTSystemState state)
    {
        HILOGI("state: %{public}d", state);

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

    void OnConnectionStateChanged(const RawAddress &rawAddr, int state, int cause)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("address: %{public}s, state: %{public}d, cause: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr),
            state, cause);
        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, state, cause](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr, static_cast<int32_t>(state), cause);
            }
        });
    }

    void OnPressButton(const RawAddress &rawAddr, uint8_t button, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("address: %{public}s, button: %{public}d, result: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), button, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, button, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnPressButton(randomAddr,
                    static_cast<int32_t>(button),
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnReleaseButton(const RawAddress &rawAddr, uint8_t button, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("address: %{public}s, button: %{public}d, result: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), button, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, button, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnReleaseButton(randomAddr,
                    static_cast<int32_t>(button),
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
        const std::vector<std::string> &folderNames, int result, int detail)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), result, detail);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, uidCounter, numOfItems, folderNames,
            result, detail](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                tokenId, rawAddr, randomAddr)) {
                observer->OnSetBrowsedPlayer(randomAddr,
                    static_cast<int32_t>(uidCounter),
                    static_cast<int32_t>(numOfItems),
                    folderNames,
                    static_cast<int32_t>(result),
                    static_cast<int32_t>(detail));
            }
        });
    }

    void OnGetCapabilities(const RawAddress &rawAddr, const std::vector<uint32_t> &companies,
        const std::vector<uint8_t> &events, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, companies, events, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnGetCapabilities(randomAddr,
                    companies,
                    events,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnGetPlayerAppSettingAttributes(const RawAddress &rawAddr, std::vector<uint8_t> attributes, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, attributes, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnGetPlayerAppSettingAttributes(randomAddr,
                    attributes, static_cast<int32_t>(result));
            }
        });
    }

    void OnGetPlayerAppSettingValues(
        const RawAddress &rawAddr, uint8_t attribute, const std::vector<uint8_t> values, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, attribute: %{public}d, res: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), attribute, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, attribute, values,
            result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                tokenId, rawAddr, randomAddr)) {
                observer->OnGetPlayerAppSettingValues(randomAddr,
                    static_cast<int32_t>(attribute), values,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, attributes, values,
            result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                tokenId, rawAddr, randomAddr)) {
                observer->OnGetPlayerAppSettingCurrentValue(randomAddr,
                    attributes,
                    values,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnSetPlayerAppSettingCurrentValue(randomAddr,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<std::string> &attributeName, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, attributes, attributeName,
            result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                tokenId, rawAddr, randomAddr)) {
                observer->OnGetPlayerAppSettingAttributeText(randomAddr,
                    attributes,
                    attributeName,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr, const std::vector<uint8_t> &values,
        const std::vector<std::string> &valueName, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, values, valueName,
            result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                tokenId, rawAddr, randomAddr)) {
                observer->OnGetPlayerAppSettingValueText(randomAddr,
                    values, valueName, static_cast<int32_t>(result));
            }
        });
    }

    void OnGetElementAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &valueName, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, attributes, valueName,
            result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(
                tokenId, rawAddr, randomAddr)) {
                observer->OnGetElementAttributes(randomAddr,
                    attributes,
                    valueName,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnGetPlayStatus(
        const RawAddress &rawAddr, uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, songLength: %{public}u, songPosition: %{public}u, playStatus: %{public}d, "
            "res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), songLength, songPosition, playStatus, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, songLength, songPosition,
            playStatus, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnGetPlayStatus(randomAddr,
                    songLength,
                    songPosition,
                    static_cast<int32_t>(playStatus),
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnPlayItem(const RawAddress &rawAddr, int result, int detail)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), result, detail);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, result, detail](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnPlayItem(randomAddr,
                    static_cast<int32_t>(result),
                    static_cast<int32_t>(detail));
            }
        });
    }

    void OnGetMediaPlayers(
        const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMpItem> &items, int result, int detail)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), result, detail);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, uidCounter, items,
            result, detail](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                std::vector<BluetoothAvrcpMpItem> myItems;
                for (size_t i = 0; i < items.size(); i++) {
                    myItems.push_back(BluetoothAvrcpMpItem(items.at(i)));
                }
                observer->OnGetMediaPlayers(randomAddr,
                    static_cast<int32_t>(uidCounter),
                    myItems,
                    static_cast<int32_t>(result),
                    static_cast<int32_t>(detail));
            }
        });
    }

    void OnGetFolderItems(
        const RawAddress &rawAddr, uint16_t uidCounter, const std::vector<AvrcMeItem> &items, int result, int detail)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, uidCounter: %{public}hu, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), uidCounter, result, detail);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, uidCounter, items,
            result, detail](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                std::vector<BluetoothAvrcpMeItem> myItems;
                for (size_t i = 0; i < items.size(); i++) {
                    myItems.push_back(BluetoothAvrcpMeItem(items.at(i)));
                }
                observer->OnGetFolderItems(randomAddr,
                    static_cast<int32_t>(uidCounter),
                    myItems,
                    static_cast<int32_t>(result),
                    static_cast<int32_t>(detail));
            }
        });
    }

    void OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &values, int result, int detail)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), result, detail);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, attributes, values,
            result, detail](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnGetItemAttributes(randomAddr,
                    attributes,
                    values,
                    static_cast<int32_t>(result),
                    static_cast<int32_t>(detail));
            }
        });
    }

    void OnGetTotalNumberOfItems(
        const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems, int result, int detail)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, uidCounter: %{public}hu, numOfItems: %{public}u, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), uidCounter, numOfItems, result, detail);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, uidCounter, numOfItems,
            result, detail](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnGetTotalNumberOfItems(randomAddr,
                    static_cast<int32_t>(uidCounter),
                    numOfItems,
                    static_cast<int32_t>(result),
                    static_cast<int32_t>(detail));
            }
        });
    }

    void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, volume: %{public}d, res: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), volume, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, volume, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnSetAbsoluteVolume(randomAddr,
                    static_cast<int32_t>(volume),
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnPlaybackStatusChanged(const RawAddress &rawAddr, uint8_t playStatus, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, playStatus: %{public}d, res: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), playStatus, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, playStatus, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnPlaybackStatusChanged(randomAddr,
                    static_cast<int32_t>(playStatus),
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnTrackChanged(const RawAddress &rawAddr, uint64_t uid, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, uid: %{public}llu, res: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), static_cast<unsigned long long>(uid), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, uid, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnTrackChanged(randomAddr,
                    uid,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnTrackReachedEnd(const RawAddress &rawAddr, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnTrackReachedEnd(randomAddr,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnTrackReachedStart(const RawAddress &rawAddr, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnTrackReachedStart(randomAddr,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnPlaybackPosChanged(const RawAddress &rawAddr, uint32_t playbackPos, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, playbackPos: %{public}u, res: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), playbackPos, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, playbackPos, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnPlaybackPosChanged(randomAddr,
                    playbackPos,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnPlayerAppSettingChanged(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, attributes, values,
            result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnPlayerAppSettingChanged(randomAddr,
                    attributes,
                    values,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnNowPlayingContentChanged(const RawAddress &rawAddr, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnNowPlayingContentChanged(randomAddr,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnAvailablePlayersChanged(const RawAddress &rawAddr, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, res: %{public}d", GET_ENCRYPT_AVRCP_ADDR(rawAddr), result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnAvailablePlayersChanged(randomAddr,
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnAddressedPlayerChanged(const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, playerId: %{public}hu, uidCounter: %{public}hu, res: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), playerId, uidCounter, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, playerId, uidCounter,
            result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnAddressedPlayerChanged(randomAddr,
                    static_cast<int32_t>(playerId),
                    static_cast<int32_t>(uidCounter),
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnUidChanged(const RawAddress &rawAddr, uint16_t uidCounter, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, uidCounter: %{public}hu, res: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), uidCounter, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, uidCounter, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnUidChanged(randomAddr,
                    static_cast<int32_t>(uidCounter),
                    static_cast<int32_t>(result));
            }
        });
    }

    void OnVolumeChanged(const RawAddress &rawAddr, uint8_t volume, int result)
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, volume: %{public}d, res: %{public}d",
            GET_ENCRYPT_AVRCP_ADDR(rawAddr), volume, result);

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([this, rawAddr, volume, result](IBluetoothAvrcpCtObserver *observer) {
            uint64_t tokenId = appContainer_->GetTokenId(observer->AsObject());
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, rawAddr, randomAddr)) {
                observer->OnVolumeChanged(randomAddr,
                    static_cast<int32_t>(volume),
                    static_cast<int32_t>(result));
            }
        });
    }

    std::mutex serviceMutex_;
    IProfileAvrcpCt *service_;

    std::mutex observerMutex_;
    RemoteObserverList<IBluetoothAvrcpCtObserver> observers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;

    std::unique_ptr<ObserverImpl> observer_;
    std::unique_ptr<SysStsObserverImpl> sysObserver_;
};

BluetoothAvrcpCtServer::BluetoothAvrcpCtServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    pimpl->appContainer_->Init();
}

BluetoothAvrcpCtServer::~BluetoothAvrcpCtServer()
{
    pimpl = nullptr;
}

void BluetoothAvrcpCtServer::RegisterObserver(const sptr<IBluetoothAvrcpCtObserver> &observer)
{
    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);

    if (observer == nullptr) {
        HILOGE("observer is NULL.");
        return ;
    }
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ too much");
        return;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
    return ;
}

void BluetoothAvrcpCtServer::UnregisterObserver(const sptr<IBluetoothAvrcpCtObserver> &observer)
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

std::vector<RawAddress> BluetoothAvrcpCtServer::GetConnectedDevices()
{
    std::vector<RawAddress> devices {};
    if (pimpl->IsEnabled()) {
        std::vector<RawAddress> realDevices = pimpl->service_->GetConnectedDevices();
        BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(realDevices, devices);
    } else {
        HILOGE("service is null or disable");
    }
    return devices;
}

std::vector<RawAddress> BluetoothAvrcpCtServer::GetDevicesByStates(const std::vector<int32_t> &states)
{
    std::vector<RawAddress> devices {};
    if (pimpl->IsEnabled()) {
        std::vector<int> convertStates;
        for (auto state : states) {
            convertStates.push_back(static_cast<int>(state));
        }

        std::vector<RawAddress> realDevices = pimpl->service_->GetDevicesByStates(convertStates);
        BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(realDevices, devices);
    } else {
        HILOGE("service is null or disable");
    }
    return devices;
}

int32_t BluetoothAvrcpCtServer::GetDeviceState(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetDeviceState(BluetoothRawAddress(realAddr));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::Connect(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->Connect(BluetoothRawAddress(realAddr));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::Disconnect(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->Disconnect(BluetoothRawAddress(realAddr));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::PressButton(const RawAddress &device, int32_t button)
{
    HILOGI("addr: %{public}s, button: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), button);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->PressButton(BluetoothRawAddress(realAddr), static_cast<uint8_t>(button));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::ReleaseButton(const RawAddress &device, int32_t button)
{
    HILOGI("addr: %{public}s, button: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), button);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->ReleaseButton(BluetoothRawAddress(realAddr), static_cast<uint8_t>(button));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetUnitInfo(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetUnitInfo(BluetoothRawAddress(realAddr));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetSubUnitInfo(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetSubUnitInfo(BluetoothRawAddress(realAddr));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetSupportedCompanies(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetSupportedCompanies(BluetoothRawAddress(realAddr));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetSupportedEvents(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetSupportedEvents(realAddr);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingAttributes(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetPlayerAppSettingAttributes(BluetoothRawAddress(realAddr));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingValues(const RawAddress &device, int32_t attribute)
{
    HILOGI("addr: %{public}s, attribute: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), attribute);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetPlayerAppSettingValues(BluetoothRawAddress(realAddr),
            static_cast<uint8_t>(attribute));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingCurrentValue(const RawAddress &device,
    const std::vector<int32_t> &attributes)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    std::vector<uint8_t> myAttributes;
    for (auto attribute : attributes) {
        HILOGI("attribute: %{public}d", attribute);
        myAttributes.push_back(static_cast<uint8_t>(attribute));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetPlayerAppSettingCurrentValue(BluetoothRawAddress(realAddr), myAttributes);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::SetPlayerAppSettingCurrentValue(const RawAddress &device,
    const std::vector<int32_t> &attributes, const std::vector<int32_t> &values)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<uint8_t> myAttributes;
    std::vector<uint8_t> myValues;
    for (auto attribute : attributes) {
        HILOGI("attribute: %{public}d", attribute);
        myAttributes.push_back(static_cast<uint8_t>(attribute));
    }
    for (auto value : values) {
        HILOGI("value: %{public}d", value);
        myValues.push_back(static_cast<uint8_t>(value));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->SetPlayerAppSettingCurrentValue(BluetoothRawAddress(realAddr),
            myAttributes, myValues);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingAttributeText(const RawAddress &device,
    const std::vector<int32_t> &attributes)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        std::vector<uint8_t> attrs;
        for (auto attribute : attributes) {
            HILOGI("attribute: %{public}d", attribute);
            attrs.push_back(static_cast<uint8_t>(attribute));
        }
        result = pimpl->service_->GetPlayerAppSettingAttributeText(BluetoothRawAddress(realAddr), attrs);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayerAppSettingValueText(const RawAddress &device, int32_t attributes,
    const std::vector<int32_t> &values)
{
    HILOGI("addr: %{public}s, attributes: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), attributes);
    int32_t result = 0;
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        std::vector<uint8_t> myValues;
        for (auto value : values) {
            HILOGI("value: %{public}d", value);
            myValues.push_back(static_cast<uint8_t>(value));
        }
        result =
            pimpl->service_->GetPlayerAppSettingValueText(BluetoothRawAddress(realAddr),
                static_cast<uint8_t>(attributes), myValues);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetElementAttributes(const RawAddress &device,
    const std::vector<int32_t> &attributes)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        std::vector<uint32_t> attrs;
        for (auto attribute : attributes) {
            HILOGI("attribute: %{public}d", attribute);
            attrs.push_back(static_cast<uint32_t>(attribute));
        }
        result = pimpl->service_->GetElementAttributes(
            BluetoothRawAddress(realAddr), AVRC_ELEMENT_ATTRIBUTES_IDENTIFIER_PLAYING, attrs);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetPlayStatus(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetPlayStatus(BluetoothRawAddress(realAddr));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::PlayItem(const RawAddress &device, int32_t scope, int64_t uid, int32_t uidCounter)
{
    HILOGI("res: %{public}s, scope: %{public}d, uid: %{public}jd, uidCounter: %{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), scope, uid, uidCounter);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->PlayItem(
            BluetoothRawAddress(realAddr), static_cast<uint8_t>(scope),
            static_cast<uint64_t>(uid), static_cast<uint16_t>(uidCounter));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetFolderItems(const RawAddress &device, int32_t startItem, int32_t endItem,
    const std::vector<int32_t> &attributes)
{
    HILOGI("res: %{public}s, startItem: %{public}d, endItem: %{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), startItem, endItem);
    int32_t result = 0;
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        std::vector<uint32_t> attrs;
        for (auto attribute : attributes) {
            HILOGI("attribute: %{public}d,", attribute);
            attrs.push_back(static_cast<uint32_t>(attribute));
        }
        result = pimpl->service_->GetFolderItems(BluetoothRawAddress(realAddr),
            AVRC_MEDIA_SCOPE_NOW_PLAYING,
            static_cast<uint32_t>(startItem),
            static_cast<uint32_t>(endItem),
            attrs);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetTotalNumberOfItems(const RawAddress &device, int32_t scope)
{
    HILOGI("addr: %{public}s, scope: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), scope);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetTotalNumberOfItems(BluetoothRawAddress(realAddr), static_cast<uint8_t>(scope));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::SetAbsoluteVolume(const RawAddress &device, int32_t volume)
{
    HILOGI("addr: %{public}s, volume: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), volume);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->SetAbsoluteVolume(BluetoothRawAddress(realAddr), static_cast<uint8_t>(volume));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::EnableNotification(const RawAddress &device,
    const std::vector<int32_t> &events, int32_t interval)
{
    HILOGI("addr: %{public}s, interval: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), interval);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<uint8_t> myEvents;
    for (auto event : events) {
        HILOGI("event: %{public}d", event);
        myEvents.push_back(static_cast<uint8_t>(event));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->EnableNotification(BluetoothRawAddress(realAddr),
            myEvents, static_cast<uint32_t>(interval));
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::DisableNotification(const RawAddress &device, const std::vector<int32_t> &events)
{
    HILOGI("addr: %{public}s", GetEncryptAddr(device.GetAddress()).c_str());
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<uint8_t> myEvents;
    for (auto event : events) {
        HILOGI("event: %{public}d,", event);
        myEvents.push_back(static_cast<uint8_t>(event));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->DisableNotification(BluetoothRawAddress(realAddr), myEvents);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetItemAttributes(const RawAddress &device, int64_t uid, int32_t uidCounter,
    const std::vector<int32_t> &attributes)
{
    HILOGI("res: %{public}s, uid: %{public}jd, uidCounter: %{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), uid, uidCounter);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<uint32_t> myAttribtues;
    for (auto attribue : attributes) {
        HILOGI("attribue = %{public}d", attribue);
        myAttribtues.push_back(static_cast<uint32_t>(attribue));
    }

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetItemAttributes(BluetoothRawAddress(realAddr),
            AVRC_MEDIA_SCOPE_NOW_PLAYING, (uint64_t)uid, (uint16_t)uidCounter, myAttribtues);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::SetBrowsedPlayer(const RawAddress &device, int32_t playerId)
{
    HILOGI("addr: %{public}s, playerId: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), playerId);
    int32_t result = 0;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->IsEnabled()) {
        result = pimpl->service_->SetBrowsedPlayer(BluetoothRawAddress(realAddr), (uint16_t)playerId);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

int32_t BluetoothAvrcpCtServer::GetMeidaPlayerList(const RawAddress &device, int32_t startItem, int32_t endItem)
{
    HILOGI("res: %{public}s: startItem: %{public}d, endItem: %{public}d",
        GetEncryptAddr(device.GetAddress()).c_str(), startItem, endItem);
    int32_t result = 0;
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<uint32_t> attrs;

    if (pimpl->IsEnabled()) {
        result = pimpl->service_->GetFolderItems(BluetoothRawAddress(realAddr),
            AVRC_MEDIA_SCOPE_PLAYER_LIST,
            static_cast<uint32_t>(startItem),
            static_cast<uint32_t>(endItem),
            attrs);
    } else {
        HILOGE("service is null or disable");
    }
    return result;
}

}  // namespace Bluetooth

}  // namespace OHOS
