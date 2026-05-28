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

#ifndef OHOS_BLUETOOTH_IBLUETOOTHBAS_H
#define OHOS_BLUETOOTH_IBLUETOOTHBAS_H

#include <map>
#include <string_ex.h>
#include <iremote_broker.h>
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_device_battery_observer.h"

namespace OHOS {
namespace Bluetooth {
using namespace OHOS::bluetooth;

class IBluetoothBas : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Bluetooth.IBluetoothBas");

    virtual int32_t RegisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer) = 0;
    virtual int32_t DeregisterDeviceBatteryObserver(const sptr<IBluetoothDeviceBatteryObserver> &observer) = 0;
    virtual int32_t GetBatteryLevel(const std::string &address) = 0;
    virtual int32_t GetConnectedDeviceBatteryInfos(std::map<std::string, int32_t> &batteryInfos) = 0;
};
} // namespace Bluetooth
} // namespace OHOS
#endif  // OHOS_BLUETOOTH_IBLUETOOTHBAS_H