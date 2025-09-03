/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef LOG_TAG
#define LOG_TAG "bt_ble_impl_ohbluetooth"
#endif

#include "stdexcept"

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_utils.h"
#include "bluetooth_log.h"
#include "bluetooth_host.h"

#include "taihe_bluetooth_ble_advertise_callback.h"
#include "taihe_bluetooth_gatt_client_callback.h"
#include "taihe_bluetooth_gatt_server_callback.h"
#include "taihe_bluetooth_gatt_server.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_ble_central_manager_callback.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::ble;

std::shared_ptr<BleAdvertiser> BleAdvertiserGetInstance(void)
{
    static auto instance = BleAdvertiser::CreateInstance();
    return instance;
}

BleCentralManager *BleCentralManagerGetInstance(void)
{
    static BleCentralManager instance(TaiheBluetoothBleCentralManagerCallback::GetInstance());
    return &instance;
}

class GattClientDeviceImpl {
public:
    explicit GattClientDeviceImpl(string_view deviceId)
    {
        HILOGI("enter");
        std::string remoteAddr = std::string(deviceId);
        device_ = std::make_shared<BluetoothRemoteDevice>(remoteAddr, 1);
        client_ = std::make_shared<GattClient>(*device_);
        client_->Init();
        callback_ = std::make_shared<TaiheGattClientCallback>();
        callback_->SetDeviceAddr(remoteAddr);
    }
    ~GattClientDeviceImpl() = default;
    
    void SetBLEMtuSize(int mtu)
    {
        ANI_BT_ASSERT(client_ != nullptr, BT_ERR_INTERNAL_ERROR, "SetBLEMtuSize ani assert failed");

        int ret = client_->RequestBleMtuSize(mtu);
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "SetBLEMtuSize return error");
    }

    void Connect()
    {
        ANI_BT_ASSERT(client_ != nullptr, BT_ERR_INTERNAL_ERROR, "Connect ani assert failed");
        ANI_BT_ASSERT(callback_ != nullptr, BT_ERR_INTERNAL_ERROR, "Connect ani assert failed");

        int ret = client_->Connect(callback_, false, GATT_TRANSPORT_TYPE_LE);
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "Connect return error");
    }

    void Disconnect()
    {
        ANI_BT_ASSERT(client_ != nullptr, BT_ERR_INTERNAL_ERROR, "Disconnect ani assert failed");

        int ret = client_->Disconnect();
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "Disconnect return error");
    }

    void Close()
    {
        ANI_BT_ASSERT(client_ != nullptr, BT_ERR_INTERNAL_ERROR, "Close ani assert failed");
        int ret = client_->Close();
        HILOGI("ret: %{public}d", ret);

        ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "Close return error");
    }
private:
    std::shared_ptr<GattClient> client_ = nullptr;
    std::shared_ptr<TaiheGattClientCallback> callback_;
    std::shared_ptr<BluetoothRemoteDevice> device_ = nullptr;
};

class BleScannerImpl {
public:
    BleScannerImpl()
    {
        callback_ = std::make_shared<TaiheBluetoothBleCentralManagerCallback>(true);
        bleCentralManager_ = std::make_shared<BleCentralManager>(callback_);
    }
    ~BleScannerImpl() = default;

private:
    std::shared_ptr<BleCentralManager> bleCentralManager_ = nullptr;
    std::shared_ptr<TaiheBluetoothBleCentralManagerCallback> callback_ = nullptr;
};

void StopAdvertising()
{
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    ANI_BT_ASSERT(bleAdvertiser != nullptr, BT_ERR_INTERNAL_ERROR, "bleAdvertiser ani assert failed");

    std::vector<std::shared_ptr<BleAdvertiseCallback>> callbacks = bleAdvertiser->GetAdvObservers();
    if (callbacks.empty()) {
        int ret = bleAdvertiser->StopAdvertising(TaiheBluetoothBleAdvertiseCallback::GetInstance());
        ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "StopAdvertising return error");
    }
}

