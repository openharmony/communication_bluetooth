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
#define LOG_TAG "bt_fwk_hfp_ag"
#endif

#include "bluetooth_hfp_ag.h"
#include <unistd.h>
#include "bluetooth_device.h"
#include "bluetooth_host.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_phone_state.h"
#include "i_bluetooth_hfp_ag.h"
#include "bluetooth_hfp_ag_observer_stub.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
std::mutex g_hfpProxyMutex;
class AgServiceObserver : public BluetoothHfpAgObserverStub {
public:
    explicit AgServiceObserver(BluetoothObserverList<HandsFreeAudioGatewayObserver> &observers) : observers_(observers)
    {
        HILOGD("enter");
    }
    ~AgServiceObserver() override
    {
        HILOGD("enter");
    };

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {
        HILOGD("hfpAg conn state, device: %{public}s, state: %{public}s, cause: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), GetProfileConnStateName(state).c_str(), cause);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state, cause](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state, cause);
        });
    }

    void OnScoStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t reason) override
    {
        HILOGI("enter, device: %{public}s, state: %{public}u, reason: %{public}u",
            GET_ENCRYPT_RAW_ADDR(device), state, reason);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state, reason](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnScoStateChanged(remoteDevice, state, reason);
        });
    }

    void OnActiveDeviceChanged(const BluetoothRawAddress &device) override
    {
        HILOGD("enter, device: %{public}s", GET_ENCRYPT_RAW_ADDR(device));
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnActiveDeviceChanged(remoteDevice);
        });
    }

    void OnHfEnhancedDriverSafetyChanged(
        const BluetoothRawAddress &device, int32_t indValue) override
    {
        HILOGI("enter, device: %{public}s, indValue: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), indValue);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, indValue](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnHfEnhancedDriverSafetyChanged(remoteDevice, indValue);
        });
    }

    void OnHfpStackChanged(const BluetoothRawAddress &device, int32_t action) override
    {
        HILOGI("enter, device: %{public}s, action: %{public}s",
            GET_ENCRYPT_RAW_ADDR(device), GetUpdateOutputStackActionName(action).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, action](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnHfpStackChanged(remoteDevice, action);
        });
    }

    void OnVirtualDeviceChanged(int32_t action, std::string address) override
    {
        HILOGI("enter, device: %{public}s, action: %{public}d", GetEncryptAddr(address).c_str(), action);
        observers_.ForEach([action, address](std::shared_ptr<HandsFreeAudioGatewayObserver> observer) {
            observer->OnVirtualDeviceChanged(action, address);
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

    int32_t GetConnectedDevices(std::vector<BluetoothRemoteDevice>& devices)
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_SERVICE_DISCONNECTED, "failed: no proxy");
        std::vector<BluetoothRawAddress> ori;
        int32_t ret = proxy->GetConnectDevices(ori);
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
        HILOGD("enter");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy != nullptr) {
            std::vector<BluetoothRawAddress> rawDevices;
            std::vector<int32_t> tmpstates;
            for (int state : states) {
                int32_t value = (int32_t)state;
                tmpstates.push_back(value);
            }
            proxy->GetDevicesByStates(tmpstates, rawDevices);
            for (BluetoothRawAddress rawDevice : rawDevices) {
                BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), 0);
                remoteDevices.push_back(remoteDevice);
            }
        }
        return remoteDevices;
    }

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
    {
        HILOGD("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }

        return proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()), state);
    }

    int32_t Connect(const BluetoothRemoteDevice &device)
    {
        HILOGI("hfp connect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }
        return proxy->Connect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    int32_t Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGI("hfp disconnect remote device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy == nullptr || !device.IsValidBluetoothRemoteDevice()) {
            HILOGE("invalid param.");
            return BT_ERR_INVALID_PARAM;
        }
        return proxy->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
    }

    int GetScoState(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy->GetScoState(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return HFP_AG_SCO_STATE_DISCONNECTED;
    }

    int32_t ConnectSco(uint8_t callType)
    {
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_PARAM, "failed: no proxy");
        return proxy->ConnectSco(callType);
    }

    int32_t DisconnectSco(uint8_t callType)
    {
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INVALID_PARAM, "failed: no proxy");
        return proxy->DisconnectSco(callType);
    }

    bool ConnectSco()
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");
        return proxy->ConnectSco();
    }

    bool DisconnectSco()
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");
        return proxy->DisconnectSco();
    }

    bool IsValidCallType(uint8_t callType)
    {
        if (callType == static_cast<uint8_t>(BTCallType::CALL_TYPE_CELLULAR) ||
            callType == static_cast<uint8_t>(BTCallType::CALL_TYPE_VIRTUAL) ||
            callType == static_cast<uint8_t>(BTCallType::CALL_TYPE_RECOGNITION)) {
            return true;
        }
        return false;
    }

    void PhoneStateChanged(BluetoothPhoneState &phoneState)
    {
        HILOGI("numActive: %{public}d, numHeld: %{public}d, callState: %{public}d, type: %{public}d",
            phoneState.GetActiveNum(), phoneState.GetHeldNum(), phoneState.GetCallState(), phoneState.GetCallType());
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is null");
        proxy->PhoneStateChanged(phoneState);
    }

    void ClccResponse(int index, int direction, int status, int mode, bool mpty, std::string number, int type)
    {
        HILOGI("enter, index: %{public}d, direction: %{public}d, status: %{public}d, mode: %{public}d, mpty: "
            "%{public}d, type: %{public}d", index, direction, status, mode, mpty, type);
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is null");
        proxy->ClccResponse(index, direction, status, mode, mpty, number, type);
    }

    bool OpenVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->OpenVoiceRecognition(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool CloseVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->CloseVoiceRecognition(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool SetActiveDevice(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->SetActiveDevice(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool IntoMock(const BluetoothRemoteDevice &device, int state)
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy != nullptr && IS_BT_ENABLED()) {
            return proxy->IntoMock(BluetoothRawAddress(device.GetDeviceAddr()), state);
        }
        return false;
    }

    bool SendNoCarrier(const BluetoothRemoteDevice &device)
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy->SendNoCarrier(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    BluetoothRemoteDevice GetActiveDevice()
    {
        HILOGD("enter");
        BluetoothRemoteDevice device;
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        if (proxy != nullptr) {
            std::string address = proxy->GetActiveDevice();
            BluetoothRemoteDevice remoteDevice(address, 0);
            device = remoteDevice;
        }
        return device;
    }

    int SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is null");
        return proxy->SetConnectStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
    }

    int GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is null");
        return proxy->GetConnectStrategy(BluetoothRawAddress(device.GetDeviceAddr()), strategy);
    }

    int IsInbandRingingEnabled(bool &isEnabled) const
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is null");
        return proxy->IsInbandRingingEnabled(isEnabled);
    }

    void CallDetailsChanged(int callId, int callState)
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is null");
        proxy->CallDetailsChanged(callId, callState);
    }

    int IsVgsSupported(const BluetoothRemoteDevice &device, bool &isSupported) const
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "proxy is null");
        return proxy->IsVgsSupported(BluetoothRawAddress(device.GetDeviceAddr()), isSupported);
    }

    void EnableBtCallLog(bool state)
    {
        HILOGD("enter");
        sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "proxy is null");
        proxy->EnableBtCallLog(state);
    }

    void RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
    {
        HILOGD("enter");
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
    {
        HILOGD("enter");
        observers_.Deregister(observer);
        HILOGI("end");
    }

    int32_t profileRegisterId = 0;
private:
    const static int HFP_AG_SLC_STATE_DISCONNECTED = static_cast<int>(BTConnectState::DISCONNECTED);
    const static int HFP_AG_SCO_STATE_DISCONNECTED = 3;

    BluetoothObserverList<HandsFreeAudioGatewayObserver> observers_;
    sptr<AgServiceObserver> serviceObserver_;
};

