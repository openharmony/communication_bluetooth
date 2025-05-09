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
#define LOG_TAG "bt_fwk_avrcp_tg"
#endif

#include <deque>
#include <list>
#include <mutex>

#include "bluetooth_avrcp_ct.h"
#include "bluetooth_avrcp_ct_observer_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "i_bluetooth_avrcp_ct.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
std::mutex g_avrcpProxyMutex;
AvrcpCtResponse::AvrcpCtResponse(uint8_t type, int resp) : type_(type), resp_(resp)
{
    HILOGI("enter");
}

AvrcpCtResponse::~AvrcpCtResponse()
{
    HILOGI("enter");
}

struct AvrcpController::impl {
public:
    class ObserverImpl : public BluetoothAvrcpCtObserverStub {
    public:
        explicit ObserverImpl(AvrcpController::impl *impl) : impl_(impl)
        {}
        ~ObserverImpl() override = default;

        void OnConnectionStateChanged(const RawAddress &rawAddr, int state, int cause) override
        {
            HILOGD("enter, address: %{public}s, state: %{public}d, cause: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), state, cause);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnConnectionStateChanged(device, static_cast<int>(state), cause);

            return;
        }

        void OnPressButton(const RawAddress &rawAddr, uint8_t button, int result) override
        {
            HILOGI("enter, address: %{public}s, button: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), button, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnPressButton(device, static_cast<uint8_t>(button), static_cast<int>(result));

            return;
        }

        void OnReleaseButton(const RawAddress &rawAddr, uint8_t button,  int result) override
        {
            HILOGI("enter, address: %{public}s, button: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), button, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnReleaseButton(device, static_cast<uint8_t>(button), static_cast<int>(result));

            return;
        }

        void OnSetBrowsedPlayer(const RawAddress &rawAddr, uint16_t uidCounter,
            uint32_t numberOfItems, const std::vector<std::string> &folderNames, int result, int detail) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d, detail: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), result, detail);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnSetBrowsedPlayer(device,
                static_cast<uint16_t>(uidCounter),
                static_cast<uint32_t>(numberOfItems),
                folderNames,
                result,
                detail);
            return;
        }

        void OnGetCapabilities(const RawAddress &rawAddr, const std::vector<uint32_t> &companies,
            const std::vector<uint8_t> &events, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnGetCapabilities(device, companies, events, result);

            return;
        }

        void OnGetPlayerAppSettingAttributes(
            const RawAddress &rawAddr, std::vector<uint8_t> attributes, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnGetPlayerAppSettingAttributes(device, attributes, static_cast<int>(result));

            return;
        }

        void OnGetPlayerAppSettingValues(const RawAddress &rawAddr,
            int attribute, const std::vector<uint8_t> &values, int result) override
        {
            HILOGI("enter, address: %{public}s, attribute: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), attribute, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnGetPlayerAppSettingValues(
                device, static_cast<uint8_t>(attribute), values, static_cast<int>(result));

            return;
        }

        void OnGetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, const std::vector<uint8_t> attributes,
            const std::vector<uint8_t> &values, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnGetPlayerAppSettingCurrentValue(device, attributes, values, static_cast<int>(result));

            return;
        }

        void OnSetPlayerAppSettingCurrentValue(const RawAddress &rawAddr, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnSetPlayerAppSettingCurrentValue(device, static_cast<int>(result));

            return;
        }

        void OnGetPlayerAppSettingAttributeText(const RawAddress &rawAddr,
            const std::vector<uint8_t> attribtues, const std::vector<std::string> &attributeName, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnGetPlayerAppSettingAttributeText(device, attribtues, attributeName, static_cast<int>(result));

            return;
        }

        void OnGetPlayerAppSettingValueText(const RawAddress &rawAddr,
            const std::vector<uint8_t> &values, const std::vector<std::string> &valueName, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnGetPlayerAppSettingValueText(device, values, valueName, static_cast<int>(result));

            return;
        }

        void OnGetElementAttributes(const RawAddress &rawAddr,
        const std::vector<uint32_t> &attribtues, const std::vector<std::string> &valueName, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnGetElementAttributes(device, attribtues, valueName, static_cast<int>(result));

            return;
        }

        void OnGetPlayStatus(const RawAddress &rawAddr,
            uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result) override
        {
            HILOGI("enter, address: %{public}s, status: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), playStatus, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnGetPlayStatus(device,
                static_cast<uint32_t>(songLength),
                static_cast<uint32_t>(songPosition),
                static_cast<uint8_t>(playStatus),
                static_cast<int>(result));

            return;
        }

