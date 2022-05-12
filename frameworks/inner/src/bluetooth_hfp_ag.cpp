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

#include "bluetooth_hfp_ag.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "i_bluetooth_hfp_ag.h"
#include "bluetooth_hfp_ag_observer_stub.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
class AgServiceObserver : public BluetoothHfpAgObserverStub {
public:
    explicit AgServiceObserver(BluetoothObserverList<HandsFreeAudioGatewayObserver> &observers) : observers_(observers)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    }
    ~AgServiceObserver() override 
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    };

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state);
        });
    }

    void OnScoStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnScoStateChanged(remoteDevice, state);
        });
    }

    void OnActiveDeviceChanged(const BluetoothRawAddress &device) override
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnActiveDeviceChanged(remoteDevice);
        });
    }

    void OnHfEnhancedDriverSafetyChanged(
        const BluetoothRawAddress &device, int32_t indValue) override
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, indValue](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnHfEnhancedDriverSafetyChanged(remoteDevice, indValue);
        });
    }

private:
    BluetoothObserverList<HandsFreeAudioGatewayObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AgServiceObserver);
};

std::string HfpAgServiceName = "bluetooth-hfp-ag-server";

struct HandsFreeAudioGateway::impl {
    impl();
    ~impl();
    std::vector<BluetoothRemoteDevice> GetConnectedDevices()
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        std::vector<BluetoothRemoteDevice> devices;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            std::vector<BluetoothRawAddress> ori;
            proxy_->GetConnectDevices(ori);
            for (auto it = ori.begin(); it != ori.end(); it++) {
                devices.push_back(BluetoothRemoteDevice(it->GetAddress(), 0));
            }
        }
        return devices;
    }

    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        std::vector<BluetoothRemoteDevice> remoteDevices;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            std::vector<BluetoothRawAddress> rawDevices;
            std::vector<int32_t> tmpstates;
            for (int state : states) {
                int32_t value = (int32_t)state;
                tmpstates.push_back(value);
            }
            proxy_->GetDevicesByStates(tmpstates, rawDevices);
            for (BluetoothRawAddress rawDevice : rawDevices) {
                BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), 0);
                remoteDevices.push_back(remoteDevice);
            }
        }
        return remoteDevices;
    }

    int GetDeviceState(const BluetoothRemoteDevice &device)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);

        if (proxy_ == nullptr)
        {
            HILOGI("proxy_ == nullptr");
            printf("proxy_ == nullptr");
        }

        if (!IS_BT_ENABLED())
        {
            HILOGI("Not IS_BT_ENABLED");
            printf("Not IS_BT_ENABLED");
        }

        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return HFP_AG_SLC_STATE_DISCONNECTED;
    }

    bool Connect(const BluetoothRemoteDevice &device)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);

        if (proxy_ == nullptr)
        {
            HILOGI("proxy_ == nullptr");
            printf("proxy_ == nullptr");
        }

        if (!IS_BT_ENABLED())
        {
            HILOGI("Not IS_BT_ENABLED");
            printf("Not IS_BT_ENABLED");
        }

        if (proxy_ != nullptr && IS_BT_ENABLED() && !BluetoothHost::GetDefaultHost().IsBtDiscovering() &&
            device.IsValidBluetoothRemoteDevice()) {
            if (proxy_->Connect(BluetoothRawAddress(device.GetDeviceAddr())) == NO_ERROR) {
                return true;
            }
        }
        return false;
    }

    bool Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);

        if (proxy_ == nullptr)
        {
            HILOGI("proxy_ == nullptr");
            printf("proxy_ == nullptr");
        }

        if (!IS_BT_ENABLED())
        {
            HILOGI("Not IS_BT_ENABLED");
            printf("Not IS_BT_ENABLED");
        }

        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            if (proxy_->Disconnect(BluetoothRawAddress(device.GetDeviceAddr())) == NO_ERROR) {
                return true;
            }
        }
        return false;
    }

    int GetScoState(const BluetoothRemoteDevice &device)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->GetScoState(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return HFP_AG_SCO_STATE_DISCONNECTED;
    }

    bool ConnectSco()
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            return proxy_->ConnectSco();
        }
        return false;
    }

    bool DisconnectSco()
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            return proxy_->DisconnectSco();
        }
        return false;
    }

    void PhoneStateChanged(
        int numActive, int numHeld, int callState, const std::string &number, int type, const std::string &name)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->PhoneStateChanged(numActive, numHeld, callState, number, type, name);
        }
    }

    void ClccResponse(int index, int direction, int status, int mode, bool mpty, std::string number, int type)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            proxy_->ClccResponse(index, direction, status, mode, mpty, number, type);
        }
    }

    bool OpenVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->OpenVoiceRecognition(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool CloseVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->CloseVoiceRecognition(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool SetActiveDevice(const BluetoothRemoteDevice &device)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->SetActiveDevice(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    BluetoothRemoteDevice GetActiveDevice()
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        BluetoothRemoteDevice device;
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            std::string address = proxy_->GetActiveDevice();
            BluetoothRemoteDevice remoteDevice(address, 0);
            device = remoteDevice;
        }
        return device;
    }

    void RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
    {
        HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
        observers_.Deregister(observer);
        HILOGD("[hfpag]: %{public}s(): end!", __FUNCTION__);
    }
    
private:
    const static int HFP_AG_SLC_STATE_DISCONNECTED = (int)BTConnectState::DISCONNECTED;
    const static int HFP_AG_SCO_STATE_DISCONNECTED = 3;

    BluetoothObserverList<HandsFreeAudioGatewayObserver> observers_;
    sptr<AgServiceObserver> serviceObserver_;
    sptr<IBluetoothHfpAg> proxy_;
    class HandsFreeAudioGatewayDeathRecipient;
    sptr<HandsFreeAudioGatewayDeathRecipient> deathRecipient_;
};

