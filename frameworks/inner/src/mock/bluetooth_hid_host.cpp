/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "bluetooth_hid_host.h"
#include <unistd.h>
#include "bluetooth_device.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_hid_host_observer_stub.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_hid_host.h"
#include "i_bluetooth_host.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {

struct HidHost::impl {
    impl();
    ~impl();
    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice>& result)
    {
        return BT_ERR_INVALID_STATE;
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

    void RegisterObserver(std::shared_ptr<HidHostObserver> observer)
    {
        return;
    }

    void DeregisterObserver(std::shared_ptr<HidHostObserver> observer)
    {
        return;
    }

    void HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        return;
    }

    void HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        return;
    }

    void HidHostSetReport(std::string device, uint8_t type, uint16_t size, uint8_t report)
    {
        return;
    }

    void HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
    {
        return;
    }

private:
    BluetoothObserverList<HidHostObserver> observers_;
    sptr<IBluetoothHidHost> proxy_;
    void GetHidProxy()
    {
        return;
    }
};

HidHost::impl::impl()
{
    return;
}

HidHost::impl::~impl()
{
    return;
}

HidHost::HidHost()
{
    pimpl = std::make_unique<impl>();
}

HidHost::~HidHost()
{}

HidHost *HidHost::GetProfile()
{
    return nullptr;
}

int32_t HidHost::GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result)
{
    return pimpl->GetDevicesByStates(states, result);
}

int32_t HidHost::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    return pimpl->GetDeviceState(device, state);
}

int32_t HidHost::Connect(const BluetoothRemoteDevice &device)
{
    return pimpl->Connect(device);
}

int32_t HidHost::Disconnect(const BluetoothRemoteDevice &device)
{
    return pimpl->Disconnect(device);
}

void HidHost::RegisterObserver(HidHostObserver *observer)
{
    std::shared_ptr<HidHostObserver> observerPtr(observer, [](HidHostObserver *) {});
    return pimpl->RegisterObserver(observerPtr);
}

void HidHost::DeregisterObserver(HidHostObserver *observer)
{
    std::shared_ptr<HidHostObserver> observerPtr(observer, [](HidHostObserver *) {});
    return pimpl->DeregisterObserver(observerPtr);
}

void HidHost::HidHostVCUnplug(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    return pimpl->HidHostVCUnplug(device, id, size, type);
}

void HidHost::HidHostSendData(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    return pimpl->HidHostSendData(device, id, size, type);
}

void HidHost::HidHostSetReport(std::string device, uint8_t type, uint16_t size, uint8_t report)
{
    return pimpl->HidHostSetReport(device, type, size, report);
}

void HidHost::HidHostGetReport(std::string device, uint8_t id, uint16_t size, uint8_t type)
{
    return pimpl->HidHostGetReport(device, id, size, type);
}
}  // namespace Bluetooth
}  // namespace OHOS