        void OnPlayItem(const RawAddress &rawAddr, int status, int result) override
        {
            HILOGI("enter, address: %{public}s, status: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), status, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnPlayItem(device, static_cast<int>(status), static_cast<int>(result));

            return;
        }

        void OnGetMediaPlayers(const RawAddress &rawAddr, uint16_t uidCounter,
            std::vector<BluetoothAvrcpMpItem> &items, int result, int detail) override
        {
            HILOGI("enter, address: %{public}s, uidCounter: %{public}d, res: %{public}d, detail: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), uidCounter, result, detail);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            std::vector<AvrcMpItem> myItems;
            for (size_t i = 0; i < items.size(); i++) {
                AvrcMpItem myItem;
                myItem.itemType_ = items.at(i).avrcpMpItem_.itemType_;
                myItem.playerId_ = items.at(i).avrcpMpItem_.playerId_;
                myItem.majorType_ = items.at(i).avrcpMpItem_.majorType_;
                myItem.subType_ = items.at(i).avrcpMpItem_.subType_;
                myItem.playStatus_ = items.at(i).avrcpMpItem_.playStatus_;
                myItem.features_ = items.at(i).avrcpMpItem_.features_;
                myItem.name_ = items.at(i).avrcpMpItem_.name_;
                myItems.push_back(myItem);
            }

            impl_->OnGetMediaPlayers(
                device, static_cast<uint32_t>(uidCounter), myItems, static_cast<int>(result), static_cast<int>(detail));

            return;
        }

        void OnGetFolderItems(const RawAddress &rawAddr, uint16_t uidCounter, std::vector<BluetoothAvrcpMeItem> &items,
            int result, int detail) override
        {
            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            std::vector<AvrcMeItem> myItems;
            for (size_t i = 0; i < items.size(); i++) {
                AvrcMeItem myItem;
                myItem.uid_ = items.at(i).uid_;
                myItem.type_ = items.at(i).type_;
                myItem.playable_ = items.at(i).playable_;
                myItem.name_ = items.at(i).name_;
                myItem.attributes_ = items.at(i).attributes_;
                myItem.values_ = items.at(i).values_;

                myItems.push_back(myItem);
            }
            impl_->OnGetFolderItems(
                device, static_cast<uint32_t>(uidCounter), myItems, static_cast<int>(result), static_cast<int>(detail));

            return;
        }

        void OnGetItemAttributes(const RawAddress &rawAddr, const std::vector<uint32_t> &attribtues,
            const std::vector<std::string> &values, int result, int detail) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d, detail: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), result, detail);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);

            impl_->OnGetItemAttributes(device, attribtues, values, static_cast<int>(result), static_cast<int>(detail));

            return;
        }

        void OnGetTotalNumberOfItems(const RawAddress &rawAddr, uint16_t uidCounter, uint32_t numOfItems,
            int result, int detail) override
        {
            HILOGI("address: %{public}s, uidCounter: %{public}d, numOfItems: %{public}d, res: %{public}d, "
                "detail: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), uidCounter, numOfItems, result, detail);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnGetTotalNumberOfItems(device,
                static_cast<uint32_t>(uidCounter),
                static_cast<uint32_t>(numOfItems),
                static_cast<uint8_t>(result),
                static_cast<int>(detail));

            return;
        }

        void OnSetAbsoluteVolume(const RawAddress &rawAddr, uint8_t volume, int result) override
        {
            HILOGI("enter, address: %{public}s, volume: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), volume, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnSetAbsoluteVolume(device, static_cast<uint8_t>(volume), result);

            return;
        }

        void OnPlaybackStatusChanged(const RawAddress &rawAddr, uint8_t playStatus, int result) override
        {
            HILOGI("enter, address: %{public}s, status: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), playStatus, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnPlaybackStatusChanged(device, static_cast<uint8_t>(playStatus), static_cast<int>(result));

            return;
        }

        void OnTrackChanged(const RawAddress &rawAddr, uint64_t uid, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnTrackChanged(device, static_cast<uint64_t>(uid), static_cast<int>(result));

            return;
        }

        void OnTrackReachedEnd(const RawAddress &rawAddr, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnTrackReachedEnd(device, static_cast<int>(result));

            return;
        }

        void OnTrackReachedStart(const RawAddress &rawAddr, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnTrackReachedStart(device, static_cast<int>(result));

            return;
        }

        void OnPlaybackPosChanged(const RawAddress &rawAddr, uint32_t playbackPos, int result) override
        {
            HILOGI("enter, address: %{public}s, playbackPos: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), playbackPos, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnPlaybackPosChanged(device, static_cast<uint32_t>(playbackPos), static_cast<int>(result));

            return;
        }

        void OnPlayerAppSettingChanged(const RawAddress &rawAddr, const std::vector<uint8_t> &attributes,
            const std::vector<uint8_t> &values, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnPlayerAppSettingChanged(device, attributes, values, static_cast<int>(result));

            return;
        }

        void OnNowPlayingContentChanged(const RawAddress &rawAddr, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnNowPlayingContentChanged(device, static_cast<int>(result));

            return;
        }

        void OnAvailablePlayersChanged(const RawAddress &rawAddr, int result) override
        {
            HILOGI("enter, address: %{public}s, res: %{public}d", GET_ENCRYPT_RAW_ADDR(rawAddr), result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnAvailablePlayersChanged(device, static_cast<int>(result));

            return;
        }

        void OnAddressedPlayerChanged(
            const RawAddress &rawAddr, uint16_t playerId, uint16_t uidCounter, int result) override
        {
            HILOGI("enter, address: %{public}s, playerId: %{public}d, uidCounter: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), playerId, uidCounter, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnAddressedPlayerChanged(
                device, static_cast<uint16_t>(playerId), static_cast<uint16_t>(uidCounter), static_cast<int>(result));

            return;
        }

        void OnUidChanged(const RawAddress &rawAddr, uint16_t uidCounter, int result) override
        {
            HILOGI("enter, address: %{public}s, uidCounter: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), uidCounter, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnUidChanged(device, static_cast<uint16_t>(uidCounter), static_cast<int>(result));

            return;
        }

        void OnVolumeChanged(const RawAddress &rawAddr, uint8_t volume, int result) override
        {
            HILOGI("enter, address: %{public}s, volume: %{public}d, res: %{public}d",
                GET_ENCRYPT_RAW_ADDR(rawAddr), volume, result);

            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnVolumeChanged(device, static_cast<uint8_t>(volume), static_cast<int>(result));

            return;
        }

    private:
        AvrcpController::impl *impl_;
    };

    impl();
    ~impl()
    {
        HILOGI("enter");
        BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
        sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->UnregisterObserver(observer_);
    }

    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state, int cause)
    {
        HILOGI("enter, device: %{public}s, state: %{public}d, cause: %{public}d",
            GET_ENCRYPT_ADDR(device), state, cause);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, state, cause](std::shared_ptr<IObserver> observer) {
            observer->OnConnectionStateChanged(device, state, cause);
        });
    }

    void OnPressButton(const BluetoothRemoteDevice &device, uint8_t button, int result)
    {
        HILOGI("enter, device: %{public}s, button: %{public}d, res: %{public}d",
            GET_ENCRYPT_ADDR(device), button, result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, button, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_PRESS_BUTTON, result);
            resp.button_ = std::make_unique<AvrcpCtResponse::Button>(button);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnReleaseButton(const BluetoothRemoteDevice &device, uint8_t button, int result)
    {
        HILOGI("enter, device: %{public}s, button: %{public}d, res: %{public}d",
            GET_ENCRYPT_ADDR(device), button, result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, button, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_RELEASE_BUTTON, result);
            resp.button_ = std::make_unique<AvrcpCtResponse::Button>(button);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnSetBrowsedPlayer(const BluetoothRemoteDevice &device, uint16_t uidCounter, uint32_t numOfItems,
        const std::vector<std::string> &folderNames, int result, int detail)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_ADDR(device), result, detail);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_SET_BROWSED_PLAYER, result);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetCapabilities(const BluetoothRemoteDevice &device, const std::vector<uint32_t> &companies,
        const std::vector<uint8_t> &events, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, companies, events, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_CAPABILITIES, result);
            if (companies.size() == 0) {
                resp.capabilities_ = std::make_unique<AvrcpCtResponse::Capabilities>(events);
            } else {
                resp.capabilities_ = std::make_unique<AvrcpCtResponse::Capabilities>(companies);
            }

            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetPlayerAppSettingAttributes(
        const BluetoothRemoteDevice &device, std::vector<uint8_t> attributes, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, attributes, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_PLAYER_APP_SETTING_ATTRIBUTES, result);
            resp.playerAttrs_ = std::make_unique<AvrcpCtResponse::PlayerSettingAttributes>(attributes);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetPlayerAppSettingValues(
        const BluetoothRemoteDevice &device, uint8_t attribute, std::vector<uint8_t> values, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, attribute, values, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_PLAYER_APP_SETTING_VALUES, result);
            resp.playerVals_ = std::make_unique<AvrcpCtResponse::PlayerSettingValues>(attribute, values);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetPlayerAppSettingCurrentValue(
        const BluetoothRemoteDevice &device, std::vector<uint8_t> attributes, std::vector<uint8_t> values, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, attributes, values, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_PLAYER_APP_SETTING_CURRENT_VALUE, result);
            resp.playerCurVal_ = std::make_unique<AvrcpCtResponse::PlayerSettingCurrentValue>(attributes, values);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnSetPlayerAppSettingCurrentValue(const BluetoothRemoteDevice &device, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_SET_PLAYER_APP_SETTING_CURRENT_VALUE, result);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetPlayerAppSettingAttributeText(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes,
        const std::vector<std::string> &valueName, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, attributes, valueName, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_PLAYER_APP_SETTING_ATTRIBUTE_TEXT, result);
            resp.playerText_ = std::make_unique<AvrcpCtResponse::PlayerGettingText>(attributes, valueName);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetPlayerAppSettingValueText(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &values,
        const std::vector<std::string> &valueName, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result, values, valueName](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_PLAYER_APP_SETTING_VALUE_TEXT, result);
            resp.playerText_ = std::make_unique<AvrcpCtResponse::PlayerGettingText>(values, valueName);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetElementAttributes(const BluetoothRemoteDevice &device, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &valueName, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result, attributes, valueName](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_ELEMENT_ATTRIBUTES, result);
            resp.eleSts_ = std::make_unique<AvrcpCtResponse::ElementAttributes>(attributes, valueName);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetPlayStatus(
        const BluetoothRemoteDevice &device, uint32_t songLength, uint32_t songPosition, uint8_t playStatus, int result)
    {
        HILOGI("enter, device: %{public}s, songLength: %{public}d, songPosition: %{public}d, playStatus: %{public}d,"
            " res: %{public}d", GET_ENCRYPT_ADDR(device), songLength, songPosition, playStatus, result);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_PLAY_STATUS, result);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnPlayItem(const BluetoothRemoteDevice &device, int result, int detail)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_ADDR(device), result, detail);

        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_PLAY_ITEM, result);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetMediaPlayers(const BluetoothRemoteDevice &device, uint16_t uidCounter,
        const std::vector<AvrcMpItem> &items, int result, int detail)
    {
        HILOGI("enter, device: %{public}s, uidCounter: %{public}d, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_ADDR(device), uidCounter, result, detail);

        std::vector<AvrcpCtResponse::MediaPlayers::MediaPlayer> MediaPlayers;
        for (int i = 0; i < static_cast<int>(items.size()); i++) {
            AvrcpCtResponse::MediaPlayers::MediaPlayer mediaPlayer;
            mediaPlayer.itemType_ = items.at(i).itemType_;
            mediaPlayer.playerId_ = items.at(i).playerId_;
            mediaPlayer.majorType_ = items.at(i).majorType_;
            mediaPlayer.subType_ = items.at(i).subType_;
            mediaPlayer.playStatus_ = items.at(i).playStatus_;
            mediaPlayer.features_ = items.at(i).features_;
            mediaPlayer.name_ = items.at(i).name_;

            MediaPlayers.push_back(mediaPlayer);
        }

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([device, result, MediaPlayers, uidCounter](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_MEDIA_PLAYER_LIST, result);
            resp.mediaPlayers_ = std::make_unique<AvrcpCtResponse::MediaPlayers>(uidCounter, MediaPlayers);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetFolderItems(const BluetoothRemoteDevice &device, uint16_t uidCounter,
        const std::vector<AvrcMeItem> &items, int result, int detail)
    {
        HILOGI("enter, device: %{public}s, uidCounter: %{public}d, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_ADDR(device), uidCounter, result, detail);

        std::vector<AvrcpCtResponse::MediaItems::MediaItem> mediaItems;
        for (int i = 0; i < static_cast<int>(items.size()); i++) {
            AvrcpCtResponse::MediaItems::MediaItem mediaItem;
            mediaItem.uid_ = items.at(i).uid_;
            mediaItem.type_ = items.at(i).type_;
            mediaItem.playable_ = items.at(i).playable_;
            mediaItem.name_ = items.at(i).name_;
            mediaItem.attributes_ = items.at(i).attributes_;
            mediaItem.values_ = items.at(i).values_;

            mediaItems.push_back(mediaItem);
        }

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([device, result, mediaItems, uidCounter](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_FOLDER_ITEMS, result);
            resp.mediaItems_ = std::make_unique<AvrcpCtResponse::MediaItems>(uidCounter, mediaItems);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetItemAttributes(const BluetoothRemoteDevice &device, const std::vector<uint32_t> &attributes,
        const std::vector<std::string> &values, int result, int detail)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_ADDR(device), result, detail);
        std::vector<AvrcpCtResponse::ItemAttributes::ItemAttribute> itemAttrs;
        for (int i = 0; i < static_cast<int>(attributes.size()); i++) {
            AvrcpCtResponse::ItemAttributes::ItemAttribute itemAttr;
            itemAttr.attribute_ = attributes.at(i);
            itemAttr.value_ = values.at(i);
            itemAttrs.push_back(itemAttr);
        }

        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([device, result, itemAttrs](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_ITEM_ATTRIBUTES, result);
            resp.itemAttrs_ = std::make_unique<AvrcpCtResponse::ItemAttributes>(itemAttrs);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnGetTotalNumberOfItems(
        const BluetoothRemoteDevice &device, uint16_t uidCounter, uint32_t numOfItems, int result, int detail)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d, detail: %{public}d",
            GET_ENCRYPT_ADDR(device), result, detail);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, uidCounter, numOfItems, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_GET_TOTAL_NUMBER_OF_ITEMS, result);
            resp.totalItems_ = std::make_unique<AvrcpCtResponse::TotalNumberOfItems>(uidCounter, numOfItems);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnSetAbsoluteVolume(const BluetoothRemoteDevice &device, uint16_t volume, int result)
    {
        HILOGI("enter, device: %{public}s, volume: %{public}d, res: %{public}d",
            GET_ENCRYPT_ADDR(device), volume, result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, volume, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_SET_ABSOLUTE_VOLUME, result);
            resp.absVolume_ = std::make_unique<AvrcpCtResponse::AbsoluteVolume>(volume);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnPlaybackStatusChanged(const BluetoothRemoteDevice &device, uint8_t playStatus, int result)
    {
        HILOGI("enter, device: %{public}s, playStatus: %{public}d, res: %{public}d",
            GET_ENCRYPT_ADDR(device), playStatus, result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, playStatus, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_PLAYBACK_STATUS_CHANGED, result);
            resp.notify_ = std::make_unique<AvrcpCtResponse::Notification>(playStatus, 0x00);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnTrackChanged(const BluetoothRemoteDevice &device, uint64_t uid, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, uid, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_TRACK_CHANGED, result);
            resp.notify_ = std::make_unique<AvrcpCtResponse::Notification>(uid);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnTrackReachedEnd(const BluetoothRemoteDevice &device, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_TRACK_REACHED_END, result);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnTrackReachedStart(const BluetoothRemoteDevice &device, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_TRACK_REACHED_START, result);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnPlaybackPosChanged(const BluetoothRemoteDevice &device, uint32_t playbackPos, int result)
    {
        HILOGI("enter, device: %{public}s, playbackPos: %{public}d, res: %{public}d",
            GET_ENCRYPT_ADDR(device), playbackPos, result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, playbackPos, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_PLAYBACK_POS_CHANGED, result);
            resp.notify_ = std::make_unique<AvrcpCtResponse::Notification>(playbackPos);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnPlayerAppSettingChanged(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes,
        const std::vector<uint8_t> &values, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, attributes, values, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_PLAYER_APPLICATION_SETTING_CHANGED, result);
            resp.notify_ = std::make_unique<AvrcpCtResponse::Notification>(attributes, values);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnNowPlayingContentChanged(const BluetoothRemoteDevice &device, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_NOW_PLAYING_CONTENT_CHANGED, result);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnAvailablePlayersChanged(const BluetoothRemoteDevice &device, int result)
    {
        HILOGI("enter, device: %{public}s, res: %{public}d", GET_ENCRYPT_ADDR(device), result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_AVAILABLE_PLAYERS_CHANGED, result);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnAddressedPlayerChanged(
        const BluetoothRemoteDevice &device, uint16_t playerId, uint16_t uidCounter, int result)
    {
        HILOGI("enter, device: %{public}s, playerId: %{public}d, uidCounter: %{public}d, res: %{public}d",
            GET_ENCRYPT_ADDR(device), playerId, uidCounter, result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, uidCounter, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_ADDRESSED_PLAYER_CHANGED, result);
            resp.notify_ = std::make_unique<AvrcpCtResponse::Notification>(uidCounter);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnUidChanged(const BluetoothRemoteDevice &device, uint16_t uidCounter, int result)
    {
        HILOGI("enter, device: %{public}s, uidCounter: %{public}d, res: %{public}d",
            GET_ENCRYPT_ADDR(device), uidCounter, result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, result, uidCounter](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_UIDS_CHANGED, result);
            resp.notify_ = std::make_unique<AvrcpCtResponse::Notification>(uidCounter);
            observer->OnActionCompleted(device, resp);
        });
    }

    void OnVolumeChanged(const BluetoothRemoteDevice &device, uint8_t volume, int result)
    {
        HILOGI("enter, device: %{public}s, volume: %{public}d, res: %{public}d",
            GET_ENCRYPT_ADDR(device), volume, result);
        std::lock_guard<std::mutex> lock(observerMutex_);

        observers_.ForEach([device, volume, result](std::shared_ptr<IObserver> observer) {
            AvrcpCtResponse resp(AVRC_ACTION_TYPE_NOTIFY_VOLUME_CHANGED, result);
            resp.notify_ = std::make_unique<AvrcpCtResponse::Notification>(0x00, volume);
            observer->OnActionCompleted(device, resp);
        });
    }
    std::mutex observerMutex_;
    BluetoothObserverList<AvrcpController::IObserver> observers_;

    sptr<ObserverImpl> observer_;
    int32_t profileRegisterId = 0;
};

AvrcpController::impl::impl()
{
    observer_ = new (std::nothrow) ObserverImpl(this);
    CHECK_AND_RETURN_LOG(observer_ != nullptr, "observer_ is nullptr");
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_AVRCP_CT,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothAvrcpCt> proxy = iface_cast<IBluetoothAvrcpCt>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(observer_);
    });
}

AvrcpController *AvrcpController::GetProfile(void)
{
    HILOGI("enter");
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<AvrcpController> instance;
    return instance.get();
#else
    static AvrcpController instance;
    return &instance;
#endif
}

/******************************************************************
 * REGISTER / UNREGISTER OBSERVER                                 *
 ******************************************************************/

void AvrcpController::RegisterObserver(std::shared_ptr<IObserver> observer)
{
    HILOGD("enter");
    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);
    pimpl->observers_.Register(observer);
}

void AvrcpController::UnregisterObserver(std::shared_ptr<IObserver> observer)
{
    HILOGD("enter");
    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);
    pimpl->observers_.Deregister(observer);
}

/******************************************************************
 * CONNECTION                                                     *
 ******************************************************************/

std::vector<BluetoothRemoteDevice> AvrcpController::GetConnectedDevices(void)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }
    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        std::vector<BluetoothRemoteDevice>(), "failed: no proxy");

    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);
    std::vector<BluetoothRemoteDevice> devices;
    std::vector<RawAddress> rawAddrs = proxy->GetConnectedDevices();
    for (auto rawAddr : rawAddrs) {
        BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
        devices.push_back(device);
    }
    return devices;
}

