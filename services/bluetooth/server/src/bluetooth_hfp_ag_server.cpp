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
#include <list>

#include "bluetooth_def.h"
#include "bluetooth_hfp_ag_server.h"
#include "bluetooth_log.h"
#include "interface_profile_hfp_ag.h"
#include "interface_profile_manager.h"
#include "interface_profile.h"
#include "interface_adapter_manager.h"
#include "remote_observer_list.h"


namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;

class HfpAgServerObserver : public HfpAgServiceObserver {
public:
    HfpAgServerObserver() = default;
    ~HfpAgServerObserver() override = default;

    void OnConnectionStateChanged(const RawAddress& device, int state) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, state](IBluetoothHfpAgObserver* observer) {
            observer->OnConnectionStateChanged(device, state);
        });
    }

    void OnScoStateChanged(const RawAddress& device, int state) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, state](IBluetoothHfpAgObserver* observer) {
            observer->OnScoStateChanged(device, state);
        });
    }

    void OnActiveDeviceChanged(const RawAddress& device) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device](IBluetoothHfpAgObserver* observer) {
            observer->OnActiveDeviceChanged(device);
        });
    }

    void OnHfEnhancedDriverSafetyChanged(const RawAddress& device, int indValue) override
    {
        HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
        observers_->ForEach([device, indValue](IBluetoothHfpAgObserver* observer) {
            observer->OnHfEnhancedDriverSafetyChanged(device, indValue);
        });
    }

    void SetObserver(RemoteObserverList<IBluetoothHfpAgObserver>* observers)
    {
        observers_ = observers;
    }

private:
    RemoteObserverList<IBluetoothHfpAgObserver>* observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HfpAgServerObserver);
};

struct BluetoothHfpAgServer::impl {
    RemoteObserverList<IBluetoothHfpAgObserver> observers_;
    std::unique_ptr<HfpAgServerObserver> observerImp_{std::make_unique<HfpAgServerObserver>()};
    IProfileHfpAg* HfpAgService_ = nullptr;  

    class HfpAgSystemObserver : public ISystemStateObserver {
    public:
        explicit HfpAgSystemObserver(BluetoothHfpAgServer::impl* pimpl) : pimpl_(pimpl) {};
        void OnSystemStateChange(const BTSystemState state) override
        {
            HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
            IProfileManager* serviceMgr = IProfileManager::GetInstance();
            switch (state) {
            case BTSystemState::ON:
                if (serviceMgr != nullptr) {
                    pimpl_->HfpAgService_ = (IProfileHfpAg*)serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG);
                    if (pimpl_->HfpAgService_ != nullptr) {
                        pimpl_->HfpAgService_->RegisterObserver(*pimpl_->observerImp_);
                    }
                }
                break;
            case BTSystemState::OFF:
                if (serviceMgr != nullptr) {
                    pimpl_->HfpAgService_ = (IProfileHfpAg*)serviceMgr->GetProfileService(PROFILE_NAME_HFP_AG);
                    if (pimpl_->HfpAgService_ != nullptr) {
                        pimpl_->HfpAgService_->DeregisterObserver(*pimpl_->observerImp_);
                    }
                }
                pimpl_->HfpAgService_ = nullptr;
                break;
            default:
                break;
            }
        };

    private:
        BluetoothHfpAgServer::impl* pimpl_;
    };

    std::unique_ptr<HfpAgSystemObserver> HfpAgSystemObserver_;
};


BluetoothHfpAgServer::BluetoothHfpAgServer()
{
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    pimpl = std::make_unique<impl>();
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
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    IAdapterManager::GetInstance()->DeregisterSystemStateObserver(*(pimpl->HfpAgSystemObserver_));
    if (pimpl->HfpAgService_ != nullptr) {
        pimpl->HfpAgService_->DeregisterObserver(*pimpl->observerImp_);
    }
}

int BluetoothHfpAgServer::GetConnectDevices(std::vector<BluetoothRawAddress> &devices) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::list<RawAddress> deviceList;
    if (pimpl->HfpAgService_  != nullptr) {
        deviceList = pimpl->HfpAgService_ ->GetConnectDevices();
    } else {
        return BT_FAILURE;
    }
    for (RawAddress device : deviceList) {
        devices.push_back(BluetoothRawAddress(device));
    }
    return BT_SUCCESS;
}

