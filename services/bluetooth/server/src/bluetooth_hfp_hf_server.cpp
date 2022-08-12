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

#include <vector>
#include "bluetooth_def.h"
#include "bluetooth_hfp_hf_server.h"
#include "bluetooth_log.h"
#include "interface_profile_hfp_hf.h"
#include "interface_profile_manager.h"
#include "interface_profile.h"
#include "interface_adapter_manager.h"
#include "permission_utils.h"
#include "remote_observer_list.h"

using namespace bluetooth;

namespace OHOS {
namespace Bluetooth {
class HfpHfServerObserver : public HfpHfServiceObserver {
public:
    HfpHfServerObserver() = default;
    ~HfpHfServerObserver() override = default;

    void OnConnectionStateChanged(const RawAddress& device, int state) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, state](IBluetoothHfpHfObserver* observer) {
            observer->OnConnectionStateChanged(device, state);
        });
    }

    void OnScoStateChanged(const RawAddress& device,  int state) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, state](IBluetoothHfpHfObserver* observer) {
            observer->OnScoStateChanged(device, state);
        });
    }

    void OnCallChanged(const RawAddress& device, const HandsFreeUnitCalls& call) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, call](IBluetoothHfpHfObserver* observer) {
            observer->OnCallChanged(device, call);
        });
    }

    void OnSignalStrengthChanged(const RawAddress& device,  int signal) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, signal](IBluetoothHfpHfObserver* observer) {
            observer->OnSignalStrengthChanged(device, signal);
        });
    }

    void OnRegistrationStatusChanged(const RawAddress& device,  int status) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, status](IBluetoothHfpHfObserver* observer) {
            observer->OnRegistrationStatusChanged(device, status);
        });
    }

    void OnRoamingStatusChanged(const RawAddress& device,  int status) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, status](IBluetoothHfpHfObserver* observer) {
            observer->OnRoamingStatusChanged(device, status);
        });
    }

    void OnOperatorSelectionChanged(const RawAddress& device, const std::string& name) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, name](IBluetoothHfpHfObserver* observer) {
            observer->OnOperatorSelectionChanged(device, name);
        });
    }

    void OnSubscriberNumberChanged(const RawAddress& device, const std::string& number) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, number](IBluetoothHfpHfObserver* observer) {
            observer->OnSubscriberNumberChanged(device, number);
        });
    }

    void OnVoiceRecognitionStatusChanged(const RawAddress& device, int status) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, status](IBluetoothHfpHfObserver* observer) {
            observer->OnVoiceRecognitionStatusChanged(device, status);
        });
    }

    void OnInBandRingToneChanged(const RawAddress& device, int status) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    }

    void SetObserver(RemoteObserverList<IBluetoothHfpHfObserver>* observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothHfpHfObserver>* observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HfpHfServerObserver);
};

struct BluetoothHfpHfServer::impl {
    RemoteObserverList<IBluetoothHfpHfObserver> observers_;
    std::unique_ptr<HfpHfServerObserver> observerImp_{std::make_unique<HfpHfServerObserver>()};
    IProfileHfpHf* HfpHfService_ = nullptr;  

    class HfpHfSystemObserver : public ISystemStateObserver {
    public:
        explicit HfpHfSystemObserver(BluetoothHfpHfServer::impl* pimpl) : pimpl_(pimpl) {};
        void OnSystemStateChange(const BTSystemState state) override
        {
            HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
            IProfileManager* serviceMgr = IProfileManager::GetInstance();
            switch (state) {
            case BTSystemState::ON:
                if (serviceMgr != nullptr) {
                    pimpl_->HfpHfService_ = (IProfileHfpHf*)serviceMgr->GetProfileService(PROFILE_NAME_HFP_HF);
                    if (pimpl_->HfpHfService_ != nullptr) {
                        pimpl_->HfpHfService_->RegisterObserver(*pimpl_->observerImp_);
                    }
                }
                break;
            case BTSystemState::OFF:
                if (serviceMgr != nullptr) {
                    pimpl_->HfpHfService_ = (IProfileHfpHf*)serviceMgr->GetProfileService(PROFILE_NAME_HFP_HF);
                    if (pimpl_->HfpHfService_ != nullptr) {
                        pimpl_->HfpHfService_->DeregisterObserver(*pimpl_->observerImp_);
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

    std::unique_ptr<HfpHfSystemObserver> HfpHfSystemObserver_;
};

BluetoothHfpHfServer::BluetoothHfpHfServer()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    pimpl = std::make_unique<impl>();
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
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->HfpHfSystemObserver_));
    if (pimpl->HfpHfService_ != nullptr) {
        pimpl->HfpHfService_->DeregisterObserver(*pimpl->observerImp_);
    }
}

bool BluetoothHfpHfServer::ConnectSco(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->ConnectSco(addr);
    }
    return false;
}

bool BluetoothHfpHfServer::DisconnectSco(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->DisconnectSco(addr);
    }
    return false;
}

