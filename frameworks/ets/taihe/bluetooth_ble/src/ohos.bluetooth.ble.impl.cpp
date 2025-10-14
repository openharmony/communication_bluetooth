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
#define LOG_TAG "bt_taihe_gatt_client"
#endif

#include "bluetooth_ble_advertiser.h"
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_client.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_remote_device.h"
#include "bluetooth_utils.h"
#include "ohos.bluetooth.ble.impl.h"
#include "stdexcept"
#include "taihe_bluetooth_ble_advertise_callback.h"
#include "taihe_bluetooth_ble_central_manager_callback.h"
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_error.h"
#include "taihe_bluetooth_gatt_client_callback.h"
#include "taihe_bluetooth_gatt_server.h"
#include "taihe_bluetooth_gatt_server_callback.h"
#include "taihe_bluetooth_utils.h"
#include "taihe_parser_utils.h"
#include "taihe/array.hpp"
#include "taihe/optional.hpp"
#include "taihe/platform/ani.hpp"

namespace OHOS {
namespace Bluetooth {
namespace {
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
}  // namespace {}

class GattClientDeviceImpl {
public:
    explicit GattClientDeviceImpl(taihe::string_view deviceId)
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

    std::shared_ptr<BluetoothRemoteDevice> GetDevice()
    {
        return device_;
    }

    void SetBLEMtuSize(int mtu)
    {
        HILOGI("enter");
        TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);

