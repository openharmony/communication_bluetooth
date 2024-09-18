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

#ifndef BLUETOOTH_CONNECTION_FFI_H
#define BLUETOOTH_CONNECTION_FFI_H

#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"

#include <cstdint>

extern "C" {
typedef struct {
    int majorClass;
    int majorMinorClass;
    int classOfDevice;
} DeviceClass;

typedef struct {
    char *deviceId;
    char *pinCode;
} CPinRequiredParam;

typedef struct {
    char *deviceId;
    int32_t state;
    int32_t cause;
} CBondStateParam;

typedef struct {
    int batteryLevel;
    int leftEarBatteryLevel;
    int leftEarChargeState;
    int rightEarBatteryLevel;
    int rightEarChargeState;
    int boxBatteryLevel;
    int boxChargeState;
} CBatteryInfo;

const int32_t REGISTER_BATTERY_CHANGE_TYPE = 0;
const int32_t REGISTER_DEVICE_FIND_TYPE = 1;
const int32_t REGISTER_BOND_STATE_TYPE = 2;
const int32_t REGISTER_PIN_REQUEST_TYPE = 3;

FFI_EXPORT void FfiBluetoothConPairDevice(const char* deviceId, int32_t* errCode);
FFI_EXPORT char* FfiBluetoothConGetRemoteDeviceName(const char* deviceId, int32_t* errCode);
FFI_EXPORT DeviceClass FfiBluetoothConGetRemoteDeviceClass(const char* deviceId, int32_t* errCode);
FFI_EXPORT CArrString FfiBluetoothConGetRemoteProfileUuids(const char* deviceId, int32_t* errCode);
FFI_EXPORT char* FfiBluetoothConGetLocalName(int32_t* errCode);
FFI_EXPORT CArrString FfiBluetoothConGetPairedDevices(int32_t* errCode);
FFI_EXPORT int32_t FfiBluetoothConGetPairState(const char* deviceId, int32_t* errCode);
FFI_EXPORT int32_t FfiBluetoothConGetProfileConnectionState(int32_t profileId, int32_t* errCode);
FFI_EXPORT void FfiBluetoothConSetDevicePairingConfirmation(const char* deviceId, bool accept, int32_t* errCode);
FFI_EXPORT void FfiBluetoothConSetDevicePinCode(const char* deviceId, const char* code, int32_t* errCode);
FFI_EXPORT void FfiBluetoothConSetLocalName(const char* localName, int32_t* errCode);
FFI_EXPORT void FfiBluetoothConSetBluetoothScanMode(int32_t mode, int32_t duration, int32_t* errCode);
FFI_EXPORT int32_t FfiBluetoothConGetBluetoothScanMode(int32_t* errCode);
FFI_EXPORT void FfiBluetoothConStartBluetoothDiscovery(int32_t* errCode);
FFI_EXPORT void FfiBluetoothConStopBluetoothDiscovery(int32_t* errCode);
FFI_EXPORT bool FfiBluetoothConIsBluetoothDiscovering(int32_t* errCode);
FFI_EXPORT void FfiBluetoothConSetRemoteDeviceName(const char* deviceId, const char* name, int32_t* errCode);
FFI_EXPORT CBatteryInfo FfiBluetoothConGetRemoteDeviceBatteryInfo(const char* deviceId, int32_t* errCode);
FFI_EXPORT void FfiBluetoothConOn(int32_t callbackType, void (*callback)(), int32_t* errCode);
}

#endif // BLUETOOTH_CONNECTION_FFI_H