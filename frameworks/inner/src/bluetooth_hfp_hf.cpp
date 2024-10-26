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
#define LOG_TAG "bt_fwk_hfp_hf"
#endif

#include "bluetooth_hfp_hf.h"
#include "bluetooth_host.h"
#include "bluetooth_profile_manager.h"
#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "bluetooth_observer_list.h"
#include "i_bluetooth_hfp_hf.h"
#include "bluetooth_hfp_hf_observer_stub.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
std::mutex g_hfpHFProxyMutex;
class HfServiceObserver : public BluetoothHfpHfObserverStub {
public:
    explicit HfServiceObserver(BluetoothObserverList<HandsFreeUnitObserver> &observers) : observers_(observers)
    {
        HILOGI("enter");
    }
    ~HfServiceObserver() override
    {
        HILOGI("enter");
    }

    void OnConnectionStateChanged(const BluetoothRawAddress &device, int32_t state, int32_t cause) override
    {
        HILOGD("enter, device: %{public}s, state: %{public}d, cause: %{public}d",
            GET_ENCRYPT_RAW_ADDR(device), state, cause);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state, cause](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnConnectionStateChanged(remoteDevice, state, cause);
        });
    }

    void OnScoStateChanged(const BluetoothRawAddress &device, int32_t state) override
    {
        HILOGI("enter, device: %{public}s, state: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), state);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, state](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnScoStateChanged(remoteDevice, state);
        });
    }

    void OnCallChanged(const BluetoothRawAddress &device,
        const BluetoothHfpHfCall &call) override
    {
        HILOGI("enter, device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        UUID uuid = UUID::ConvertFrom128Bits(call.GetUuid().ConvertTo128Bits());
        HandsFreeUnitCall tmpCall(call.GetRemoteDevice(),
            call.GetId(),
            call.GetState(),
            call.GetNumber(),
            uuid,
            call.IsMultiParty(),
            call.IsOutgoing(),
            call.IsInBandRing(),
            call.GetCreationTime());
        observers_.ForEach([remoteDevice, tmpCall](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnCallChanged(remoteDevice, tmpCall);
        });
    }

    void OnSignalStrengthChanged(const BluetoothRawAddress &device, int32_t signal) override
    {
        HILOGI("enter, device: %{public}s, signal: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), signal);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, signal](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnSignalStrengthChanged(remoteDevice, signal);
        });
    }

    void OnRegistrationStatusChanged(const BluetoothRawAddress &device, int32_t status) override
    {
        HILOGI("enter, device: %{public}s, status: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), status);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, status](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnRegistrationStatusChanged(remoteDevice, status);
        });
    }

    void OnRoamingStatusChanged(const BluetoothRawAddress &device, int32_t status) override
    {
        HILOGI("enter, device: %{public}s, status: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), status);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, status](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnRoamingStatusChanged(remoteDevice, status);
        });
    }

    void OnOperatorSelectionChanged(
        const BluetoothRawAddress &device, const std::string &name) override
    {
        HILOGI("enter, device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, name](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnOperatorSelectionChanged(remoteDevice, name);
        });
    }

    void OnSubscriberNumberChanged(
        const BluetoothRawAddress &device, const std::string &number) override
    {
        HILOGI("enter, device: %{public}s", GetEncryptAddr((device).GetAddress()).c_str());
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, number](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnSubscriberNumberChanged(remoteDevice, number);
        });
    }

    void OnVoiceRecognitionStatusChanged(
        const BluetoothRawAddress &device, int32_t status) override
    {
        HILOGI("enter, device: %{public}s, status: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), status);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, status](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnVoiceRecognitionStatusChanged(remoteDevice, status);
        });
    }

    void OnInBandRingToneChanged(const BluetoothRawAddress &device, int32_t status) override
    {
        HILOGI("enter, device: %{public}s, status: %{public}d", GetEncryptAddr((device).GetAddress()).c_str(), status);
        BluetoothRemoteDevice remoteDevice(device.GetAddress(), 0);
        observers_.ForEach([remoteDevice, status](std::shared_ptr<HandsFreeUnitObserver> observer) {
            observer->OnInBandRingToneChanged(remoteDevice, status);
        });
    }