void StopBLEScan()
{
    int ret = BleCentralManagerGetInstance()->StopScan();
    ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "StopBLEScan return error");
}

GattClientDevice CreateGattClientDevice(string_view deviceId)
{
    std::string remoteAddr = std::string(deviceId);
    return make_holder<GattClientDeviceImpl, GattClientDevice>(remoteAddr);
}

ohos::bluetooth::ble::GattServer CreateGattServer()
{
    return make_holder<GattServerImpl, ohos::bluetooth::ble::GattServer>();
}

ohos::bluetooth::ble::BleScanner CreateBleScanner()
{
    return make_holder<BleScannerImpl, ohos::bluetooth::ble::BleScanner>();
}

static ani_status ParseScanFilterDeviceIdParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                    BleScanFilter &bleScanFilter)
{
    if (scanFilter.deviceId.has_value()) {
        std::string deviceId = std::string(scanFilter.deviceId.value());
        if (!IsValidAddress(deviceId)) {
            HILOGE("Invalid deviceId: %{public}s", deviceId.c_str());
            return ani_invalid_arg;
        }
        bleScanFilter.SetDeviceId(deviceId);
    }
    return ani_ok;
}

static ani_status ParseScanFilterLocalNameParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                     BleScanFilter &bleScanFilter)
{
    if (scanFilter.name.has_value()) {
        std::string name = std::string(scanFilter.name.value());
        if (name.empty()) {
            HILOGE("name is empty");
            return ani_invalid_arg;
        }
        bleScanFilter.SetName(name);
    }
    return ani_ok;
}

static ani_status ParseScanFilterServiceUuidParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                       BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceUuid.has_value()) {
        UUID uuid {};
        if (ParseUuidParams(std::string(scanFilter.serviceUuid.value()), uuid)) {
            HILOGI("Scan filter serviceUuid is %{public}s", uuid.ToString().c_str());
            bleScanFilter.SetServiceUuid(uuid);
        }
    }

    if (scanFilter.serviceUuidMask.has_value()) {
        UUID uuidMask {};
        if (ParseUuidParams(std::string(scanFilter.serviceUuidMask.value()), uuidMask)) {
            HILOGI("Scan filter serviceUuidMask is %{public}s", uuidMask.ToString().c_str());
            bleScanFilter.SetServiceUuidMask(uuidMask);
        }
    }
    return ani_ok;
}

static ani_status ParseScanFilterSolicitationUuidParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                            BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceSolicitationUuid.has_value()) {
        UUID uuid {};
        if (ParseUuidParams(std::string(scanFilter.serviceSolicitationUuid.value()), uuid)) {
            HILOGI("Scan filter serviceSolicitationUuid is %{public}s", uuid.ToString().c_str());
            bleScanFilter.SetServiceSolicitationUuid(uuid);
        }
    }

    if (scanFilter.serviceSolicitationUuidMask.has_value()) {
        UUID uuidMask {};
        if (ParseUuidParams(std::string(scanFilter.serviceSolicitationUuidMask.value()), uuidMask)) {
            HILOGI("Scan filter serviceSolicitationUuidMask is %{public}s", uuidMask.ToString().c_str());
            bleScanFilter.SetServiceSolicitationUuidMask(uuidMask);
        }
    }
    return ani_ok;
}

static ani_status ParseScanFilterServiceDataParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                       BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceData.has_value()) {
        std::vector<uint8_t> data {};
        ParseArrayBufferParams(scanFilter.serviceData.value(), data);
        bleScanFilter.SetServiceData(std::move(data));
    }
    if (scanFilter.serviceDataMask.has_value()) {
        std::vector<uint8_t> dataMask {};
        ParseArrayBufferParams(scanFilter.serviceDataMask.value(), dataMask);
        bleScanFilter.SetServiceDataMask(std::move(dataMask));
    }
    return ani_ok;
}

static ani_status ParseScanFilterManufactureDataParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                           BleScanFilter &bleScanFilter)
{
    if (scanFilter.manufactureId.has_value()) {
        bleScanFilter.SetManufacturerId(scanFilter.manufactureId.value());
    }

    if (scanFilter.manufactureData.has_value()) {
        std::vector<uint8_t> data {};
        ParseArrayBufferParams(scanFilter.manufactureData.value(), data);
        bleScanFilter.SetManufactureData(std::move(data));
    }

    if (scanFilter.manufactureDataMask.has_value()) {
        std::vector<uint8_t> dataMask {};
        ParseArrayBufferParams(scanFilter.manufactureDataMask.value(), dataMask);
        bleScanFilter.SetManufactureDataMask(std::move(dataMask));
    }
    return ani_ok;
}

static ani_status ParseScanFilter(const ohos::bluetooth::ble::ScanFilter &scanFilter, BleScanFilter &bleScanFilter)
{
    ANI_BT_CALL_RETURN(ParseScanFilterDeviceIdParameters(scanFilter, bleScanFilter));
    ANI_BT_CALL_RETURN(ParseScanFilterLocalNameParameters(scanFilter, bleScanFilter));
    ANI_BT_CALL_RETURN(ParseScanFilterServiceUuidParameters(scanFilter, bleScanFilter));
    ANI_BT_CALL_RETURN(ParseScanFilterSolicitationUuidParameters(scanFilter, bleScanFilter));
    ANI_BT_CALL_RETURN(ParseScanFilterServiceDataParameters(scanFilter, bleScanFilter));
    ANI_BT_CALL_RETURN(ParseScanFilterManufactureDataParameters(scanFilter, bleScanFilter));
    return ani_ok;
}

static ani_status ParseScanFilterParameters(array<ohos::bluetooth::ble::ScanFilter> filters,
                                            std::vector<BleScanFilter> &params)
{
    if (filters.empty()) {
        HILOGE("Requires array length > 0");
        return ani_invalid_arg;
    }

    for (const auto& scanFilter : filters) {
        BleScanFilter bleScanFilter;
        ANI_BT_CALL_RETURN(ParseScanFilter(scanFilter, bleScanFilter));
        params.push_back(bleScanFilter);
    }
    return ani_ok;
}

static void SetReportDelay(ohos::bluetooth::ble::ScanOptions &scanOptions, BleScanSettings &outSettinngs)
{
    long reportDelayFloorValueBatch = 5000;
    if ((scanOptions.reportMode.value()).get_key() == ohos::bluetooth::ble::ScanReportMode::key_t::BATCH
        && scanOptions.interval.value() < reportDelayFloorValueBatch) {
        outSettinngs.SetReportDelay(reportDelayFloorValueBatch);
        HILOGW("reportDelay should be at least 5000 under batch mode, got %{public}d, enforced to 5000.",
            scanOptions.interval.value());
    } else {
        outSettinngs.SetReportDelay(scanOptions.interval.value());
    }
}

static void SetCbTypeSensMode(ohos::bluetooth::ble::ScanOptions &scanOptions, BleScanSettings &outSettinngs)
{
    // reportMode -> callbackType + sensitivityMode
    uint8_t callbackType = BLE_SCAN_CALLBACK_TYPE_ALL_MATCH;
    uint8_t sensitivityMode = static_cast<uint8_t>(SensitivityMode::SENSITIVITY_MODE_HIGH);

    switch ((scanOptions.reportMode.value()).get_key()) {
        case ohos::bluetooth::ble::ScanReportMode::key_t::NORMAL:
            callbackType = BLE_SCAN_CALLBACK_TYPE_ALL_MATCH;
            break;
        case ohos::bluetooth::ble::ScanReportMode::key_t::FENCE_SENSITIVITY_LOW:
            callbackType = BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH;
            sensitivityMode = static_cast<uint8_t>(SensitivityMode::SENSITIVITY_MODE_LOW);
            break;
        case ohos::bluetooth::ble::ScanReportMode::key_t::FENCE_SENSITIVITY_HIGH:
            callbackType = BLE_SCAN_CALLBACK_TYPE_FIRST_AND_LOST_MATCH;
            sensitivityMode = static_cast<uint8_t>(SensitivityMode::SENSITIVITY_MODE_HIGH);
            break;
        default:
            break;
    }
    outSettinngs.SetCallbackType(callbackType);
    outSettinngs.SetSensitivityMode(sensitivityMode);
}

