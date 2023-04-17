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

#include "bluetooth_a2dp_src.h"
#include <unistd.h>
#include "bluetooth_a2dp_codec.h"
#include "i_bluetooth_a2dp_src.h"
#include "bluetooth_a2dp_src_observer_stub.h"
#include "bluetooth_device.h"
#include "bluetooth_host_proxy.h"
#include "bluetooth_observer_list.h"
#include "raw_address.h"
#include "bluetooth_def.h"
#include "bluetooth_host.h"

#include "bluetooth_log.h"
#include "bluetooth_utils.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

struct A2dpSource::impl {
    impl();
    ~impl();

    BluetoothObserverList<A2dpSourceObserver> observers_;
    sptr<IBluetoothA2dpSrc> proxy_ = nullptr;

private:
    void GetA2dpSrcProxy();
};

A2dpSource::impl::impl()
{
    return;
};

A2dpSource::impl::~impl()
{
    return;
}

void A2dpSource::impl::GetA2dpSrcProxy()
{
    return;
}

A2dpSource::A2dpSource()
{
    return;
}

A2dpSource::~A2dpSource()
{
    HILOGI("start");
}

void A2dpSource::RegisterObserver(A2dpSourceObserver *observer)
{
    return;
}

void A2dpSource::DeregisterObserver(A2dpSourceObserver *observer)
{
    return;
}

std::vector<BluetoothRemoteDevice> A2dpSource::GetDevicesByStates(std::vector<int> states) const
{
    HILOGI("enter");

    std::vector<BluetoothRemoteDevice> devices;
    return devices;
}

int A2dpSource::GetDeviceState(const BluetoothRemoteDevice &device) const
{
    return RET_BAD_STATUS;
}

int32_t A2dpSource::GetPlayingState(const BluetoothRemoteDevice &device) const
{
    return RET_BAD_STATUS;
}

int32_t A2dpSource::GetPlayingState(const BluetoothRemoteDevice &device, int &state) const
{
    return BT_ERR_INVALID_STATE;
}

int32_t A2dpSource::Connect(const BluetoothRemoteDevice &device)
{
    return BT_ERR_INVALID_STATE;
}

int32_t A2dpSource::Disconnect(const BluetoothRemoteDevice &device)
{
    return BT_ERR_INVALID_STATE;
}

A2dpSource *A2dpSource::GetProfile()
{
    HILOGI("enter");
    return nullptr;
}

int A2dpSource::SetActiveSinkDevice(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

const BluetoothRemoteDevice &A2dpSource::GetActiveSinkDevice() const
{
    HILOGI("enter");

    static BluetoothRemoteDevice deviceInfo;
    return deviceInfo;
}

bool A2dpSource::SetConnectStrategy(const BluetoothRemoteDevice &device, int strategy)
{
    return false;
}

int A2dpSource::GetConnectStrategy(const BluetoothRemoteDevice &device) const
{
    return RET_BAD_STATUS;
}

A2dpCodecStatus A2dpSource::GetCodecStatus(const BluetoothRemoteDevice &device) const
{
    HILOGI("enter, device: %{public}s", GET_ENCRYPT_ADDR(device));

    A2dpCodecStatus ret;
    return ret;
}

int A2dpSource::SetCodecPreference(const BluetoothRemoteDevice &device, const A2dpCodecInfo &info)
{
    return RET_BAD_STATUS;
}

void A2dpSource::SwitchOptionalCodecs(const BluetoothRemoteDevice &device, bool isEnable)
{
    return;
}

int A2dpSource::GetOptionalCodecsSupportState(const BluetoothRemoteDevice &device) const
{
    return RET_BAD_STATUS;
}

int A2dpSource::StartPlaying(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

int A2dpSource::SuspendPlaying(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

int A2dpSource::StopPlaying(const BluetoothRemoteDevice &device)
{
    return RET_BAD_STATUS;
}

int A2dpSource::WriteFrame(const uint8_t *data, uint32_t size)
{
    return RET_BAD_STATUS;
}

void A2dpSource::GetRenderPosition(uint16_t &delayValue, uint16_t &sendDataSize, uint32_t &timeStamp)
{
    return;
}

}  // namespace Bluetooth
}  // namespace OHOS