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
#define LOG_TAG "bt_fwk_a2dp_src"
#endif

#include "bluetooth_a2dp_src.h"
#include <unistd.h>
#include "bluetooth_a2dp_codec.h"
#include "bluetooth_a2dp_src_proxy.h"
#include "bluetooth_a2dp_src_observer_stub.h"
#include "bluetooth_device.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_profile_manager.h"
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
    BluetoothObserverList<A2dpSourceObserver> observers_;
    class BluetoothA2dpSourceObserverImp;
    sptr<BluetoothA2dpSourceObserverImp> observerImp_ = nullptr;
    int32_t profileRegisterId = 0;
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

    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {
        HILOGD("a2dpSrc conn state, device: %{public}s, state: %{public}s, cause: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), GetProfileConnStateName(state).c_str(), cause);
        a2dpSource_.observers_.ForEach([device, state, cause](std::shared_ptr<A2dpSourceObserver> observer) {
            observer->OnConnectionStateChanged(BluetoothRemoteDevice(device.GetAddress(), 0), state, cause);
        });
    }

    void OnCaptureConnectionStateChanged(const RawAddress &device, int state,
        const BluetoothA2dpCodecInfo &info) override
    {
        HILOGD("hdap conn state, device: %{public}s, state: %{public}d", GET_ENCRYPT_RAW_ADDR(device), state);
        a2dpSource_.observers_.ForEach([device, state, info](std::shared_ptr<A2dpSourceObserver> observer) {
            A2dpCodecInfo codecInfo{};
            codecInfo.bitsPerSample = info.bitsPerSample;
            codecInfo.channelMode = info.channelMode;
            codecInfo.codecType = info.codecType;
            codecInfo.sampleRate = info.sampleRate;
            observer->OnCaptureConnectionStateChanged(BluetoothRemoteDevice(device.GetAddress(), 0), state, codecInfo);
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
        HILOGD("device: %{public}s, error: %{public}d", GetEncryptAddr(device.GetAddress()).c_str(), error);
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

    void OnMediaStackChanged(const RawAddress &device, int action) override
    {
        HILOGI("device: %{public}s, action: %{public}s",
            GET_ENCRYPT_RAW_ADDR(device), GetUpdateOutputStackActionName(action).c_str());
        a2dpSource_.observers_.ForEach([device, action](std::shared_ptr<A2dpSourceObserver> observer) {
            observer->OnMediaStackChanged(BluetoothRemoteDevice(device.GetAddress(), 0), action);
        });
    }

    void OnVirtualDeviceChanged(int action, std::string address) override
    {
        HILOGI("device: %{public}s, action: %{public}d", GetEncryptAddr(address).c_str(), action);
        a2dpSource_.observers_.ForEach([action, address](std::shared_ptr<A2dpSourceObserver> observer) {
            observer->OnVirtualDeviceChanged(action, address);
        });
    }
private:
    A2dpSource::impl &a2dpSource_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothA2dpSourceObserverImp);
};

A2dpSource::impl::impl()
{
    observerImp_ = new (std::nothrow) BluetoothA2dpSourceObserverImp(*this);
    CHECK_AND_RETURN_LOG(observerImp_ != nullptr, "observerImp_ is nullptr");
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_A2DP_SRC,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothA2dpSrc> proxy = iface_cast<IBluetoothA2dpSrc>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(observerImp_);
    });
};

A2dpSource::impl::~impl()
{
    HILOGD("start");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(observerImp_);
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

void A2dpSource::RegisterObserver(std::shared_ptr<A2dpSourceObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->observers_.Register(observer);
}

void A2dpSource::DeregisterObserver(std::shared_ptr<A2dpSourceObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->observers_.Deregister(observer);
}

int A2dpSource::GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRemoteDevice> &devices) const
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    std::vector<int32_t> convertStates;
    for (auto state : states) {
        convertStates.push_back(static_cast<int32_t>(state));
    }

    std::vector<RawAddress> rawAddrs;
    int ret = proxy->GetDevicesByStates(convertStates, rawAddrs);
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
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    int ret = proxy->GetDeviceState(RawAddress(device.GetDeviceAddr()), state);
    HILOGD("state: %{public}d", ret);
    return ret;
}

int32_t A2dpSource::GetPlayingState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    int ret = RET_NO_ERROR;
    proxy->GetPlayingState(RawAddress(device.GetDeviceAddr()), ret);
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
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    return proxy->GetPlayingState(RawAddress(device.GetDeviceAddr()), state);
}

int32_t A2dpSource::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("a2dp connect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return proxy->Connect(RawAddress(device.GetDeviceAddr()));
}

int32_t A2dpSource::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("a2dp disconnect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    return proxy->Disconnect(RawAddress(device.GetDeviceAddr()));
}

A2dpSource *A2dpSource::GetProfile()
{
    HILOGD("enter");
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<A2dpSource> service;
    return service.get();
#else
    static A2dpSource service;
    return &service;
#endif
}

