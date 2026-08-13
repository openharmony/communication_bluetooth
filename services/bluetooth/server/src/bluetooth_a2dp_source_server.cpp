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
#define LOG_TAG "bt_server_a2dp_source"
#endif

#include "bluetooth_a2dp_source_server.h"

#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hitrace.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "hisysevent.h"
#include "interface_profile_manager.h"
#include "interface_profile_a2dp_src.h"
#include "interface_adapter_manager.h"
#include "ipc_skeleton.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"
#include "log_utils.h"
#include "bluetooth_ble_central_manager_server.h"

namespace OHOS {
namespace Bluetooth {
struct BluetoothA2dpSourceServer::impl {
    impl();
    ~impl();

    /// sys state observer
    class SystemStateObserver;
    std::unique_ptr<SystemStateObserver> systemStateObserver_ = nullptr;

    class A2dpSourceObserver;
    std::unique_ptr<A2dpSourceObserver> observerImp_ = nullptr;

    RemoteObserverList<IBluetoothA2dpSourceObserver> observers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;
    IProfileA2dpSrc *a2dpSrcService_ = nullptr;
};

class BluetoothA2dpSourceServer::impl::SystemStateObserver : public ISystemStateObserver {
public:
    explicit SystemStateObserver(BluetoothA2dpSourceServer::impl *pimpl) : pimpl_(pimpl) {};
    ~SystemStateObserver() override = default;

    void OnSystemStateChange(const BTSystemState state) override
    {
        IProfileManager *serviceMgr = IProfileManager::GetInstance();
        if (!pimpl_) {
            HILOGI("failed: pimpl_ is null");
            return;
        }

        switch (state) {
            case BTSystemState::ON:
                if (serviceMgr != nullptr) {
                    pimpl_->a2dpSrcService_ = (IProfileA2dpSrc *)serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC);
                    if (pimpl_->a2dpSrcService_ != nullptr) {
                        pimpl_->a2dpSrcService_->RegisterObserver(
                            reinterpret_cast<IA2dpObserver *>(pimpl_->observerImp_.get()));
                    }
                }
                break;
            case BTSystemState::OFF:
                pimpl_->a2dpSrcService_ = nullptr;
                break;
            default:
                break;
        }
    }

private:
    BluetoothA2dpSourceServer::impl *pimpl_ = nullptr;
};

class BluetoothA2dpSourceServer::impl::A2dpSourceObserver : public IA2dpObserver {
public:
    explicit A2dpSourceObserver(BluetoothA2dpSourceServer::impl *pimpl) : pimpl_(pimpl) {};
    ~A2dpSourceObserver() override = default;