private:
    BluetoothObserverList<HandsFreeUnitObserver> &observers_;
    BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(HfServiceObserver);
};

std::string HfpHfServiceName = "bluetooth-hfp-hf-server";

struct HandsFreeUnit::impl {
    impl();
    ~impl();

    bool ConnectSco(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->ConnectSco(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool DisconnectSco(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->DisconnectSco(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states)
    {
        HILOGI("enter");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr) {
            std::vector<BluetoothRawAddress> rawDevices;
            std::vector<int32_t> tmpStates;
            for (int state : states) {
                tmpStates.push_back((int32_t)state);
            }

            proxy->GetDevicesByStates(tmpStates, rawDevices);
            for (BluetoothRawAddress rawDevice : rawDevices) {
                BluetoothRemoteDevice remoteDevice(rawDevice.GetAddress(), 0);
                remoteDevices.push_back(remoteDevice);
            }
        }
        return remoteDevices;
    }

    int GetDeviceState(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->GetDeviceState(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return HFP_HF_SLC_STATE_DISCONNECTED;
    }

    int GetScoState(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->GetScoState(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return HFP_HF_SCO_STATE_DISCONNECTED;
    }

    bool SendDTMFTone(const BluetoothRemoteDevice &device, uint8_t code)
    {
        HILOGI("enter, device: %{public}s, code: %{public}d", GET_ENCRYPT_ADDR(device), code);
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->SendDTMFTone(BluetoothRawAddress(device.GetDeviceAddr()), code);
        }
        return false;
    }

    bool Connect(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        bool isDiscovering = false;
        BluetoothHost::GetDefaultHost().IsBtDiscovering(isDiscovering);
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && !isDiscovering && device.IsValidBluetoothRemoteDevice()) {
            return proxy->Connect(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        HILOGE("fw return false!");
        return false;
    }

    bool Disconnect(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->Disconnect(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        HILOGE("fw return false!");
        return false;
    }

    bool OpenVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->OpenVoiceRecognition(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool CloseVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->CloseVoiceRecognition(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    std::vector<HandsFreeUnitCall> GetExistingCalls(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        std::vector<HandsFreeUnitCall> calls;
        std::vector<BluetoothHfpHfCall> callsList;
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            proxy->GetCurrentCallList(BluetoothRawAddress(device.GetDeviceAddr()), callsList);
            for (BluetoothHfpHfCall call : callsList) {
                UUID uuid = UUID::ConvertFrom128Bits(call.GetUuid().ConvertTo128Bits());
                HandsFreeUnitCall tmpCall(call.GetRemoteDevice(),
                    call.GetId(),
                    call.GetState(),
                    call.GetNumber(),
                    uuid,
                    call.IsMultiParty(),
                    call.IsOutgoing(),
                    call.IsInBandRing(),
                    call.GetCreationTime());
                calls.push_back(tmpCall);
            }
        }
        return calls;
    }

    bool AcceptIncomingCall(const BluetoothRemoteDevice &device, int flag)
    {
        HILOGI("enter, device: %{public}s, flag: %{public}d", GET_ENCRYPT_ADDR(device), flag);
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->AcceptIncomingCall(BluetoothRawAddress(device.GetDeviceAddr()), (int32_t)flag);
        }
        return false;
    }

    bool HoldActiveCall(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->HoldActiveCall(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool RejectIncomingCall(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->RejectIncomingCall(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool SendKeyPressed(const BluetoothRemoteDevice &device)
    {
        HILOGD("%{public}s(): Enter!", __FUNCTION__);
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy->SendKeyPressed(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool HandleIncomingCall(const BluetoothRemoteDevice &device, int flag)
    {
        HILOGI("Enter!");
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->HandleIncomingCall(BluetoothRawAddress(device.GetDeviceAddr()), (int32_t)flag);
        }
        return false;
    }

    bool HandleMultiCall(const BluetoothRemoteDevice &device, int flag, int index)
    {
        HILOGI("Enter!");
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->HandleMultiCall(BluetoothRawAddress(device.GetDeviceAddr()), (int32_t)flag, (int32_t)index);
        }
        return false;
    }

    bool DialLastNumber(const BluetoothRemoteDevice &device)
    {
        HILOGI("Enter!");
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->DialLastNumber(BluetoothRawAddress(device.GetDeviceAddr()));
        }
        return false;
    }

    bool DialMemory(const BluetoothRemoteDevice &device, int index)
    {
        HILOGD("Enter! index = %{public}d", index);
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            return proxy->DialMemory(BluetoothRawAddress(device.GetDeviceAddr()), index);
        }
        return false;
    }

    bool SendVoiceTag(const BluetoothRemoteDevice &device, int index)
    {
        HILOGD("%{public}s(): Enter! index = %{public}d", __FUNCTION__, index);
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && IS_BT_ENABLED() && device.IsValidBluetoothRemoteDevice()) {
            return proxy->SendVoiceTag(BluetoothRawAddress(device.GetDeviceAddr()), index);
        }
        return false;
    }

    bool FinishActiveCall(const BluetoothRemoteDevice &device, const HandsFreeUnitCall &call)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            bluetooth::Uuid uuid = bluetooth::Uuid::ConvertFrom128Bits(call.GetUuid().ConvertTo128Bits());
            bluetooth::HandsFreeUnitCalls calls(call.GetRemoteDevice(),
                call.GetId(),
                call.GetState(),
                call.GetNumber(),
                uuid,
                call.IsMultiParty(),
                call.IsOutgoing(),
                call.IsInBandRing(),
                call.GetCreationTime());
            return proxy->FinishActiveCall(BluetoothRawAddress(device.GetDeviceAddr()), calls);
        }
        return false;
    }

    std::optional<HandsFreeUnitCall> StartDial(const BluetoothRemoteDevice &device, const std::string &number)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
        if (proxy != nullptr && device.IsValidBluetoothRemoteDevice()) {
            BluetoothHfpHfCall calls;
            proxy->StartDial(BluetoothRawAddress(device.GetDeviceAddr()), number, calls);
            UUID uuid = UUID::ConvertFrom128Bits(calls.GetUuid().ConvertTo128Bits());
            HandsFreeUnitCall call(calls.GetRemoteDevice(),
                calls.GetId(),
                calls.GetState(),
                calls.GetNumber(),
                uuid,
                calls.IsMultiParty(),
                calls.IsOutgoing(),
                calls.IsInBandRing(),
                calls.GetCreationTime());
            return call;
        }
        return std::nullopt;
    }

    void RegisterObserver(std::shared_ptr<HandsFreeUnitObserver> observer)
    {
        HILOGI("enter");
        observers_.Register(observer);
    }

    void DeregisterObserver(std::shared_ptr<HandsFreeUnitObserver> observer)
    {
        HILOGI("enter");
        observers_.Deregister(observer);
    }
    int32_t profileRegisterId = 0;
private:
    const static int HFP_HF_SLC_STATE_DISCONNECTED = static_cast<int>(BTConnectState::DISCONNECTED);
    const static int HFP_HF_SCO_STATE_DISCONNECTED = 3;

    BluetoothObserverList<HandsFreeUnitObserver> observers_;
    sptr<HfServiceObserver> serviceObserver_;
};

HandsFreeUnit::impl::impl()
{
    serviceObserver_ = new HfServiceObserver(observers_);
    profileRegisterId = BluetoothProfileManager::GetInstance().RegisterFunc(PROFILE_HFP_HF,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothHfpHf> proxy = iface_cast<IBluetoothHfpHf>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterObserver(serviceObserver_);
    });
}

HandsFreeUnit::impl::~impl()
{
    HILOGI("enter");
    BluetoothProfileManager::GetInstance().DeregisterFunc(profileRegisterId);
    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterObserver(serviceObserver_);
}

HandsFreeUnit::HandsFreeUnit()
{
    pimpl = std::make_unique<impl>();
}

HandsFreeUnit::~HandsFreeUnit()
{}

HandsFreeUnit *HandsFreeUnit::GetProfile()
{
#ifdef DTFUZZ_TEST
    static BluetoothNoDestructor<HandsFreeUnit> instance;
    return instance.get();
#else
    static HandsFreeUnit instance;
    return &instance;
#endif
}

bool HandsFreeUnit::ConnectSco(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->ConnectSco(device);
}

bool HandsFreeUnit::DisconnectSco(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->DisconnectSco(device);
}

std::vector<BluetoothRemoteDevice> HandsFreeUnit::GetDevicesByStates(std::vector<int> states) const
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<BluetoothRemoteDevice>();
    }
    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        std::vector<BluetoothRemoteDevice>(), "failed: no proxy");

    return pimpl->GetDevicesByStates(states);
}