std::vector<BluetoothRemoteDevice> AvrcpController::GetDevicesByStates(const std::vector<int> &states)
{
    HILOGI("enter");

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }
    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        std::vector<BluetoothRemoteDevice>(), "failed: no proxy");

    std::vector<BluetoothRemoteDevice> devices;
    if (proxy != nullptr) {
        std::vector<int32_t> convertStates;
        for (auto state : states) {
            convertStates.push_back(static_cast<int32_t>(state));
        }
        std::vector<RawAddress> rawAddrs = proxy->GetDevicesByStates(convertStates);
        for (auto rawAddr : rawAddrs) {
            BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
            devices.push_back(device);
        }
    }

    return devices;
}

int AvrcpController::GetDeviceState(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        static_cast<int>(BTConnectState::DISCONNECTED), "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetDeviceState(rawAddr);
}

bool AvrcpController::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    int result = proxy->Connect(rawAddr);
    return result == RET_NO_ERROR;
}

bool AvrcpController::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    int result = proxy->Disconnect(rawAddr);
    return result == RET_NO_ERROR;
}

/******************************************************************
 * BUTTON OPERATION                                               *
 ******************************************************************/

int AvrcpController::PressButton(const BluetoothRemoteDevice &device, uint8_t button)
{
    HILOGI("enter, device: %{public}s, button: %{public}d", GET_ENCRYPT_ADDR(device), button);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    int result = RET_BAD_STATUS;
    switch (button) {
        case AVRC_KEY_OPERATION_VOLUME_UP:
        case AVRC_KEY_OPERATION_VOLUME_DOWN:
        case AVRC_KEY_OPERATION_MUTE:
        case AVRC_KEY_OPERATION_PLAY:
        case AVRC_KEY_OPERATION_STOP:
        case AVRC_KEY_OPERATION_PAUSE:
        case AVRC_KEY_OPERATION_REWIND:
        case AVRC_KEY_OPERATION_FAST_FORWARD:
        case AVRC_KEY_OPERATION_FORWARD:
        case AVRC_KEY_OPERATION_BACKWARD: {
            BluetoothRawAddress rawAddr(device.GetDeviceAddr());
            result = proxy->PressButton(rawAddr, static_cast<int32_t>(button));
            break;
        }
        default:
            result = RET_NO_SUPPORT;
            if (button >= AVRC_KEY_OPERATION_INVALID) {
                result = RET_BAD_PARAM;
            }
            break;
    }
    return result;
}

