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
#ifndef LOG_TAG
#define LOG_TAG "bt_cj_connection_ffi"
#endif

#include "bluetooth_connection_ffi.h"

#include "bluetooth_connection_impl.h"
#include "bluetooth_log.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothConnection {

extern "C" {
void FfiBluetoothConPairDevice(const char* deviceId, int32_t* errCode)
{
    return ConnectionImpl::PairDevice(deviceId, errCode);
}

char* FfiBluetoothConGetRemoteDeviceName(const char* deviceId, int32_t* errCode)
{
    return ConnectionImpl::GetRemoteDeviceName(deviceId, errCode);
}

DeviceClass FfiBluetoothConGetRemoteDeviceClass(const char* deviceId, int32_t* errCode)
{
    return ConnectionImpl::GetRemoteDeviceClass(deviceId, errCode);
}

CArrString FfiBluetoothConGetRemoteProfileUuids(const char* deviceId, int32_t* errCode)
{
    return ConnectionImpl::GetRemoteProfileUuids(deviceId, errCode);
}

char* FfiBluetoothConGetLocalName(int32_t* errCode)
{
    return ConnectionImpl::GetLocalName(errCode);
}

CArrString FfiBluetoothConGetPairedDevices(int32_t* errCode)
{
    return ConnectionImpl::GetPairedDevices(errCode);
}

int32_t FfiBluetoothConGetPairState(const char* deviceId, int32_t* errCode)
{
    return ConnectionImpl::GetPairState(deviceId, errCode);
}

int32_t FfiBluetoothConGetProfileConnectionState(int32_t profileId, int32_t* errCode)
{
    return ConnectionImpl::GetProfileConnectionState(profileId, errCode);
}

void FfiBluetoothConSetDevicePairingConfirmation(const char* deviceId, bool accept, int32_t* errCode)
{
    return ConnectionImpl::SetDevicePairingConfirmation(deviceId, accept, errCode);
}

void FfiBluetoothConSetDevicePinCode(const char* deviceId, const char* code, int32_t* errCode)
{
    return ConnectionImpl::SetDevicePinCode(deviceId, code, errCode);
}

void FfiBluetoothConSetLocalName(const char* localName, int32_t* errCode)
{
    return ConnectionImpl::SetLocalName(localName, errCode);
}

void FfiBluetoothConSetBluetoothScanMode(int32_t mode, int32_t duration, int32_t* errCode)
{
    return ConnectionImpl::SetBluetoothScanMode(mode, duration, errCode);
}

int32_t FfiBluetoothConGetBluetoothScanMode(int32_t* errCode)
{
    return ConnectionImpl::GetBluetoothScanMode(errCode);
}

void FfiBluetoothConStartBluetoothDiscovery(int32_t* errCode)
{
    return ConnectionImpl::StartBluetoothDiscovery(errCode);
}

void FfiBluetoothConStopBluetoothDiscovery(int32_t* errCode)
{
    return ConnectionImpl::StoptBluetoothDiscovery(errCode);
}

bool FfiBluetoothConIsBluetoothDiscovering(int32_t* errCode)
{
    return ConnectionImpl::IsBluetoothDiscovering(errCode);
}

void FfiBluetoothConSetRemoteDeviceName(const char* deviceId, const char* name, int32_t* errCode)
{
    return ConnectionImpl::SetRemoteDeviceName(deviceId, name, errCode);
}

CBatteryInfo FfiBluetoothConGetRemoteDeviceBatteryInfo(const char* deviceId, int32_t* errCode)
{
    return ConnectionImpl::GetRemoteDeviceBatteryInfo(deviceId, errCode);
}

void FfiBluetoothConOn(int32_t callbackType, void (*callback)(), int32_t* errCode)
{
    return ConnectionImpl::RegisterConnectionObserver(callbackType, callback, errCode);
}
}
} // namespace BluetoothConnection
} // namespace CJSystemapi
} // namespace OHOS