    void OnConnectionStateChanged(const RawAddress &device, int state, int cause) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOG_COMM_INFO("OnConnectionStateChanged: %{public}s, state: %{public}d, cause: %{public}d",
            GET_ENCRYPT_ADDR(device), state, cause);
        if (state == static_cast<int>(BTConnectState::CONNECTED) ||
            state == static_cast<int>(BTConnectState::DISCONNECTED)) {
            HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "A2DP_CONNECTED_STATE",
                OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "STATE", state);
        }
        observers_->ForEach([this, device, state, cause](sptr<IBluetoothA2dpSourceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            if (PermissionManager::IsNativeCaller(tokenId) ||
                PermissionManager::GetApiVersion(tokenId) >= API_VERSION_10) {
                CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                    "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                    PermissionManager::GetCallingName(tokenId).c_str());
            }
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnConnectionStateChanged(randomAddr, state, cause);
            }
        });
    }

    void OnCaptureConnectionStateChanged(const RawAddress &device, int state, const A2dpSrcCodecInfo &info) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, state: %{public}d", GET_ENCRYPT_ADDR(device), state);
        observers_->ForEach([this, device, state, info](sptr<IBluetoothA2dpSourceObserver> observer) {
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            int32_t pid = this->pimpl_->appContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            CHECK_AND_RETURN_LOG(PermissionManager::VerifyPermission(ACCESS_BLUETOOTH, tokenId),
                "[PERMISSION] check ACCESS_BLUETOOTH permission failed, callingName(%{public}s)",
                PermissionManager::GetCallingName(tokenId).c_str());
            BluetoothA2dpCodecInfo tmpInfo{};
            tmpInfo.bitsPerSample = info.bitsPerSample;
            tmpInfo.channelMode = info.channelMode;
            tmpInfo.codecType = info.codecType;
            tmpInfo.sampleRate = info.sampleRate;
            RawAddress randomAddr;
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnCaptureConnectionStateChanged(randomAddr, state, tmpInfo);
            }
        });
    }

    void OnPlayingStatusChaned(const RawAddress &device, int playingState, int error) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, state: %{public}d, error: %{public}d",
            GET_ENCRYPT_ADDR(device), playingState, error);
        observers_->ForEach([this, device, playingState, error](sptr<IBluetoothA2dpSourceObserver> observer) {
            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            int32_t pid = this->pimpl_->appContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnPlayingStatusChanged(randomAddr, playingState, error);
            }
        });
    }

    void OnConfigurationChanged(const RawAddress &device, const A2dpSrcCodecInfo &info, int error) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, error: %{public}d", GET_ENCRYPT_ADDR(device), error);
        observers_->ForEach([this, device, info, error](sptr<IBluetoothA2dpSourceObserver> observer) {
            BluetoothA2dpCodecInfo  tmpInfo {};
            tmpInfo.bitsPerSample = info.bitsPerSample;
            tmpInfo.channelMode = info.channelMode;
            tmpInfo.codecPriority = info.codecPriority;
            tmpInfo.codecType = info.codecType;
            tmpInfo.sampleRate = info.sampleRate;
            tmpInfo.codecSpecific1 = info.codecSpecific1;
            tmpInfo.codecSpecific2 = info.codecSpecific2;
            tmpInfo.codecSpecific3 = info.codecSpecific3;
            tmpInfo.codecSpecific4 = info.codecSpecific4;

            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            int32_t pid = this->pimpl_->appContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnConfigurationChanged(randomAddr, tmpInfo, error);
            }
        });
    }

    void OnMediaStackChanged(const RawAddress &device, int action) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, action: %{public}d", GET_ENCRYPT_ADDR(device), action);
        observers_->ForEach([this, device, action](sptr<IBluetoothA2dpSourceObserver> observer) {
            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            int32_t pid = this->pimpl_->appContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnMediaStackChanged(randomAddr, action);
            }
        });
    }

    void OnVirtualDeviceChanged(int32_t action, std::string address) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, action: %{public}d", GetEncryptAddr(address).c_str(), action);

        observers_->ForEach([this, action, address](sptr<IBluetoothA2dpSourceObserver> observer) {
            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            int32_t pid = this->pimpl_->appContainer_->GetRemotePid(observer->AsObject());
            if (BluetoothBleCentralManagerServer::IsResourceScheduleApp(pid)) {
                HILOGD("pid:%{public}d is proxy pid, not callback.", pid);
                return;
            }
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, RawAddress(address),
                                                                              randomAddr)) {
                std::string strAddress = randomAddr.GetAddress();
                observer->OnVirtualDeviceChanged(action, strAddress);
            }
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothA2dpSourceObserver> *observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothA2dpSourceObserver> *observers_;
    BluetoothA2dpSourceServer::impl *pimpl_ = nullptr;
};

BluetoothA2dpSourceServer::impl::impl()
{
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothA2dpSourceServer::impl::~impl()
{
}

BluetoothA2dpSourceServer::BluetoothA2dpSourceServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->systemStateObserver_ = std::make_unique<impl::SystemStateObserver>(pimpl.get());
    pimpl->observerImp_ = std::make_unique<impl::A2dpSourceObserver>(pimpl.get());
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->systemStateObserver_));

    IProfileManager *serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr != nullptr) {
        pimpl->a2dpSrcService_ = (IProfileA2dpSrc *)serviceMgr->GetProfileService(PROFILE_NAME_A2DP_SRC);
        if (pimpl->a2dpSrcService_ != nullptr) {
            pimpl->a2dpSrcService_->RegisterObserver(pimpl->observerImp_.get());
        }
    }
}

BluetoothA2dpSourceServer::~BluetoothA2dpSourceServer()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->systemStateObserver_));
    if (pimpl->a2dpSrcService_ != nullptr) {
        pimpl->a2dpSrcService_->DeregisterObserver(pimpl->observerImp_.get());
    }
}