int AvrcpController::ReleaseButton(const BluetoothRemoteDevice &device, uint8_t button)
{
    HILOGI("enter, device: %{public}s, button: %{public}d", GET_ENCRYPT_ADDR(device), button);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    int result = RET_BAD_STATUS;
    switch (button) {
        case AVRC_KEY_OPERATION_VOLUME_UP:
        case AVRC_KEY_OPERATION_VOLUME_DOWN:
        case AVRC_KEY_OPERATION_MUTE:
        case AVRC_KEY_OPERATION_PLAY:
        case AVRC_KEY_OPERATION_STOP:
        case AVRC_KEY_OPERATION_PAUSE:
        case AVRC_KEY_OPERATION_REWIND:
        case AVRC_KEY_OPERATION_FAST_FORWARD:
        case AVRC_KEY_OPERATION_FORWARD:
        case AVRC_KEY_OPERATION_BACKWARD: {
            BluetoothRawAddress rawAddr(device.GetDeviceAddr());
            result = proxy->ReleaseButton(rawAddr, static_cast<int32_t>(button));
            break;
        }
        default:
            result = RET_NO_SUPPORT;
            if (button >= AVRC_KEY_OPERATION_INVALID) {
                result = RET_BAD_PARAM;
            }
            break;
    }
    return result;
}

