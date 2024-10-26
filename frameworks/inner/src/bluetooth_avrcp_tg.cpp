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
#define LOG_TAG "bt_fwk_avrcp_tg"
#endif

#include <list>
#include <memory>
#include <mutex>

#include "bluetooth_def.h"
#include "bluetooth_avrcp_tg_proxy.h"
#include "bluetooth_avrcp_tg_observer_stub.h"
#include "bluetooth_host.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "iservice_registry.h"
#include "raw_address.h"
#include "system_ability_definition.h"
#include "bluetooth_avrcp_tg.h"

namespace OHOS {
namespace Bluetooth {
std::mutex g_avrcpTgMutex;
struct AvrcpTarget::impl {
public:
    class ObserverImpl : public BluetoothAvrcpTgObserverStub {
    public:
        explicit ObserverImpl(AvrcpTarget::impl *impl) : impl_(impl)
        {}
        ~ObserverImpl() override = default;

        void OnConnectionStateChanged(const BluetoothRawAddress &addr, int32_t state, int32_t cause) override
        {
            HILOGD("enter, address: %{public}s, state: %{public}d, cause: %{public}d",
                GET_ENCRYPT_RAW_ADDR(addr), state, cause);
            BluetoothRemoteDevice device(addr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnConnectionStateChanged(device, static_cast<int>(state), cause);

            return;
        }

    private:
        AvrcpTarget::impl *impl_;
    };

    impl()
    {
        observer_ = new (std::nothrow) ObserverImpl(this);
        CHECK_AND_RETURN_LOG(observer_ != nullptr, "observer_ is nullptr");
        profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_AVRCP_TG,
            [this](sptr<IRemoteObject> remote) {
            sptr<IBluetoothAvrcpTg> proxy = iface_cast<IBluetoothAvrcpTg>(remote);
            CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
            proxy->RegisterObserver(observer_);
        });
    }

    ~impl()
    {
        HILOGI("enter");
        BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
        sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
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

    std::mutex observerMutex_;
    BluetoothObserverList<AvrcpTarget::IObserver> observers_;
    sptr<ObserverImpl> observer_;
    int32_t profileRegisterId = 0;
};

AvrcpTarget *AvrcpTarget::GetProfile(void)
{
    HILOGI("enter");
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<AvrcpTarget> instance;
    return instance.get();
#else
    static AvrcpTarget instance;
    return &instance;
#endif
}

int32_t AvrcpTarget::SetDeviceAbsoluteVolume(const BluetoothRemoteDevice &device, int32_t volumeLevel)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");
    return proxy->SetDeviceAbsoluteVolume(BluetoothRawAddress(device.GetDeviceAddr()), volumeLevel);
}

int32_t AvrcpTarget::SetDeviceAbsVolumeAbility(const BluetoothRemoteDevice &device, int32_t ability)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");
    return proxy->SetDeviceAbsVolumeAbility(BluetoothRawAddress(device.GetDeviceAddr()), ability);
}

int32_t AvrcpTarget::GetDeviceAbsVolumeAbility(const BluetoothRemoteDevice &device, int32_t &ability)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is nullptr");
    return proxy->GetDeviceAbsVolumeAbility(BluetoothRawAddress(device.GetDeviceAddr()), ability);
}

/******************************************************************
 * REGISTER / UNREGISTER OBSERVER                                 *
 ******************************************************************/

void AvrcpTarget::RegisterObserver(std::shared_ptr<AvrcpTarget::IObserver> observer)
{
    HILOGD("enter");
    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->observers_.Register(observer);
}

void AvrcpTarget::UnregisterObserver(std::shared_ptr<AvrcpTarget::IObserver> observer)
{
    HILOGD("enter");
    std::lock_guard<std::mutex> lock(pimpl->observerMutex_);
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->observers_.Deregister(observer);
}

/******************************************************************
 * CONNECTION                                                     *
 ******************************************************************/

void AvrcpTarget::SetActiveDevice(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    proxy->SetActiveDevice(rawAddr);
}

std::vector<BluetoothRemoteDevice> AvrcpTarget::GetConnectedDevices(void)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::vector<BluetoothRemoteDevice>(), "proxy is nullptr");

    std::vector<BluetoothRemoteDevice> devices;
    std::vector<BluetoothRawAddress> rawAddrs = proxy->GetConnectedDevices();
    for (auto rawAddr : rawAddrs) {
        BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
        devices.push_back(device);
    }
    return devices;
}

