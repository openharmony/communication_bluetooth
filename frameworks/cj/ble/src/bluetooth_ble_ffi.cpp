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

#include "bluetooth_ble_ffi.h"

#include "bluetooth_ble_common.h"
#include "bluetooth_ble_impl.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BT_ERR_INTERNAL_ERROR;
using Bluetooth::BT_NO_ERROR;

extern "C" {
int64_t FfiBluetoothBleCreateGattServer(int32_t *errCode)
{
    FfiGattServer *nativeGattServer = nullptr;
    *errCode = BleImpl::CreateGattServer(nativeGattServer);
    if (*errCode != BT_NO_ERROR) {
        return -1;
    }
    return nativeGattServer->GetID();
}

int64_t FfiBluetoothBleCreateGattClientDevice(const char *deviceId, int32_t *errCode)
{
    FfiClientDevice *nativeGattClientDevice = nullptr;
    *errCode = BleImpl::CreateGattClientDevice(deviceId, nativeGattClientDevice);
    if (*errCode != BT_NO_ERROR) {
        return -1;
    }
    return nativeGattClientDevice->GetID();
}

CArrString FfiBluetoothBleGetConnectedBleDevices(int32_t *errCode)
{
    CArrString res{};
    *errCode = BleImpl::GetConnectedBleDevices(res);
    return res;
}

void FfiBluetoothBleStartBleScan(CArrNativeScanFilter filters, NativeScanOptions *options, int32_t *errCode)
{
    *errCode = BleImpl::StartBleScan(filters, options);
    return;
}

void FfiBluetoothBleStopBleScan(int32_t *errCode)
{
    *errCode = BleImpl::StopBleScan();
    return;
}

void FfiBluetoothBleStartAdvertising(NativeAdvertiseSetting setting, NativeAdvertiseData advData,
                                     NativeAdvertiseData *advResponse, int32_t *errCode)
{
    *errCode = BleImpl::StartAdvertising(setting, advData, advResponse);
    return;
}

void FfiBluetoothBleStopAdvertising(int32_t *errCode)
{
    *errCode = BleImpl::StopAdvertising();
    return;
}

int32_t FfiBluetoothBleStartAdvertisingWithId(NativeAdvertisingParams advertisingParams, int32_t *errCode)
{
    int32_t id = -1;
    *errCode = BleImpl::StartAdvertising(advertisingParams, id);
    return id;
}

void FfiBluetoothBleEnableAdvertising(NativeAdvertisingEnableParams advertisingEnableParams, int32_t *errCode)
{
    *errCode = BleImpl::EnableAdvertising(advertisingEnableParams);
    return;
}

void FfiBluetoothBleDisableAdvertising(NativeAdvertisingDisableParams advertisingDisableParams, int32_t *errCode)
{
    *errCode = BleImpl::DisableAdvertising(advertisingDisableParams);
    return;
}

void FfiBluetoothBleStopAdvertisingWithId(uint32_t advertisingId, int32_t *errCode)
{
    *errCode = BleImpl::StopAdvertising(advertisingId);
    return;
}

void FfiBluetoothBleOn(int32_t callbackType, void (*callback)(), int32_t *errCode)
{
    *errCode = BleImpl::RegisterBleObserver(callbackType, callback);
    return;
}

void FfiBluetoothBleGattClientDeviceConnect(int64_t id, int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->Connect();
    return;
}

void FfiBluetoothBleGattClientDeviceDisconnect(int64_t id, int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->Disconnect();
    return;
}

void FfiBluetoothBleGattClientDeviceClose(int64_t id, int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->Close();
    return;
}

char *FfiBluetoothBleGattClientDeviceGetDeviceName(int64_t id, int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return nullptr;
    }
    auto name = clientDevice->GetDeviceName(errCode);
    return MallocCString(name);
}

CArrGattService FfiBluetoothBleGattClientDeviceGetServices(int64_t id, int32_t *errCode)
{
    CArrGattService service{};
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return service;
    }
    *errCode = clientDevice->GetServices(service);
    return service;
}