void BluetoothA2dpSourceServer::RegisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return;
    }
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return;
    }
    if (pimpl->appContainer_->Size() > MAX_MAP_SIZE || pimpl->observers_.Size() > MAX_MAP_SIZE) {
        HILOGE("appContainer_ or observers_ too much");
        return;
    }
    pimpl->appContainer_->AddObject(observer->AsObject());
    pimpl->observers_.Register(observer);
    if (pimpl->a2dpSrcService_ == nullptr) {
        return;
    }
}

void BluetoothA2dpSourceServer::DeregisterObserver(const sptr<IBluetoothA2dpSourceObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return;
    }

    if (pimpl != nullptr) {
        pimpl->observers_.Deregister(observer);
        pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
    }
}

#define CHECK_PIMPL_NULL_RETURN() \
do { \
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) { \
        HILOGE("pimpl or a2dpSrcService_ is null"); \
        return BT_ERR_INTERNAL_ERROR; \
    } \
} while (0)

int32_t BluetoothA2dpSourceServer::Connect(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    if (PermissionManager::GetApiVersion() >= API_VERSION_10) {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::IsSystemHap(),
            BT_ERR_SYSTEM_PERMISSION_FAILED, "[PERMISSION] system hap needed.");
    }
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    OHOS::Bluetooth::BluetoothHiTrace::BluetoothStartAsyncTrace("A2DP_SRC_CONNECT", 1);
    int32_t result = pimpl->a2dpSrcService_->Connect(realAddr);
    if (result != RET_NO_ERROR) {
        result = BT_ERR_INTERNAL_ERROR;
    }
    OHOS::Bluetooth::BluetoothHiTrace::BluetoothFinishAsyncTrace("A2DP_SRC_CONNECT", 1);
    return result;
}

int32_t BluetoothA2dpSourceServer::Disconnect(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    if (PermissionManager::GetApiVersion() >= API_VERSION_10) {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::IsSystemHap(),
            BT_ERR_SYSTEM_PERMISSION_FAILED, "[PERMISSION] system hap needed.");
    }
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->Disconnect(realAddr);
}

int BluetoothA2dpSourceServer::GetDeviceState(const RawAddress &device, int &state)
{
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(device, realAddr)) {
        HILOGE("address not exists");
        return BT_ERR_INTERNAL_ERROR;
    }
    state = pimpl->a2dpSrcService_->GetDeviceState(realAddr);
    return NO_ERROR;
}

int BluetoothA2dpSourceServer::GetDevicesByStates(const std::vector<int32_t> &states, std::vector<RawAddress> &rawAddrs)
{
    std::vector<int> tmpStates;
    for (int32_t state : states) {
        HILOGI("state: %{public}d", state);
        tmpStates.push_back((int)state);
    }

    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<RawAddress> realAddrs = pimpl->a2dpSrcService_->GetDevicesByStates(tmpStates);
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(realAddrs, rawAddrs);
    return NO_ERROR;
}

int32_t BluetoothA2dpSourceServer::GetPlayingState(const RawAddress &device, int &state)
{
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(device, realAddr)) {
        HILOGE("address not exists");
        return BT_ERR_INTERNAL_ERROR;
    }
    int ret = pimpl->a2dpSrcService_->GetPlayingState(realAddr, state);
    HILOGI("addr: %{public}s, state: %{public}d", GET_ENCRYPT_ADDR(device), state);
    if (ret != BT_NO_ERROR) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return ret;
}

int BluetoothA2dpSourceServer::SetConnectStrategy(const RawAddress &device, int strategy)
{
    HILOGI("addr: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int res = BT_ERR_INTERNAL_ERROR;
    if (pimpl->a2dpSrcService_) {
        res = pimpl->a2dpSrcService_->SetConnectStrategy(realAddr, strategy);
    }
    return res;
}

int BluetoothA2dpSourceServer::GetConnectStrategy(const RawAddress &device, int &strategy)
{
    HILOGD("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    strategy = pimpl->a2dpSrcService_->GetConnectStrategy(realAddr);
    return BT_NO_ERROR;
}

int BluetoothA2dpSourceServer::SetActiveSinkDevice(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->SetActiveSinkDevice(realAddr);
}

RawAddress BluetoothA2dpSourceServer::GetActiveSinkDevice()
{
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return RawAddress(INVALID_MAC_ADDRESS);
    }
    RawAddress device = pimpl->a2dpSrcService_->GetActiveSinkDevice();

    bool isUseRealAddr = PermissionManager::IsUseRealAddr();
    if (isUseRealAddr) {
        return device;
    } else {
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_SUCCESS) {
            return randomAddr;
        }
    }
    return RawAddress(INVALID_MAC_ADDRESS);
}

