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
#define LOG_TAG "bt_server_hfp_ag"
#endif

#include <list>

#include "hisysevent.h"
#include "ipc_skeleton.h"

#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_hfp_ag_server.h"
#include "bluetooth_hitrace.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_application_container.h"
#include "bluetooth_utils_server.h"
#include "interface_profile_hfp_ag.h"
#include "interface_profile_manager.h"
#include "interface_profile.h"
#include "interface_adapter_manager.h"
#include "permission_manager.h"
#include "remote_observer_list.h"
#include "safe_map.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

struct BluetoothHfpAgServer::impl {
    RemoteObserverList<IBluetoothHfpAgObserver> observers_;
    std::shared_ptr<BluetoothObserverApplicationContainer> appContainer_ = nullptr;
    IProfileHfpAg* HfpAgService_ = nullptr;

    impl();

    class HfpAgSystemObserver;
    std::unique_ptr<HfpAgSystemObserver> HfpAgSystemObserver_ = nullptr;
    class HfpAgServerObserver;
    std::unique_ptr<HfpAgServerObserver> observerImp_ = nullptr;
};

class BluetoothHfpAgServer::impl::HfpAgSystemObserver : public ISystemStateObserver {
public:
    explicit HfpAgSystemObserver(BluetoothHfpAgServer::impl *pimpl) : pimpl_(pimpl){};
    void OnSystemStateChange(const BTSystemState state) override
    {
        HILOGD("state:%{public}d", state);
        IProfileManager *serviceMgr = IProfileManager::GetInstance();
        switch (state) {
            case BTSystemState::ON:
                if (serviceMgr != nullptr) {
                    pimpl_->HfpAgService_ = (IProfileHfpAg *)serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG);
                    if (pimpl_->HfpAgService_ != nullptr) {
                        pimpl_->HfpAgService_->RegisterObserver(
                            *reinterpret_cast<HfpAgServiceObserver *>(pimpl_->observerImp_.get()));
                    }
                }
                break;
            case BTSystemState::OFF:
                if (serviceMgr != nullptr) {
                    pimpl_->HfpAgService_ = (IProfileHfpAg *)serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG);
                    if (pimpl_->HfpAgService_ != nullptr) {
                        pimpl_->HfpAgService_->DeregisterObserver(
                            *reinterpret_cast<HfpAgServiceObserver *>(pimpl_->observerImp_.get()));
                    }
                }
                pimpl_->HfpAgService_ = nullptr;
                break;
            default:
                break;
        }
    };

private:
    BluetoothHfpAgServer::impl *pimpl_;
};

class BluetoothHfpAgServer::impl::HfpAgServerObserver : public HfpAgServiceObserver {
public:
    explicit HfpAgServerObserver(BluetoothHfpAgServer::impl *pimpl) : pimpl_(pimpl) {};
    ~HfpAgServerObserver() override = default;