        int ret = client_->RequestBleMtuSize(mtu);
        HILOGI("ret: %{public}d", ret);
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    void Connect()
    {
        HILOGI("enter");
        TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
        TAIHE_BT_ASSERT_RETURN_VOID(callback_ != nullptr, BT_ERR_INTERNAL_ERROR);

        int ret = client_->Connect(callback_, false, GATT_TRANSPORT_TYPE_LE);
        HILOGI("ret: %{public}d", ret);
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    void Disconnect()
    {
        HILOGI("enter");
        TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);

        int ret = client_->Disconnect();
        HILOGI("ret: %{public}d", ret);
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    void Close()
    {
        HILOGI("enter");
        TAIHE_BT_ASSERT_RETURN_VOID(client_ != nullptr, BT_ERR_INTERNAL_ERROR);
        int ret = client_->Close();
        HILOGI("ret: %{public}d", ret);

        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }

    taihe::string GetDeviceNameSync()
    {
        HILOGD("start");
        std::string deviceAddr = "";
        if (GetDevice()) {
            deviceAddr = device_->GetDeviceAddr();
        }

        std::string deviceName = "";
        int32_t err = BluetoothHost::GetDefaultHost().GetRemoteDevice(
            deviceAddr, BT_TRANSPORT_BLE).GetDeviceName(deviceName);
        TAIHE_BT_ASSERT_RETURN(err == BT_NO_ERROR, err, deviceName);

        HILOGI("err: %{public}d, deviceName: %{private}s", err, deviceName.c_str());

        return deviceName;
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
    HILOGD("enter");
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    TAIHE_BT_ASSERT_RETURN_VOID(bleAdvertiser != nullptr, BT_ERR_INTERNAL_ERROR);
 
    std::vector<std::shared_ptr<BleAdvertiseCallback>> callbacks = bleAdvertiser->GetAdvObservers();
    if (callbacks.empty()) {
        int ret = bleAdvertiser->StopAdvertising(TaiheBluetoothBleAdvertiseCallback::GetInstance());
        TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
    }
}

void StopBLEScan()
{
    HILOGD("enter");
    int ret = BleCentralManagerGetInstance()->StopScan();
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

ohos::bluetooth::ble::GattClientDevice CreateGattClientDevice(taihe::string_view deviceId)
{
    HILOGI("enter");
    std::string remoteAddr = std::string(deviceId);
    bool checkRet = CheckDeviceIdParam(remoteAddr);
    TAIHE_BT_ASSERT_RETURN(checkRet, BT_ERR_INVALID_PARAM,
        (taihe::make_holder<GattClientDeviceImpl, ohos::bluetooth::ble::GattClientDevice>(nullptr)));
    return taihe::make_holder<GattClientDeviceImpl, ohos::bluetooth::ble::GattClientDevice>(remoteAddr);
}

ohos::bluetooth::ble::GattServer CreateGattServer()
{
    HILOGI("enter");
    return taihe::make_holder<GattServerImpl, ohos::bluetooth::ble::GattServer>();
}

ohos::bluetooth::ble::BleScanner CreateBleScanner()
{
    HILOGI("enter");
    return taihe::make_holder<BleScannerImpl, ohos::bluetooth::ble::BleScanner>();
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

taihe_status CheckAdvertisingEnableParams(ohos::bluetooth::ble::AdvertisingEnableParams advertisingEnableParams,
    uint32_t &outAdvHandle, uint16_t &outDuration, std::shared_ptr<BleAdvertiseCallback> &callback)
{
    outAdvHandle = advertisingEnableParams.advertisingId;
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    TAIHE_BT_RETURN_IF(bleAdvertiser == nullptr, "bleAdvertiser is nullptr", taihe_invalid_arg);
    if (outAdvHandle != BLE_INVALID_ADVERTISING_HANDLE) {
        callback = bleAdvertiser->GetAdvObserver(outAdvHandle);
        TAIHE_BT_RETURN_IF(callback == nullptr, "callback is nullptr", taihe_invalid_arg);
    }

    uint32_t duration = 0;
    if (advertisingEnableParams.duration.has_value()) {
        duration = advertisingEnableParams.duration.value();
        if (!IsValidAdvertiserDuration(duration)) {
            HILOGE("Invalid duration: %{public}d", duration);
            return taihe_invalid_arg;
        }
        HILOGI("duration: %{public}u", duration);
    }
    outDuration = duration;

    return taihe_ok;
}

void EnableAdvertisingSync(ohos::bluetooth::ble::AdvertisingEnableParams advertisingEnableParams)
{
    HILOGI("enter");
    uint32_t advHandle = 0xFF;
    uint16_t duration = 0;
    std::shared_ptr<BleAdvertiseCallback> baseCallback;
    auto status = CheckAdvertisingEnableParams(advertisingEnableParams, advHandle, duration, baseCallback);
    TAIHE_BT_ASSERT_RETURN_VOID(status == taihe_ok, BT_ERR_INVALID_PARAM);
    // compatible with XTS
    TAIHE_BT_ASSERT_RETURN_VOID(advHandle != BLE_INVALID_ADVERTISING_HANDLE,
        GetSDKAdaptedStatusCode(BT_ERR_BLE_INVALID_ADV_ID)); // Adaptation for old sdk
    std::shared_ptr<TaiheBluetoothBleAdvertiseCallback> callback =
        std::static_pointer_cast<TaiheBluetoothBleAdvertiseCallback>(baseCallback);
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    if (bleAdvertiser == nullptr) {
        HILOGE("bleAdvertiser is nullptr");
        TAIHE_BT_ASSERT_RETURN_VOID(bleAdvertiser == nullptr, BT_ERR_INTERNAL_ERROR);
    }
    int ret = bleAdvertiser->EnableAdvertising(advHandle, duration, callback);
    ret = GetSDKAdaptedStatusCode(ret); // Adaptation for old sdk
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

static taihe_status ParseScanFilterDeviceIdParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                      BleScanFilter &bleScanFilter)
{
    if (scanFilter.deviceId.has_value()) {
        std::string deviceId = std::string(scanFilter.deviceId.value());
        if (!IsValidAddress(deviceId)) {
            HILOGE("Invalid deviceId: %{public}s", deviceId.c_str());
            return taihe_invalid_arg;
        }
        HILOGI("Scan filter device id is %{public}s", GetEncryptAddr(deviceId).c_str());
        bleScanFilter.SetDeviceId(deviceId);
    }
    return taihe_ok;
}

static taihe_status ParseScanFilterLocalNameParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                       BleScanFilter &bleScanFilter)
{
    if (scanFilter.name.has_value()) {
        std::string name = std::string(scanFilter.name.value());
        if (name.empty()) {
            HILOGE("name is empty");
            return taihe_invalid_arg;
        }
        HILOGI("Scan filter name is %{public}s", name.c_str());
        bleScanFilter.SetName(name);
    }
    return taihe_ok;
}

static taihe_status ParseScanFilterServiceUuidParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                         BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceUuid.has_value()) {
        UUID uuid {};
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(scanFilter.serviceUuid.value()), uuid));
        bleScanFilter.SetServiceUuid(uuid);
    }

