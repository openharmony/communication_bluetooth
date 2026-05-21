/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "bluetooth_device_battery_observer_stub.h"
#include "bluetooth_log.h"
#include "ipc_types.h"
#include "string_ex.h"

namespace OHOS {
namespace Bluetooth {
BluetoothDeviceBatteryObserverStub::BluetoothDeviceBatteryObserverStub()
{
    memberFuncMap_ = {
        {static_cast<uint32_t>(BluetoothDeviceBatteryObserverInterfaceCode::BT_DEVICE_GET_BATTERYLEVEL_EVENT),
            &BluetoothDeviceBatteryObserverStub::OnGetBatteryLevelEventInner},
        {static_cast<uint32_t>(BluetoothDeviceBatteryObserverInterfaceCode::BT_DEVICE_BATTERYLEVEL_CHANGED),
            &BluetoothDeviceBatteryObserverStub::OnBatteryLevelChangedInner},
    };
}

BluetoothDeviceBatteryObserverStub::~BluetoothDeviceBatteryObserverStub()
{
    memberFuncMap_.clear();
}

int32_t BluetoothDeviceBatteryObserverStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (BluetoothDeviceBatteryObserverStub::GetDescriptor() != data.ReadInterfaceToken()) {
        HILOGE("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    HILOGW("default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode BluetoothDeviceBatteryObserverStub::OnGetBatteryLevelEventInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (device == nullptr) {
        HILOGE("device transaction wrong.");
        return TRANSACTION_ERR;
    }
    int32_t batteryLevel = data.ReadInt32();
    OnGetBatteryLevelEvent(*device, batteryLevel);
    return NO_ERROR;
}

ErrCode BluetoothDeviceBatteryObserverStub::OnBatteryLevelChangedInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<BluetoothRawAddress> device(data.ReadParcelable<BluetoothRawAddress>());
    if (device == nullptr) {
        HILOGE("device transaction wrong.");
        return TRANSACTION_ERR;
    }
    int32_t batteryLevel = data.ReadInt32();
    OnBatteryLevelChanged(*device, batteryLevel);
    return NO_ERROR;
}
} // namespace Bluetooth
} // namespace OHOS
#endif