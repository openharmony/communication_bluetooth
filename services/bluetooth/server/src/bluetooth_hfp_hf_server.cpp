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
#define LOG_TAG "bt_server_hfp_hf"
#endif

#include <vector>
#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hfp_hf_server.h"
#include "bluetooth_log.h"
#include "bluetooth_utils_server.h"
#include "interface_profile_hfp_hf.h"
#include "interface_profile_manager.h"
#include "interface_profile.h"
#include "interface_adapter_manager.h"
#include "bluetooth_observer_application_container.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "hitrace_meter.h"

using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
struct BluetoothHfpHfServer::impl {
    RemoteObserverList<IBluetoothHfpHfObserver> observers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;
    IProfileHfpHf* HfpHfService_ = nullptr;
    impl();
    class HfpHfServerObserver;
    std::unique_ptr<HfpHfServerObserver> observerImp_ = nullptr;
    class HfpHfSystemObserver;
    std::unique_ptr<HfpHfSystemObserver> HfpHfSystemObserver_;
};

class BluetoothHfpHfServer::impl::HfpHfSystemObserver : public ISystemStateObserver {
public:
    explicit HfpHfSystemObserver(BluetoothHfpHfServer::impl* pimpl) : pimpl_(pimpl) {};
    void OnSystemStateChange(const BTSystemState state) override
    {
        HILOGI("state: %{public}d", state);
        IProfileManager* serviceMgr = IProfileManager::GetInstance();
        switch (state) {
            case BTSystemState::ON:
                if (serviceMgr != nullptr) {
                    pimpl_->HfpHfService_ = (IProfileHfpHf*)serviceMgr->GetProfileService(PROFILE_NAME_HFP_HF);
                    if (pimpl_->HfpHfService_ != nullptr) {
                        pimpl_->HfpHfService_->RegisterObserver(
                            *reinterpret_cast<HfpHfServiceObserver *>(pimpl_->observerImp_.get()));
                    }
                }
                break;
            case BTSystemState::OFF:
                if (serviceMgr != nullptr) {
                    pimpl_->HfpHfService_ = (IProfileHfpHf*)serviceMgr->GetProfileService(PROFILE_NAME_HFP_HF);
                    if (pimpl_->HfpHfService_ != nullptr) {
                        pimpl_->HfpHfService_->DeregisterObserver(
                            *reinterpret_cast<HfpHfServiceObserver *>(pimpl_->observerImp_.get()));
                    }
                }
                pimpl_->HfpHfService_ = nullptr;
                break;
            default:
                break;
        }
    };

private:
    BluetoothHfpHfServer::impl* pimpl_;
};

class BluetoothHfpHfServer::impl::HfpHfServerObserver : public HfpHfServiceObserver {
public:
    explicit HfpHfServerObserver(BluetoothHfpHfServer::impl *pimpl) : pimpl_(pimpl) {};
    ~HfpHfServerObserver() override = default;

