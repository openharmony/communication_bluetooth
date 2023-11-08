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

#include "bluetooth_a2dp_src.h"
#include <unistd.h>
#include "bluetooth_a2dp_codec.h"
#include "bluetooth_a2dp_src_proxy.h"
#include "bluetooth_a2dp_src_observer_stub.h"
#include "bluetooth_device.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_load_system_ability.h"
#include "bluetooth_observer_list.h"
#include "raw_address.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;
std::mutex g_a2dpProxyMutex;
struct A2dpSource::impl {
    impl();
    ~impl();
    bool InitA2dpSrcProxy(void);

    BluetoothObserverList<A2dpSourceObserver> observers_;
    sptr<IBluetoothA2dpSrc> proxy_ = nullptr;
    class BluetoothA2dpSourceObserverImp;
    sptr<BluetoothA2dpSourceObserverImp> observerImp_ = nullptr;
    class BluetoothA2dpSourceDeathRecipient;
    sptr<BluetoothA2dpSourceDeathRecipient> deathRecipient_ = nullptr;
};

class A2dpSource::impl::BluetoothA2dpSourceObserverImp : public BluetoothA2dpSrcObserverStub {
public:
    explicit BluetoothA2dpSourceObserverImp(A2dpSource::impl &a2dpSource) : a2dpSource_(a2dpSource) {};
    ~BluetoothA2dpSourceObserverImp() override{};

    void Register(std::shared_ptr<A2dpSourceObserver> &observer)
    {
        HILOGI("enter");
        a2dpSource_.observers_.Register(observer);
    }

    void Deregister(std::shared_ptr<A2dpSourceObserver> &observer)
    {
        HILOGI("enter");
        a2dpSource_.observers_.Deregister(observer);
    }

    void OnConnectionStateChanged(const RawAddress &device, int state) override
    {
        HILOGD("a2dpSrc conn state, device: %{public}s, state: %{public}s",
            GetEncryptAddr((device).GetAddress()).c_str(), GetProfileConnStateName(state).c_str());
        a2dpSource_.observers_.ForEach([device, state](std::shared_ptr<A2dpSourceObserver> observer) {
            observer->OnConnectionStateChanged(BluetoothRemoteDevice(device.GetAddress(), 0), state);
        });
    }

    void OnPlayingStatusChanged(const RawAddress &device, int playingState, int error) override
    {
        HILOGI("device: %{public}s, playingState: %{public}d, error: %{public}d",
            GetEncryptAddr(device.GetAddress()).c_str(), playingState, error);
        a2dpSource_.observers_.ForEach([device, playingState, error](std::shared_ptr<A2dpSourceObserver> observer) {
            observer->OnPlayingStatusChanged(BluetoothRemoteDevice(device.GetAddress(), 0), playingState, error);
        });
    }

    void OnConfigurationChanged(const RawAddress &device, const BluetoothA2dpCodecInfo &info, int error) override
    {
        HILOGI("device: %{public}s, error: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), error);
        a2dpSource_.observers_.ForEach([device, info, error](std::shared_ptr<A2dpSourceObserver> observer) {
            A2dpCodecInfo codecInfo;

            codecInfo.bitsPerSample = info.bitsPerSample;
            codecInfo.channelMode = info.channelMode;
            codecInfo.codecPriority = info.codecPriority;
            codecInfo.codecType = info.codecType;
            codecInfo.sampleRate = info.sampleRate;

            observer->OnConfigurationChanged(BluetoothRemoteDevice(device.GetAddress(), 0), codecInfo, error);
        });
    };

private:
    A2dpSource::impl &a2dpSource_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSourceObserverImp);
};

class A2dpSource::impl::BluetoothA2dpSourceDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit BluetoothA2dpSourceDeathRecipient(A2dpSource::impl &a2dpSrcDeath) : a2dpSrcDeath_(a2dpSrcDeath)
    {};
    ~BluetoothA2dpSourceDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSourceDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("enter");
        std::lock_guard<std::mutex> lock(g_a2dpProxyMutex);
        if (!a2dpSrcDeath_.proxy_) {
            return;
        }
        a2dpSrcDeath_.proxy_ = nullptr;
    }

private:
    A2dpSource::impl &a2dpSrcDeath_;
};

A2dpSource::impl::impl()
{
    if (proxy_) {
        return;
    }
    BluetootLoadSystemAbility::GetInstance()->RegisterNotifyMsg(PROFILE_ID_A2DP_SRC);
    if (!BluetootLoadSystemAbility::GetInstance()->HasSubscribedBluetoothSystemAbility()) {
        BluetootLoadSystemAbility::GetInstance()->SubScribeBluetoothSystemAbility();
        return;
    }
    InitA2dpSrcProxy();
};