    void OnConnectionStateChanged(const RawAddress& device, int state, int cause) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOG_COMM_INFO("OnConnectionStateChanged: %{public}s, state:%{public}d, cause:%{public}d",
            GET_ENCRYPT_ADDR(device), state, cause);
        if (state == static_cast<int>(BTConnectState::CONNECTED) ||
            state == static_cast<int>(BTConnectState::DISCONNECTED)) {
            HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::BT_SERVICE, "HFP_CONNECTED_STATE",
                OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC, "STATE", state);
        }
        observers_->ForEach([this, device, state, cause](IBluetoothHfpAgObserver* observer) {
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

    void OnScoStateChanged(const RawAddress& device, int state, int reason) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOG_COMM_INFO("device:%{public}s, state:%{public}d, reason:%{public}d",
            GET_ENCRYPT_ADDR(device), state, reason);
        observers_->ForEach([this, device, state, reason](IBluetoothHfpAgObserver* observer) {
            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnScoStateChanged(randomAddr, state, reason);
            }
        });
    }

    void OnActiveDeviceChanged(const RawAddress& device) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device:%{public}s", GET_ENCRYPT_ADDR(device));
        observers_->ForEach([this, device](IBluetoothHfpAgObserver* observer) {
            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnActiveDeviceChanged(randomAddr);
            }
        });
    }

    void OnHfEnhancedDriverSafetyChanged(const RawAddress& device, int indValue) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device:%{public}s, indValue:%{public}d", GET_ENCRYPT_ADDR(device), indValue);
        observers_->ForEach([this, device, indValue](IBluetoothHfpAgObserver* observer) {
            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnHfEnhancedDriverSafetyChanged(randomAddr, indValue);
            }
        });
    }

    void OnHfpStackChanged(const RawAddress& device, int action) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device:%{public}s, action:%{public}d", GET_ENCRYPT_ADDR(device), action);
        observers_->ForEach([this, device, action](IBluetoothHfpAgObserver* observer) {
            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, device, randomAddr)) {
                observer->OnHfpStackChanged(randomAddr, action);
            }
        });
    }

    void OnVirtualDeviceChanged(int32_t action, std::string address) override
    {
        HITRACE_METER(BT_TRACE_TAG);
        HILOGI("device:%{public}s, action:%{public}d", GetEncryptAddr(address).c_str(), action);
        observers_->ForEach([this, address, action](IBluetoothHfpAgObserver *observer) {
            RawAddress randomAddr;
            uint64_t tokenId = this->pimpl_->appContainer_->GetTokenId(observer->AsObject());
            if (BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(tokenId, RawAddress(address),
                                                                              randomAddr)) {
                std::string strAddress = randomAddr.GetAddress();
                observer->OnVirtualDeviceChanged(action, strAddress);
            }
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothHfpAgObserver>* observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothHfpAgObserver>* observers_;
    BluetoothHfpAgServer::impl *pimpl_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HfpAgServerObserver);
};

BluetoothHfpAgServer::impl::impl()
{
    appContainer_ = std::make_shared<BluetoothObserverApplicationContainer>();
    appContainer_->Init();
}

BluetoothHfpAgServer::BluetoothHfpAgServer()
{
    pimpl = std::make_unique<impl>();
    pimpl->observerImp_ = std::make_unique<impl::HfpAgServerObserver>(pimpl.get());
    pimpl->observerImp_->SetObserver(&(pimpl->observers_));
    pimpl->HfpAgSystemObserver_ = std::make_unique<impl::HfpAgSystemObserver>(pimpl.get());
    IAdapterManager::GetInstance()->RegisterSystemStateObserver(*(pimpl->HfpAgSystemObserver_));

    IProfileManager* serviceMgr = IProfileManager::GetInstance();
    if (serviceMgr != nullptr) {
        pimpl->HfpAgService_ = (IProfileHfpAg*)serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG);
        if (pimpl->HfpAgService_ != nullptr) {
            pimpl->HfpAgService_->RegisterObserver(*pimpl->observerImp_);
        }
    }
}

BluetoothHfpAgServer::~BluetoothHfpAgServer()
{
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->HfpAgSystemObserver_));
    if (pimpl->HfpAgService_ != nullptr) {
        pimpl->HfpAgService_->DeregisterObserver(*pimpl->observerImp_);
    }
}

