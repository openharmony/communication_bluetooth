/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#include "bluetooth_connection_ffi.h"
#include "bluetooth_log.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace CJSystemapi {
namespace BluetoothConnection {

extern "C" {
void FfiBluetoothConPairDevice(char* deviceId, int32_t* errCode)
{
    return;
}

char* FfiBluetoothConGetRemoteDeviceName(char* deviceId, int32_t* errCode)
{
    return nullptr;
}

DeviceClass FfiBluetoothConGetRemoteDeviceClass(char* deviceId, int32_t* errCode)
{
    DeviceClass ret{0};
    return ret;
}

CArrString FfiBluetoothConGetRemoteProfileUuids(char* deviceId, int32_t* errCode)
{
    CArrString ret{0};
    return ret;
}

char* FfiBluetoothConGetLocalName(int32_t* errCode)
{
    return nullptr;
}

CArrString FfiBluetoothConGetPairedDevices(int32_t* errCode)
{
    CArrString ret{0};
    return ret;
}

int32_t FfiBluetoothConGetPairedState(char* deviceId, int32_t* errCode)
{
    return 0;
}

int32_t FfiBluetoothConGetProfileConnectionState(int32_t profileIdId, int32_t* errCode)
{
    return 0;
}

void FfiBluetoothConSetDevicePairingConfirmation(char* deviceId, bool accept, int32_t* errCode)
{
    return;
}

void FfiBluetoothConSetDevicePinCode(char* deviceId, char* code, int32_t* errCode)
{
    return;
}

void FfiBluetoothConSetLocalName(char* name, int32_t* errCode)
{
    return;
}

void FfiBluetoothConSetBluetoothScanMode(int32_t mode, int32_t duration, int32_t* errCode)
{
    return;
}

int32_t FfiBluetoothConGetBluetoothScanMode(int32_t* errCode)
{
    return 0;
}

void FfiBluetoothConStartBluetoothDiscovery(int32_t* errCode)
{
    return;
}

void FfiBluetoothConStoptBluetoothDiscovery(int32_t* errCode)
{
    return;
}

bool FfiBluetoothConIstBluetoothDiscovery(int32_t* errCode)
{
    return false;
}

void FfiBluetoothConSetRemoteDeviceName(char* deviceId, char* name, int32_t* errCode)
{
    return;
}

CBatteryInfo FfiBluetoothConGetRemoteDeviceBatteryInfo(char* deviceId, int32_t* errCode)
{
    CBatteryInfo info{0};
    return info;
}
}

} // namespace BluetoothConnection
} // namespace CJSystemapi
} // namespace OHOS