A2dpSource::impl::~impl()
{
    HILOGD("start");
    if (proxy_ != nullptr) {
        proxy_->DeregisterObserver(observerImp_);
        proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
}

bool A2dpSource::impl::InitA2dpSrcProxy(void)
{
    std::lock_guard<std::mutex> lock(g_a2dpProxyMutex);
    if (proxy_) {
        return true;
    }
    HILOGI("enter");
    proxy_ = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    if (!proxy_) {
        HILOGE("get A2dpSource proxy_ failed");
        return false;
    }

    deathRecipient_ = new BluetoothA2dpSourceDeathRecipient(*this);
    if (deathRecipient_ != nullptr) {
        proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
    }

    observerImp_ = new (std::nothrow) BluetoothA2dpSourceObserverImp(*this);
    if (observerImp_ != nullptr) {
        proxy_->RegisterObserver(observerImp_);
    }
    return true;
}

A2dpSource::A2dpSource()
{
    pimpl = std::make_unique<impl>();
    if (!pimpl) {
        HILOGE("fails: no pimpl");
    }
}

A2dpSource::~A2dpSource()
{
    HILOGD("start");
}

void A2dpSource::Init()
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    if (!pimpl->InitA2dpSrcProxy()) {
        HILOGE("A2dpSrc proxy is nullptr");
        return;
    }
}

void A2dpSource::RegisterObserver(A2dpSourceObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<A2dpSourceObserver> pointer(observer, [](A2dpSourceObserver *) {});
    pimpl->observers_.Register(pointer);
}

void A2dpSource::DeregisterObserver(A2dpSourceObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<A2dpSourceObserver> pointer(observer, [](A2dpSourceObserver *) {});
    pimpl->observers_.Deregister(pointer);
}

int A2dpSource::GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRemoteDevice> &devices) const
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    std::vector<int32_t> convertStates;
    for (auto state : states) {
        convertStates.push_back(static_cast<int32_t>(state));
    }

    std::vector<RawAddress> rawAddrs;
    int ret = pimpl->proxy_->GetDevicesByStates(convertStates, rawAddrs);
    if (ret != BT_NO_ERROR) {
        HILOGE("GetDevicesByStates return error.");
        return ret;
    }
    for (auto rawAddr : rawAddrs) {
        BluetoothRemoteDevice device(rawAddr.GetAddress(), BTTransport::ADAPTER_BREDR);
        devices.push_back(device);
    }
    return BT_NO_ERROR;
}

int A2dpSource::GetDeviceState(const BluetoothRemoteDevice &device, int &state) const
{
    HILOGD("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    int ret = pimpl->proxy_->GetDeviceState(RawAddress(device.GetDeviceAddr()), state);
    HILOGI("state: %{public}d", ret);
    return ret;
}

int32_t A2dpSource::GetPlayingState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    int ret = RET_NO_ERROR;
    pimpl->proxy_->GetPlayingState(RawAddress(device.GetDeviceAddr()), ret);
    HILOGI("state: %{public}d", ret);
    return ret;
}

int32_t A2dpSource::GetPlayingState(const BluetoothRemoteDevice &device, int &state) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    return pimpl->proxy_->GetPlayingState(RawAddress(device.GetDeviceAddr()), state);
}

int32_t A2dpSource::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("a2dp connect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    int cod = 0;
    int32_t err = device.GetDeviceClass(cod);
    if (err != BT_NO_ERROR) {
        HILOGE("GetDeviceClass Failed.");
        return BT_ERR_INTERNAL_ERROR;
    }
    BluetoothDeviceClass devClass = BluetoothDeviceClass(cod);
    if (!devClass.IsProfileSupported(BluetoothDevice::PROFILE_A2DP)) {
        HILOGE("a2dp connect failed. The remote device does not support A2DP service.");
        return BT_ERR_INTERNAL_ERROR;
    }

    return pimpl->proxy_->Connect(RawAddress(device.GetDeviceAddr()));
}

int32_t A2dpSource::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("a2dp disconnect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    return pimpl->proxy_->Disconnect(RawAddress(device.GetDeviceAddr()));
}

A2dpSource *A2dpSource::GetProfile()
{
    HILOGD("enter");
    static A2dpSource service;
    return &service;
}

int A2dpSource::SetActiveSinkDevice(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return pimpl->proxy_->SetActiveSinkDevice(RawAddress(device.GetDeviceAddr()));
}

const BluetoothRemoteDevice &A2dpSource::GetActiveSinkDevice() const
{
    HILOGI("enter");
    static BluetoothRemoteDevice deviceInfo;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return deviceInfo;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return deviceInfo;
    }

    BluetoothRawAddress rawAddress = pimpl->proxy_->GetActiveSinkDevice();
    deviceInfo = BluetoothRemoteDevice(rawAddress.GetAddress(), 0);
    return deviceInfo;
}

int A2dpSource::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    if ((!device.IsValidBluetoothRemoteDevice()) || (
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) &&
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)))) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    return pimpl->proxy_->SetConnectStrategy(RawAddress(device.GetDeviceAddr()), strategy);
}

