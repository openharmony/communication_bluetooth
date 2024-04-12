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

#ifndef OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_INTERFACE_H
#define OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_INTERFACE_H

#include "../parcel/bluetooth_raw_address.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "../common/bt_uuid.h"
#include "iremote_broker.h"
#include "../parcel/bluetooth_device_battery_info.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothRemoteDeviceObserver : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ipc.IBluetoothRemoteDeviceObserver");

    virtual void OnAclStateChanged(const BluetoothRawAddress &device, int32_t state, uint32_t reason) = 0;
    virtual void OnPairStatusChanged(const int32_t transport, const BluetoothRawAddress &device,
        int32_t status, int32_t cause) = 0;
    virtual void OnRemoteUuidChanged(const BluetoothRawAddress &device, const std::vector<bluetooth::Uuid> uuids) = 0;
    virtual void OnRemoteNameChanged(const BluetoothRawAddress &device, const std::string deviceName) = 0;
    virtual void OnRemoteAliasChanged(const BluetoothRawAddress &device, const std::string alias) = 0;
    virtual void OnRemoteCodChanged(const BluetoothRawAddress &device, int32_t cod) = 0;
    virtual void OnRemoteBatteryChanged(const BluetoothRawAddress &device, const BluetoothBatteryInfo &batteryInfo) = 0;
    virtual void OnRemoteDeviceCommonInfoReport(const BluetoothRawAddress &device,
        const std::vector<uint8_t> &value) = 0;
};
}  // namespace Bluetooth
}  // namespace OHOS
#endif  // OHOS_BLUETOOTH_STANDARD_REMOTE_DEVICE_OBSERVER_INTERFACE_H