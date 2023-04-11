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
#include <unistd.h>
#include "bluetooth_device.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
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

std::string HfpAgServiceName = "bluetooth-hfp-ag-server";

struct HandsFreeAudioGateway::impl {
    impl();
    ~impl();
    int32_t GetConnectedDevices(std::vector<BluetoothRemoteDevice>& devices)
    {
        return BT_ERR_INVALID_STATE;
    }

    std::vector<BluetoothRemoteDevice> GetDevicesByStates(std::vector<int> states)
    {
        HILOGI("enter");
        std::vector<BluetoothRemoteDevice> remoteDevices;
        return remoteDevices;
    }

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
    {
        return BT_ERR_INVALID_STATE;
    }

    int32_t Connect(const BluetoothRemoteDevice &device)
    {
        return BT_ERR_INVALID_STATE;
    }

    int32_t Disconnect(const BluetoothRemoteDevice &device)
    {
        return BT_ERR_INVALID_STATE;
    }

    int GetScoState(const BluetoothRemoteDevice &device)
    {
        return HFP_AG_SCO_STATE_DISCONNECTED;
    }

    bool ConnectSco()
    {
        return false;
    }

    bool DisconnectSco()
    {
        return false;
    }

    void PhoneStateChanged(
        int numActive, int numHeld, int callState, const std::string &number, int type, const std::string &name)
    {
        return;
    }

    void ClccResponse(int index, int direction, int status, int mode, bool mpty, std::string number, int type)
    {
        return;
    }

    bool OpenVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool CloseVoiceRecognition(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool SetActiveDevice(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    bool IntoMock(const BluetoothRemoteDevice &device, int state)
    {
        return false;
    }

    bool SendNoCarrier(const BluetoothRemoteDevice &device)
    {
        return false;
    }

    BluetoothRemoteDevice GetActiveDevice()
    {
        HILOGI("enter");
        BluetoothRemoteDevice device;
        return device;
    }

    void RegisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
    {
        return;
    }

    void DeregisterObserver(std::shared_ptr<HandsFreeAudioGatewayObserver> observer)
    {
        return;
    }
    
private:
    const static int HFP_AG_SLC_STATE_DISCONNECTED = (int)BTConnectState::DISCONNECTED;
    const static int HFP_AG_SCO_STATE_DISCONNECTED = 3;

    BluetoothObserverList<HandsFreeAudioGatewayObserver> observers_;
    sptr<IBluetoothHfpAg> proxy_;
    
    void GetHfpAgProxy()
    {
        return;
    }
};

HandsFreeAudioGateway::impl::impl()
{
    return;
}

HandsFreeAudioGateway::impl::~impl()
{
    return;
}

HandsFreeAudioGateway *HandsFreeAudioGateway::GetProfile()
{
    HILOGI("enter");
    return nullptr;
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

std::vector<BluetoothRemoteDevice> HandsFreeAudioGateway::GetConnectedDevices() const
{
    std::vector<BluetoothRemoteDevice> devices;
    pimpl->GetConnectedDevices(devices);
    return devices;
}

int32_t HandsFreeAudioGateway::GetConnectedDevices(std::vector<BluetoothRemoteDevice> &devices)
{
    HILOGI("enter");
    return pimpl->GetConnectedDevices(devices);
}

std::vector<BluetoothRemoteDevice> HandsFreeAudioGateway::GetDevicesByStates(std::vector<int> states)
{
    HILOGI("enter");
    return pimpl->GetDevicesByStates(states);
}

int32_t HandsFreeAudioGateway::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->GetDeviceState(device, state);
}

int32_t HandsFreeAudioGateway::Connect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->Connect(device);
}

int32_t HandsFreeAudioGateway::Disconnect(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->Disconnect(device);
}

int HandsFreeAudioGateway::GetScoState(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->GetScoState(device);
}

bool HandsFreeAudioGateway::ConnectSco()
{
    HILOGI("enter");
    return pimpl->ConnectSco();
}

bool HandsFreeAudioGateway::DisconnectSco()
{
    HILOGI("enter");
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
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->OpenVoiceRecognition(device);
}

bool HandsFreeAudioGateway::CloseVoiceRecognition(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->CloseVoiceRecognition(device);
}

bool HandsFreeAudioGateway::SetActiveDevice(const BluetoothRemoteDevice &device)
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));
    return pimpl->SetActiveDevice(device);
}

bool HandsFreeAudioGateway::IntoMock(const BluetoothRemoteDevice &device, int state)
{
    return pimpl->IntoMock(device, state);
}

bool HandsFreeAudioGateway::SendNoCarrier(const BluetoothRemoteDevice &device)
{
    return pimpl->SendNoCarrier(device);
}

BluetoothRemoteDevice HandsFreeAudioGateway::GetActiveDevice() const
{
    return pimpl->GetActiveDevice();
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