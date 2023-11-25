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

#include <list>
#include <memory>
#include <mutex>

#include "bluetooth_def.h"
#include "bluetooth_avrcp_tg_proxy.h"
#include "bluetooth_avrcp_tg_observer_stub.h"
#include "bluetooth_host.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_load_system_ability.h"
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

        void OnConnectionStateChanged(const BluetoothRawAddress &addr, int32_t state) override
        {
            HILOGD("enter, address: %{public}s, state: %{public}d", GetEncryptAddr(addr.GetAddress()).c_str(), state);
            BluetoothRemoteDevice device(addr.GetAddress(), BTTransport::ADAPTER_BREDR);
            impl_->OnConnectionStateChanged(device, static_cast<int>(state));

            return;
        }

    private:
        AvrcpTarget::impl *impl_;
    };

    class AvrcpTgDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        explicit AvrcpTgDeathRecipient(AvrcpTarget::impl &avrcpTgServer) : avrcpTgServer_(avrcpTgServer) {};
        ~AvrcpTgDeathRecipient() final = default;
        BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AvrcpTgDeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote) final
        {
            HILOGI("starts");
            std::lock_guard<std::mutex> lock(g_avrcpTgMutex);
            if (!avrcpTgServer_.proxy_) {
                return;
            }
            avrcpTgServer_.proxy_ = nullptr;
        }

    private:
        AvrcpTarget::impl &avrcpTgServer_;
    };

    impl()
    {
        if (proxy_) {
            return;
        }
        BluetootLoadSystemAbility::GetInstance()->RegisterNotifyMsg(PROFILE_ID_AVRCP_TG);
        if (!BluetootLoadSystemAbility::GetInstance()->HasSubscribedBluetoothSystemAbility()) {
            BluetootLoadSystemAbility::GetInstance()->SubScribeBluetoothSystemAbility();
            return;
        }
        InitAvrcpTgProxy();
    }

    ~impl()
    {
        HILOGI("enter");
        if (proxy_ != nullptr) {
            proxy_->UnregisterObserver(observer_);
            proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
        }
    }

    bool InitAvrcpTgProxy(void)
    {
        std::lock_guard<std::mutex> lock(g_avrcpTgMutex);
        if (proxy_) {
            return true;
        }
        proxy_ = GetRemoteProxy<IBluetoothAvrcpTg>(PROFILE_AVRCP_TG);
        if (!proxy_) {
            HILOGE("get AvrcpTarget proxy failed");
            return false;
        }

        observer_ = new (std::nothrow) ObserverImpl(this);
        if (observer_ != nullptr) {
            proxy_->RegisterObserver(observer_);
        }

        deathRecipient_ = new AvrcpTgDeathRecipient(*this);
        if (deathRecipient_ != nullptr) {
            proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
        }
        return true;
    }

    bool IsEnabled(void)
    {
        HILOGI("enter");
        bool isDiscovering = false;
        BluetoothHost::GetDefaultHost().IsBtDiscovering(isDiscovering);
        return (proxy_ != nullptr && !isDiscovering);
    }

    void OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state)
    {
        HILOGI("enter, device: %{public}s, state: %{public}d", GET_ENCRYPT_ADDR(device), state);
        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.ForEach([device, state](std::shared_ptr<IObserver> observer) {
            observer->OnConnectionStateChanged(device, state);
        });
    }

    std::mutex observerMutex_;
    sptr<IBluetoothAvrcpTg> proxy_;
    BluetoothObserverList<AvrcpTarget::IObserver> observers_;
    sptr<ObserverImpl> observer_;
    sptr<AvrcpTgDeathRecipient> deathRecipient_;
};

AvrcpTarget *AvrcpTarget::GetProfile(void)
{
    HILOGI("enter");

    static AvrcpTarget instance;

    return &instance;
}

void AvrcpTarget::Init()
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    if (!pimpl->InitAvrcpTgProxy()) {
        HILOGE("AvrcpTarget proxy is nullptr");
        return;
    }
}

int32_t AvrcpTarget::SetDeviceAbsoluteVolume(const BluetoothRemoteDevice &device, int32_t volumeLevel)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }
    return pimpl->proxy_->SetDeviceAbsoluteVolume(BluetoothRawAddress(device.GetDeviceAddr()), volumeLevel);
}

int32_t AvrcpTarget::SetDeviceAbsVolumeAbility(const BluetoothRemoteDevice &device, int32_t ability)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }
    return pimpl->proxy_->SetDeviceAbsVolumeAbility(BluetoothRawAddress(device.GetDeviceAddr()), ability);
}

