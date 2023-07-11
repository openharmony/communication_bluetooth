﻿/*
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
#include <unistd.h>
#include "bluetooth_device.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_load_system_ability.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
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
        HILOGI("enter");
    }
    ~AgServiceObserver() override
    {
        HILOGI("enter");
    };

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        HILOGD("hfpAg conn state, device: %{public}s, state: %{public}s",
            GetEncryptAddr((device).GetAddress()).c_str(), GetProfileConnStateName(state).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state);
        });
    }

    void OnScoStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        HILOGI("enter, device: %{public}s, state: %{public}u", GetEncryptAddr((device).GetAddress()).c_str(), state);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnScoStateChanged(remoteDevice, state);
        });
    }

    void OnActiveDeviceChanged(const BluetoothRawAddress &device) override
    {
        HILOGI("enter, device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnActiveDeviceChanged(remoteDevice);
        });
    }

    void OnHfEnhancedDriverSafetyChanged(
        const BluetoothRawAddress &device, int32_t indValue) override
    {
        HILOGI("enter, device: %{public}s, indValue: %{public}d",
            GetEncryptAddr((device).GetAddress()).c_str(), indValue);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, indValue](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnHfEnhancedDriverSafetyChanged(remoteDevice, indValue);
        });
    }

private:
    BluetoothObserverList<HandsFreeAudioGatewayObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(AgServiceObserver);
};

std::mutex hfpProxyMutex;
std::string HfpAgServiceName = "bluetooth-hfp-ag-server";

struct HandsFreeAudioGateway::impl {
    impl();
    ~impl();
    bool InitHfpAgProxy(void);
    void UnInitHfpAgProxy(void);

    int32_t GetConnectedDevices(std::vector<BluetoothRemoteDevice>& devices)
    {
        HILOGI("enter");
        if (!proxy_) {
            HILOGE("proxy_ is nullptr.");
            return BT_ERR_SERVICE_DISCONNECTED;
        }
        std::vector<BluetoothRawAddress> ori;
        int32_t ret = proxy_->GetConnectDevices(ori);
        if (ret != BT_NO_ERROR) {
            HILOGE("inner error.");
            return ret;
        }
        for (auto it = ori.begin(); it != ori.end(); it++) {
            devices.push_back(BluetoothRemoteDevice(it->GetAddress(), 0));
        }
        return BT_NO_ERROR;
    }

    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states)
    {
        HILOGI("enter");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        if (proxy_ != nullptr) {
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

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        return proxy_->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
    }

    int32_t Connect(const BluetoothRemoteDevice &device)
    {
        HILOGI("hfp connect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        int cod = 0;
        int32_t err = device.GetDeviceClass(cod);
        if (err != BT_NO_ERROR) {
            HILOGE("GetDeviceClass Failed.");
            return BT_ERR_INVALID_PARAM;
        }
        BluetoothDeviceClass devClass = BluetoothDeviceClass(cod);
        if (!devClass.IsProfileSupported(BluetoothDevice::PROFILE_HEADSET)) {
            HILOGE("hfp connect failed. The remote device does not support HFP service.");
            return BT_ERR_PROFILE_DISABLED;
        }
        return proxy_->Connect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    int32_t Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGI("hfp disconnect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }
        return proxy_->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    int GetScoState(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->GetScoState(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return HFP_AG_SCO_STATE_DISCONNECTED;
    }

    bool ConnectSco()
    {
        HILOGI("enter");
        if (proxy_ != nullptr) {
            return proxy_->ConnectSco();
        }
        return false;
    }

    bool DisconnectSco()
    {
        HILOGI("enter");
        if (proxy_ != nullptr) {
            return proxy_->DisconnectSco();
        }
        return false;
    }

    void PhoneStateChanged(
        int numActive, int numHeld, int callState, const std::string &number, int type, const std::string &name)
    {
        HILOGI("numActive: %{public}d, numHeld: %{public}d, callState: %{public}d, type: %{public}d",
            numActive, numHeld, callState, type);
        if (proxy_ != nullptr) {
            proxy_->PhoneStateChanged(numActive, numHeld, callState, number, type, name);
        }
    }

    void ClccResponse(int index, int direction, int status, int mode, bool mpty, std::string number, int type)
    {
        HILOGI("enter, index: %{public}d, direction: %{public}d, status: %{public}d, mode: %{public}d, mpty: "
            "%{public}d, type: %{public}d", index, direction, status, mode, mpty, type);
        if (proxy_ != nullptr) {
            proxy_->ClccResponse(index, direction, status, mode, mpty, number, type);
        }
    }

    bool OpenVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->OpenVoiceRecognition(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool CloseVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->CloseVoiceRecognition(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool SetActiveDevice(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        if (proxy_ != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->SetActiveDevice(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool IntoMock(const BluetoothRemoteDevice &device, int state)
    {
        HILOGI("enter");
        if (proxy_ != nullptr && IS_BT_ENABLED()) {
            return proxy_->IntoMock(BluetoothRawAddress(device.GetDeviceAddr()), state);
        }
        return false;
    }

    bool SendNoCarrier(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter");
        if (proxy_ != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy_->SendNoCarrier(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    BluetoothRemoteDevice GetActiveDevice()
    {
        HILOGI("enter");
        BluetoothRemoteDevice device;
        if (proxy_ != nullptr) {
            std::string address = proxy_->GetActiveDevice();
            BluetoothRemoteDevice remoteDevice(address, 0);
            device = remoteDevice;
        }
        return device;
    }

    void RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
    {
        HILOGI("enter");
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
    {
        HILOGI("enter");
        observers_.Deregister(observer);
        HILOGI("end");
    }

private:
    const static int HFP_AG_SLC_STATE_DISCONNECTED = static_cast<int>(BTConnectState::DISCONNECTED);
    const static int HFP_AG_SCO_STATE_DISCONNECTED = 3;

    BluetoothObserverList<HandsFreeAudioGatewayObserver> observers_;
    sptr<AgServiceObserver> serviceObserver_;
    sptr<IBluetoothHfpAg> proxy_;
    class HandsFreeAudioGatewayDeathRecipient;
    sptr<HandsFreeAudioGatewayDeathRecipient> deathRecipient_;
};

class HandsFreeAudioGateway::impl::HandsFreeAudioGatewayDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    explicit HandsFreeAudioGatewayDeathRecipient(HandsFreeAudioGateway::impl &impl) : impl_(impl)
    {};
    ~HandsFreeAudioGatewayDeathRecipient() final = default;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HandsFreeAudioGatewayDeathRecipient);

    void OnRemoteDied(const wptr<IRemoteObject> &remote) final
    {
        HILOGI("starts");
        if (!impl_.proxy_) {
            return;
        }
        impl_.proxy_->DeregisterObserver(impl_.serviceObserver_);
        impl_.proxy_->AsObject()->RemoveDeathRecipient(impl_.deathRecipient_);
        impl_.proxy_ = nullptr;
    }

private:
    HandsFreeAudioGateway::impl &impl_;
};

HandsFreeAudioGateway::impl::impl()
{
    HILOGI("start");
    if (proxy_) {
        return;
    }
    BluetootLoadSystemAbility::GetInstance().RegisterNotifyMsg(PROFILE_ID_HFP_AG);
    if (!BluetootLoadSystemAbility::GetInstance().HasSubscribedBluetoothSystemAbility()) {
        BluetootLoadSystemAbility::GetInstance().SubScribeBluetoothSystemAbility();
        return;
    }
    InitHfpAgProxy();
}

HandsFreeAudioGateway::impl::~impl()
{
    HILOGI("enter");
    if (proxy_ == nullptr) {
        return;
    }
    proxy_->DeregisterObserver(serviceObserver_);
    proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
}

bool HandsFreeAudioGateway::impl::InitHfpAgProxy(void)
{
    HILOGI("enter");
    std::lock_guard<std::mutex> lock(hfpProxyMutex);
    if (proxy_) {
        return true;
    }
    proxy_ = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    if (!proxy_) {
        HILOGE("get HfpAg proxy failed");
        return false;
    }

    serviceObserver_ = new AgServiceObserver(observers_);
    if (serviceObserver_ != nullptr) {
        proxy_->RegisterObserver(serviceObserver_);
    }

    deathRecipient_ = new HandsFreeAudioGatewayDeathRecipient(*this);
    if (deathRecipient_ != nullptr) {
        proxy_->AsObject()->AddDeathRecipient(deathRecipient_);
    }
    return true;
}

void HandsFreeAudioGateway::impl::UnInitHfpAgProxy(void)
{
    HILOGI("enter");
    std::lock_guard<std::mutex> lock(hfpProxyMutex);
    if (!proxy_) {
        HILOGE(" UnInitHfpAgProxy failed");
        return;
    }
    proxy_->DeregisterObserver(serviceObserver_);
    proxy_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    proxy_ = nullptr;
    HILOGI(" UnInitHfpAgProxy success");
}

HandsFreeAudioGateway *HandsFreeAudioGateway::GetProfile()
{
    HILOGI("enter");
    static HandsFreeAudioGateway instance;
    return &instance;
}

HandsFreeAudioGateway::HandsFreeAudioGateway()
{
    HILOGI("enter");
    pimpl = std::make_unique<impl>();
}

HandsFreeAudioGateway::~HandsFreeAudioGateway()
{
    HILOGI("enter");
}

void HandsFreeAudioGateway::Init()
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    if (!pimpl->InitHfpAgProxy()) {
        HILOGE("HfpAG proxy is nullptr");
        return;
    }
}

void HandsFreeAudioGateway::UnInit()
{
    if (!pimpl) {
        HILOGE("fails: no pimpl");
        return;
    }
    pimpl->UnInitHfpAgProxy();
}

std::vector<BluetoothRemoteDevice> HandsFreeAudioGateway::GetConnectedDevices() const
{
    std::vector<BluetoothRemoteDevice> devices;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return devices;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return devices;
    }

    pimpl->GetConnectedDevices(devices);
    return devices;
}

int32_t HandsFreeAudioGateway::GetConnectedDevices(std::vector<BluetoothRemoteDevice> &devices)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->GetConnectedDevices(devices);
}

std::vector<BluetoothRemoteDevice> HandsFreeAudioGateway::GetDevicesByStates(std::vector<int> states)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return std::vector<BluetoothRemoteDevice>();
    }

    return pimpl->GetDevicesByStates(states);
}

int32_t HandsFreeAudioGateway::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->GetDeviceState(device, state);
}

int32_t HandsFreeAudioGateway::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->Connect(device);
}

int32_t HandsFreeAudioGateway::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return BT_ERR_UNAVAILABLE_PROXY;
    }

    return pimpl->Disconnect(device);
}

int HandsFreeAudioGateway::GetScoState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return static_cast<int>(HfpScoConnectState::SCO_DISCONNECTED);
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return static_cast<int>(HfpScoConnectState::SCO_DISCONNECTED);
    }

    return pimpl->GetScoState(device);
}

bool HandsFreeAudioGateway::ConnectSco()
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return false;
    }

    return pimpl->ConnectSco();
}

bool HandsFreeAudioGateway::DisconnectSco()
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return false;
    }

    return pimpl->DisconnectSco();
}

void HandsFreeAudioGateway::PhoneStateChanged(
    int numActive, int numHeld, int callState, const std::string &number, int type, const std::string &name)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return;
    }

    pimpl->PhoneStateChanged(numActive, numHeld, callState, number, type, name);
}

void HandsFreeAudioGateway::ClccResponse(
    int index, int direction, int status, int mode, bool mpty, const std::string &number, int type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return;
    }

    pimpl->ClccResponse(index, direction, status, mode, mpty, number, type);
}

bool HandsFreeAudioGateway::OpenVoiceRecognition(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return false;
    }

    return pimpl->OpenVoiceRecognition(device);
}

bool HandsFreeAudioGateway::CloseVoiceRecognition(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return false;
    }

    return pimpl->CloseVoiceRecognition(device);
}

bool HandsFreeAudioGateway::SetActiveDevice(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return false;
    }

    return pimpl->SetActiveDevice(device);
}

BluetoothRemoteDevice HandsFreeAudioGateway::GetActiveDevice() const
{
    BluetoothRemoteDevice device;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return device;
    }

    if (pimpl == nullptr || !pimpl->InitHfpAgProxy()) {
        HILOGE("pimpl or hfpAG proxy is nullptr");
        return device;
    }

    device = pimpl->GetActiveDevice();
    return device;
}

void HandsFreeAudioGateway::RegisterObserver(HandsFreeAudioGatewayObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<HandsFreeAudioGatewayObserver> observerPtr(observer, [](HandsFreeAudioGatewayObserver *) {});
    pimpl->RegisterObserver(observerPtr);
}

void HandsFreeAudioGateway::DeregisterObserver(HandsFreeAudioGatewayObserver *observer)
{
    HILOGI("enter");
    std::shared_ptr<HandsFreeAudioGatewayObserver> observerPtr(observer, [](HandsFreeAudioGatewayObserver *) {});
    if (pimpl == nullptr) {
        HILOGI("pimpl is nullptr!");
        return;
    }
    pimpl->DeregisterObserver(observerPtr);
    HILOGI("end");
}
}  // namespace Bluetooth
}  // namespace OHOS