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

#ifndef BLUETOOTH_BLE_COMMON_H
#define BLUETOOTH_BLE_COMMON_H

#include "bluetooth_ble_ffi.h"
#include "bluetooth_gatt_service.h"
#include "cj_common_ffi.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::GattCharacteristic;
using Bluetooth::GattDescriptor;
using Bluetooth::GattService;

enum ProfileConnectionState {
    STATE_DISCONNECTED = 0, // the current profile is disconnected
    STATE_CONNECTING = 1,   // the current profile is being connected
    STATE_CONNECTED = 2,    // the current profile is connected
    STATE_DISCONNECTING = 3 // the current profile is being disconnected
};

char *MallocCString(const std::string &origin);
CArrString Convert2CArrString(std::vector<std::string> &tids);
CArrUI8 Convert2CArrUI8(std::vector<uint8_t> vec);
std::string GetGattClientDeviceId();
CArrBLECharacteristic Convert2CArrBLECharacteristic(std::vector<GattCharacteristic> characteristics);
CArrGattService Convert2CArrGattService(std::vector<GattService> services);
CArrBLEDescriptor Convert2CArrBLEDescriptor(std::vector<GattDescriptor> &descriptors);
NativeGattProperties ConvertGattPropertiesToCJ(int properties);
NativeBLECharacteristic ConvertBLECharacteristicToCJ(GattCharacteristic &characteristic);
NativeGattService ConvertGattServiceToCJ(GattService service);
NativeBLEDescriptor ConvertBLEDescriptorToCJ(GattDescriptor &descriptor);
uint16_t ConvertGattPermissions(const NativeGattPermission &nativePermissions);
uint16_t ConvertGattProperties(const NativeGattProperties &nativeProperties);
int GetProfileConnectionState(int state);
void FreeNativeBLECharacteristic(NativeBLECharacteristic characteristic);
void FreeNativeBLEDescriptor(NativeBLEDescriptor descriptor);
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS

#endif // BLUETOOTH_BLE_COMMON_H