int BluetoothHfpHfServer::GetDevicesByStates(const std::vector<int> &states, 
    std::vector<BluetoothRawAddress> &devices) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (PermissionUtils::VerifyUseBluetoothPermission() == PERMISSION_DENIED) {
        HILOGE("GetDevicesByStates() false, check permission failed");
        return BT_FAILURE;
    }
    std::vector<int> tmpStates;
    for (int32_t state : states) {
        HILOGD("state = %{public}d", state);
        tmpStates.push_back((int)state);
    }
    std::vector<RawAddress> rawDevices;
     
    if (pimpl->HfpHfService_ != nullptr) {
        rawDevices = pimpl->HfpHfService_->GetDevicesByStates(tmpStates);
    } else {
        return BT_FAILURE;
    }
    for (RawAddress device : rawDevices) {
        devices.push_back(BluetoothRawAddress(device));
    }
    return BT_SUCCESS;
}

int BluetoothHfpHfServer::GetDeviceState(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (PermissionUtils::VerifyUseBluetoothPermission() == PERMISSION_DENIED) {
        HILOGE("GetDeviceState() false, check permission failed");
        return BT_FAILURE;
    }
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->GetDeviceState(addr);
    }
    return BT_FAILURE;
}

int BluetoothHfpHfServer::GetScoState(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->GetScoState(addr);
    }
    return BT_FAILURE;
}

bool BluetoothHfpHfServer::SendDTMFTone(const BluetoothRawAddress &device, uint8_t code) {
    HILOGD("[%{public}s]: %{public}s(): Enter, code = %{public}d", __FILE__, __FUNCTION__, code);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->SendDTMFTone(addr, code);
    }
    return false;
}

int BluetoothHfpHfServer::Connect(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->Connect(addr);
    }
    return BT_FAILURE;
}

int BluetoothHfpHfServer::Disconnect(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->Disconnect(addr);
    }
    return BT_FAILURE;
}

bool BluetoothHfpHfServer::OpenVoiceRecognition(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->OpenVoiceRecognition(addr);
    }
    return false;
}

bool BluetoothHfpHfServer::CloseVoiceRecognition(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->CloseVoiceRecognition(addr);
    }
    return false;
}

int BluetoothHfpHfServer::GetCurrentCallList(const BluetoothRawAddress &device, 
    std::vector<BluetoothHfpHfCall> &calls) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::vector<HandsFreeUnitCalls> callVector;
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        callVector = pimpl->HfpHfService_->GetCurrentCallList(addr);
    }
    for (HandsFreeUnitCalls call : callVector) {
        calls.push_back(BluetoothHfpHfCall(call));
    }
    return BT_FAILURE;
}

bool BluetoothHfpHfServer::AcceptIncomingCall(const BluetoothRawAddress &device, int flag) {
    HILOGD("[%{public}s]: %{public}s(): Enter, flag = %{public}d", __FILE__, __FUNCTION__, flag);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->AcceptIncomingCall(addr, (int)flag);
    }
    return false;
}

bool BluetoothHfpHfServer::HoldActiveCall(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->HoldActiveCall(addr);
    }
    return false;
}

bool BluetoothHfpHfServer::RejectIncomingCall(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->RejectIncomingCall(addr);
    }
    return false;
}

bool BluetoothHfpHfServer::HandleIncomingCall(const BluetoothRawAddress &device, int flag)
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->HandleIncomingCall(addr, flag);
    }
    return false;
}

bool BluetoothHfpHfServer::DialLastNumber(const BluetoothRawAddress &device)
{
    HILOGI("Enter!");
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->DialLastNumber(addr);
    }
    return false;
}

bool BluetoothHfpHfServer::DialMemory(const BluetoothRawAddress &device, int index)
{
    HILOGI("Enter!");
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->DialMemory(addr, index);
    }
    return false;
}

bool BluetoothHfpHfServer::HandleMultiCall(const BluetoothRawAddress &device, int flag, int index)
{
    HILOGI("Enter!");
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->HandleMultiCall(addr, flag, index);
    }
    return false;
}

bool BluetoothHfpHfServer::FinishActiveCall(const BluetoothRawAddress &device, const BluetoothHfpHfCall &call) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        return pimpl->HfpHfService_->FinishActiveCall(addr, call);
    }
    return false;
}

int BluetoothHfpHfServer::StartDial(const BluetoothRawAddress &device, const std::string &number, 
    BluetoothHfpHfCall &call) {
    HILOGD("[%{public}s]: %{public}s(): Enter, number = %{public}s", __FILE__, __FUNCTION__, number.c_str());
    std::optional<HandsFreeUnitCalls> ret;
    HandsFreeUnitCalls calls;
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpHfService_ != nullptr) {
        ret = pimpl->HfpHfService_->StartDial(addr, number);
    }
    if (ret == std::nullopt) {
        call = calls;
        return BT_FAILURE;
    } else {
        call = *ret;
        return BT_SUCCESS;
    }
}

void BluetoothHfpHfServer::RegisterObserver(const sptr<IBluetoothHfpHfObserver> &observer) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    pimpl->observers_.Register(observer);
}

void BluetoothHfpHfServer::DeregisterObserver(const sptr<IBluetoothHfpHfObserver> &observer) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    pimpl->observers_.Deregister(observer);
}


}  // namespace Bluetooth
}  // namespace OHOS