ani_status CheckBleScanParams(ohos::bluetooth::ble::ScanFilterNullValue const &filters,
                              optional_view<ohos::bluetooth::ble::ScanOptions> options,
                              std::vector<BleScanFilter> &outScanfilters,
                              BleScanSettings &outSettinngs)
{
    std::vector<BleScanFilter> scanfilters;
    if (filters.get_tag() == ohos::bluetooth::ble::ScanFilterNullValue::tag_t::nValue) {
        BleScanFilter emptyFilter;
        scanfilters.push_back(emptyFilter);
    } else {
        array<ohos::bluetooth::ble::ScanFilter> inFilters = filters.get_filters_ref();
        ANI_BT_CALL_RETURN(ParseScanFilterParameters(inFilters, scanfilters));
    }

    if (options.has_value()) {
        auto scanOptions = options.value();
        if (scanOptions.interval.has_value()) {
            SetReportDelay(scanOptions, outSettinngs);
        }
        if (scanOptions.reportMode.has_value()) {
            outSettinngs.SetReportMode((scanOptions.reportMode.value()).get_value());
        }
        if (scanOptions.dutyMode.has_value()) {
            outSettinngs.SetScanMode((scanOptions.dutyMode.value()).get_value());
        }
        if (scanOptions.phyType.has_value()) {
            outSettinngs.SetPhy((scanOptions.phyType.value()).get_value());
        }
        if (scanOptions.reportMode.has_value()) {
            SetCbTypeSensMode(scanOptions, outSettinngs);
        }
    }
    outScanfilters = std::move(scanfilters);
    return ani_ok;
}

void StartBLEScan(ohos::bluetooth::ble::ScanFilterNullValue const &filters,
                  optional_view<ohos::bluetooth::ble::ScanOptions> options)
{
    std::vector<BleScanFilter> scanfilters;
    BleScanSettings settings;
    auto status = CheckBleScanParams(filters, options, scanfilters, settings);
    ANI_BT_ASSERT(status == ani_ok, BT_ERR_INVALID_PARAM, "ParseScanFilterParameters return error");

    int ret = BleCentralManagerGetInstance()->StartScan(settings, scanfilters);
    ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "StartBLEScan return error");
}

array<string> GetConnectedBLEDevices()
{
    std::lock_guard<std::mutex> lock(GattServerImpl::deviceListMutex_);
    std::vector<std::string> dstDevicesvec = GattServerImpl::deviceList_;
    array<string> result(taihe::copy_data_t{}, dstDevicesvec.data(), dstDevicesvec.size());
    return result;
}

static ani_status ParseAdvertisingSettingsParameters(const ohos::bluetooth::ble::AdvertiseSetting &inSettings,
                                                     BleAdvertiserSettings &outSettings)
{
    if (inSettings.interval.has_value()) {
        uint32_t interval = inSettings.interval.value();
        const uint32_t minInterval = 32;
        const uint32_t maxInterval = 16384;
        if (interval < minInterval || interval > maxInterval) {
            HILOGE("Invalid interval: %{public}d", interval);
            return ani_invalid_arg;
        }
        HILOGI("interval: %{public}u", interval);
        outSettings.SetInterval(interval);
    }
    if (inSettings.txPower.has_value()) {
        int32_t txPower = inSettings.txPower.value();
        const int32_t minTxPower = -127;
        const int32_t maxTxPower = 1;
        if (txPower < minTxPower || txPower > maxTxPower) {
            HILOGE("Invalid tx power: %{public}d", txPower);
            return ani_invalid_arg;
        }
        HILOGI("txPower is %{public}d", txPower);
        outSettings.SetTxPower(txPower);
    }
    if (inSettings.connectable.has_value()) {
        bool connectable = inSettings.connectable.value();
        HILOGI("connectable: %{public}d", connectable);
        outSettings.SetConnectable(connectable);
    }
    return ani_ok;
}

