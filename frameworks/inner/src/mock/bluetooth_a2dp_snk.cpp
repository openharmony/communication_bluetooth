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
#include "bluetooth_a2dp_snk.h"
#include <cstdint>
#include "bluetooth_a2dp_sink_observer_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_types.h"
#include "bluetooth_utils.h"
#include "functional"
#include "i_bluetooth_a2dp_sink.h"
#include "i_bluetooth_a2dp_sink_observer.h"
#include "i_bluetooth_host.h"
#include "if_system_ability_manager.h"
#include "iosfwd"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "list"
#include "memory"
#include "new"
#include "raw_address.h"
#include "refbase.h"
#include "string"
#include "system_ability_definition.h"
#include "vector"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

struct A2dpSink::impl {
    impl();
    ~impl();

    BluetoothObserverList<A2dpSinkObserver> observers_;
    sptr<IBluetoothA2dpSink> proxy_ = nullptr;

private:
    void GetProxy();
};

A2dpSink::impl::impl()
{
    return;
};

A2dpSink::impl::~impl()
{
    return;
}

void A2dpSink::impl::GetProxy()
{
    return;
}

A2dpSink::A2dpSink()
{
    pimpl = std::make_unique<impl>();
    if (!pimpl) {
        HILOGE("fails: no pimpl");
    }
}

A2dpSink::~A2dpSink()
{
    HILOGI("start");
}

void A2dpSink::RegisterObserver(A2dpSinkObserver *observer)
{
    return;
}

void A2dpSink::DeregisterObserver(A2dpSinkObserver *observer)
{
    return;
}

int A2dpSink::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    return RET_BAD_STATUS;
}

std::vector<BluetoothRemoteDevice> A2dpSink::GetDevicesByStates(std::vector<int> states) const
{
    HILOGI("enter");
    std::vector<BluetoothRemoteDevice> devices;
    return devices;
}

int A2dpSink::GetPlayingState(const BluetoothRemoteDevice &device) const
{
    return RET_BAD_STATUS;
}

int A2dpSink::GetPlayingState(const BluetoothRemoteDevice &device, int &state) const
{
    return BT_ERR_SERVICE_DISCONNECTED;
}

bool A2dpSink::Connect(const BluetoothRemoteDevice &device)
{
    return false;
}

bool A2dpSink::Disconnect(const BluetoothRemoteDevice &device)
{
    return false;
}

A2dpSink *A2dpSink::GetProfile()
{
    HILOGI("enter");
    return nullptr;
}

bool A2dpSink::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    return false;
}

int A2dpSink::GetConnectStrategy(const BluetoothRemoteDevice &device) const
{
    return RET_NO_ERROR;
}

bool A2dpSink::SendDelay(const BluetoothRemoteDevice &device, uint16_t delayValue)
{
    return false;
}
}  // namespace Bluetooth
}  // namespace OHOS