int A2dpSource::SetActiveSinkDevice(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return proxy->SetActiveSinkDevice(RawAddress(device.GetDeviceAddr()));
}

const BluetoothRemoteDevice &A2dpSource::GetActiveSinkDevice() const
{
    HILOGI("enter");
    static BluetoothRemoteDevice deviceInfo;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return deviceInfo;
    }
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, deviceInfo, "failed: no proxy");

    BluetoothRawAddress rawAddress = proxy->GetActiveSinkDevice();
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
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if ((!device.IsValidBluetoothRemoteDevice()) || (
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) &&
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)))) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    return proxy->SetConnectStrategy(RawAddress(device.GetDeviceAddr()), strategy);
}

int A2dpSource::GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    return proxy->GetConnectStrategy(RawAddress(device.GetDeviceAddr()), strategy);
}

A2dpCodecStatus A2dpSource::GetCodecStatus(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    A2dpCodecStatus ret;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return ret;
    }
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, ret, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return ret;
    }

    BluetoothA2dpCodecStatus codecStatus = proxy->GetCodecStatus(RawAddress(device.GetDeviceAddr()));
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
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }

    BluetoothA2dpCodecInfo serviceInfo;
    int ret = proxy->GetCodecPreference(RawAddress(device.GetDeviceAddr()), serviceInfo);
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
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

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

    return proxy->SetCodecPreference(RawAddress(device.GetDeviceAddr()), serviceInfo);
}

void A2dpSource::SwitchOptionalCodecs(const BluetoothRemoteDevice &device, bool isEnable)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return;
    }

    proxy->SwitchOptionalCodecs(RawAddress(device.GetDeviceAddr()), isEnable);
}

int A2dpSource::GetOptionalCodecsSupportState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return proxy->GetOptionalCodecsSupportState(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::StartPlaying(const BluetoothRemoteDevice &device)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return proxy->StartPlaying(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::SuspendPlaying(const BluetoothRemoteDevice &device)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return proxy->SuspendPlaying(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::StopPlaying(const BluetoothRemoteDevice &device)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, RET_BAD_STATUS, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGE("input parameter error.");
        return RET_BAD_PARAM;
    }

    return proxy->StopPlaying(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::WriteFrame(const uint8_t *data, uint32_t size)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }

    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");

    return proxy->WriteFrame(data, size);
}

int A2dpSource::GetRenderPosition(const BluetoothRemoteDevice &device, uint32_t &delayValue, uint64_t &sendDataSize,
                                  uint32_t &timeStamp)
{
    HILOGI("enter");
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return RET_BAD_STATUS;
    }
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device err");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "a2dpSrc proxy is nullptr");
    return proxy->GetRenderPosition(RawAddress(device.GetDeviceAddr()), delayValue, sendDataSize, timeStamp);
}

int A2dpSource::OffloadStartPlaying(const BluetoothRemoteDevice &device, const std::vector<int32_t> &sessionsId)
{
    HILOGI("enter, start playing device:%{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device err");
    CHECK_AND_RETURN_LOG_RET(device.GetDeviceAddr() != INVALID_MAC_ADDRESS, BT_ERR_INVALID_PARAM, "invaild mac");
    CHECK_AND_RETURN_LOG_RET(sessionsId.size() != 0, BT_ERR_INVALID_PARAM, "session size zero.");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY,
        "a2dpSrc proxy is nullptr");
    return proxy->OffloadStartPlaying(RawAddress(device.GetDeviceAddr()), sessionsId);
}

int A2dpSource::OffloadStopPlaying(const BluetoothRemoteDevice &device, const std::vector<int32_t> &sessionsId)
{
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "device err");
    CHECK_AND_RETURN_LOG_RET(device.GetDeviceAddr() != INVALID_MAC_ADDRESS, BT_ERR_INVALID_PARAM, "invaild mac");
    CHECK_AND_RETURN_LOG_RET(sessionsId.size() != 0, BT_ERR_INVALID_PARAM, "session size zero.");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY,
        "a2dpSrc proxy is nullptr");
    return proxy->OffloadStopPlaying(RawAddress(device.GetDeviceAddr()), sessionsId);
}

