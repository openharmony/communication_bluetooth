/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bluetooth_ble_impl.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::AdvertisingState;

FfiBluetoothBleAdvertiseCallback::FfiBluetoothBleAdvertiseCallback()
{
}

std::shared_ptr<FfiBluetoothBleAdvertiseCallback> FfiBluetoothBleAdvertiseCallback::GetInstance(void)
{
    static std::shared_ptr<FfiBluetoothBleAdvertiseCallback> instance =
        std::make_shared<FfiBluetoothBleAdvertiseCallback>();
    return instance;
}

void FfiBluetoothBleAdvertiseCallback::RegisterAdvertisingStateChangeFunc(
    std::function<void(CAdvertisingStateChangeInfo)> cjCallback)
{
    advertisingStateChangeFunc = cjCallback;
}

void FfiBluetoothBleAdvertiseCallback::OnStartResultEvent(int result, int advHandle)
{
    CAdvertisingStateChangeInfo info{0};
    info.advertisingId = advHandle;
    info.state = static_cast<int>(AdvertisingState::STARTED);
    if (advertisingStateChangeFunc != nullptr) {
        advertisingStateChangeFunc(info);
    }
    return;
}

void FfiBluetoothBleAdvertiseCallback::OnEnableResultEvent(int result, int advHandle)
{
    CAdvertisingStateChangeInfo info{0};
    info.advertisingId = advHandle;
    info.state = static_cast<int>(AdvertisingState::ENABLED);
    if (advertisingStateChangeFunc != nullptr) {
        advertisingStateChangeFunc(info);
    }
    return;
}

void FfiBluetoothBleAdvertiseCallback::OnDisableResultEvent(int result, int advHandle)
{
    CAdvertisingStateChangeInfo info{0};
    info.advertisingId = advHandle;
    info.state = static_cast<int>(AdvertisingState::DISABLED);
    if (advertisingStateChangeFunc != nullptr) {
        advertisingStateChangeFunc(info);
    }
    return;
}

void FfiBluetoothBleAdvertiseCallback::OnStopResultEvent(int result, int advHandle)
{
    CAdvertisingStateChangeInfo info{0};
    info.advertisingId = advHandle;
    info.state = static_cast<int>(AdvertisingState::STOPPED);
    if (advertisingStateChangeFunc != nullptr) {
        advertisingStateChangeFunc(info);
    }
    return;
}

void FfiBluetoothBleAdvertiseCallback::OnGetAdvHandleEvent(int result, int advHandle)
{
    handleEvent = advHandle;
    return;
}

void FfiBluetoothBleAdvertiseCallback::OnChangeAdvResultEvent(int result, int advHandle)
{}

int32_t FfiBluetoothBleAdvertiseCallback::GetAdvHandleEvent()
{
    return handleEvent;
}

} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS