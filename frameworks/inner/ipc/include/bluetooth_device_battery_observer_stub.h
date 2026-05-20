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

#ifndef BLUETOOTH_DEVICE_BATTERY_OBSERVER_STUB_H
#define BLUETOOTH_DEVICE_BATTERY_OBSERVER_STUB_H

#include <map>
#include "iremote_stub.h"
#include "i_bluetooth_device_battery_observer.h"
#include "bluetooth_service_ipc_interface_code.h"

namespace {
namespace Bluetooth {
class BluetoothDeviceBatteryObserverStub : public IRemoteStub<IBluetoothDeviceBatteryObserver> {
};
}
}
#endif