std::vector<BluetoothRemoteDevice> AvrcpTarget::GetDevicesByStates(std::vector<int> states)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }
    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::vector<BluetoothRemoteDevice>(), "proxy is nullptr");

    std::vector<int32_t> convertStates;
    for (auto state : states) {
        convertStates.push_back(static_cast<int32_t>(state));
    }

    std::vector<BluetoothRemoteDevice> devices;
    std::vector<BluetoothRawAddress> rawAddrs = proxy->GetDevicesByStates(convertStates);
    for (auto rawAddr : rawAddrs) {
        BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
        devices.push_back(device);
    }

    return devices;
}

int AvrcpTarget::GetDeviceState(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return static_cast<int32_t>(BTConnectState::DISCONNECTED);
    }
    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        static_cast<int32_t>(BTConnectState::DISCONNECTED), "proxy is nullptr");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    int32_t result = proxy->GetDeviceState(rawAddr);
    return static_cast<int>(result);
}

bool AvrcpTarget::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }
    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "proxy is nullptr");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    int result = proxy->Connect(rawAddr);
    return result == RET_NO_ERROR;
}

bool AvrcpTarget::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "proxy is nullptr");

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    int result = proxy->Disconnect(rawAddr);
    return result == RET_NO_ERROR;
}

/******************************************************************
 * NOTIFICATION                                                   *
 ******************************************************************/

void AvrcpTarget::NotifyPlaybackStatusChanged(uint8_t playStatus, uint32_t playbackPos)
{
    HILOGI("enter, playStatus: %{public}d, playbackPos: %{public}d", playStatus, playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyPlaybackStatusChanged(static_cast<int32_t>(playStatus), static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyTrackChanged(uint64_t uid, uint32_t playbackPos)
{
    HILOGI("enter, playbackPos: %{public}d", playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyTrackChanged(static_cast<int64_t>(uid), static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyTrackReachedEnd(uint32_t playbackPos)
{
    HILOGI("enter, playbackPos: %{public}d", playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyTrackReachedEnd(static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyTrackReachedStart(uint32_t playbackPos)
{
    HILOGI("enter, playbackPos: %{public}d", playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyTrackReachedStart(static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyPlaybackPosChanged(uint32_t playbackPos)
{
    HILOGI("enter, playbackPos: %{public}d", playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyPlaybackPosChanged(static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyPlayerAppSettingChanged(const std::vector<uint8_t> &attributes,
    const std::vector<uint8_t> &values)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    std::vector<int32_t> attrs;
    for (auto attribute : attributes) {
        attrs.push_back(attribute);
    }
    std::vector<int32_t> vals;
    for (auto value : values) {
        vals.push_back(value);
    }

    proxy->NotifyPlayerAppSettingChanged(attrs, vals);
}

void AvrcpTarget::NotifyNowPlayingContentChanged(void)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyNowPlayingContentChanged();
}

void AvrcpTarget::NotifyAvailablePlayersChanged(void)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyAvailablePlayersChanged();
}

void AvrcpTarget::NotifyAddressedPlayerChanged(uint16_t playerId, uint16_t uidCounter)
{
    HILOGI("enter, playerId: %{public}d, uidCounter: %{public}d", playerId, uidCounter);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyAddressedPlayerChanged(static_cast<int32_t>(playerId), static_cast<int32_t>(uidCounter));
}

void AvrcpTarget::NotifyUidChanged(uint16_t uidCounter)
{
    HILOGI("enter, uidCounter: %{public}d", uidCounter);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyUidChanged(static_cast<int32_t>(uidCounter));
}

void AvrcpTarget::NotifyVolumeChanged(uint8_t volume)
{
    HILOGI("enter, volume: %{public}d", volume);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");

    proxy->NotifyVolumeChanged(static_cast<int32_t>(volume));
}

AvrcpTarget::AvrcpTarget(void)
{
    HILOGI("enter");

    pimpl = std::make_unique<impl>();
}

AvrcpTarget::~AvrcpTarget(void)
{
    HILOGI("enter");
    sptr<IBluetoothAvrcpTg> proxy = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is nullptr");
    pimpl = nullptr;
}
} // namespace Bluetooth
} // namespace OHOS