int BluetoothHfpAgServer::GetDevicesByStates(const std::vector<int> &states, 
    std::vector<BluetoothRawAddress> &devices) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    std::vector<int> tmpStates;
    for (int32_t state : states) {
        
        HILOGD("state = %{public}d", state);
        tmpStates.push_back((int)state);
    }
    std::vector<RawAddress> rawDevices;
    if (pimpl->HfpAgService_ != nullptr) {
        rawDevices = pimpl->HfpAgService_ ->GetDevicesByStates(tmpStates);
    } else {
        return BT_FAILURE;
    }
    for (RawAddress device : rawDevices) {
        devices.push_back(BluetoothRawAddress(device));
    }
    return BT_SUCCESS;
}
int BluetoothHfpAgServer::GetDeviceState(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpAgService_ ) {
        return pimpl->HfpAgService_ ->GetDeviceState(addr);
    } else {
        return BT_FAILURE;
    }
    return BT_SUCCESS;
}

int BluetoothHfpAgServer::Connect(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_->Connect(addr);
    }
    return BT_FAILURE;
}

int BluetoothHfpAgServer::Disconnect(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
     RawAddress addr(device.GetAddress());
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_ ->Disconnect(addr);
    }
    return BT_FAILURE;
}

int BluetoothHfpAgServer::GetScoState(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_ ->GetScoState(addr);
    }
    return BT_FAILURE;    
}

bool BluetoothHfpAgServer::ConnectSco() {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_ ->ConnectSco();
    }
    return false;
}

bool BluetoothHfpAgServer::DisconnectSco() {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_ ->DisconnectSco();
    }
    return false;    
}

void BluetoothHfpAgServer::PhoneStateChanged(int numActive, int numHeld, int callState, const std::string &number, 
    int type, const std::string &name) {
    HILOGD("[%{public}s]: %{public}s(): Enter, numActive = %{public}d, numHeld = %{public}d, callState = %{public}d, number = %{public}s, type = %{public}d, name = %{public}s", 
           __FILE__, __FUNCTION__, numActive, numHeld,callState, number.c_str(), type, name.c_str());
    if (pimpl->HfpAgService_ != nullptr) {
        pimpl->HfpAgService_ ->PhoneStateChanged(numActive, numHeld, callState, number, type, name);
    }

}

void BluetoothHfpAgServer::ClccResponse(int index, int direction, int status, int mode, bool mpty, 
    const std::string &number, int type) {
    HILOGD("[%{public}s]: %{public}s(): Enter, index = %{public}d, direction = %{public}d, status = %{public}d,mode = %{public}d, mpty = %{public}d, number = %{public}s, type = %{public}d",
           __FILE__, __FUNCTION__, index, direction, status, mode, mpty, number.c_str(), type);
    if (pimpl->HfpAgService_ != nullptr) {
        pimpl->HfpAgService_ ->ClccResponse(index, direction, status, mode, mpty, number, type);
    }
}

bool BluetoothHfpAgServer::OpenVoiceRecognition(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_ ->OpenVoiceRecognition(addr);
    }
    return false;
}

bool BluetoothHfpAgServer::CloseVoiceRecognition(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
     RawAddress addr(device.GetAddress());
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_ ->CloseVoiceRecognition(addr);
    }
    return false;
}

bool BluetoothHfpAgServer::SetActiveDevice(const BluetoothRawAddress &device) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    RawAddress addr(device.GetAddress());
    if (pimpl->HfpAgService_ ) {
        return pimpl->HfpAgService_ ->SetActiveDevice(addr);
    }
    return false;
}

std::string BluetoothHfpAgServer::GetActiveDevice() {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    if (pimpl->HfpAgService_ != nullptr) {
        return pimpl->HfpAgService_ ->GetActiveDevice();
    }
    return "";
}

void BluetoothHfpAgServer::RegisterObserver(const sptr<IBluetoothHfpAgObserver> &observer) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    pimpl->observers_.Register(observer);
}

void BluetoothHfpAgServer::DeregisterObserver(const sptr<IBluetoothHfpAgObserver> &observer) {
    HILOGD("[%{public}s]: %{public}s(): Enter!", __FILE__, __FUNCTION__);
    pimpl->observers_.Deregister(observer);
}

}  // namespace Bluetooth
}  // namespace OHOS