class HandsFreeAudioGateway::impl::HandsFreeAudioGatewayDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    HandsFreeAudioGatewayDeathRecipient(HandsFreeAudioGateway::impl &impl) : impl_(impl)
    {};
    ~HandsFreeAudioGatewayDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HandsFreeAudioGatewayDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("HandsFreeAudioGateway::impl::HandsFreeAudioGatewayDeathRecipient::OnRemoteDied starts");
        impl_.proxy_->AsObject()->RemoveDeathRecipient(impl_.deathRecipient_);
        impl_.proxy_ = nullptr;
    }

private:
    HandsFreeAudioGateway::impl &impl_;
};

HandsFreeAudioGateway::impl::impl()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> hostRemote = samgr->GetSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID);

    if (!hostRemote) {
        HILOGE("HandsFreeAudioGateway::impl:impl() failed: no hostRemote");
        return;
    }
    sptr<IBluetoothHost> hostProxy = iface_cast<IBluetoothHost>(hostRemote);
    sptr<IRemoteObject> remote = hostProxy->GetProfile(PROFILE_HFP_AG);

    if (!remote) {
        HILOGE("HandsFreeAudioGateway::impl:impl() failed: no remote");
        return;
    }
    HILOGI("HandsFreeAudioGateway::impl:impl() remote obtained");

    serviceObserver_ = new AgServiceObserver(observers_);

    proxy_ = iface_cast<IBluetoothHfpAg>(remote);
    proxy_->RegisterObserver(serviceObserver_);
    deathRecipient_ = new HandsFreeAudioGatewayDeathRecipient(*this);
    proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
}

HandsFreeAudioGateway::impl::~impl()
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    if (proxy_ == nullptr) {
        return;
    }
    proxy_->DeregisterObserver(serviceObserver_);
    proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
}

HandsFreeAudioGateway *HandsFreeAudioGateway::GetProfile()
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    static HandsFreeAudioGateway instance;
    return &instance;
}

HandsFreeAudioGateway::HandsFreeAudioGateway()
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    pimpl = std::make_unique<impl>();
}

HandsFreeAudioGateway::~HandsFreeAudioGateway()
{}

std::vector<BluetoothRemoteDevice> HandsFreeAudioGateway::GetConnectedDevices() const
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    return pimpl->GetConnectedDevices();
}

std::vector<BluetoothRemoteDevice> HandsFreeAudioGateway::GetDevicesByStates(std::vector<int> states)
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    return pimpl->GetDevicesByStates(states);
}

int HandsFreeAudioGateway::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    return pimpl->GetDeviceState(device);
}

bool HandsFreeAudioGateway::Connect(const BluetoothRemoteDevice &device)
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    return pimpl->Connect(device);
}

bool HandsFreeAudioGateway::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    return pimpl->Disconnect(device);
}

int HandsFreeAudioGateway::GetScoState(const BluetoothRemoteDevice &device) const
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    return pimpl->GetScoState(device);
}

bool HandsFreeAudioGateway::ConnectSco()
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    return pimpl->ConnectSco();
}

bool HandsFreeAudioGateway::DisconnectSco()
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    return pimpl->DisconnectSco();
}

void HandsFreeAudioGateway::PhoneStateChanged(
    int numActive, int numHeld, int callState, const std::string &number, int type, const std::string &name)
{
    pimpl->PhoneStateChanged(numActive, numHeld, callState, number, type, name);
}

void HandsFreeAudioGateway::ClccResponse(
    int index, int direction, int status, int mode, bool mpty, const std::string &number, int type)
{
    pimpl->ClccResponse(index, direction, status, mode, mpty, number, type);
}

bool HandsFreeAudioGateway::OpenVoiceRecognition(const BluetoothRemoteDevice &device)
{
    return pimpl->OpenVoiceRecognition(device);
}

bool HandsFreeAudioGateway::CloseVoiceRecognition(const BluetoothRemoteDevice &device)
{
    return pimpl->CloseVoiceRecognition(device);
}

bool HandsFreeAudioGateway::SetActiveDevice(const BluetoothRemoteDevice &device)
{
    return pimpl->SetActiveDevice(device);
}

BluetoothRemoteDevice HandsFreeAudioGateway::GetActiveDevice() const
{
    return pimpl->GetActiveDevice();
}

void HandsFreeAudioGateway::RegisterObserver(HandsFreeAudioGatewayObserver *observer)
{
    
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    std::shared_ptr<HandsFreeAudioGatewayObserver> observerPtr(observer, [](HandsFreeAudioGatewayObserver *) {});
    pimpl->RegisterObserver(observerPtr);
}

void HandsFreeAudioGateway::DeregisterObserver(HandsFreeAudioGatewayObserver *observer)
{
    HILOGD("[hfpag]: %{public}s(): Enter!", __FUNCTION__);
    std::shared_ptr<HandsFreeAudioGatewayObserver> observerPtr(observer, [](HandsFreeAudioGatewayObserver *) {});
    pimpl->DeregisterObserver(observerPtr);
    HILOGD("[hfpag]: %{public}s(): end!", __FUNCTION__);
}
}  // namespace Bluetooth
}  // namespace OHOS