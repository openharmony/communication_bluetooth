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

#include "bluetooth_ble_impl.h"

#include "bluetooth_ble_central_manager.h"
#include "bluetooth_ble_common.h"
#include "bluetooth_ble_ffi.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"
#include "cj_lambda.h"
#include "napi_bluetooth_utils.h"

namespace OHOS {
namespace CJSystemapi {
namespace CJBluetoothBle {
using Bluetooth::BleAdvertiser;
using Bluetooth::BleAdvertiserData;
using Bluetooth::BleScanFilter;
using Bluetooth::BleScanSettings;
using Bluetooth::IsValidAddress;
using Bluetooth::IsValidUuid;
using Bluetooth::ParcelUuid;
using OHOS::FFI::FFIData;

using Bluetooth::NapiAdvManufactureData;
using Bluetooth::NapiAdvServiceData;

using Bluetooth::BT_ERR_BLE_SCAN_ALREADY_STARTED;
using Bluetooth::BT_ERR_INTERNAL_ERROR;
using Bluetooth::BT_ERR_INVALID_PARAM;
using Bluetooth::BT_NO_ERROR;

namespace {
using Bluetooth::BleCentralManager;

std::shared_ptr<BleAdvertiser> BleAdvertiserGetInstance(void)
{
    static auto instance = BleAdvertiser::CreateInstance();
    return instance;
}

BleCentralManager *BleCentralManagerGetInstance(void)
{
    static BleCentralManager instance(FfiBluetoothBleCentralManagerCallback::GetInstance());
    return &instance;
}
} // namespace

int32_t BleImpl::CreateGattServer(FfiGattServer *&ffiGattServer)
{
    ffiGattServer = FFIData::Create<FfiGattServer>();
    if (ffiGattServer == nullptr) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BleImpl::CreateGattClientDevice(std::string deviceId, FfiClientDevice *&ffiClientDevice)
{
    if (!IsValidAddress(deviceId)) {
        HILOGE("Invalid deviceId: %{public}s", deviceId.c_str());
        return BT_ERR_INVALID_PARAM;
    }
    ffiClientDevice = FFIData::Create<FfiClientDevice>(deviceId);
    return BT_NO_ERROR;
}

int32_t BleImpl::GetConnectedBleDevices(CArrString &res)
{
    std::lock_guard<std::mutex> lock(FfiGattServer::deviceListMutex_);
    std::vector<std::string> devicesList = FfiGattServer::deviceList_;
    res = Convert2CArrString(devicesList);
    return BT_NO_ERROR;
}

static void ParseScanFilterDeviceIdParameters(int32_t &status, NativeScanFilter &scanFilter,
                                              BleScanFilter &bleScanFilter)
{
    if (scanFilter.deviceId != nullptr) {
        std::string deviceId = std::string(scanFilter.deviceId);
        if (!IsValidAddress(deviceId)) {
            HILOGE("Invalid deviceId: %{public}s", deviceId.c_str());
            status = BT_ERR_INVALID_PARAM;
        }
        bleScanFilter.SetDeviceId(deviceId);
    }
}

static void ParseScanFilterLocalNameParameters(int32_t &status, NativeScanFilter &scanFilter,
                                               BleScanFilter &bleScanFilter)
{
    if (scanFilter.name != nullptr) {
        std::string name = std::string(scanFilter.name);
        if (name.empty()) {
            HILOGE("name is empty");
            status = BT_ERR_INVALID_PARAM;
        }
        bleScanFilter.SetName(name);
    }
}

static int32_t ParseUuidParams(char *name, UUID &outUuid)
{
    std::string uuid = std::string(name);
    if (!IsValidUuid(uuid)) {
        HILOGE("match the UUID faild.");
        return BT_ERR_INVALID_PARAM;
    }
    outUuid = ParcelUuid::FromString(uuid);
    return BT_NO_ERROR;
}

static void ParseScanFilterServiceUuidParameters(int32_t &status, NativeScanFilter &scanFilter,
                                                 BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceUuid != nullptr) {
        UUID uuid{};
        status = ParseUuidParams(scanFilter.serviceUuid, uuid);
        if (status != BT_NO_ERROR) {
            return;
        }
        bleScanFilter.SetServiceUuid(uuid);
    }

    if (scanFilter.serviceUuidMask != nullptr) {
        UUID uuidMask{};
        status = ParseUuidParams(scanFilter.serviceUuidMask, uuidMask);
        if (status != BT_NO_ERROR) {
            return;
        }
        bleScanFilter.SetServiceUuidMask(uuidMask);
    }
}

static void ParseScanFilterSolicitationUuidParameters(int32_t &status, NativeScanFilter &scanFilter,
                                                      BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceSolicitationUuid != nullptr) {
        UUID uuid{};
        status = ParseUuidParams(scanFilter.serviceSolicitationUuid, uuid);
        if (status != BT_NO_ERROR) {
            return;
        }
        bleScanFilter.SetServiceSolicitationUuid(uuid);
    }