int HandsFreeUnit::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return static_cast<int>(BTConnectState::DISCONNECTED);
    }
    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        static_cast<int>(BTConnectState::DISCONNECTED), "failed: no proxy");

    return pimpl->GetDeviceState(device);
}

int HandsFreeUnit::GetScoState(const BluetoothRemoteDevice &device) const
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return static_cast<int>(HfpScoConnectState::SCO_DISCONNECTED);
    }
    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr,
        static_cast<int>(HfpScoConnectState::SCO_DISCONNECTED), "failed: no proxy");

    return pimpl->GetScoState(device);
}

bool HandsFreeUnit::SendDTMFTone(const BluetoothRemoteDevice &device, uint8_t code)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }
    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->SendDTMFTone(device, code);
}

bool HandsFreeUnit::Connect(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->Connect(device);
}

bool HandsFreeUnit::Disconnect(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->Disconnect(device);
}

bool HandsFreeUnit::OpenVoiceRecognition(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->OpenVoiceRecognition(device);
}

bool HandsFreeUnit::CloseVoiceRecognition(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->CloseVoiceRecognition(device);
}

std::vector<HandsFreeUnitCall> HandsFreeUnit::GetExistingCalls(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::vector<HandsFreeUnitCall>();
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::vector<HandsFreeUnitCall>(), "failed: no proxy");

    return pimpl->GetExistingCalls(device);
}

