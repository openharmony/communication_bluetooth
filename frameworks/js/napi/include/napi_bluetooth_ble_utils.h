/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef NAPI_BLUETOOTH_BLE_UTILS_H
#define NAPI_BLUETOOTH_BLE_UTILS_H

#include "bluetooth_ble_central_manager.h"
#include "bluetooth_gatt_characteristic.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_gatt_descriptor.h"
#include "bluetooth_gatt_server.h"
#include "bluetooth_gatt_service.h"
#include "bluetooth_log.h"
#include "napi_native_object.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <cstdint>
#include <string>
#include <vector>
#include "uv.h"


namespace OHOS {
namespace Bluetooth {
namespace {
std::string deviceAddr;
} // namespace

enum class NapiGattWriteType {
    WRITE = 1,
    WRITE_NO_RESPONSE = 2,
};

const char * const REGISTER_BLE_FIND_DEVICE_TYPE = "BLEDeviceFind";
const char * const REGISTER_SYS_BLE_SCAN_TYPE = "sysBLEScan";
const char * const REGISTER_SYS_BLE_FIND_DEVICE_TYPE = "sysBLEDeviceFonud";
const char * const REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE = "advertisingStateChange";

void ConvertGattServiceToJS(napi_env env, napi_value result, GattService &service);
void ConvertGattServiceVectorToJS(napi_env env, napi_value result, std::vector<GattService> &services);
void ConvertBLECharacteristicToJS(napi_env env, napi_value result, GattCharacteristic &characteristic);
void ConvertBLECharacteristicVectorToJS(
    napi_env env, napi_value result, std::vector<GattCharacteristic> &characteristics);
void ConvertBLEDescriptorToJS(napi_env env, napi_value result, GattDescriptor &descriptor);
void ConvertBLEDescriptorVectorToJS(napi_env env, napi_value result, std::vector<GattDescriptor> &descriptors);
void ConvertCharacteristicReadReqToJS(napi_env env, napi_value result, const std::string &device,
    GattCharacteristic &characteristic, int requestId);
void ConvertCharacteristicWriteReqToJS(napi_env env, napi_value result, const std::string &device,
    GattCharacteristic &characteristic, int requestId);
void ConvertDescriptorReadReqToJS(
    napi_env env, napi_value result, const std::string &device, GattDescriptor &descriptor, int requestId);
void ConvertDescriptorWriteReqToJS(
    napi_env env, napi_value result, const std::string &device, GattDescriptor &descriptor, int requestId);

napi_value ScanReportTypeInit(napi_env env);
napi_value ScanDutyInit(napi_env env);
napi_value MatchModeInit(napi_env env);
napi_value PhyTypeInit(napi_env env);
napi_value ScanReportModeInit(napi_env env);

void SetGattClientDeviceId(const std::string &deviceId);
std::string GetGattClientDeviceId();
napi_status CheckBleScanParams(napi_env env, napi_callback_info info, std::vector<BleScanFilter> &outScanfilters,
    BleScanSettings &outSettinngs);

struct GattCharacteristicCallbackInfo : public BluetoothCallbackInfo {
    GattCharacteristic characteristic_ = {UUID::FromString("0"), 0, 0};
};

struct GattDescriptorCallbackInfo : public BluetoothCallbackInfo {
    GattDescriptor descriptor_ = {UUID::FromString("0"), 0};
};

class NapiNativeBleCharacteristic : public NapiNativeObject {
public:
    NapiNativeBleCharacteristic(const GattCharacteristic &character) : character_(character) {}
    ~NapiNativeBleCharacteristic() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    GattCharacteristic character_;
};

class NapiNativeBleDescriptor : public NapiNativeObject {
public:
    NapiNativeBleDescriptor(const GattDescriptor &descriptor) : descriptor_(descriptor) {}
    ~NapiNativeBleDescriptor() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    GattDescriptor descriptor_;
};

class NapiNativeBleScanResult : public NapiNativeObject {
public:
    NapiNativeBleScanResult(const BleScanResult &scanResult) : scanResult_(scanResult) {}
    ~NapiNativeBleScanResult() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    BleScanResult scanResult_;
};

class NapiNativeBleScanReport : public NapiNativeObject {
public:
    NapiNativeBleScanReport(const BleScanResult &scanResult, ScanReportType scanReportType)
        : scanResult_(scanResult), scanReportType_(scanReportType) {}
    ~NapiNativeBleScanReport() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    BleScanResult scanResult_;
    ScanReportType scanReportType_;
};

class NapiNativeGattServiceArray : public NapiNativeObject {
public:
    NapiNativeGattServiceArray(const std::vector<GattService> &gattServices) : gattServices_(gattServices) {}
    ~NapiNativeGattServiceArray() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    std::vector<GattService> gattServices_ {};
};

class NapiNativeAdvertisingStateInfo : public NapiNativeObject {
public:
    NapiNativeAdvertisingStateInfo(int advHandle, int advState) : advHandle_(advHandle), advState_(advState) {}
    ~NapiNativeAdvertisingStateInfo() override = default;

    napi_value ToNapiValue(napi_env env) const override;
private:
    int advHandle_;
    int advState_;
};

}  // namespace Bluetooth
}  // namespace OHOS
#endif  // NAPI_BLUETOOTH_BLE_UTILS_H