int32_t BluetoothHfpAgServer::GetConnectDevices(std::vector<BluetoothRawAddress> &devices)
{
    std::list<RawAddress> deviceList;
    if (pimpl->HfpAgService_  != nullptr) {
        deviceList = pimpl->HfpAgService_->GetConnectDevices();
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
    RawAddress randomAddr;
    bool isUseRealAddr = PermissionManager::IsUseRealAddr();
    for (RawAddress device : deviceList) {
        if (isUseRealAddr) {
            devices.emplace_back(BluetoothRawAddress(device));
        } else {
            int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(device, randomAddr);
            if (ret == OHOS::bluetooth::RET_SUCCESS) {
                devices.emplace_back(BluetoothRawAddress(randomAddr));
            }
        }
    }
    return BT_NO_ERROR;
}

int BluetoothHfpAgServer::GetDevicesByStates(const std::vector<int> &states, std::vector<BluetoothRawAddress> &devices)
{
    std::vector<int> tmpStates;
    for (int32_t state : states) {
        HILOGI("state = %{public}d", state);
        tmpStates.push_back((int)state);
    }
    std::vector<RawAddress> rawDevices;
    if (pimpl->HfpAgService_ != nullptr) {
        rawDevices = pimpl->HfpAgService_->GetDevicesByStates(tmpStates);
    } else {
        return BT_FAILURE;
    }
    BluetoothDeviceManager::GetInstance()->ConvertToRandomAddress(rawDevices, devices);
    return BT_SUCCESS;
}

int32_t BluetoothHfpAgServer::GetDeviceState(const BluetoothRawAddress &device, int32_t &state)
{
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->GetRealUsedAddress(addr, realAddr)) {
        HILOGE("address not exists");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpAgService_) {
        state = pimpl->HfpAgService_->GetDeviceState(realAddr);
        HILOGD("state:%{public}d", state);
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHfpAgServer::Connect(const BluetoothRawAddress &device)
{
    HILOGI("target device:%{public}s()", GET_ENCRYPT_ADDR(device));
    if (PermissionManager::GetApiVersion() >= API_VERSION_10) {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::IsSystemHap(),
            BT_ERR_SYSTEM_PERMISSION_FAILED, "[PERMISSION] system hap needed.");
    }
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpAgService_ != nullptr) {
        OHOS::Bluetooth::BluetoothHiTrace::BluetoothStartAsyncTrace("HFP_AG_CONNECT", 1);
        int32_t result = pimpl->HfpAgService_->Connect(realAddr);
        OHOS::Bluetooth::BluetoothHiTrace::BluetoothFinishAsyncTrace("HFP_AG_CONNECT", 1);
        return result;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHfpAgServer::Disconnect(const BluetoothRawAddress &device)
{
    HILOGI("target device:%{public}s()", GET_ENCRYPT_ADDR(device));
    if (PermissionManager::GetApiVersion() >= API_VERSION_10) {
        CHECK_AND_RETURN_LOG_RET(PermissionManager::IsSystemHap(),
            BT_ERR_SYSTEM_PERMISSION_FAILED, "[PERMISSION] system hap needed.");
    }
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_->Disconnect(realAddr);
    }
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothHfpAgServer::GetScoState(const BluetoothRawAddress &device)
{
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_->GetScoState(realAddr);
    }
    return BT_FAILURE;
}

int32_t BluetoothHfpAgServer::ConnectSco(uint8_t callType)
{
    HILOGI("callType: %{public}d", callType);
    CHECK_AND_RETURN_LOG_RET((pimpl->HfpAgService_ != nullptr), BT_ERR_INTERNAL_ERROR,
        "hfp ag service not exist.");
    return pimpl->HfpAgService_->ConnectSco(callType);
}

int32_t BluetoothHfpAgServer::DisconnectSco(uint8_t callType)
{
    HILOGI("callType: %{public}d", callType);
    CHECK_AND_RETURN_LOG_RET((pimpl->HfpAgService_ != nullptr), BT_ERR_INTERNAL_ERROR,
        "hfp ag service not exist.");
    return pimpl->HfpAgService_->DisconnectSco(callType);
}

bool BluetoothHfpAgServer::ConnectSco()
{
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_->ConnectSco();
    }
    return false;
}

bool BluetoothHfpAgServer::DisconnectSco()
{
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_->DisconnectSco();
    }
    return false;
}

void BluetoothHfpAgServer::PhoneStateChanged(BluetoothPhoneState &phoneState)
{
    HILOGI("numActive:%{public}d, numHeld:%{public}d, callState:%{public}d, type:%{public}d",
        phoneState.GetActiveNum(), phoneState.GetHeldNum(), phoneState.GetCallState(), phoneState.GetCallType());
    if (pimpl->HfpAgService_ != nullptr) {
        pimpl->HfpAgService_->PhoneStateChanged(phoneState);
    }
}