    if (scanFilter.serviceUuidMask.has_value()) {
        UUID uuidMask {};
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(scanFilter.serviceUuidMask.value()), uuidMask));
        bleScanFilter.SetServiceUuidMask(uuidMask);
    }
    return taihe_ok;
}

static taihe_status ParseScanFilterSolicitationUuidParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                              BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceSolicitationUuid.has_value()) {
        UUID uuid {};
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(scanFilter.serviceSolicitationUuid.value()), uuid));
        bleScanFilter.SetServiceSolicitationUuid(uuid);
    }

    if (scanFilter.serviceSolicitationUuidMask.has_value()) {
        UUID uuidMask {};
        TAIHE_BT_CALL_RETURN(ParseUuidParams(std::string(scanFilter.serviceSolicitationUuidMask.value()), uuidMask));
        bleScanFilter.SetServiceSolicitationUuidMask(uuidMask);
    }
    return taihe_ok;
}

static taihe_status ParseScanFilterServiceDataParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
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
    return taihe_ok;
}

static taihe_status ParseScanFilterManufactureDataParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
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
    return taihe_ok;
}

static taihe_status ParseScanFilter(const ohos::bluetooth::ble::ScanFilter &scanFilter, BleScanFilter &bleScanFilter)
{
    HILOGD("enter");
    TAIHE_BT_CALL_RETURN(ParseScanFilterDeviceIdParameters(scanFilter, bleScanFilter));
    TAIHE_BT_CALL_RETURN(ParseScanFilterLocalNameParameters(scanFilter, bleScanFilter));
    TAIHE_BT_CALL_RETURN(ParseScanFilterServiceUuidParameters(scanFilter, bleScanFilter));
    TAIHE_BT_CALL_RETURN(ParseScanFilterSolicitationUuidParameters(scanFilter, bleScanFilter));
    TAIHE_BT_CALL_RETURN(ParseScanFilterServiceDataParameters(scanFilter, bleScanFilter));
    TAIHE_BT_CALL_RETURN(ParseScanFilterManufactureDataParameters(scanFilter, bleScanFilter));
    return taihe_ok;
}

static taihe_status ParseScanFilterParameters(taihe::array<ohos::bluetooth::ble::ScanFilter> filters,
                                              std::vector<BleScanFilter> &params)
{
    HILOGD("enter");
    TAIHE_BT_RETURN_IF(filters.empty(), "Requires array length > 0", taihe_invalid_arg);

    for (const auto& scanFilter : filters) {
        BleScanFilter bleScanFilter;
        TAIHE_BT_CALL_RETURN(ParseScanFilter(scanFilter, bleScanFilter));
        params.push_back(bleScanFilter);
    }
    return taihe_ok;
}