void FfiBluetoothBleGattClientDeviceReadCharacteristicValue(int64_t id, NativeBLECharacteristic characteristic,
                                                            void (*callback)(), int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->ReadCharacteristicValue(characteristic, callback);
    return;
}

void FfiBluetoothBleGattClientDeviceReadDescriptorValue(int64_t id, NativeBLEDescriptor descriptor, void (*callback)(),
                                                        int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->ReadDescriptorValue(descriptor, callback);
    return;
}

void FfiBluetoothBleGattClientDeviceWriteCharacteristicValue(int64_t id, NativeBLECharacteristic characteristic,
                                                             int32_t writeType, void (*callback)(), int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->WriteCharacteristicValue(characteristic, writeType, callback);
    return;
}

void FfiBluetoothBleGattClientDeviceWriteDescriptorValue(int64_t id, NativeBLEDescriptor descriptor, void (*callback)(),
                                                         int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->WriteDescriptorValue(descriptor, callback);
    return;
}

void FfiBluetoothBleGattClientDeviceGetRssiValue(int64_t id, void (*callback)(), int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->GetRssiValue(callback);
    return;
}

void FfiBluetoothBleGattClientDeviceSetBLEMtuSize(int64_t id, int32_t mut, int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->SetBLEMtuSize(mut);
    return;
}

void FfiBluetoothBleGattClientDeviceSetCharacteristicChangeNotification(int64_t id,
                                                                        NativeBLECharacteristic characteristic,
                                                                        bool enable, int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->SetCharacteristicChangeNotification(characteristic, enable);
    return;
}

void FfiBluetoothBleGattClientDeviceSetCharacteristicChangeIndication(int64_t id,
                                                                      NativeBLECharacteristic characteristic,
                                                                      bool enable, int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->SetCharacteristicChangeIndication(characteristic, enable);
    return;
}

void FfiBluetoothBleGattClientDeviceOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errCode)
{
    auto clientDevice = FFIData::GetData<FfiClientDevice>(id);
    if (clientDevice == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = clientDevice->RegisterBleGattClientDeviceObserver(callbackType, callback);
    return;
}

void FfiBluetoothBleGattServerAddService(int64_t id, NativeGattService service, int32_t *errCode)
{
    auto gattServer = FFIData::GetData<FfiGattServer>(id);
    if (gattServer == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = gattServer->AddService(service);
    return;
}

void FfiBluetoothBleGattServerRemoveService(int64_t id, char *serviceUuid, int32_t *errCode)
{
    auto gattServer = FFIData::GetData<FfiGattServer>(id);
    if (gattServer == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = gattServer->RemoveService(serviceUuid);
    return;
}

void FfiBluetoothBleGattServerClose(int64_t id, int32_t *errCode)
{
    auto gattServer = FFIData::GetData<FfiGattServer>(id);
    if (gattServer == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = gattServer->Close();
    return;
}

void FfiBluetoothBleGattServerNotifyCharacteristicChanged(int64_t id, char *deviceId,
                                                          NativeNotifyCharacteristic characteristic, int32_t *errCode)
{
    auto gattServer = FFIData::GetData<FfiGattServer>(id);
    if (gattServer == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = gattServer->NotifyCharacteristicChanged(deviceId, characteristic);
    return;
}

void FfiBluetoothBleGattServerSendResponse(int64_t id, NativeServerResponse serverResponse, int32_t *errCode)
{
    auto gattServer = FFIData::GetData<FfiGattServer>(id);
    if (gattServer == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = gattServer->SendResponse(serverResponse);
    return;
}

void FfiBluetoothBleGattServerOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errCode)
{
    auto gattServer = FFIData::GetData<FfiGattServer>(id);
    if (gattServer == nullptr) {
        *errCode = BT_ERR_INTERNAL_ERROR;
        return;
    }
    *errCode = gattServer->RegisterBleGattServerObserver(callbackType, callback);
    return;
}
}
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS