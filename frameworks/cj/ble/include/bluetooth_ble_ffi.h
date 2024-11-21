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

#ifndef BLUETOOTH_BLE_FFI_H
#define BLUETOOTH_BLE_FFI_H

#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"

#include <cstdint>

extern "C" {
typedef struct {
    char *deviceId;
    char *name;
    char *serviceUuid;
    char *serviceUuidMask;
    char *serviceSolicitationUuid;
    char *serviceSolicitationUuidMask;
    CArrUI8 serviceData;
    CArrUI8 serviceDataMask;
    uint16_t manufactureId;
    CArrUI8 manufactureData;
    CArrUI8 manufactureDataMask;
} NativeScanFilter;

typedef struct {
    NativeScanFilter *head;
    int64_t size;
} CArrNativeScanFilter;

typedef struct {
    int32_t interval;
    int32_t dutyMode;
    int32_t matchMode;
    int32_t phyType;
} NativeScanOptions;

typedef struct {
    uint16_t interval;
    int8_t txPower;
    bool connectable;
} NativeAdvertiseSetting;

typedef struct {
    uint16_t manufactureId;
    CArrUI8 manufactureValue;
} NativeManufactureData;

typedef struct {
    NativeManufactureData *head;
    int64_t size;
} CArrNativeManufactureData;

typedef struct {
    char *serviceUuid;
    CArrUI8 serviceValue;
} NativeServiceData;

typedef struct {
    NativeServiceData *head;
    int64_t size;
} CArrNativeServiceData;

typedef struct {
    CArrString serviceUuids;
    CArrNativeManufactureData manufactureData;
    CArrNativeServiceData serviceData;
    bool includeDeviceName;
} NativeAdvertiseData;

typedef struct {
    NativeAdvertiseSetting advertisingSettings;
    NativeAdvertiseData advertisingData;
    NativeAdvertiseData advertisingResponse;
    uint16_t duration;
} NativeAdvertisingParams;

typedef struct {
    uint32_t advertisingId;
    uint16_t duration;
} NativeAdvertisingEnableParams;

typedef struct {
    uint32_t advertisingId;
} NativeAdvertisingDisableParams;

typedef struct {
    int32_t advertisingId;
    int32_t state;
} CAdvertisingStateChangeInfo;

typedef struct {
    char *deviceId;
    int32_t rssi;
    CArrUI8 data;
    char *deviceName;
    bool connectable;
} NativeScanResult;

typedef struct {
    NativeScanResult *head;
    int64_t size;
} CArrScanResult;

typedef struct {
    char *serviceUuid;
    char *characteristicUuid;
    char *descriptorUuid;
    CArrUI8 descriptorValue;
} NativeBLEDescriptor;

typedef struct {
    int32_t code;
    NativeBLEDescriptor data;
} RetNativeBLEDescriptor;

typedef struct {
    NativeBLEDescriptor *head;
    int64_t size;
} CArrBLEDescriptor;

typedef struct {
    bool write;
    bool writeNoResponse;
    bool read;
    bool notify;
    bool indicate;
} NativeGattProperties;

typedef struct {
    char *serviceUuid;
    char *characteristicUuid;
    CArrUI8 characteristicValue;
    CArrBLEDescriptor descriptors;
    NativeGattProperties properties;
} NativeBLECharacteristic;

typedef struct {
    int32_t code;
    NativeBLECharacteristic data;
} RetNativeBLECharacteristic;

typedef struct {
    NativeBLECharacteristic *head;
    int64_t size;
} CArrBLECharacteristic;

struct NativeGattService;

struct CArrGattService {
    NativeGattService *head;
    int64_t size;
};

struct NativeGattService {
    char *serviceUuid;
    bool isPrimary;
    CArrBLECharacteristic characteristics;
    CArrGattService includeServices;
};

typedef struct {
    char *serviceUuid;
    char *characteristicUuid;
    CArrUI8 characteristicValue;
    bool confirm;
} NativeNotifyCharacteristic;

typedef struct {
    char *deviceId;
    int32_t transId;
    int32_t status;
    int32_t offset;
    CArrUI8 value;
    bool confirm;
} NativeServerResponse;

typedef struct {
    char *deviceId;
    int32_t transId;
    int32_t offset;
    char *characteristicUuid;
    char *serviceUuid;
} NativeCharacteristicReadRequest;

typedef struct {
    char *deviceId;
    int32_t transId;
    int32_t offset;
    bool isPrepared;
    bool needRsp;
    CArrUI8 value;
    char *characteristicUuid;
    char *serviceUuid;
} NativeCharacteristicWriteRequest;

typedef struct {
    char *deviceId;
    int32_t transId;
    int32_t offset;
    char *descriptorUuid;
    char *characteristicUuid;
    char *serviceUuid;
} NativeDescriptorReadRequest;

typedef struct {
    char *deviceId;
    int32_t transId;
    int32_t offset;
    bool isPrepared;
    bool needRsp;
    CArrUI8 value;
    char *descriptorUuid;
    char *characteristicUuid;
    char *serviceUuid;
} NativeDescriptorWriteRequest;

typedef struct {
    char *deviceId;
    int32_t state;
} NativeBLEConnectionChangeState;

const int32_t REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE = 0;
const int32_t REGISTER_BLE_FIND_DEVICE_TYPE = 1;

const int32_t CHARACTERISTIC_READ = 0;
const int32_t CHARACTERISTIC_WRITE = 1;
const int32_t DESCRIPTOR_READ = 2;
const int32_t DESCRIPTOR_WRITE = 3;
const int32_t CONNECTION_STATE_CHANGE = 4;
const int32_t SERVER_BLE_MTU_CHANGE = 5;

const int32_t BLE_CHARACTERISTIC_CHANGE = 0;
const int32_t BLE_CONNECTION_STATE_CHANGE = 1;
const int32_t CLIENT_BLE_MTU_CHANGE = 2;

struct NativeGattPermission {
    bool readable = false;
    bool writeable = false;
    bool readEncrypted = false;
    bool writeEncrypted = false;
};

FFI_EXPORT int64_t FfiBluetoothBleCreateGattServer(int32_t *errCode);
FFI_EXPORT int64_t FfiBluetoothBleCreateGattClientDevice(const char *deviceId, int32_t *errCode);
FFI_EXPORT CArrString FfiBluetoothBleGetConnectedBleDevices(int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleStartBleScan(CArrNativeScanFilter filters, NativeScanOptions *options, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleStopBleScan(int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleStartAdvertising(NativeAdvertiseSetting setting, NativeAdvertiseData advData,
    NativeAdvertiseData *advResponse, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleStopAdvertising(int32_t *errCode);
FFI_EXPORT int32_t FfiBluetoothBleStartAdvertisingWithId(NativeAdvertisingParams advertisingParams, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleEnableAdvertising(NativeAdvertisingEnableParams advertisingEnableParams,
    int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleDisableAdvertising(NativeAdvertisingDisableParams advertisingDisableParams,
    int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleStopAdvertisingWithId(uint32_t advertisingId, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleOn(int32_t callbackType, void (*callback)(), int32_t *errCode);

FFI_EXPORT void FfiBluetoothBleGattClientDeviceConnect(int64_t id, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceDisconnect(int64_t id, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceClose(int64_t id, int32_t *errCode);
FFI_EXPORT char *FfiBluetoothBleGattClientDeviceGetDeviceName(int64_t id, int32_t *errCode);
FFI_EXPORT CArrGattService FfiBluetoothBleGattClientDeviceGetServices(int64_t id, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceReadCharacteristicValue(int64_t id,
    NativeBLECharacteristic characteristic, void (*callback)(), int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceReadDescriptorValue(int64_t id, NativeBLEDescriptor descriptor,
    void (*callback)(), int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceWriteCharacteristicValue(int64_t id,
    NativeBLECharacteristic characteristic, int32_t writeType, void (*callback)(), int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceWriteDescriptorValue(int64_t id, NativeBLEDescriptor descriptor,
    void (*callback)(), int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceGetRssiValue(int64_t id, void (*callback)(), int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceSetBLEMtuSize(int64_t id, int32_t mut, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceSetCharacteristicChangeNotification(int64_t id,
    NativeBLECharacteristic characteristic, bool enable, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceSetCharacteristicChangeIndication(int64_t id,
    NativeBLECharacteristic characteristic, bool enable, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattClientDeviceOn(int64_t id, int32_t callbackType, void (*callback)(),
    int32_t *errCode);

FFI_EXPORT void FfiBluetoothBleGattServerAddService(int64_t id, NativeGattService service, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattServerRemoveService(int64_t id, char *serviceUuid, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattServerClose(int64_t id, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattServerNotifyCharacteristicChanged(int64_t id, char *deviceId,
    NativeNotifyCharacteristic characteristic, int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattServerSendResponse(int64_t id, NativeServerResponse serverResponse,
    int32_t *errCode);
FFI_EXPORT void FfiBluetoothBleGattServerOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errCode);
}

#endif // BLUETOOTH_BLE_FFI_H