BluetoothA2dpCodecStatus BluetoothA2dpSourceServer::GetCodecStatus(const RawAddress &device)
{
    BluetoothA2dpCodecStatus codeStatus;
    BluetoothA2dpCodecInfo serviceInfo;
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return codeStatus;
    }
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return codeStatus;
    }
    bluetooth::A2dpSrcCodecStatus ret = pimpl->a2dpSrcService_->GetCodecStatus(realAddr);

    HILOGI("codecPriority = %{public}u, codecType = %{public}u, sampleRate = %{public}u, bitsPerSample = %{public}d, "
        "channelMode = %{public}d", ret.codecInfo.codecPriority, ret.codecInfo.codecType, ret.codecInfo.sampleRate,
        ret.codecInfo.bitsPerSample, ret.codecInfo.channelMode);

    codeStatus.codecInfo.codecPriority = ret.codecInfo.codecPriority;
    codeStatus.codecInfo.codecType = ret.codecInfo.codecType;
    codeStatus.codecInfo.sampleRate = ret.codecInfo.sampleRate;
    codeStatus.codecInfo.bitsPerSample = ret.codecInfo.bitsPerSample;
    codeStatus.codecInfo.channelMode = ret.codecInfo.channelMode;
    codeStatus.codecInfo.codecSpecific1 = ret.codecInfo.codecSpecific1;
    codeStatus.codecInfo.codecSpecific2 = ret.codecInfo.codecSpecific2;
    codeStatus.codecInfo.codecSpecific3 = ret.codecInfo.codecSpecific3;
    codeStatus.codecInfo.codecSpecific4 = ret.codecInfo.codecSpecific4;

    for (auto it = ret.codecInfoConfirmedCap.begin(); it != ret.codecInfoConfirmedCap.end(); it++) {
        serviceInfo.codecPriority = it->codecPriority;
        serviceInfo.codecType = it->codecType;
        serviceInfo.sampleRate = it->sampleRate;
        serviceInfo.bitsPerSample = it->bitsPerSample;
        serviceInfo.channelMode = it->channelMode;
        serviceInfo.codecSpecific1 = it->codecSpecific1;
        serviceInfo.codecSpecific2 = it->codecSpecific2;
        serviceInfo.codecSpecific3 = it->codecSpecific3;
        serviceInfo.codecSpecific4 = it->codecSpecific4;
        codeStatus.codecInfoConfirmCap.push_back(serviceInfo);
    }

    for (auto it = ret.codecInfoLocalCap.begin(); it != ret.codecInfoLocalCap.end(); it++) {
        serviceInfo.codecPriority = it->codecPriority;
        serviceInfo.codecType = it->codecType;
        serviceInfo.sampleRate = it->sampleRate;
        serviceInfo.bitsPerSample = it->bitsPerSample;
        serviceInfo.channelMode = it->channelMode;
        serviceInfo.codecSpecific1 = it->codecSpecific1;
        serviceInfo.codecSpecific2 = it->codecSpecific2;
        serviceInfo.codecSpecific3 = it->codecSpecific3;
        serviceInfo.codecSpecific4 = it->codecSpecific4;
        codeStatus.codecInfoLocalCap.push_back(serviceInfo);
    }

    return codeStatus;
}

int BluetoothA2dpSourceServer::GetCodecPreference(const RawAddress &device, BluetoothA2dpCodecInfo &info)
{
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    bluetooth::A2dpSrcCodecInfo setInfo;
    int ret = pimpl->a2dpSrcService_->GetCodecPreference(realAddr, setInfo);
    if (ret != BT_NO_ERROR) {
        HILOGE("GetCodecPreference error");
        return ret;
    }
    info.bitsPerSample = setInfo.bitsPerSample;
    info.channelMode = setInfo.channelMode;
    info.codecType = setInfo.codecType;
    info.sampleRate = setInfo.sampleRate;
    info.codecSpecific3 = setInfo.codecSpecific3;
    info.codecSpecific4 = setInfo.codecSpecific4;
    return ret;
}