/******************************************************************
 * TEMP UNIT INFO / SUB UNIT INFO                                 *
 ******************************************************************/

int AvrcpController::GetUnitInfo(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetUnitInfo(rawAddr);
}

int AvrcpController::GetSubUnitInfo(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetSubUnitInfo(rawAddr);
}

/******************************************************************
 * Capabilities                                                   *
 ******************************************************************/

int AvrcpController::GetSupportedCompanies(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetSupportedCompanies(rawAddr);
}

int AvrcpController::GetSupportedEvents(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetSupportedEvents(rawAddr);
}

/******************************************************************
 * PLAYER APPLICATION SETTINGS                                     *
 ******************************************************************/

int AvrcpController::GetPlayerAppSettingAttributes(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetPlayerAppSettingAttributes(rawAddr);
}

int AvrcpController::GetPlayerAppSettingValues(const BluetoothRemoteDevice &device, uint8_t attribute)
{
    HILOGI("enter, device: %{public}s, attribute: %{public}d", GET_ENCRYPT_ADDR(device), attribute);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    int result = RET_BAD_STATUS;
    do {
        if (attribute == AVRC_PLAYER_ATTRIBUTE_ILLEGAL) {
            result = RET_BAD_PARAM;
            break;
        }
        if (attribute >= AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN && attribute <= AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX) {
            result = RET_BAD_PARAM;
            break;
        }
        BluetoothRawAddress rawAddr(device.GetDeviceAddr());
        result = proxy->GetPlayerAppSettingValues(rawAddr, static_cast<int32_t>(attribute));
    } while (false);

    return result;
}