int A2dpSource::A2dpOffloadSessionRequest(const BluetoothRemoteDevice &device, const std::vector<A2dpStreamInfo> &info)
{
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), A2DP_STREAM_ENCODE_UNKNOWN, "device err");
    CHECK_AND_RETURN_LOG_RET(device.GetDeviceAddr() != INVALID_MAC_ADDRESS, A2DP_STREAM_ENCODE_UNKNOWN, "invaild mac");
    CHECK_AND_RETURN_LOG_RET(info.size() != 0, A2DP_STREAM_ENCODE_SOFTWARE, "empty stream");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), A2DP_STREAM_ENCODE_UNKNOWN, "bluetooth is off.");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, A2DP_STREAM_ENCODE_UNKNOWN, "a2dpSrc proxy is nullptr");

    std::vector<BluetoothA2dpStreamInfo> streamsInfo = {};
    BluetoothA2dpStreamInfo streamInfo;
    for (auto stream : info) {
        streamInfo.sessionId = stream.sessionId;
        streamInfo.streamType = stream.streamType;
        streamInfo.sampleRate = stream.sampleRate;
        streamInfo.isSpatialAudio = stream.isSpatialAudio;
        streamsInfo.push_back(streamInfo);
    }
    return proxy->A2dpOffloadSessionPathRequest(RawAddress(device.GetDeviceAddr()), streamsInfo);
}

A2dpOffloadCodecStatus::A2dpOffloadCodecStatus(const BluetoothA2dpOffloadCodecStatus &status)
{
    offloadInfo.mediaPacketHeader = status.offloadInfo.mediaPacketHeader;
    offloadInfo.mPt = status.offloadInfo.mPt;
    offloadInfo.ssrc = status.offloadInfo.ssrc;
    offloadInfo.boundaryFlag = status.offloadInfo.boundaryFlag;
    offloadInfo.broadcastFlag = status.offloadInfo.broadcastFlag;
    offloadInfo.codecType = status.offloadInfo.codecType;
    offloadInfo.maxLatency = status.offloadInfo.maxLatency;
    offloadInfo.scmsTEnable = status.offloadInfo.scmsTEnable;
    offloadInfo.sampleRate = status.offloadInfo.sampleRate;
    offloadInfo.encodedAudioBitrate = status.offloadInfo.encodedAudioBitrate;
    offloadInfo.bitsPerSample = status.offloadInfo.bitsPerSample;
    offloadInfo.chMode = status.offloadInfo.chMode;
    offloadInfo.aclHdl = status.offloadInfo.aclHdl;
    offloadInfo.l2cRcid = status.offloadInfo.l2cRcid;
    offloadInfo.mtu = status.offloadInfo.mtu;
    offloadInfo.codecSpecific0 = status.offloadInfo.codecSpecific0;
    offloadInfo.codecSpecific1 = status.offloadInfo.codecSpecific1;
    offloadInfo.codecSpecific2 = status.offloadInfo.codecSpecific2;
    offloadInfo.codecSpecific3 = status.offloadInfo.codecSpecific3;
    offloadInfo.codecSpecific4 = status.offloadInfo.codecSpecific4;
    offloadInfo.codecSpecific5 = status.offloadInfo.codecSpecific5;
    offloadInfo.codecSpecific6 = status.offloadInfo.codecSpecific6;
    offloadInfo.codecSpecific7 = status.offloadInfo.codecSpecific7;
}

A2dpOffloadCodecStatus A2dpSource::GetOffloadCodecStatus(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter");
    A2dpOffloadCodecStatus ret;
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), ret, "input device err");
    CHECK_AND_RETURN_LOG_RET(device.GetDeviceAddr() != INVALID_MAC_ADDRESS, ret, "invaild mac");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), ret, "bluetooth is off.");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, ret, "a2dpSrc proxy is nullptr");
    BluetoothA2dpOffloadCodecStatus offloadStatus =
        proxy->GetOffloadCodecStatus(RawAddress(device.GetDeviceAddr()));
    A2dpOffloadCodecStatus status(offloadStatus);
    HILOGI("codecType:%{public}x,mtu:%{public}d", status.offloadInfo.codecType, status.offloadInfo.mtu);
    return status;
}

int A2dpSource::EnableAutoPlay(const BluetoothRemoteDevice &device)
{
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "input device err");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "a2dpSrc proxy is nullptr");
    return proxy->EnableAutoPlay(RawAddress(device.GetDeviceAddr()));
}

int A2dpSource::DisableAutoPlay(const BluetoothRemoteDevice &device, const int duration)
{
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "input device err");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "a2dpSrc proxy is nullptr");
    return proxy->DisableAutoPlay(RawAddress(device.GetDeviceAddr()), duration);
}

int A2dpSource::GetAutoPlayDisabledDuration(const BluetoothRemoteDevice &device, int &duration)
{
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "input device err");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_STATE, "a2dpSrc proxy is nullptr");
    return proxy->GetAutoPlayDisabledDuration(RawAddress(device.GetDeviceAddr()), duration);
}

void A2dpSource::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    CHECK_AND_RETURN_LOG(IS_BT_ENABLED(), "bluetooth is off.");
    sptr<IBluetoothA2dpSrc> proxy = GetRemoteProxy<IBluetoothA2dpSrc>(PROFILE_A2DP_SRC);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "a2dpSrc proxy is nullptr");
    proxy->GetVirtualDeviceList(devices);
}
} // namespace Bluetooth
} // namespace OHOS