void BluetoothHfpAgServer::ClccResponse(int index, int direction, int status, int mode, bool mpty,
    const std::string &number, int type)
{
    HILOGI("index:%{public}d, direction:%{public}d, status:%{public}d, mode:%{public}d, mpty:%{public}d,"
        "type:%{public}d", index, direction, status, mode, mpty, type);
    if (pimpl->HfpAgService_ != nullptr) {
        pimpl->HfpAgService_->ClccResponse(index, direction, status, mode, mpty, number, type);
    }
}

bool BluetoothHfpAgServer::OpenVoiceRecognition(const BluetoothRawAddress &device)
{
    HILOGI("target device:%{public}s()", GET_ENCRYPT_ADDR(device));
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_->OpenVoiceRecognition(realAddr);
    }
    return false;
}

bool BluetoothHfpAgServer::CloseVoiceRecognition(const BluetoothRawAddress &device)
{
    HILOGI("target device:%{public}s()", GET_ENCRYPT_ADDR(device));
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_->CloseVoiceRecognition(realAddr);
    }
    return false;
}

int32_t BluetoothHfpAgServer::IsAudioConnected(bool &isAudioOn)
{
    CHECK_AND_RETURN_LOG_RET(pimpl->HfpAgService_ != nullptr, BT_ERR_INTERNAL_ERROR, "hfp service is null.");
    isAudioOn = pimpl->HfpAgService_->IsAudioConnected();
    return BT_NO_ERROR;
}

bool BluetoothHfpAgServer::SetActiveDevice(const BluetoothRawAddress &device)
{
    HILOGI("target device:%{public}s", GET_ENCRYPT_ADDR(device));
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpAgService_) {
        std::string currentDevice = pimpl->HfpAgService_->GetActiveDevice();
        bool isEmpty = currentDevice.empty() && device.GetAddress().empty();
        return isEmpty || pimpl->HfpAgService_->SetActiveDevice(realAddr);
    }
    return false;
}

bool BluetoothHfpAgServer::IntoMock(const BluetoothRawAddress &device, int state)
{
    return false;
}

bool BluetoothHfpAgServer::SendNoCarrier(const BluetoothRawAddress &device)
{
    return false;
}

std::string BluetoothHfpAgServer::GetActiveDevice()
{
    std::string dev = "";
    if (pimpl->HfpAgService_ != nullptr) {
        dev = pimpl->HfpAgService_->GetActiveDevice();
    }

    bool isUseRealAddr = PermissionManager::IsUseRealAddr();
    if (isUseRealAddr) {
        return dev;
    } else {
        RawAddress randomAddr;
        int ret = BluetoothDeviceManager::GetInstance()->GetDeviceRandomAddr(RawAddress(dev), randomAddr);
        if (ret == OHOS::bluetooth::RET_SUCCESS) {
            return randomAddr.GetAddress();
        }
    }
    HILOGI("active dev:%{public}s()", GetEncryptAddr(dev).c_str());
    return INVALID_MAC_ADDRESS;
}

void BluetoothHfpAgServer::RegisterObserver(const sptr<IBluetoothHfpAgObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return;
    }
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return;
    }
    if (pimpl->observers_.Size() > MAX_MAP_SIZE || pimpl->appContainer_->Size() > MAX_MAP_SIZE) {
        HILOGE("observers_ or appContainer_ too much");
        return;
    }
    pimpl->observers_.Register(observer);
    pimpl->appContainer_->AddObject(observer->AsObject());
}

void BluetoothHfpAgServer::DeregisterObserver(const sptr<IBluetoothHfpAgObserver> &observer)
{
    if (observer == nullptr) {
        HILOGE("observer is null");
        return;
    }
    if (pimpl == nullptr) {
        HILOGE("pimpl is null");
        return;
    }
    pimpl->observers_.Deregister(observer);
    pimpl->appContainer_->RemoveRemoteObject(observer->AsObject());
}