int AvrcpController::GetPlayerAppSettingCurrentValue(
    const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_NO_ERROR;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    int result = RET_NO_ERROR;
    do {
        std::vector<int32_t> attrs;
        for (auto attribute : attributes) {
            if (attribute == AVRC_PLAYER_ATTRIBUTE_ILLEGAL) {
                result = RET_BAD_PARAM;
                break;
            }
            if (attribute >= AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN && attribute <= AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX) {
                result = RET_BAD_PARAM;
                break;
            }
            attrs.push_back(attribute);
        }
        if (result != RET_NO_ERROR) {
            break;
        }
        BluetoothRawAddress rawAddr(device.GetDeviceAddr());
        result = proxy->GetPlayerAppSettingCurrentValue(rawAddr, attrs);
    } while (false);

    return result;
}

int AvrcpController::SetPlayerAppSettingCurrentValue(
    const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes, const std::vector<uint8_t> &values)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    int result = RET_NO_ERROR;
    std::vector<int32_t> myAttributes;
    std::vector<int32_t> myValues;
    do {
        if (attributes.size() != values.size()) {
            break;
        }
        for (auto attribute : attributes) {
            if (attribute == AVRC_PLAYER_ATTRIBUTE_ILLEGAL) {
                result = RET_BAD_PARAM;
                break;
            }
            if (attribute >= AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN && attribute <= AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX) {
                result = RET_BAD_PARAM;
                break;
            }
        }
        BluetoothRawAddress rawAddr(device.GetDeviceAddr());
        for (auto attribute : attributes) {
            myAttributes.push_back(static_cast<int32_t>(attribute));
        }
        for (auto value : values) {
            myValues.push_back(static_cast<int32_t>(value));
        }
        if (result != RET_NO_ERROR) {
            break;
        }
        result = proxy->SetPlayerAppSettingCurrentValue(rawAddr, myAttributes, myValues);
    } while (false);

    return result;
}