static void SetReportDelay(ohos::bluetooth::ble::ScanOptions &scanOptions, BleScanSettings &outSettinngs)
{
    long reportDelayFloorValueBatch = 5000;
    if (scanOptions.reportMode.has_value()
        && ((scanOptions.reportMode.value()).get_key() == ohos::bluetooth::ble::ScanReportMode::key_t::BATCH)
        && (scanOptions.interval.value() < reportDelayFloorValueBatch)) {
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

taihe_status CheckBleScanParams(ohos::bluetooth::ble::ScanFilterNullValue const &filters,
                                taihe::optional_view<ohos::bluetooth::ble::ScanOptions> options,
                                std::vector<BleScanFilter> &outScanfilters,
                                BleScanSettings &outSettinngs)
{
    std::vector<BleScanFilter> scanfilters;
    if (filters.get_tag() == ohos::bluetooth::ble::ScanFilterNullValue::tag_t::nValue) {
        BleScanFilter emptyFilter;
        scanfilters.push_back(emptyFilter);
    } else {
        taihe::array<ohos::bluetooth::ble::ScanFilter> inFilters = filters.get_filters_ref();
        TAIHE_BT_CALL_RETURN(ParseScanFilterParameters(inFilters, scanfilters));
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
    return taihe_ok;
}

void StartBLEScan(ohos::bluetooth::ble::ScanFilterNullValue const &filters,
                  taihe::optional_view<ohos::bluetooth::ble::ScanOptions> options)
{
    HILOGD("enter");
    std::vector<BleScanFilter> scanfilters;
    BleScanSettings settings;
    auto status = CheckBleScanParams(filters, options, scanfilters, settings);
    TAIHE_BT_ASSERT_RETURN_VOID(status == taihe_ok, BT_ERR_INVALID_PARAM);

    int ret = BleCentralManagerGetInstance()->StartScan(settings, scanfilters);
    TAIHE_BT_ASSERT_RETURN_VOID(ret == NO_ERROR || ret == BT_ERR_BLE_SCAN_ALREADY_STARTED, ret);
}

taihe::array<taihe::string> GetConnectedBLEDevices()
{
    HILOGD("enter");
    std::lock_guard<std::mutex> lock(GattServerImpl::deviceListMutex_);
    std::vector<std::string> dstDevicesvec = GattServerImpl::deviceList_;
    taihe::array<taihe::string> result(taihe::copy_data_t{}, dstDevicesvec.data(), dstDevicesvec.size());
    return result;
}

static taihe_status ParseAdvertisingSettingsParameters(const ohos::bluetooth::ble::AdvertiseSetting &inSettings,
                                                       BleAdvertiserSettings &outSettings)
{
    HILOGD("enter");
    if (inSettings.interval.has_value()) {
        uint32_t interval = inSettings.interval.value();
        const uint32_t minInterval = 32;
        const uint32_t maxInterval = 16384;
        if (interval < minInterval || interval > maxInterval) {
            HILOGE("Invalid interval: %{public}d", interval);
            return taihe_invalid_arg;
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
            return taihe_invalid_arg;
        }
        HILOGI("txPower is %{public}d", txPower);
        outSettings.SetTxPower(txPower);
    }
    if (inSettings.connectable.has_value()) {
        bool connectable = inSettings.connectable.value();
        HILOGI("connectable: %{public}d", connectable);
        outSettings.SetConnectable(connectable);
    }
    return taihe_ok;
}

static taihe_status ParseAdvertisDataParameters(ohos::bluetooth::ble::AdvertiseData &advDataIn,
                                                BleAdvertiserData &advDataOut)
{
    for (auto &serviceUuid: advDataIn.serviceUuids) {
        advDataOut.AddServiceUuid(UUID::FromString(std::string(serviceUuid)));
        HILOGI("Service Uuid = %{public}s", std::string(serviceUuid).c_str());
    }
    for (auto &manufacture: advDataIn.manufactureData) {
        TAIHE_BT_RETURN_IF(manufacture.manufactureId > 0xFFFF, "Invalid manufactureId", taihe_invalid_arg);
        advDataOut.AddManufacturerData(manufacture.manufactureId,
            std::string(manufacture.manufactureValue.begin(), manufacture.manufactureValue.end()));
    }
    for (auto &service: advDataIn.serviceData) {
        advDataOut.AddServiceData(UUID::FromString(std::string(service.serviceUuid)),
            std::string(service.serviceValue.begin(), service.serviceValue.end()));
    }
    if (advDataIn.includeDeviceName.has_value()) {
        HILOGI("includeDeviceName: %{public}d", advDataIn.includeDeviceName.value());
        advDataOut.SetIncludeDeviceName(advDataIn.includeDeviceName.value());
    }
    if (advDataIn.includeTxPower.has_value()) {
        HILOGI("includeTxPower: %{public}d", advDataIn.includeTxPower.value());
        advDataOut.SetIncludeTxPower(advDataIn.includeTxPower.value());
    }
    return taihe_ok;
}

void StartAdvertising(ohos::bluetooth::ble::AdvertiseSetting setting, ohos::bluetooth::ble::AdvertiseData advData,
                      taihe::optional_view<ohos::bluetooth::ble::AdvertiseData> advResponse)
{
    HILOGI("enter");
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    TAIHE_BT_ASSERT_RETURN_VOID(bleAdvertiser != nullptr, BT_ERR_INTERNAL_ERROR);

    BleAdvertiserSettings nativeSettings;
    BleAdvertiserData nativeAdvData;
    BleAdvertiserData nativeRspData;
    taihe_status status = ParseAdvertisingSettingsParameters(setting, nativeSettings);
    TAIHE_BT_ASSERT_RETURN_VOID(status == taihe_ok, BT_ERR_INVALID_PARAM);
    status = ParseAdvertisDataParameters(advData, nativeAdvData);
    TAIHE_BT_ASSERT_RETURN_VOID(status == taihe_ok, BT_ERR_INVALID_PARAM);
    if (advResponse.has_value()) {
        ohos::bluetooth::ble::AdvertiseData response = advResponse.value();
        status = ParseAdvertisDataParameters(response, nativeRspData);
        TAIHE_BT_ASSERT_RETURN_VOID(status == taihe_ok, BT_ERR_INVALID_PARAM);
    }

    int ret = bleAdvertiser->StartAdvertising(nativeSettings, nativeAdvData, nativeRspData, 0,
        TaiheBluetoothBleAdvertiseCallback::GetInstance());
    TAIHE_BT_ASSERT_RETURN_VOID(ret == BT_NO_ERROR, ret);
}

taihe_status CheckAdvertisingDataWithDuration(ani_env *env, ani_object object, BleAdvertiserSettings &outSettings,
    BleAdvertiserData &outAdvData, BleAdvertiserData &outRspData, uint16_t &outDuration)
{
    ohos::bluetooth::ble::AdvertisingParams bleAdvertiserParams = TaiheParseAdvertisingParams(env, object);
    HILOGI("CheckAdvertisingDataWithDuration parse advertisingParams finish");

    if (bleAdvertiserParams.advertisingSettings.interval.has_value()) {
        HILOGI("CheckAdvertisingDataWithDuration bleAdvertiserParams.advertisingSettings.interval: %{public}lld",
            static_cast<uint64_t>(bleAdvertiserParams.advertisingSettings.interval.value()));
    }
    if (bleAdvertiserParams.advertisingSettings.txPower.has_value()) {
        HILOGI("CheckAdvertisingDataWithDuration bleAdvertiserParams.advertisingSettings.txPower: %{public}lld",
            static_cast<uint64_t>(bleAdvertiserParams.advertisingSettings.txPower.value()));
    }

    Util::TaiheToBleAdvertiserSettings(outSettings, bleAdvertiserParams.advertisingSettings);
    HILOGI("CheckAdvertisingDataWithDuration Util::TaiheToBleAdvertiserSettings finish");
    Util::TaiheToBleAdvertiserData(outAdvData, bleAdvertiserParams.advertisingData);
    HILOGI("CheckAdvertisingDataWithDuration Util::TaiheToBleAdvertiserData finish");

    if (bleAdvertiserParams.advertisingResponse.has_value()) {
        HILOGI("CheckAdvertisingDataWithDuration bleAdvertiserParams.advertisingResponse");
        Util::TaiheToBleAdvertiserData(outRspData, bleAdvertiserParams.advertisingResponse.value());
    }
    HILOGI("CheckAdvertisingDataWithDuration Util::TaiheToBleAdvertiserData 222222 finish");

    if (bleAdvertiserParams.duration.has_value()) {
        HILOGI("CheckAdvertisingDataWithDuration bleAdvertiserParams.duration: %{public}lld",
            static_cast<uint64_t>(bleAdvertiserParams.duration.value()));
        outDuration = static_cast<uint16_t>(bleAdvertiserParams.duration.value());
        if (!IsValidAdvertiserDuration(outDuration)) {
            HILOGE("Invalid duration: %{public}d", outDuration);
            return taihe_invalid_arg;
        }
        HILOGI("duration: %{public}u", outDuration);
    }
    HILOGI("CheckAdvertisingDataWithDuration duration: %{public}d", outDuration);

    return taihe_ok;
}

ani_object StartAdvertisingAsyncPromise([[maybe_unused]] ani_env *env, ani_object advertisingParams)
{
    HILOGI("StartAdvertisingAsyncPromise enter");
    ani_vm *vm = nullptr;
    if (ANI_OK != env->GetVM(&vm)) {
        HILOGE("GetVM failed");
        return nullptr;
    }
    HILOGI("StartAdvertisingAsyncPromise get vm finish");

    std::shared_ptr<TaiheHaEventUtils> haUtils = std::make_shared<TaiheHaEventUtils>(env, "ble.StartAdvertising");
    HILOGI("StartAdvertisingAsyncPromise TaiheHaEventUtils finish");
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    HILOGI("StartAdvertisingAsyncPromise BleAdvertiserGetInstance finish");

    BleAdvertiserSettings settings;
    BleAdvertiserData advData;
    BleAdvertiserData rspData;
    uint16_t duration = 0;
    auto status = CheckAdvertisingDataWithDuration(env, advertisingParams, settings, advData, rspData, duration);
    TAIHE_BT_ASSERT_RETURN(status == taihe_ok, BT_ERR_INVALID_PARAM, nullptr);
    HILOGI("StartAdvertisingAsyncPromise CheckAdvertisingDataWithDuration finish");
    auto callback = std::make_shared<TaiheBluetoothBleAdvertiseCallback>();
    HILOGI("StartAdvertisingAsyncPromise new callback finish");

    auto func = [settings, advData, rspData, duration, bleAdvertiser, callback]() {
        int ret = bleAdvertiser->StartAdvertising(settings, advData, rspData, duration, callback);
        ret = GetSDKAdaptedStatusCode(ret); // Adaptation for old sdk
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(vm, env, nullptr, func, ASYNC_WORK_NEED_CALLBACK, haUtils);
    bool success = callback->asyncWorkMap_.TryPush(TaiheAsyncType::GET_ADVERTISING_HANDLE, asyncWork);
    HILOGI("StartAdvertisingAsyncPromise success: %{public}d", success);
    asyncWork->Run();
    return asyncWork->GetRet();
}

ani_object StartAdvertisingAsyncCallback([[maybe_unused]] ani_env *env, ani_object advertisingParams,
                                         [[maybe_unused]] ani_object object)
{
    HILOGI("StartAdvertisingAsyncCallback enter");
    ani_vm *vm = nullptr;
    if (ANI_OK != env->GetVM(&vm)) {
        HILOGE("GetVM failed");
        return nullptr;
    }
    HILOGI("StartAdvertisingAsyncCallback get vm finish");

    std::shared_ptr<TaiheHaEventUtils> haUtils = std::make_shared<TaiheHaEventUtils>(env, "ble.StartAdvertising");
    HILOGI("StartAdvertisingAsyncCallback TaiheHaEventUtils finish");
    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    HILOGI("StartAdvertisingAsyncCallback BleAdvertiserGetInstance finish");

    BleAdvertiserSettings settings;
    BleAdvertiserData advData;
    BleAdvertiserData rspData;
    uint16_t duration = 0;
    auto status = CheckAdvertisingDataWithDuration(env, advertisingParams, settings, advData, rspData, duration);
    TAIHE_BT_ASSERT_RETURN(status == taihe_ok, BT_ERR_INVALID_PARAM, nullptr);
    HILOGI("StartAdvertisingAsyncCallback CheckAdvertisingDataWithDuration finish");
    auto callback = std::make_shared<TaiheBluetoothBleAdvertiseCallback>();
    HILOGI("StartAdvertisingAsyncCallback new callback finish");
    auto func = [settings, advData, rspData, duration, bleAdvertiser, callback]() {
        int ret = bleAdvertiser->StartAdvertising(settings, advData, rspData, duration, callback);
        ret = GetSDKAdaptedStatusCode(ret); // Adaptation for old sdk
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(vm, env, object, func, ASYNC_WORK_NEED_CALLBACK, haUtils);
    bool success = callback->asyncWorkMap_.TryPush(TaiheAsyncType::GET_ADVERTISING_HANDLE, asyncWork);
    HILOGI("StartAdvertisingAsyncCallback success: %{public}d", success);
    asyncWork->Run();
    return asyncWork->GetRet();
}

taihe_status CheckAdvertisingDisableParams(ani_env *env, ani_object info, uint32_t &outAdvHandle,
    std::shared_ptr<BleAdvertiseCallback> &callback)
{
    ohos::bluetooth::ble::AdvertisingDisableParams bleAdvertisingDisableParams =
        TaiheParseAdvertisingDisableParams(env, info);
    HILOGI("CheckAdvertisingDisableParams parse advertisingDisableParams finish");

    outAdvHandle = static_cast<uint32_t>(bleAdvertisingDisableParams.advertisingId);
    HILOGI("CheckAdvertisingDisableParams outAdvHandle: %{public}u", outAdvHandle);

    std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    TAIHE_BT_RETURN_IF(bleAdvertiser == nullptr, "bleAdvertiser is nullptr", taihe_invalid_arg);
    if (outAdvHandle != BLE_INVALID_ADVERTISING_HANDLE) {
        callback = bleAdvertiser->GetAdvObserver(outAdvHandle);
        TAIHE_BT_RETURN_IF(callback == nullptr, "callback is nullptr", taihe_invalid_arg);
    }
    HILOGI("CheckAdvertisingDisableParams callback finish");

    return taihe_ok;
}

ani_object DisableAdvertisingAsyncPromise([[maybe_unused]] ani_env *env, ani_object advertisingDisableParams)
{
    HILOGI("DisableAdvertisingAsyncPromise enter");
    ani_vm *vm = nullptr;
    if (ANI_OK != env->GetVM(&vm)) {
        HILOGE("GetVM failed");
        return nullptr;
    }
    HILOGI("DisableAdvertisingAsyncPromise get vm finish");

    std::shared_ptr<TaiheHaEventUtils> haUtils = std::make_shared<TaiheHaEventUtils>(env, "ble.DisableAdvertising");
    HILOGI("DisableAdvertisingAsyncPromise TaiheHaEventUtils finish");

    uint32_t advHandle = 0xFF;
    std::shared_ptr<BleAdvertiseCallback> baseCallback;
    auto status = CheckAdvertisingDisableParams(env, advertisingDisableParams, advHandle, baseCallback);
    TAIHE_BT_ASSERT_RETURN(status == taihe_ok, BT_ERR_INVALID_PARAM, nullptr);
    // compatible with XTS
    TAIHE_BT_ASSERT_RETURN(advHandle != BLE_INVALID_ADVERTISING_HANDLE,
        GetSDKAdaptedStatusCode(BT_ERR_BLE_INVALID_ADV_ID), nullptr); // Adaptation for old sdk
    HILOGI("DisableAdvertisingAsyncPromise CheckAdvertisingDisableParams finish");
    std::shared_ptr<TaiheBluetoothBleAdvertiseCallback> callback =
        std::static_pointer_cast<TaiheBluetoothBleAdvertiseCallback>(baseCallback);
    HILOGI("DisableAdvertisingAsyncPromise new callback finish");
    auto func = [advHandle, callback]() {
        std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
        HILOGI("DisableAdvertisingAsyncPromise BleAdvertiserGetInstance finish");
        if (bleAdvertiser == nullptr) {
            HILOGE("bleAdvertiser is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = bleAdvertiser->DisableAdvertising(advHandle, callback);
        ret = GetSDKAdaptedStatusCode(ret); // Adaptation for old sdk
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(vm, env, nullptr, func, ASYNC_WORK_NEED_CALLBACK, haUtils);
    bool success = callback->asyncWorkMap_.TryPush(TaiheAsyncType::GET_ADVERTISING_HANDLE, asyncWork);
    TAIHE_BT_ASSERT_RETURN(success, BT_ERR_INTERNAL_ERROR, nullptr);
    HILOGI("DisableAdvertisingAsyncPromise success: %{public}d", success);

    asyncWork->Run();
    return asyncWork->GetRet();
}

ani_object DisableAdvertisingAsyncCallback([[maybe_unused]] ani_env *env, ani_object advertisingDisableParams,
                                           [[maybe_unused]] ani_object object)
{
    HILOGI("DisableAdvertisingAsyncCallback enter");
    ani_vm *vm = nullptr;
    if (ANI_OK != env->GetVM(&vm)) {
        HILOGE("GetVM failed");
        return nullptr;
    }
    HILOGI("DisableAdvertisingAsyncCallback get vm finish");

    std::shared_ptr<TaiheHaEventUtils> haUtils = std::make_shared<TaiheHaEventUtils>(env, "ble.DisableAdvertising");
    HILOGI("DisableAdvertisingAsyncCallback TaiheHaEventUtils finish");

    uint32_t advHandle = 0xFF;
    std::shared_ptr<BleAdvertiseCallback> baseCallback;
    auto status = CheckAdvertisingDisableParams(env, advertisingDisableParams, advHandle, baseCallback);
    TAIHE_BT_ASSERT_RETURN(status == taihe_ok, BT_ERR_INVALID_PARAM, nullptr);
    // compatible with XTS
    TAIHE_BT_ASSERT_RETURN(advHandle != BLE_INVALID_ADVERTISING_HANDLE,
        GetSDKAdaptedStatusCode(BT_ERR_BLE_INVALID_ADV_ID), nullptr); // Adaptation for old sdk
    HILOGI("DisableAdvertisingAsyncCallback CheckAdvertisingDisableParams finish");
    std::shared_ptr<TaiheBluetoothBleAdvertiseCallback> callback =
        std::static_pointer_cast<TaiheBluetoothBleAdvertiseCallback>(baseCallback);
    HILOGI("DisableAdvertisingAsyncCallback new callback finish");
    auto func = [advHandle, callback]() {
        std::shared_ptr<BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
        HILOGI("DisableAdvertisingAsyncCallback BleAdvertiserGetInstance finish");
        if (bleAdvertiser == nullptr) {
            HILOGE("bleAdvertiser is nullptr");
            return TaiheAsyncWorkRet(BT_ERR_INTERNAL_ERROR);
        }
        int ret = bleAdvertiser->DisableAdvertising(advHandle, callback);
        ret = GetSDKAdaptedStatusCode(ret); // Adaptation for old sdk
        return TaiheAsyncWorkRet(ret);
    };

    auto asyncWork = TaiheAsyncWorkFactory::CreateAsyncWork(vm, env, object, func, ASYNC_WORK_NEED_CALLBACK, haUtils);
    bool success = callback->asyncWorkMap_.TryPush(TaiheAsyncType::GET_ADVERTISING_HANDLE, asyncWork);
    TAIHE_BT_ASSERT_RETURN(success, BT_ERR_INTERNAL_ERROR, nullptr);
    HILOGI("DisableAdvertisingAsyncCallback success: %{public}d", success);

    asyncWork->Run();
    return asyncWork->GetRet();
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
TH_EXPORT_CPP_API_EnableAdvertisingSync(OHOS::Bluetooth::EnableAdvertisingSync);
// NOLINTEND