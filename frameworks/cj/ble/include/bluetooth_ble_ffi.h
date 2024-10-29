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
}

#endif // BLUETOOTH_BLE_FFI_H