HandsFreeAudioGateway::impl::impl()
{
    serviceObserver_ = new AgServiceObserver(observers_);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_HFP_AG,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothHfpAg> proxy = iface_cast<IBluetoothHfpAg>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(serviceObserver_);
    });
}

HandsFreeAudioGateway::impl::~impl()
{
    HILOGD("enter");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(serviceObserver_);
}

HandsFreeAudioGateway *HandsFreeAudioGateway::GetProfile()
{
    HILOGD("enter");
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<HandsFreeAudioGateway> instance;
    return instance.get();
#else
    static HandsFreeAudioGateway instance;
    return &instance;
#endif
}

HandsFreeAudioGateway::HandsFreeAudioGateway()
{
    HILOGD("enter");
    pimpl = std::make_unique<impl>();
}

HandsFreeAudioGateway::~HandsFreeAudioGateway()
{
    HILOGD("enter");
}

std::vector<BluetoothRemoteDevice> HandsFreeAudioGateway::GetConnectedDevices() const
{
    std::vector<BluetoothRemoteDevice> devices;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return devices;
    }
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, devices, "failed: no proxy");

    pimpl->GetConnectedDevices(devices);
    return devices;
}

int32_t HandsFreeAudioGateway::GetConnectedDevices(std::vector<BluetoothRemoteDevice> &devices)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->GetConnectedDevices(devices);
}