static ani_status ParseAdvertisDataParameters(ohos::bluetooth::ble::AdvertiseData &advDataIn,
                                              BleAdvertiserData &advDataOut)
{
    for (auto &serviceUuid: advDataIn.serviceUuids) {
        advDataOut.AddServiceUuid(UUID::FromString(std::string(serviceUuid)));
    }
    for (auto &manufacture: advDataIn.manufactureData) {
        advDataOut.AddManufacturerData(manufacture.manufactureId,
            std::string(manufacture.manufactureValue.begin(), manufacture.manufactureValue.end()));
    }
    for (auto &service: advDataIn.serviceData) {
        advDataOut.AddServiceData(UUID::FromString(std::string(service.serviceUuid)),
            std::string(service.serviceValue.begin(), service.serviceValue.end()));
    }
    if (advDataIn.includeDeviceName.has_value()) {
        advDataOut.SetIncludeDeviceName(advDataIn.includeDeviceName.value());
    }
    if (advDataIn.includeTxPower.has_value()) {
        advDataOut.SetIncludeTxPower(advDataIn.includeTxPower.value());
    }
    return ani_ok;
}

void StartAdvertising(ohos::bluetooth::ble::AdvertiseSetting setting, ohos::bluetooth::ble::AdvertiseData advData,
                      optional_view<ohos::bluetooth::ble::AdvertiseData> advResponse)
{
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    ANI_BT_ASSERT(bleAdvertiser != nullptr, BT_ERR_INTERNAL_ERROR, "BleAdvertiserGetInstance return error");

    BleAdvertiserSettings nativeSettings;
    BleAdvertiserData nativeAdvData;
    BleAdvertiserData nativeRspData;
    ani_status status = ParseAdvertisingSettingsParameters(setting, nativeSettings);
    ANI_BT_ASSERT(status == ani_ok, BT_ERR_INVALID_PARAM, "ParseAdvertisingSettingsParameters return error");
    status = ParseAdvertisDataParameters(advData, nativeAdvData);
    ANI_BT_ASSERT(status == ani_ok, BT_ERR_INVALID_PARAM, "ParseAdvertisDataParameters return error");
    if (advResponse.has_value()) {
        AdvertiseData response = advResponse.value();
        status = ParseAdvertisDataParameters(response, nativeRspData);
        ANI_BT_ASSERT(status == ani_ok, BT_ERR_INVALID_PARAM, "ParseAdvertisDataParameters return error");
    }

    int ret = bleAdvertiser->StartAdvertising(nativeSettings, nativeAdvData, nativeRspData, 0,
        TaiheBluetoothBleAdvertiseCallback::GetInstance());
    ANI_BT_ASSERT(ret == BT_NO_ERROR, ret, "StartAdvertising return error");
}
}  // Bluetooth
}  // OHOS

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateGattClientDevice(OHOS::Bluetooth::CreateGattClientDevice);
TH_EXPORT_CPP_API_CreateGattServer(OHOS::Bluetooth::CreateGattServer);
TH_EXPORT_CPP_API_CreateBleScanner(OHOS::Bluetooth::CreateBleScanner);
TH_EXPORT_CPP_API_StopAdvertising(OHOS::Bluetooth::StopAdvertising);
TH_EXPORT_CPP_API_StopBLEScan(OHOS::Bluetooth::StopBLEScan);
TH_EXPORT_CPP_API_GetConnectedBLEDevices(OHOS::Bluetooth::GetConnectedBLEDevices);
TH_EXPORT_CPP_API_StartBLEScan(OHOS::Bluetooth::StartBLEScan);
TH_EXPORT_CPP_API_StartAdvertising(OHOS::Bluetooth::StartAdvertising);
// NOLINTEND