int A2dpSource::GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_INVALID_STATE;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    return pimpl->proxy_->GetConnectStrategy(RawAddress(device.GetDeviceAddr()), strategy);
}

A2dpCodecStatus A2dpSource::GetCodecStatus(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    A2dpCodecStatus ret;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return ret;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return ret;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return ret;
    }

    BluetoothA2dpCodecStatus codecStatus = pimpl->proxy_->GetCodecStatus(RawAddress(device.GetDeviceAddr()));
    ret.codecInfo.codecType = codecStatus.codecInfo.codecType;
    ret.codecInfo.sampleRate = codecStatus.codecInfo.sampleRate;
    ret.codecInfo.channelMode = codecStatus.codecInfo.channelMode;
    ret.codecInfo.codecPriority = codecStatus.codecInfo.codecPriority;
    ret.codecInfo.bitsPerSample = codecStatus.codecInfo.bitsPerSample;

    A2dpCodecInfo serviceInfo;
    for (auto it = codecStatus.codecInfoConfirmCap.begin(); it != codecStatus.codecInfoConfirmCap.end(); it++) {
        serviceInfo.codecType = it->codecType;
        serviceInfo.sampleRate = it->sampleRate;
        serviceInfo.channelMode = it->channelMode;
        serviceInfo.codecPriority = it->codecPriority;
        serviceInfo.bitsPerSample = it->bitsPerSample;
        ret.codecInfoConfirmedCap.push_back(serviceInfo);
    }

    for (auto it = codecStatus.codecInfoLocalCap.begin(); it != codecStatus.codecInfoLocalCap.end(); it++) {
        serviceInfo.codecType = it->codecType;
        serviceInfo.sampleRate = it->sampleRate;
        serviceInfo.channelMode = it->channelMode;
        serviceInfo.codecPriority = it->codecPriority;
        serviceInfo.bitsPerSample = it->bitsPerSample;
        ret.codecInfoLocalCap.push_back(serviceInfo);
    }
    return ret;
}

int A2dpSource::GetCodecPreference(const BluetoothRemoteDevice &device, A2dpCodecInfo &info)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    BluetoothA2dpCodecInfo serviceInfo;
    int ret = pimpl->proxy_->GetCodecPreference(RawAddress(device.GetDeviceAddr()), serviceInfo);
    if (ret != BT_NO_ERROR) {
        HILOGE("GetCodecPreference error.");
        return ret;
    }
    info.codecType = serviceInfo.codecType;
    info.sampleRate = serviceInfo.sampleRate;
    info.channelMode = serviceInfo.channelMode;
    info.bitsPerSample = serviceInfo.bitsPerSample;
    return ret;
}

int A2dpSource::SetCodecPreference(const BluetoothRemoteDevice &device, const A2dpCodecInfo &info)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    BluetoothA2dpCodecInfo serviceInfo;
    serviceInfo.codecType = info.codecType;
    serviceInfo.sampleRate = info.sampleRate;
    serviceInfo.channelMode = info.channelMode;
    serviceInfo.bitsPerSample = info.bitsPerSample;
    serviceInfo.codecPriority = info.codecPriority;
    serviceInfo.codecSpecific1 = info.codecSpecific1;
    serviceInfo.codecSpecific2 = info.codecSpecific2;
    serviceInfo.codecSpecific3 = info.codecSpecific3;
    serviceInfo.codecSpecific4 = info.codecSpecific4;

    return pimpl->proxy_->SetCodecPreference(RawAddress(device.GetDeviceAddr()), serviceInfo);
}

void A2dpSource::SwitchOptionalCodecs(const BluetoothRemoteDevice &device, bool isEnable)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return;
    }

    pimpl->proxy_->SwitchOptionalCodecs(RawAddress(device.GetDeviceAddr()), isEnable);
}

int A2dpSource::GetOptionalCodecsSupportState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return pimpl->proxy_->GetOptionalCodecsSupportState(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::StartPlaying(const BluetoothRemoteDevice &device)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return pimpl->proxy_->StartPlaying(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::SuspendPlaying(const BluetoothRemoteDevice &device)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return pimpl->proxy_->SuspendPlaying(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::StopPlaying(const BluetoothRemoteDevice &device)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return RET_BAD_STATUS;
    }

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return pimpl->proxy_->StopPlaying(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::WriteFrame(const uint8_t *data, uint32_t size)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return RET_BAD_STATUS;
    }

    return pimpl->proxy_->WriteFrame(data, size);
}

void A2dpSource::GetRenderPosition(uint16_t &delayValue, uint16_t &sendDataSize, uint32_t &timeStamp)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->proxy_) {
        HILOGE("pimpl or a2dpSrc proxy is nullptr");
        return;
    }

    pimpl->proxy_->GetRenderPosition(delayValue, sendDataSize, timeStamp);
}
} // namespace Bluetooth
} // namespace OHOS