std::vector<BluetoothRemoteDevice> HandsFreeAudioGateway::GetDevicesByStates(std::vector<int> states)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::vector<BluetoothRemoteDevice>(), "failed: no proxy");

    return pimpl->GetDevicesByStates(states);
}

int32_t HandsFreeAudioGateway::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    HILOGD("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->GetDeviceState(device, state);
}

int32_t HandsFreeAudioGateway::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->Connect(device);
}

int32_t HandsFreeAudioGateway::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    return pimpl->Disconnect(device);
}

int HandsFreeAudioGateway::GetScoState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return static_cast<int>(HfpScoConnectState::SCO_DISCONNECTED);
    }
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        static_cast<int>(HfpScoConnectState::SCO_DISCONNECTED), "failed: no proxy");

    return pimpl->GetScoState(device);
}

int32_t HandsFreeAudioGateway::ConnectSco(uint8_t callType)
{
    HILOGI("enter, callType: %{public}d", callType);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY,
        "hfpAG proxy is nullptr.");
    CHECK_AND_RETURN_LOG_RET((pimpl->IsValidCallType(callType)), BT_ERR_INVALID_PARAM,
        "connect sco call type error.");
    return pimpl->ConnectSco(callType);
}

int32_t HandsFreeAudioGateway::DisconnectSco(uint8_t callType)
{
    HILOGI("enter, callType: %{public}d", callType);
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY,
        "hfpAG proxy is nullptr.");
    CHECK_AND_RETURN_LOG_RET((pimpl->IsValidCallType(callType)), BT_ERR_INVALID_PARAM,
        "disconnect sco call type error.");
    return pimpl->DisconnectSco(callType);
}

bool HandsFreeAudioGateway::ConnectSco()
{
    return true;
}

bool HandsFreeAudioGateway::DisconnectSco()
{
    return true;
}

void HandsFreeAudioGateway::PhoneStateChanged(
    int numActive, int numHeld, int callState, const std::string &number, int type, const std::string &name)
{
    CHECK_AND_RETURN_LOG(IS_BT_ENABLED(), "bluetooth is off.");
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "hfpAG proxy is nullptr.");

    BluetoothPhoneState phoneState;
    phoneState.SetActiveNum(numActive);
    phoneState.SetHeldNum(numHeld);
    phoneState.SetCallState(callState);
    phoneState.SetNumber(number);
    phoneState.SetCallType(type);
    phoneState.SetName(name);
    pimpl->PhoneStateChanged(phoneState);
}