int AvrcpController::GetPlayerApplicationSettingAttributeText(
    const BluetoothRemoteDevice &device, const std::vector<uint8_t> &attributes)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    int result = RET_BAD_STATUS;
    do {
        std::vector<int32_t> attrs;
        for (auto attribute : attributes) {
            if (attribute == AVRC_PLAYER_ATTRIBUTE_ILLEGAL) {
                break;
            }
            if (attribute >= AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN && attribute <= AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX) {
                break;
            }
            attrs.push_back(static_cast<int32_t>(attribute));
        }
        BluetoothRawAddress rawAddr(device.GetDeviceAddr());
        result = proxy->GetPlayerAppSettingAttributeText(rawAddr, attrs);
    } while (false);

    return result;
}

int AvrcpController::GetPlayerApplicationSettingValueText(
    const BluetoothRemoteDevice &device, uint8_t attributeId, const std::vector<uint8_t> &values)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    int result = RET_BAD_STATUS;
    do {
        std::vector<int32_t> myValues;
        if (attributeId == AVRC_PLAYER_ATTRIBUTE_ILLEGAL) {
            result = RET_BAD_PARAM;
            break;
        }
        if (attributeId >= AVRC_PLAYER_ATTRIBUTE_RESERVED_MIN && attributeId <= AVRC_PLAYER_ATTRIBUTE_RESERVED_MAX) {
            result = RET_BAD_PARAM;
            break;
        }
        for (auto value : values) {
            myValues.push_back(static_cast<int32_t>(value));
        }
        BluetoothRawAddress rawAddr(device.GetDeviceAddr());
        result = proxy->GetPlayerAppSettingValueText(rawAddr, static_cast<int32_t>(attributeId), myValues);
    } while (false);

    return result;
}

/******************************************************************
 * MEDIA INFORMATION                                              *
 ******************************************************************/

int AvrcpController::GetElementAttributes(const BluetoothRemoteDevice &device, const std::vector<uint32_t> &attributes)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    std::vector<int32_t> attrs;
    for (auto attribute : attributes) {
        attrs.push_back(static_cast<int32_t>(attribute));
    }
    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetElementAttributes(rawAddr, attrs);
}

/******************************************************************
 * PLAY                                                           *
 ******************************************************************/

int AvrcpController::GetPlayStatus(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetPlayStatus(rawAddr);
}

int AvrcpController::PlayItem(const BluetoothRemoteDevice &device, uint64_t uid, uint16_t uidCounter)
{
    HILOGI("enter, device: %{public}s, uidCounter: %{public}d", GET_ENCRYPT_ADDR(device), uidCounter);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }
    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->PlayItem(rawAddr,
        static_cast<int32_t>(AVRC_MEDIA_SCOPE_NOW_PLAYING),
        static_cast<int64_t>(uid),
        static_cast<int32_t>(uidCounter));
}

/******************************************************************
 * OPERATE THE VIRTUAL FILE SYSTEM                                *
 ******************************************************************/

int AvrcpController::GetFolderItems(
    const BluetoothRemoteDevice &device, uint32_t startItem, uint32_t endItem, const std::vector<uint32_t> &attributes)
{
    HILOGI("enter, device: %{public}s, startItem: %{public}d, endItem: %{public}d",
        GET_ENCRYPT_ADDR(device), startItem, endItem);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    int result = RET_NO_ERROR;
    do {
        if (startItem > endItem) {
            result = RET_BAD_PARAM;
            break;
        }
        for (auto attribute : attributes) {
            if (attribute <= AVRC_MEDIA_ATTRIBUTE_NOT_USED || attribute >= AVRC_MEDIA_ATTRIBUTE_RESERVED) {
                result = RET_BAD_PARAM;
                break;
            }
        }
        if (result != RET_NO_ERROR) {
            break;
        }
        BluetoothRawAddress rawAddr(device.GetDeviceAddr());
        std::vector<int32_t> attrs;
        for (auto attribute : attributes) {
            attrs.push_back(static_cast<int32_t>(attribute));
        }
        result = proxy->GetFolderItems(
            rawAddr, static_cast<int32_t>(startItem), static_cast<int32_t>(endItem), attrs);
    } while (false);

    return result;
}