int32_t AvrcpTarget::GetDeviceAbsVolumeAbility(const BluetoothRemoteDevice &device, int32_t &ability)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }
    return pimpl->proxy_->GetDeviceAbsVolumeAbility(BluetoothRawAddress(device.GetDeviceAddr()), ability);
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

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    pimpl->proxy_->SetActiveDevice(rawAddr);
}

std::vector<BluetoothRemoteDevice> AvrcpTarget::GetConnectedDevices(void)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return std::vector<BluetoothRemoteDevice>();
    }

    std::vector<BluetoothRemoteDevice> devices;
    std::vector<BluetoothRawAddress> rawAddrs = pimpl->proxy_->GetConnectedDevices();
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

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return std::vector<BluetoothRemoteDevice>();
    }

    std::vector<int32_t> convertStates;
    for (auto state : states) {
        convertStates.push_back(static_cast<int32_t>(state));
    }

    std::vector<BluetoothRemoteDevice> devices;
    std::vector<BluetoothRawAddress> rawAddrs = pimpl->proxy_->GetDevicesByStates(convertStates);
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

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return static_cast<int32_t>(BTConnectState::DISCONNECTED);
    }

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    int32_t result = pimpl->proxy_->GetDeviceState(rawAddr);
    return static_cast<int>(result);
}

bool AvrcpTarget::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return RET_BAD_STATUS;
    }

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    int result = pimpl->proxy_->Connect(rawAddr);
    return result == RET_NO_ERROR;
}

bool AvrcpTarget::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return RET_BAD_STATUS;
    }

    BluetoothRawAddress rawAddr(device.GetDeviceAddr());
    int result = pimpl->proxy_->Disconnect(rawAddr);
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

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyPlaybackStatusChanged(static_cast<int32_t>(playStatus), static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyTrackChanged(uint64_t uid, uint32_t playbackPos)
{
    HILOGI("enter, playbackPos: %{public}d", playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyTrackChanged(static_cast<int64_t>(uid), static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyTrackReachedEnd(uint32_t playbackPos)
{
    HILOGI("enter, playbackPos: %{public}d", playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyTrackReachedEnd(static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyTrackReachedStart(uint32_t playbackPos)
{
    HILOGI("enter, playbackPos: %{public}d", playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyTrackReachedStart(static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyPlaybackPosChanged(uint32_t playbackPos)
{
    HILOGI("enter, playbackPos: %{public}d", playbackPos);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyPlaybackPosChanged(static_cast<int32_t>(playbackPos));
}

void AvrcpTarget::NotifyPlayerAppSettingChanged(const std::vector<uint8_t> &attributes,
    const std::vector<uint8_t> &values)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    std::vector<int32_t> attrs;
    for (auto attribute : attributes) {
        attrs.push_back(attribute);
    }
    std::vector<int32_t> vals;
    for (auto value : values) {
        vals.push_back(value);
    }

    pimpl->proxy_->NotifyPlayerAppSettingChanged(attrs, vals);
}

void AvrcpTarget::NotifyNowPlayingContentChanged(void)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyNowPlayingContentChanged();
}

void AvrcpTarget::NotifyAvailablePlayersChanged(void)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyAvailablePlayersChanged();
}

void AvrcpTarget::NotifyAddressedPlayerChanged(uint16_t playerId, uint16_t uidCounter)
{
    HILOGI("enter, playerId: %{public}d, uidCounter: %{public}d", playerId, uidCounter);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyAddressedPlayerChanged(static_cast<int32_t>(playerId), static_cast<int32_t>(uidCounter));
}

void AvrcpTarget::NotifyUidChanged(uint16_t uidCounter)
{
    HILOGI("enter, uidCounter: %{public}d", uidCounter);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyUidChanged(static_cast<int32_t>(uidCounter));
}

void AvrcpTarget::NotifyVolumeChanged(uint8_t volume)
{
    HILOGI("enter, volume: %{public}d", volume);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or avrcpTarget proxy is nullptr");
        return;
    }

    pimpl->proxy_->NotifyVolumeChanged(static_cast<int32_t>(volume));
}

AvrcpTarget::AvrcpTarget(void)
{
    HILOGI("enter");

    pimpl = std::make_unique<impl>();
}

AvrcpTarget::~AvrcpTarget(void)
{
    HILOGI("enter");
    if (!pimpl || !pimpl->proxy_) {
        return;
    }
    pimpl->proxy_->AsObject()->RemoveDeathRecipient(pimpl->deathRecipient_);
    pimpl = nullptr;
}
} // namespace Bluetooth
} // namespace OHOS