void HandsFreeAudioGateway::ClccResponse(
    int index, int direction, int status, int mode, bool mpty, const std::string &number, int type)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");

    pimpl->ClccResponse(index, direction, status, mode, mpty, number, type);
}

bool HandsFreeAudioGateway::OpenVoiceRecognition(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->OpenVoiceRecognition(device);
}

bool HandsFreeAudioGateway::CloseVoiceRecognition(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->CloseVoiceRecognition(device);
}

bool HandsFreeAudioGateway::SetActiveDevice(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->SetActiveDevice(device);
}

BluetoothRemoteDevice HandsFreeAudioGateway::GetActiveDevice() const
{
    BluetoothRemoteDevice device;
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return device;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, device, "failed: no proxy");

    device = pimpl->GetActiveDevice();
    return device;
}

int HandsFreeAudioGateway::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    HILOGI("enter, device: %{public}s, strategy: %{public}d", GET_ENCRYPT_ADDR(device), strategy);
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if ((!device.IsValidBluetoothRemoteDevice()) || (
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_ALLOWED)) &&
        (strategy != static_cast<int>(BTStrategyType::CONNECTION_FORBIDDEN)))) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->SetConnectStrategy(device, strategy);
}

int HandsFreeAudioGateway::GetConnectStrategy(const BluetoothRemoteDevice &device, int &strategy) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");

    if (!device.IsValidBluetoothRemoteDevice()) {
        HILOGI("input parameter error.");
        return BT_ERR_INVALID_PARAM;
    }
    return pimpl->GetConnectStrategy(device, strategy);
}

int HandsFreeAudioGateway::IsInbandRingingEnabled(bool &isEnabled) const
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "hfpAG proxy is nullptr");
    return pimpl->IsInbandRingingEnabled(isEnabled);
}

void HandsFreeAudioGateway::CallDetailsChanged(int callId, int callState)
{
    HILOGI("enter, callId: %{public}d, callState: %{public}d", callId, callState);
    CHECK_AND_RETURN_LOG(IS_BT_ENABLED(), "bluetooth is off.");
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "hfpAG proxy is nullptr");
    pimpl->CallDetailsChanged(callId, callState);
}

void HandsFreeAudioGateway::EnableBtCallLog(bool state)
{
    HILOGI("enter");
    CHECK_AND_RETURN_LOG(IS_BT_ENABLED(), "bluetooth is off.");
    pimpl->EnableBtCallLog(state);
}

int HandsFreeAudioGateway::IsVgsSupported(const BluetoothRemoteDevice &device, bool &isSupported) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    CHECK_AND_RETURN_LOG_RET(IS_BT_ENABLED(), BT_ERR_INVALID_STATE, "bluetooth is off.");
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_UNAVAILABLE_PROXY, "failed: no proxy");
    CHECK_AND_RETURN_LOG_RET(device.IsValidBluetoothRemoteDevice(), BT_ERR_INVALID_PARAM, "input parameter error.");
    return pimpl->IsVgsSupported(device, isSupported);
}

void HandsFreeAudioGateway::GetVirtualDeviceList(std::vector<std::string> &devices) const
{
    CHECK_AND_RETURN_LOG(IS_BT_ENABLED(), "bluetooth is off.");
    sptr<IBluetoothHfpAg> proxy = GetRemoteProxy<IBluetoothHfpAg>(PROFILE_HFP_AG);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "hfpAG proxy is nullptr");
    proxy->GetVirtualDeviceList(devices);
}

void HandsFreeAudioGateway::RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->RegisterObserver(observer);
}

void HandsFreeAudioGateway::DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->DeregisterObserver(observer);
}
}  // namespace Bluetooth
}  // namespace OHOS