int BluetoothHfpAgServer::SetConnectStrategy(const BluetoothRawAddress &device, int strategy)
{
    HILOGI("addr: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    int res = BT_ERR_INTERNAL_ERROR;
    if (pimpl->HfpAgService_) {
        res = pimpl->HfpAgService_->SetConnectStrategy(realAddr, strategy);
    }
    return res;
}

int BluetoothHfpAgServer::GetConnectStrategy(const BluetoothRawAddress &device, int &strategy)
{
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->HfpAgService_) {
        strategy = pimpl->HfpAgService_->GetConnectStrategy(realAddr);
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int BluetoothHfpAgServer::IsInbandRingingEnabled(bool &isEnabled)
{
    CHECK_AND_RETURN_LOG_RET(pimpl->HfpAgService_ != nullptr, BT_ERR_INTERNAL_ERROR, "hfp service is null.");
    isEnabled = pimpl->HfpAgService_->IsInbandRingingEnabled();
    return BT_NO_ERROR;
}

void BluetoothHfpAgServer::CallDetailsChanged(int callId, int callState)
{
    HILOGI("callId:%{public}d, callState:%{public}d", callId, callState);
    CHECK_AND_RETURN_LOG(pimpl->HfpAgService_ != nullptr, "hfp service is null.");
    pimpl->HfpAgService_->CallDetailsChanged(callId, callState);
}

void BluetoothHfpAgServer::EnableBtCallLog(bool state)
{
    HILOGI("log state:%{public}d", state);
    CHECK_AND_RETURN_LOG(pimpl->HfpAgService_ != nullptr, "hfp service is null.");
    pimpl->HfpAgService_->EnableBtCallLog(state);
}

int BluetoothHfpAgServer::IsHfpFeatureSupported(const BluetoothRawAddress &device, bool &isSupported, int type)
{
    HILOGI("addr: %{public}s, type:%{public}d", GET_ENCRYPT_ADDR(device), type);
    RawAddress addr(device.GetAddress());
    RawAddress realAddr;
    if (!BluetoothDeviceManager::GetInstance()->ConvertToRealAddress(addr, realAddr)) {
        return BT_ERR_INTERNAL_ERROR;
    }
    CHECK_AND_RETURN_LOG_RET(pimpl->HfpAgService_ != nullptr, BT_ERR_INTERNAL_ERROR, "hfp service is null.");
    if (type == static_cast<int>(bluetooth::HfpFeatureType::VGS)) {
        return pimpl->HfpAgService_->IsVgsSupported(realAddr, isSupported);
    } else if (type == static_cast<int>(bluetooth::HfpFeatureType::VOICE_RECOGNITION)) {
        return pimpl->HfpAgService_->IsVoiceRecognitionSupported(realAddr, isSupported);
    }
    return BT_ERR_INVALID_PARAM;
}

void BluetoothHfpAgServer::GetVirtualDeviceList(std::vector<std::string> &devices)
{
    HILOGI("intro");
    if (pimpl == nullptr || pimpl->HfpAgService_ == nullptr) {
        HILOGE("pimpl or HfpAgService_ is null");
        return;
    }
    
    pimpl->HfpAgService_->GetVirtualDeviceList(devices);
}

void BluetoothHfpAgServer::UpdateVirtualDevice(int32_t action, const std::string &address)
{
    HILOGI("intro");
    if (pimpl == nullptr || pimpl->HfpAgService_ == nullptr) {
        HILOGE("pimpl or HfpAgService_ is null");
        return;
    }
    
    pimpl->HfpAgService_->UpdateVirtualDevice(action, address);
}

int BluetoothHfpAgServer::GetCurrentCallType(int &callType)
{
    if (pimpl->HfpAgService_ != nullptr) {
        pimpl->HfpAgService_->GetCurrentCallType(callType);
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

}  // namespace Bluetooth
}  // namespace OHOS