    if (scanFilter.serviceSolicitationUuidMask != nullptr) {
        UUID uuidMask{};
        status = ParseUuidParams(scanFilter.serviceSolicitationUuidMask, uuidMask);
        if (status != BT_NO_ERROR) {
            return;
        }
        bleScanFilter.SetServiceSolicitationUuidMask(uuidMask);
    }
}

static void ParseScanFilterServiceDataParameters(int32_t &status, NativeScanFilter &scanFilter,
                                                 BleScanFilter &bleScanFilter)
{
    std::vector<uint8_t> data{};
    if (scanFilter.serviceData.head != nullptr) {
        bleScanFilter.SetServiceData(std::move(data));
    }

    std::vector<uint8_t> dataMask{};
    if (scanFilter.serviceDataMask.head != nullptr) {
        bleScanFilter.SetServiceDataMask(std::move(dataMask));
    }
}

static void ParseScanFilterManufactureDataParameters(int32_t &status, NativeScanFilter &scanFilter,
                                                     BleScanFilter &bleScanFilter)
{
    if (scanFilter.manufactureId != 0) {
        bleScanFilter.SetManufacturerId(scanFilter.manufactureId);
        HILOGI("Scan filter manufacturerId is %{public}#x", scanFilter.manufactureId);
    }

    std::vector<uint8_t> data{};
    if (scanFilter.manufactureData.head != nullptr) {
        for (int64_t i = 0; i < scanFilter.manufactureData.size; i++) {
            data.push_back(scanFilter.manufactureData.head[i]);
        }
        bleScanFilter.SetManufactureData(std::move(data));
    }

    std::vector<uint8_t> dataMask{};
    if (scanFilter.manufactureDataMask.head != nullptr) {
        for (int64_t i = 0; i < scanFilter.manufactureDataMask.size; i++) {
            dataMask.push_back(scanFilter.manufactureDataMask.head[i]);
        }
        bleScanFilter.SetManufactureDataMask(std::move(dataMask));
    }
}

static void ParseScanFilter(int32_t &status, NativeScanFilter &scanFilter, BleScanFilter &bleScanFilter)
{
    ParseScanFilterDeviceIdParameters(status, scanFilter, bleScanFilter);
    ParseScanFilterLocalNameParameters(status, scanFilter, bleScanFilter);
    ParseScanFilterServiceUuidParameters(status, scanFilter, bleScanFilter);
    ParseScanFilterSolicitationUuidParameters(status, scanFilter, bleScanFilter);
    ParseScanFilterServiceDataParameters(status, scanFilter, bleScanFilter);
    ParseScanFilterManufactureDataParameters(status, scanFilter, bleScanFilter);
}

static int32_t CheckBleScanParams(CArrNativeScanFilter filters, NativeScanOptions *scanOptions,
                                  std::vector<BleScanFilter> &outScanfilters, BleScanSettings &outSettinngs)
{
    std::vector<BleScanFilter> scanfilters;
    uint32_t length = static_cast<uint32_t>(filters.size);
    int32_t status = BT_NO_ERROR;
    for (uint32_t i = 0; i < length; i++) {
        BleScanFilter bleScanFilter;
        ParseScanFilter(status, filters.head[i], bleScanFilter);
        scanfilters.push_back(bleScanFilter);
    }
    if (status != BT_NO_ERROR) {
        return status;
    }

    if (scanOptions != nullptr) {
        outSettinngs.SetReportDelay(scanOptions->interval);
        outSettinngs.SetScanMode(scanOptions->dutyMode);
        outSettinngs.SetPhy(scanOptions->phyType);
    }

    outScanfilters = std::move(scanfilters);
    return BT_NO_ERROR;
}

int32_t BleImpl::StartBleScan(CArrNativeScanFilter filters, NativeScanOptions *options)
{
    std::vector<BleScanFilter> scanfilters;
    BleScanSettings settings;

    int status = CheckBleScanParams(filters, options, scanfilters, settings);
    if (status != BT_NO_ERROR) {
        return status;
    }

    int ret = BleCentralManagerGetInstance()->StartScan(settings, scanfilters);
    if (ret != BT_NO_ERROR && ret != BT_ERR_BLE_SCAN_ALREADY_STARTED) {
        return ret;
    }
    return BT_NO_ERROR;
}

int32_t BleImpl::StopBleScan()
{
    return BleCentralManagerGetInstance()->StopScan();
}

static void ParseAdvertisingSettingsParameters(int32_t *errCode, NativeAdvertiseSetting &setting,
                                               BleAdvertiserSettings &outSettings)
{
    uint32_t interval = setting.interval;
    const uint32_t minInterval = 32;
    const uint32_t maxInterval = 16384;
    if (interval < minInterval || interval > maxInterval) {
        HILOGE("Invalid interval: %{public}d", interval);
        *errCode = BT_ERR_INVALID_PARAM;
        return;
    }
    HILOGI("interval: %{public}u", interval);
    outSettings.SetInterval(interval);

    int32_t txPower = setting.txPower;
    const int32_t minTxPower = -127;
    const int32_t maxTxPower = 1;
    if (txPower < minTxPower || txPower > maxTxPower) {
        HILOGE("Invalid tx power: %{public}d", txPower);
        *errCode = BT_ERR_INVALID_PARAM;
        return;
    }
    HILOGI("txPower is %{public}d", txPower);
    outSettings.SetTxPower(txPower);

    bool connectable = setting.connectable;
    HILOGI("connectable: %{public}d", connectable);
    outSettings.SetConnectable(connectable);

    return;
}

static void ParseServiceUuidParameters(int32_t *errCode, NativeAdvertiseData advData, BleAdvertiserData &outData)
{
    for (int64_t i = 0; i < advData.serviceUuids.size; ++i) {
        UUID uuid{};
        *errCode = ParseUuidParams(advData.serviceUuids.head[i], uuid);
        if (*errCode != BT_NO_ERROR) {
            return;
        }
        outData.AddServiceUuid(uuid);
        HILOGI("Service Uuid = %{public}s", uuid.ToString().c_str());
    }
    return;
}

static void ParseManufactureDataParameters(int32_t *errCode, NativeAdvertiseData advData, BleAdvertiserData &outData)
{
    CArrNativeManufactureData parameter = advData.manufactureData;
    for (int64_t i = 0; i < parameter.size; ++i) {
        NativeManufactureData manufactureData = parameter.head[i];
        CArrUI8 manufactureValue = manufactureData.manufactureValue;
        std::string value(reinterpret_cast<char *>(manufactureValue.head), manufactureValue.size);
        outData.AddManufacturerData(manufactureData.manufactureId, value);
    }
    return;
}

static void ParseServiceDataParameters(int32_t *errCode, NativeAdvertiseData advData, BleAdvertiserData &outData)
{
    CArrNativeServiceData data = advData.serviceData;
    for (int64_t i = 0; i < data.size; ++i) {
        NativeServiceData serviceData = data.head[i];
        std::string value(reinterpret_cast<char *>(serviceData.serviceValue.head), serviceData.serviceValue.size);
        outData.AddServiceData(ParcelUuid::FromString(std::string(serviceData.serviceUuid)), value);
    }
    return;
}

static void ParseAdvertisDataParameters(int32_t *errCode, NativeAdvertiseData advData, BleAdvertiserData &outData)
{
    ParseServiceUuidParameters(errCode, advData, outData);
    ParseManufactureDataParameters(errCode, advData, outData);
    ParseServiceDataParameters(errCode, advData, outData);
    bool includeDeviceName = advData.includeDeviceName;
    HILOGI("includeDeviceName: %{public}d", includeDeviceName);
    outData.SetIncludeDeviceName(includeDeviceName);
    return;
}

int32_t BleImpl::StartAdvertising(NativeAdvertiseSetting setting, NativeAdvertiseData advData,
                                  NativeAdvertiseData *advResponse)
{
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    BleAdvertiserSettings nativeSettings;
    BleAdvertiserData nativeAdvData;
    BleAdvertiserData nativeRspData;
    int32_t errCode = BT_NO_ERROR;

    ParseAdvertisingSettingsParameters(&errCode, setting, nativeSettings);
    ParseAdvertisDataParameters(&errCode, advData, nativeAdvData);
    if (advResponse != nullptr) {
        ParseAdvertisDataParameters(&errCode, *advResponse, nativeRspData);
    }

    if (errCode != BT_NO_ERROR) {
        return errCode;
    }
    return bleAdvertiser->StartAdvertising(nativeSettings, nativeAdvData, nativeRspData, 0,
                                           FfiBluetoothBleAdvertiseCallback::GetInstance());
}

int32_t BleImpl::StopAdvertising()
{
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    if (bleAdvertiser == nullptr) {
        HILOGE("bleAdvertiser is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return bleAdvertiser->StopAdvertising(FfiBluetoothBleAdvertiseCallback::GetInstance());
}

static bool IsValidAdvertiserDuration(uint32_t duration)
{
    const uint32_t MIN_DURATION = 0;
    const uint32_t MAX_DURATION = 65535;
    if (duration < MIN_DURATION || duration > MAX_DURATION) {
        return false;
    }
    return true;
}

int32_t BleImpl::StartAdvertising(NativeAdvertisingParams advertisingParams, int32_t &id)
{
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    BleAdvertiserSettings nativeSettings;
    BleAdvertiserData nativeAdvData;
    BleAdvertiserData nativeRspData;

    int32_t errCode = BT_NO_ERROR;
    ParseAdvertisingSettingsParameters(&errCode, advertisingParams.advertisingSettings, nativeSettings);
    ParseAdvertisDataParameters(&errCode, advertisingParams.advertisingData, nativeAdvData);
    ParseAdvertisDataParameters(&errCode, advertisingParams.advertisingResponse, nativeRspData);
    uint16_t duration = advertisingParams.duration;
    if (!IsValidAdvertiserDuration(duration)) {
        HILOGE("Invalid duration: %{public}d", duration);
        return BT_ERR_INVALID_PARAM;
    }
    HILOGI("duration: %{public}u", duration);
    errCode = bleAdvertiser->StartAdvertising(nativeSettings, nativeAdvData, nativeRspData, duration,
                                              FfiBluetoothBleAdvertiseCallback::GetInstance());
    if (errCode != BT_NO_ERROR) {
        return errCode;
    }
    id = FfiBluetoothBleAdvertiseCallback::GetInstance()->GetAdvHandleEvent();
    if (id < 0) {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BleImpl::EnableAdvertising(NativeAdvertisingEnableParams advertisingEnableParams)
{
    uint32_t advHandle = advertisingEnableParams.advertisingId;
    uint16_t duration = advertisingEnableParams.duration;

    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    if (bleAdvertiser == nullptr) {
        HILOGE("bleAdvertiser is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return bleAdvertiser->EnableAdvertising(advHandle, duration, FfiBluetoothBleAdvertiseCallback::GetInstance());
}

int32_t BleImpl::DisableAdvertising(NativeAdvertisingDisableParams advertisingDisableParams)
{
    uint32_t advHandle = advertisingDisableParams.advertisingId;

    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    if (bleAdvertiser == nullptr) {
        HILOGE("bleAdvertiser is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return bleAdvertiser->DisableAdvertising(advHandle, FfiBluetoothBleAdvertiseCallback::GetInstance());
}

int32_t BleImpl::StopAdvertising(uint32_t advertisingId)
{
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    if (bleAdvertiser == nullptr) {
        HILOGE("bleAdvertiser is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    uint32_t advHandle = advertisingId;
    if (advHandle != bleAdvertiser->GetAdvHandle(FfiBluetoothBleAdvertiseCallback::GetInstance())) {
        HILOGE("Invalid advHandle: %{public}d", advHandle);
        return BT_ERR_INVALID_PARAM;
    }
    bleAdvertiser->StopAdvertising(FfiBluetoothBleAdvertiseCallback::GetInstance());
    return BT_NO_ERROR;
}

int32_t BleImpl::RegisterBleObserver(int32_t callbackType, void (*callback)())
{
    if (callbackType == REGISTER_BLE_ADVERTISING_STATE_INFO_TYPE) {
        auto callbackFunc = CJLambda::Create(reinterpret_cast<void (*)(CAdvertisingStateChangeInfo)>(callback));
        FfiBluetoothBleAdvertiseCallback::GetInstance()->RegisterAdvertisingStateChangeFunc(callbackFunc);
    } else if (callbackType == REGISTER_BLE_FIND_DEVICE_TYPE) {
        auto callbackFunc = CJLambda::Create(reinterpret_cast<void (*)(CArrScanResult)>(callback));
        FfiBluetoothBleCentralManagerCallback::GetInstance().RegisterBLEDeviceFindFunc(callbackFunc);
    } else {
        HILOGE("Unsupported callback type");
        return BT_ERR_INVALID_PARAM;
    }
    return BT_NO_ERROR;
}
} // namespace CJBluetoothBle
} // namespace CJSystemapi
} // namespace OHOS