int BluetoothA2dpSourceServer::SetCodecPreference(const RawAddress &device, const BluetoothA2dpCodecInfo &info)
{
    HILOGI("codecPriority = %{public}u, codecType = %{public}u, sampleRate = %{public}u, "
           "bitsPerSample = %{public}d, channelMode = %{public}d, codecSpecific1 = %{public}llu, "
           "codecSpecific2 = %{public}llu, codecSpecific3 = %{public}llu, codecSpecific4 = %{public}llu",
           info.codecPriority, info.codecType, info.sampleRate, info.bitsPerSample, info.channelMode,
           static_cast<unsigned long long>(info.codecSpecific1), static_cast<unsigned long long>(info.codecSpecific2),
           static_cast<unsigned long long>(info.codecSpecific3), static_cast<unsigned long long>(info.codecSpecific4));

    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    bluetooth::A2dpSrcCodecInfo setInfo;

    setInfo.bitsPerSample = info.bitsPerSample;
    setInfo.channelMode = info.channelMode;
    setInfo.codecPriority = info.codecPriority;
    setInfo.codecType = info.codecType;
    setInfo.sampleRate = info.sampleRate;
    setInfo.codecSpecific1 = info.codecSpecific1;
    setInfo.codecSpecific2 = info.codecSpecific2;
    setInfo.codecSpecific3 = info.codecSpecific3;
    setInfo.codecSpecific4 = info.codecSpecific4;

    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->SetCodecPreference(realAddr, setInfo);
}

void BluetoothA2dpSourceServer::SwitchOptionalCodecs(const RawAddress &device, bool isEnable)
{
    HILOGI("addr: %{public}s, isEnable = %{public}d", GET_ENCRYPT_ADDR(device), isEnable);
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return;
    }
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return;
    }
    pimpl->a2dpSrcService_->SwitchOptionalCodecs(realAddr, isEnable);
}

int BluetoothA2dpSourceServer::GetOptionalCodecsSupportState(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->GetOptionalCodecsSupportState(realAddr);
}

int BluetoothA2dpSourceServer::StartPlaying(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->StartPlaying(realAddr);
}

int BluetoothA2dpSourceServer::SuspendPlaying(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->SuspendPlaying(realAddr);
}

int BluetoothA2dpSourceServer::StopPlaying(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->StopPlaying(realAddr);
}

int BluetoothA2dpSourceServer::WriteFrame(const uint8_t *data, uint32_t size)
{
    HILOGI("size = %{public}u", size);
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->WriteFrame(data, size);
}

int BluetoothA2dpSourceServer::GetRenderPosition(const RawAddress &device, uint32_t &delayValue, uint64_t &sendDataSize,
                                                 uint32_t &timeStamp)
{
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }
    return pimpl->a2dpSrcService_->GetRenderPosition(device, delayValue, sendDataSize, timeStamp);
}

int BluetoothA2dpSourceServer::OffloadStartPlaying(const RawAddress &device, const std::vector<int32_t> &sessionsId)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    std::vector<int> sessions = {};
    for (auto sessionId : sessionsId) {
        sessions.push_back(sessionId);
    }
    return pimpl->a2dpSrcService_->OffloadStartPlaying(realAddr, sessions);
}