bool HandsFreeUnit::AcceptIncomingCall(const BluetoothRemoteDevice &device, int flag)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }
    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->AcceptIncomingCall(device, flag);
}

bool HandsFreeUnit::HoldActiveCall(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->HoldActiveCall(device);
}

bool HandsFreeUnit::RejectIncomingCall(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->RejectIncomingCall(device);
}

bool HandsFreeUnit::SendKeyPressed(const BluetoothRemoteDevice &device)
{
    return pimpl->SendKeyPressed(device);
}

bool HandsFreeUnit::HandleIncomingCall(const BluetoothRemoteDevice &device, int flag)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->HandleIncomingCall(device, flag);
}

bool HandsFreeUnit::HandleMultiCall(const BluetoothRemoteDevice &device, int flag, int index)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->HandleMultiCall(device, flag, index);
}

bool HandsFreeUnit::DialLastNumber(const BluetoothRemoteDevice &device)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->DialLastNumber(device);
}

bool HandsFreeUnit::DialMemory(const BluetoothRemoteDevice &device, int index)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->DialMemory(device, index);
}

bool HandsFreeUnit::SendVoiceTag(const BluetoothRemoteDevice &device, int index)
{
    return pimpl->SendVoiceTag(device, index);
}


bool HandsFreeUnit::FinishActiveCall(const BluetoothRemoteDevice &device, const HandsFreeUnitCall &call)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return false;
    }

    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, false, "failed: no proxy");

    return pimpl->FinishActiveCall(device, call);
}

std::optional<HandsFreeUnitCall> HandsFreeUnit::StartDial(
    const BluetoothRemoteDevice &device, const std::string &number)
{
    if (!IS_BT_ENABLED()) {
        HILOGE("bluetooth is off.");
        return std::nullopt;
    }
    sptr<IBluetoothHfpHf> proxy = GetRemoteProxy<IBluetoothHfpHf>(PROFILE_HFP_HF);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, std::nullopt, "failed: no proxy");

    return pimpl->StartDial(device, number);
}

void HandsFreeUnit::RegisterObserver(std::shared_ptr<HandsFreeUnitObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->RegisterObserver(observer);
}

void HandsFreeUnit::DeregisterObserver(std::shared_ptr<HandsFreeUnitObserver> observer)
{
    HILOGD("enter");
    CHECK_AND_RETURN_LOG(pimpl != nullptr, "pimpl is null.");
    pimpl->DeregisterObserver(observer);
}
}  // namespace Bluetooth
}  // namespace OHOS