int AvrcpController::GetMeidaPlayerList(const BluetoothRemoteDevice &device, uint32_t startItem, uint32_t endItem)
{
    HILOGI("enter, device: %{public}s, startItem: %{public}d, endItem: %{public}d",
        GET_ENCRYPT_ADDR(device), startItem, endItem);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetMeidaPlayerList(rawAddr, static_cast<int32_t>(startItem), static_cast<int32_t>(endItem));
}

int AvrcpController::GetTotalNumberOfItems(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    uint8_t scope = AVRC_MEDIA_SCOPE_NOW_PLAYING;
    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetTotalNumberOfItems(rawAddr, static_cast<int32_t>(scope));
}

/******************************************************************
 * ABSOLUTE VOLUME                                                *
 ******************************************************************/

int AvrcpController::SetAbsoluteVolume(const BluetoothRemoteDevice &device, uint8_t volume)
{
    HILOGI("enter, device: %{public}s, volume: %{public}d", GET_ENCRYPT_ADDR(device), volume);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->SetAbsoluteVolume(rawAddr, static_cast<int32_t>(volume));
}

/******************************************************************
 * NOTIFY                                                         *
 ******************************************************************/

int AvrcpController::EnableNotification(
    const BluetoothRemoteDevice &device, const std::vector<uint8_t> &events, uint32_t interval)
{
    HILOGI("enter, device: %{public}s, interval: %{public}d", GET_ENCRYPT_ADDR(device), interval);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    std::vector<int32_t> myEvents;
    for (auto event : events) {
        myEvents.push_back(static_cast<int32_t>(event));
    }

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->EnableNotification(rawAddr, myEvents, static_cast<int32_t>(interval));
}

int AvrcpController::DisableNotification(const BluetoothRemoteDevice &device, const std::vector<uint8_t> &events)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    std::vector<int32_t> myEvents;
    for (auto event : events) {
        myEvents.push_back(static_cast<int32_t>(event));
    }

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->DisableNotification(rawAddr, myEvents);
}

/******************************************************************
 * DO NOT EXPOSE THE INTERFACE                                    *
 ******************************************************************/

int AvrcpController::SetAddressedPlayer(const BluetoothRemoteDevice &device, uint16_t playerId)
{
    HILOGI("enter, device: %{public}s, playerId: %{public}d", GET_ENCRYPT_ADDR(device), playerId);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    return RET_BAD_STATUS;
}

int AvrcpController::SetBrowsedPlayer(const BluetoothRemoteDevice &device, uint16_t playerId)
{
    HILOGI("enter, device: %{public}s, playerId: %{public}d", GET_ENCRYPT_ADDR(device), playerId);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->SetBrowsedPlayer(rawAddr, (int32_t)playerId);
}

int AvrcpController::ChangePath(
    const BluetoothRemoteDevice &device, uint16_t uidCounter, uint16_t direction, uint64_t folderUid)
{
    HILOGI("enter, device: %{public}s, uidCounter: %{public}d, direction: %{public}d",
        GET_ENCRYPT_ADDR(device), uidCounter, direction);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    return RET_BAD_STATUS;
}

int AvrcpController::GetItemAttributes(
    const BluetoothRemoteDevice &device, uint64_t uid, uint16_t uidCounter, const std::vector<uint32_t> &attributes)
{
    HILOGI("enter, device: %{public}s, uidCounter: %{public}d", GET_ENCRYPT_ADDR(device), uidCounter);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    std::vector<int32_t> attrs;
    for (auto attribute : attributes) {
        attrs.push_back(static_cast<int32_t>(attribute));
    }

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    return proxy->GetItemAttributes(rawAddr, (int64_t)uid, (int32_t)uidCounter, attrs);
}

int AvrcpController::RequestContinuingResponse(const BluetoothRemoteDevice &device, uint8_t pduId)
{
    HILOGI("enter, device: %{public}s, pduId: %{public}d", GET_ENCRYPT_ADDR(device), pduId);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    return RET_BAD_STATUS;
}

int AvrcpController::AbortContinuingResponse(const BluetoothRemoteDevice &device, uint8_t pduId)
{
    HILOGI("enter, device: %{public}s, pduId: %{public}d", GET_ENCRYPT_ADDR(device), pduId);

    int result = RET_BAD_STATUS;

    return result;
}

int AvrcpController::AddToNowPlaying(const BluetoothRemoteDevice &device, uint64_t uid, uint16_t uidCounter)
{
    HILOGI("enter, device: %{public}s, uidCounter: %{public}d", GET_ENCRYPT_ADDR(device), uidCounter);

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    return RET_BAD_STATUS;
}

AvrcpController::AvrcpController(void)
{
    HILOGI("enter");

    pimpl = std::make_unique<AvrcpController::impl>();
}

AvrcpController::~AvrcpController(void)
{
    HILOGI("enter");
    sptr<IBluetoothAvrcpCt> proxy = GetRemoteProxy<IBluetoothAvrcpCt>(PROFILE_AVRCP_CT);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    pimpl = nullptr;
}

}  // namespace Bluetooth
}  // namespace OHOS