int BluetoothA2dpSourceServer::OffloadStopPlaying(const RawAddress &device, const std::vector<int32_t> &sessionsId)
{
    HILOGD("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<int> sessions = {};
    for (auto sessionId : sessionsId) {
        sessions.push_back(sessionId);
    }
    return pimpl->a2dpSrcService_->OffloadStopPlaying(realAddr, sessions);
}

int BluetoothA2dpSourceServer::A2dpOffloadSessionPathRequest(const RawAddress &device,
    const std::vector<BluetoothA2dpStreamInfo> &info)
{
    HILOGI_TIME_LIMIT(GET_ENCRYPT_ADDR(device), "addr: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_PIMPL_NULL_RETURN();
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }

    std::vector<A2dpSrcStreamInfo> streams = {};
    A2dpSrcStreamInfo stream;
    for (auto streamInfo : info) {
        stream.sessionId = streamInfo.sessionId;
        stream.streamType = streamInfo.streamType;
        stream.sampleRate = streamInfo.sampleRate;
        stream.isSpatialAudio = streamInfo.isSpatialAudio;
        streams.push_back(stream);
    }
    return pimpl->a2dpSrcService_->A2dpOffloadSessionPathRequest(realAddr, streams);
}

BluetoothA2dpOffloadCodecStatus BluetoothA2dpSourceServer::GetOffloadCodecStatus(const RawAddress &device)
{
    HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
    BluetoothA2dpOffloadCodecStatus offloadStatus;
    RawAddress realAddr;
    CHECK_AND_RETURN_LOG_RET(BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(device, realAddr),
        offloadStatus, "ConvertToRealAddress error");
    CHECK_AND_RETURN_LOG_RET(((pimpl != nullptr) && (pimpl->a2dpSrcService_ != nullptr)), offloadStatus,
        "pimpl or a2dpSrcService_ is null");
    bluetooth::A2dpSrcOffloadCodecStatus ret = pimpl->a2dpSrcService_->GetA2dpOffloadCodecStatus(realAddr);
    offloadStatus.offloadInfo.mediaPacketHeader = ret.codecInfo.mediaPacketHeader;
    offloadStatus.offloadInfo.mPt = ret.codecInfo.mPt;
    offloadStatus.offloadInfo.ssrc = ret.codecInfo.ssrc;
    offloadStatus.offloadInfo.boundaryFlag = ret.codecInfo.boundaryFlag;
    offloadStatus.offloadInfo.broadcastFlag = ret.codecInfo.broadcastFlag;
    offloadStatus.offloadInfo.codecType = ret.codecInfo.codecType;
    offloadStatus.offloadInfo.maxLatency = ret.codecInfo.maxLatency;
    offloadStatus.offloadInfo.scmsTEnable = ret.codecInfo.scmsTEnable;
    offloadStatus.offloadInfo.sampleRate = ret.codecInfo.sampleRate;
    offloadStatus.offloadInfo.encodedAudioBitrate = ret.codecInfo.encodedAudioBitrate;
    offloadStatus.offloadInfo.bitsPerSample = ret.codecInfo.bitsPerSample;
    offloadStatus.offloadInfo.chMode = ret.codecInfo.chMode;
    offloadStatus.offloadInfo.aclHdl = ret.codecInfo.aclHdl;
    offloadStatus.offloadInfo.l2cRcid = ret.codecInfo.l2cRcid;
    offloadStatus.offloadInfo.mtu = ret.codecInfo.mtu;
    offloadStatus.offloadInfo.codecSpecific0 = ret.codecInfo.codecSpecific0;
    offloadStatus.offloadInfo.codecSpecific1 = ret.codecInfo.codecSpecific1;
    offloadStatus.offloadInfo.codecSpecific2 = ret.codecInfo.codecSpecific2;
    offloadStatus.offloadInfo.codecSpecific3 = ret.codecInfo.codecSpecific3;
    offloadStatus.offloadInfo.codecSpecific4 = ret.codecInfo.codecSpecific4;
    offloadStatus.offloadInfo.codecSpecific5 = ret.codecInfo.codecSpecific5;
    offloadStatus.offloadInfo.codecSpecific6 = ret.codecInfo.codecSpecific6;
    offloadStatus.offloadInfo.codecSpecific7 = ret.codecInfo.codecSpecific7;
    return offloadStatus;
}

int BluetoothA2dpSourceServer::EnableAutoPlay(const RawAddress &device)
{
    CHECK_PIMPL_NULL_RETURN();
    return pimpl->a2dpSrcService_->EnableAutoPlay(device);
}

int BluetoothA2dpSourceServer::DisableAutoPlay(const RawAddress &device, const int duration)
{
    CHECK_PIMPL_NULL_RETURN();
    return pimpl->a2dpSrcService_->DisableAutoPlay(device, duration);
}

int BluetoothA2dpSourceServer::GetAutoPlayDisabledDuration(const RawAddress &device, int &duration)
{
    CHECK_PIMPL_NULL_RETURN();
    return pimpl->a2dpSrcService_->GetAutoPlayDisabledDuration(device, duration);
}
void BluetoothA2dpSourceServer::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    HILOGI("intro");
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return;
    }

    pimpl->a2dpSrcService_->GetVirtualDeviceList(devices);
}

void BluetoothA2dpSourceServer::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    HILOGI("intro");
    if (pimpl == nullptr || pimpl->a2dpSrcService_ == nullptr) {
        HILOGE("pimpl or a2dpSrcService_ is null");
        return;
    }

    pimpl->a2dpSrcService_->UpdateVirtualDevice(action, address);
}
}  // namespace Bluetooth
}  // namespace OHOS
