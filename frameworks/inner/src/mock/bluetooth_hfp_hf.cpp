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

#include "bluetooth_hfp_hf.h"
#include "bluetooth_host.h"
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

std::string HfpHfServiceName = "bluetooth-hfp-hf-server";

struct HandsFreeUnit::impl {
    impl();
    ~impl();

    bool ConnectSco(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool DisconnectSco(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states)
    {
        HILOGI("enter");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        return remoteDevices;
    }

    int GetDeviceState(const BluetoothRemoteDevice &device)
    {
        return HFP_HF_SLC_STATE_DISCONNECTED;
    }

    int GetScoState(const BluetoothRemoteDevice &device)
    {
        return HFP_HF_SCO_STATE_DISCONNECTED;
    }

    bool SendDTMFTone(const BluetoothRemoteDevice &device, uint8_t code)
    {
        return false;
    }

    bool Connect(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool Disconnect(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool OpenVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool CloseVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    std::vector<HandsFreeUnitCall> GetExistingCalls(const BluetoothRemoteDevice &device)
    {
        HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
        std::vector<HandsFreeUnitCall> calls;
        return calls;
    }

    bool AcceptIncomingCall(const BluetoothRemoteDevice &device, int flag)
    {
        return false;
    }

    bool HoldActiveCall(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool RejectIncomingCall(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool SendKeyPressed(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool HandleIncomingCall(const BluetoothRemoteDevice &device, int flag)
    {
        return false;
    }

    bool HandleMultiCall(const BluetoothRemoteDevice &device, int flag, int index)
    {
        return false;
    }

    bool DialLastNumber(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool DialMemory(const BluetoothRemoteDevice &device, int index)
    {
        return false;
    }

    bool SendVoiceTag(const BluetoothRemoteDevice &device, int index)
    {
        return false;
    }

    bool FinishActiveCall(const BluetoothRemoteDevice &device, const HandsFreeUnitCall &call)
    {
        return false;
    }

    std::optional<HandsFreeUnitCall> StartDial(const BluetoothRemoteDevice &device, const std::string &number)
    {
        return std::nullopt;
    }

    void RegisterObserver(std::shared_ptr<HandsFreeUnitObserver> observer)
    {
        return;
    }

    void DeregisterObserver(std::shared_ptr<HandsFreeUnitObserver> observer)
    {
        return;
    }

private:
    const static int HFP_HF_SLC_STATE_DISCONNECTED = (int)BTConnectState::DISCONNECTED;
    const static int HFP_HF_SCO_STATE_DISCONNECTED = 3;

    BluetoothObserverList<HandsFreeUnitObserver> observers_;
    sptr<IBluetoothHfpHf> proxy_;
};

HandsFreeUnit::impl::impl()
{
    return;
}

HandsFreeUnit::impl::~impl()
{
    return;
}

HandsFreeUnit::HandsFreeUnit()
{
    pimpl = std::make_unique<impl>();
}

HandsFreeUnit::~HandsFreeUnit()
{}

HandsFreeUnit *HandsFreeUnit::GetProfile()
{
    return nullptr;
}

bool HandsFreeUnit::ConnectSco(const BluetoothRemoteDevice &device)
{
    return pimpl->ConnectSco(device);
}

bool HandsFreeUnit::DisconnectSco(const BluetoothRemoteDevice &device)
{
    return pimpl->DisconnectSco(device);
}

std::vector<BluetoothRemoteDevice> HandsFreeUnit::GetDevicesByStates(std::vector<int> states) const
{
    return pimpl->GetDevicesByStates(states);
}

int HandsFreeUnit::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    return pimpl->GetDeviceState(device);
}

int HandsFreeUnit::GetScoState(const BluetoothRemoteDevice &device) const
{
    return pimpl->GetScoState(device);
}

bool HandsFreeUnit::SendDTMFTone(const BluetoothRemoteDevice &device, uint8_t code)
{
    return pimpl->SendDTMFTone(device, code);
}

bool HandsFreeUnit::Connect(const BluetoothRemoteDevice &device)
{
    return pimpl->Connect(device);
}

bool HandsFreeUnit::Disconnect(const BluetoothRemoteDevice &device)
{
    return pimpl->Disconnect(device);
}

bool HandsFreeUnit::OpenVoiceRecognition(const BluetoothRemoteDevice &device)
{
    return pimpl->OpenVoiceRecognition(device);
}

bool HandsFreeUnit::CloseVoiceRecognition(const BluetoothRemoteDevice &device)
{
    return pimpl->CloseVoiceRecognition(device);
}

std::vector<HandsFreeUnitCall> HandsFreeUnit::GetExistingCalls(const BluetoothRemoteDevice &device)
{
    return pimpl->GetExistingCalls(device);
}

bool HandsFreeUnit::AcceptIncomingCall(const BluetoothRemoteDevice &device, int flag)
{
    return pimpl->AcceptIncomingCall(device, flag);
}

bool HandsFreeUnit::HoldActiveCall(const BluetoothRemoteDevice &device)
{
    return pimpl->HoldActiveCall(device);
}

bool HandsFreeUnit::RejectIncomingCall(const BluetoothRemoteDevice &device)
{
    return pimpl->RejectIncomingCall(device);
}

bool HandsFreeUnit::SendKeyPressed(const BluetoothRemoteDevice &device)
{
    return pimpl->SendKeyPressed(device);
}

bool HandsFreeUnit::HandleIncomingCall(const BluetoothRemoteDevice &device, int flag)
{
    return pimpl->HandleIncomingCall(device, flag);
}

bool HandsFreeUnit::HandleMultiCall(const BluetoothRemoteDevice &device, int flag, int index)
{
    return pimpl->HandleMultiCall(device, flag, index);
}

bool HandsFreeUnit::DialLastNumber(const BluetoothRemoteDevice &device)
{
    return pimpl->DialLastNumber(device);
}

bool HandsFreeUnit::DialMemory(const BluetoothRemoteDevice &device, int index)
{
    return pimpl->DialMemory(device, index);
}

bool HandsFreeUnit::SendVoiceTag(const BluetoothRemoteDevice &device, int index)
{
    return pimpl->SendVoiceTag(device, index);
}

bool HandsFreeUnit::FinishActiveCall(const BluetoothRemoteDevice &device, const HandsFreeUnitCall &call)
{
    return pimpl->FinishActiveCall(device, call);
}

std::optional<HandsFreeUnitCall> HandsFreeUnit::StartDial(
    const BluetoothRemoteDevice &device, const std::string &number)
{
    return pimpl->StartDial(device, number);
}

void HandsFreeUnit::RegisterObserver(HandsFreeUnitObserver *observer)
{
    std::shared_ptr<HandsFreeUnitObserver> observerPtr(observer, [](HandsFreeUnitObserver *) {});
    return pimpl->RegisterObserver(observerPtr);
}

void HandsFreeUnit::DeregisterObserver(HandsFreeUnitObserver *observer)
{
    std::shared_ptr<HandsFreeUnitObserver> observerPtr(observer, [](HandsFreeUnitObserver *) {});
    return pimpl->DeregisterObserver(observerPtr);
}
}  // namespace Bluetooth
}  // namespace OHOS