    void OnConnectionStateChanged(const RawAddress& device, int state, int cause) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        if (this->pimpl_->appContainer_ == nullptr) {
            HILOGE("appContainer_ is null, cannot process OnConnectionStateChanged");
            return;
        }
        observers_->ForEach([this, device, state, cause](IBluetoothHfpHfObserver* observer) {
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
                std::string addr = randomAddr.GetAddress();
                HILOGI("HfpHfServerObserver, device: %{public}s, state: %{public}d, cause: %{public}d",
                    addr.c_str(), state, cause);
            }
        });
    }

    void OnScoStateChanged(const RawAddress& device, int state) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, state: %{public}d", GET_ENCRYPT_ADDR(device), state);
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            return;
        }
        observers_->ForEach([randomAddr, state](IBluetoothHfpHfObserver* observer) {
            observer->OnScoStateChanged(randomAddr, state);
        });
    }

    void OnCallChanged(const RawAddress& device, const HandsFreeUnitCalls& call) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            return;
        }
        observers_->ForEach([randomAddr, call](IBluetoothHfpHfObserver* observer) {
            observer->OnCallChanged(randomAddr, call);
        });
    }

    void OnSignalStrengthChanged(const RawAddress& device, int signal) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, signal: %{public}d", GET_ENCRYPT_ADDR(device), signal);
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            return;
        }
        observers_->ForEach([randomAddr, signal](IBluetoothHfpHfObserver* observer) {
            observer->OnSignalStrengthChanged(randomAddr, signal);
        });
    }

    void OnRegistrationStatusChanged(const RawAddress& device, int status) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, status: %{public}d", GET_ENCRYPT_ADDR(device), status);
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            return;
        }
        observers_->ForEach([randomAddr, status](IBluetoothHfpHfObserver* observer) {
            observer->OnRegistrationStatusChanged(randomAddr, status);
        });
    }

    void OnRoamingStatusChanged(const RawAddress& device, int status) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, status: %{public}d", GET_ENCRYPT_ADDR(device), status);
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            return;
        }
        observers_->ForEach([randomAddr, status](IBluetoothHfpHfObserver* observer) {
            observer->OnRoamingStatusChanged(randomAddr, status);
        });
    }

    void OnOperatorSelectionChanged(const RawAddress& device, const std::string& name) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, name: %{public}s", GET_ENCRYPT_ADDR(device), name.c_str());
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            return;
        }
        observers_->ForEach([randomAddr, name](IBluetoothHfpHfObserver* observer) {
            observer->OnOperatorSelectionChanged(randomAddr, name);
        });
    }

    void OnSubscriberNumberChanged(const RawAddress& device, const std::string& number) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s", GET_ENCRYPT_ADDR(device));
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            return;
        }
        observers_->ForEach([randomAddr, number](IBluetoothHfpHfObserver* observer) {
            observer->OnSubscriberNumberChanged(randomAddr, number);
        });
    }

    void OnVoiceRecognitionStatusChanged(const RawAddress& device, int status) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("addr: %{public}s, status: %{public}d", GET_ENCRYPT_ADDR(device), status);
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
        if (ret == OHOS::bluetooth::RET_NO_EXIST) {
            return;
        }
        observers_->ForEach([randomAddr, status](IBluetoothHfpHfObserver* observer) {
            observer->OnVoiceRecognitionStatusChanged(randomAddr, status);
        });
    }

    void OnInBandRingToneChanged(const RawAddress& device, int status) override
    {
        HILOGI("addr: %{public}s, status: %{public}d", GET_ENCRYPT_ADDR(device), status);
    }

    void SetObserver(RemoteObserverList<IBluetoothHfpHfObserver>* observers)
    {
        observers_ = observers;
    }

private:
    BluetoothHfpHfServer::impl *pimpl_;
    RemoteObserverList<IBluetoothHfpHfObserver>* observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HfpHfServerObserver);
};

BluetoothHfpHfServer::impl::impl()
{
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothHfpHfServer::BluetoothHfpHfServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_ = std::make_unique<impl::HfpHfServerObserver>(pimpl.get());
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    pimpl->HfpHfSystemObserver_ = std::make_unique<impl::HfpHfSystemObserver>(pimpl.get());
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->HfpHfSystemObserver_));

    IProfileManager* serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr != nullptr) {
        pimpl->HfpHfService_ = (IProfileHfpHf*)serviceMgr->GetProfileService(PROFILE_NAME_HFP_HF);
        if (pimpl->HfpHfService_ != nullptr) {
            pimpl->HfpHfService_->RegisterObserver(*pimpl->observerImp_);
        }
    }
}

BluetoothHfpHfServer::~BluetoothHfpHfServer()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->HfpHfSystemObserver_));
    if (pimpl->HfpHfService_ != nullptr) {
        pimpl->HfpHfService_->DeregisterObserver(*pimpl->observerImp_);
    }
}

bool BluetoothHfpHfServer::ConnectSco(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->ConnectSco(realAddr);
    }
    return false;
}

bool BluetoothHfpHfServer::DisconnectSco(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->DisconnectSco(realAddr);
    }
    return false;
}

int BluetoothHfpHfServer::GetDevicesByStates(const std::vector<int> &states,
    std::vector<BluetoothRawAddress> &devices)
{
    std::vector<int> tmpStates;
    for (int32_t state : states) {
        HILOGI("state = %{public}d", state);
        tmpStates.push_back((int)state);
    }
    std::vector<RawAddress> rawDevices;

    if (pimpl->HfpHfService_ != nullptr) {
        rawDevices = pimpl->HfpHfService_->GetDevicesByStates(tmpStates);
    } else {
        return BT_FAILURE;
    }
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(rawDevices, devices);
    return BT_SUCCESS;
}

int BluetoothHfpHfServer::GetDeviceState(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->GetDeviceState(realAddr);
    }
    return BT_FAILURE;
}

int BluetoothHfpHfServer::GetScoState(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->GetScoState(realAddr);
    }
    return BT_FAILURE;
}

bool BluetoothHfpHfServer::SendDTMFTone(const BluetoothRawAddress &device, uint8_t code)
{
    HILOGI("addr: %{public}s, code: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), code);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->SendDTMFTone(realAddr, code);
    }
    return false;
}

