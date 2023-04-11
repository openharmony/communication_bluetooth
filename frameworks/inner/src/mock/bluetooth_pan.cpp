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

#include "bluetooth_errorcode.h"
#include "bluetooth_pan.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_pan_observer_stub.h"
#include "i_bluetooth_pan.h"
#include "i_bluetooth_host.h"
#include "bluetooth_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
struct Pan::impl {
    impl();
    ~impl();

    int32_t GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice>& result)
    {
        return BT_ERR_SERVICE_DISCONNECTED;
    }

    int32_t GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
    {
        return BT_ERR_INVALID_STATE;
    }

    int32_t Disconnect(const BluetoothRemoteDevice &device)
    {
        return BT_ERR_INVALID_STATE;
    }

    void RegisterObserver(std::shared_ptr<PanObserver> observer)
    {
        return;
    }

    void DeregisterObserver(std::shared_ptr<PanObserver> observer)
    {
        return;
    }

    int32_t SetTethering(bool value)
    {
        return BT_ERR_INVALID_STATE;
    }

    int32_t IsTetheringOn(bool &value)
    {
        return BT_ERR_INVALID_STATE;
    }

private:
    BluetoothObserverList<PanObserver> observers_;
    sptr<IBluetoothPan> proxy_;
};

Pan::impl::impl()
{
    return;
}

Pan::impl::~impl()
{}

Pan::Pan()
{
    pimpl = std::make_unique<impl>();
}

Pan::~Pan()
{}

Pan *Pan::GetProfile()
{
    return nullptr;
}

int32_t Pan::GetDevicesByStates(std::vector<int> states, std::vector<BluetoothRemoteDevice> &result)
{
    return pimpl->GetDevicesByStates(states, result);
}

int32_t Pan::GetDeviceState(const BluetoothRemoteDevice &device, int32_t &state)
{
    return pimpl->GetDeviceState(device, state);
}

int32_t Pan::Disconnect(const BluetoothRemoteDevice &device)
{
    return pimpl->Disconnect(device);
}

void Pan::RegisterObserver(PanObserver *observer)
{
    std::shared_ptr<PanObserver> observerPtr(observer, [](PanObserver *) {});
    return pimpl->RegisterObserver(observerPtr);
}

void Pan::DeregisterObserver(PanObserver *observer)
{
    std::shared_ptr<PanObserver> observerPtr(observer, [](PanObserver *) {});
    return pimpl->DeregisterObserver(observerPtr);
}

int32_t Pan::SetTethering(bool value)
{
    return pimpl->SetTethering(value);
}

int32_t Pan::IsTetheringOn(bool &value)
{
    return pimpl->IsTetheringOn(value);
}
}  // namespace Bluetooth
}  // namespace OHOS