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

#include "bluetooth_connection_util.h"
#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"

#include <cstdint>

extern "C" {
    FFI_EXPORT void FfiBluetoothConPairDevice(char* deviceId, int32_t* errCode);
    FFI_EXPORT char* FfiBluetoothConGetRemoteDeviceName(char* deviceId, int32_t* errCode);
    FFI_EXPORT DeviceClass FfiBluetoothConGetRemoteDeviceClass(char* deviceId, int32_t* errCode);
    FFI_EXPORT CArrString FfiBluetoothConGetRemoteProfileUuids(char* deviceId, int32_t* errCode);
    FFI_EXPORT char* FfiBluetoothConGetLocalName(int32_t* errCode);
    FFI_EXPORT CArrString FfiBluetoothConGetPairedDevices(int32_t* errCode);
    FFI_EXPORT int32_t FfiBluetoothConGetPairedState(char* deviceId, int32_t* errCode);
    FFI_EXPORT int32_t FfiBluetoothConGetProfileConnectionState(int32_t profileIdId, int32_t* errCode);
    FFI_EXPORT void FfiBluetoothConSetDevicePairingConfirmation(char* deviceId, bool accept, int32_t* errCode);
    FFI_EXPORT void FfiBluetoothConSetDevicePinCode(char* deviceId, char* code, int32_t* errCode);
    FFI_EXPORT void FfiBluetoothConSetLocalName(char* name, int32_t* errCode);
    FFI_EXPORT void FfiBluetoothConSetBluetoothScanMode(int32_t mode, int32_t duration, int32_t* errCode);
    FFI_EXPORT int32_t FfiBluetoothConGetBluetoothScanMode(int32_t* errCode);
    FFI_EXPORT void FfiBluetoothConStartBluetoothDiscovery(int32_t* errCode);
    FFI_EXPORT void FfiBluetoothConStoptBluetoothDiscovery(int32_t* errCode);
    FFI_EXPORT bool FfiBluetoothConIstBluetoothDiscovery(int32_t* errCode);
    FFI_EXPORT void FfiBluetoothConSetRemoteDeviceName(char* deviceId, char* name, int32_t* errCode);
    FFI_EXPORT CBatteryInfo FfiBluetoothConGetRemoteDeviceBatteryInfo(char* deviceId, int32_t* errCode);
}

#endif // BLUETOOTH_CONNECTION_FFI_H