int BluetoothHfpHfServer::Connect(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->Connect(realAddr);
    }
    return BT_FAILURE;
}

int BluetoothHfpHfServer::Disconnect(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->Disconnect(realAddr);
    }
    return BT_FAILURE;
}

bool BluetoothHfpHfServer::OpenVoiceRecognition(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->OpenVoiceRecognition(realAddr);
    }
    return false;
}

bool BluetoothHfpHfServer::CloseVoiceRecognition(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->CloseVoiceRecognition(realAddr);
    }
    return false;
}

int BluetoothHfpHfServer::GetCurrentCallList(const BluetoothRawAddress &device,
    std::vector<BluetoothHfpHfCall> &calls)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    std::vector<HandsFreeUnitCalls> callVector;
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        callVector = pimpl->HfpHfService_->GetCurrentCallList(realAddr);
    }
    for (HandsFreeUnitCalls call : callVector) {
        calls.push_back(BluetoothHfpHfCall(call));
    }
    return BT_FAILURE;
}

bool BluetoothHfpHfServer::AcceptIncomingCall(const BluetoothRawAddress &device, int flag)
{
    HILOGI("addr: %{public}s, flag: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), flag);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->AcceptIncomingCall(realAddr, (int)flag);
    }
    return false;
}

bool BluetoothHfpHfServer::HoldActiveCall(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->HoldActiveCall(realAddr);
    }
    return false;
}

bool BluetoothHfpHfServer::RejectIncomingCall(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->RejectIncomingCall(realAddr);
    }
    return false;
}

bool BluetoothHfpHfServer::SendKeyPressed(const BluetoothRawAddress &device)
{
    return false;
}

bool BluetoothHfpHfServer::HandleIncomingCall(const BluetoothRawAddress &device, int flag)
{
    HILOGI("addr: %{public}s, flag: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), flag);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->HandleIncomingCall(realAddr, flag);
    }
    return false;
}

bool BluetoothHfpHfServer::DialLastNumber(const BluetoothRawAddress &device)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->DialLastNumber(realAddr);
    }
    return false;
}

bool BluetoothHfpHfServer::DialMemory(const BluetoothRawAddress &device, int index)
{
    HILOGI("addr: %{public}s, index: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), index);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->DialMemory(realAddr, index);
    }
    return false;
}

bool BluetoothHfpHfServer::HandleMultiCall(const BluetoothRawAddress &device, int flag, int index)
{
    HILOGI("addr: %{public}s, flag: %{public}d, index: %{public}d",
        GetEncryptAddr((device).GetAddress()).c_str(), flag, index);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->HandleMultiCall(realAddr, flag, index);
    }
    return false;
}

bool BluetoothHfpHfServer::SendVoiceTag(const BluetoothRawAddress &device, int index)
{
    return false;
}

bool BluetoothHfpHfServer::FinishActiveCall(const BluetoothRawAddress &device, const BluetoothHfpHfCall &call)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return false;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->FinishActiveCall(realAddr, call);
    }
    return false;
}

int BluetoothHfpHfServer::StartDial(const BluetoothRawAddress &device, const std::string &number,
    BluetoothHfpHfCall &call)
{
    HILOGI("addr: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
    std::optional<HandsFreeUnitCalls> ret;
    HandsFreeUnitCalls calls;
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_ != nullptr) {
        ret = pimpl->HfpHfService_->StartDial(realAddr, number);
    }
    if (ret == std::nullopt) {
        call = calls;
        return BT_FAILURE;
    } else {
        call = *ret;
        return BT_SUCCESS;
    }
}

void BluetoothHfpHfServer::RegisterObserver(const sptr<IBluetoothHfpHfObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return;
    }
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ or appContainer_ too much");
        return;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
}

void BluetoothHfpHfServer::DeregisterObserver(const sptr<IBluetoothHfpHfObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return;
    }
    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
}

int BluetoothHfpHfServer::SetConnectStrategy(const BluetoothRawAddress &device, int strategy)
{
    HILOGI("addr: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int res = BT_ERR_INTERNAL_ERROR;
    if (pimpl->HfpHfService_) {
        res = pimpl->HfpHfService_->SetConnectStrategy(realAddr, strategy);
    }
    return res;
}

int BluetoothHfpHfServer::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpHfService_) {
        strategy = pimpl->HfpHfService_->GetConnectStrategy(realAddr);
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}


}  // namespace Bluetooth
}  // namespace OHOS
