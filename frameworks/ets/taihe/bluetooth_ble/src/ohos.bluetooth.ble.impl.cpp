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

#include "ohos.bluetooth.ble.proj.hpp"
#include "ohos.bluetooth.ble.impl.hpp"
#include "taihe/runtime.hpp"
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
#include "taihe_bluetooth_ble_utils.h"
#include "taihe_bluetooth_ble_central_manager_callback.h"

namespace OHOS {
namespace Bluetooth {

using namespace taihe;
using namespace ohos::bluetooth::ble;

#ifndef ANI_BT_ASSERT_RETURN
#define ANI_BT_ASSERT_RETURN(cond, errCode, errMsg)     \
do {                                                     \
    if (!(cond)) {                                      \
        set_business_error(errCode, errMsg);            \
        HILOGE("bluetoothManager ani assert failed.");  \
        return;                                         \
    }                                                   \
} while (0)
#endif

enum class SensitivityMode {
    SENSITIVITY_MODE_HIGH = 1,  //  high sensitivity mode
    SENSITIVITY_MODE_LOW = 2    //  low sensitivity mode
};

std::shared_ptr<OHOS::Bluetooth::BleAdvertiser> BleAdvertiserGetInstance(void)
{
    static auto instance = OHOS::Bluetooth::BleAdvertiser::CreateInstance();
    return instance;
}

OHOS::Bluetooth::BleCentralManager *BleCentralManagerGetInstance(void)
{
    static OHOS::Bluetooth::BleCentralManager
        instance(OHOS::Bluetooth::TaiheBluetoothBleCentralManagerCallback::GetInstance());
    return &instance;
}

class GattClientDeviceImpl {
public:
    explicit GattClientDeviceImpl(string_view deviceId)
    {
        HILOGI("enter");
        std::string remoteAddr = std::string(deviceId);
        device_ = std::make_shared<OHOS::Bluetooth::BluetoothRemoteDevice>(remoteAddr, 1);
        client_ = std::make_shared<OHOS::Bluetooth::GattClient>(*device_);
        client_->Init();
        callback_ = std::make_shared<OHOS::Bluetooth::TaiheGattClientCallback>();
        callback_->SetDeviceAddr(remoteAddr);
    }
    ~GattClientDeviceImpl() = default;
    
    void SetBLEMtuSize(int mtu)
    {
        ANI_BT_ASSERT_RETURN(client_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "SetBLEMtuSize ani assert failed");

        int ret = client_->RequestBleMtuSize(mtu);
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "SetBLEMtuSize return error");
    }

    void Connect()
    {
        ANI_BT_ASSERT_RETURN(client_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "Connect ani assert failed");
        ANI_BT_ASSERT_RETURN(callback_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "Connect ani assert failed");

        int ret = client_->Connect(callback_, false, OHOS::Bluetooth::GATT_TRANSPORT_TYPE_LE);
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Connect return error");
    }

    void Disconnect()
    {
        ANI_BT_ASSERT_RETURN(client_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "Disconnect ani assert failed");

        int ret = client_->Disconnect();
        HILOGI("ret: %{public}d", ret);
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Disconnect return error");
    }

    void Close()
    {
        ANI_BT_ASSERT_RETURN(client_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR, "Close ani assert failed");
        int ret = client_->Close();
        HILOGI("ret: %{public}d", ret);

        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Close return error");
    }
private:
    std::shared_ptr<OHOS::Bluetooth::GattClient> client_ = nullptr;
    std::shared_ptr<OHOS::Bluetooth::TaiheGattClientCallback> callback_;
    std::shared_ptr<OHOS::Bluetooth::BluetoothRemoteDevice> device_ = nullptr;
};

class GattServerImpl {
public:
    GattServerImpl()
    {
        HILOGI("enter");
        callback_ = std::make_shared<OHOS::Bluetooth::TaiheGattServerCallback>();
        std::shared_ptr<OHOS::Bluetooth::GattServerCallback> tmp =
            std::static_pointer_cast<OHOS::Bluetooth::GattServerCallback>(callback_);
        server_  = OHOS::Bluetooth::GattServer::CreateInstance(tmp);
    }
    ~GattServerImpl() = default;

    void Close()
    {
        ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR, "Close ani assert failed");
        int ret = server_->Close();
        HILOGI("ret: %{public}d", ret);

        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "Close return error");
    }

    void AddService(ohos::bluetooth::ble::GattService service)
    {
        ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR, "AddService ani assert failed");
        std::unique_ptr<GattService> gattService {nullptr};
        GattServiceType type = service.isPrimary ? GattServiceType::PRIMARY : GattServiceType::SECONDARY;
        UUID serviceUuid = UUID::FromString(std::string(service.serviceUuid));
        gattService = std::make_unique<GattService>(serviceUuid, type);

        for (auto &characteristic : service.characteristics) {
            int charPermissions = 0;
            int charProperties = 0;
            UUID characteristicUuid = UUID::FromString(std::string(characteristic.serviceUuid));
            GattCharacteristic character(characteristicUuid, charPermissions, charProperties);
            character.SetValue(characteristic.characteristicValue.data(), characteristic.characteristicValue.size());

            for (const auto &descriptor : characteristic.descriptors) {
                UUID descriptorUuid = UUID::FromString(std::string(descriptor.descriptorUuid));
                GattDescriptor gattDescriptor(descriptorUuid, 0);
                gattDescriptor.SetValue(descriptor.descriptorValue.data(), descriptor.descriptorValue.size());
                character.AddDescriptor(gattDescriptor);
            }
            gattService->AddCharacteristic(character);
        }
        int ret = server_->AddService(*gattService);
        ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "AddService return error");
    }

    void RemoveService(string_view serviceUuid)
    {
        ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "RemoveService ani assert failed");
        UUID uuid = UUID::FromString(std::string(serviceUuid));

        int ret = BT_NO_ERROR;
        auto primaryService = server_->GetService(uuid, true);
        if (primaryService.has_value()) {
            ret = server_->RemoveGattService(primaryService.value());
            ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "Primary RemoveService return error");
        }

        auto secondService = server_->GetService(uuid, false);
        if (secondService.has_value()) {
            ret = server_->RemoveGattService(secondService.value());
            ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "Second RemoveService return error");
        }
    }

    void SendResponse(ServerResponse serverResponse)
    {
        ANI_BT_ASSERT_RETURN(server_ != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
            "SendResponse ani assert failed");
        BluetoothRemoteDevice remoteDevice(std::string(serverResponse.deviceId), BTTransport::ADAPTER_BLE);
        int32_t transId = serverResponse.transId;
        int32_t status = serverResponse.status;
        int32_t offset = serverResponse.offset;
        int ret = server_->SendResponse(remoteDevice, transId, status, offset,
            serverResponse.value.data(), serverResponse.value.size());
        ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "SendResponse return error");
    }
private:
    std::shared_ptr<OHOS::Bluetooth::GattServer> server_ = nullptr;
    std::shared_ptr<OHOS::Bluetooth::TaiheGattServerCallback> callback_;
};

class BleScannerImpl {
public:
    BleScannerImpl()
    {
        callback_ = std::make_shared<OHOS::Bluetooth::TaiheBluetoothBleCentralManagerCallback>(true);
        bleCentralManager_ = std::make_shared<OHOS::Bluetooth::BleCentralManager>(callback_);
    }
    ~BleScannerImpl() = default;

private:
    std::shared_ptr<OHOS::Bluetooth::BleCentralManager> bleCentralManager_ = nullptr;
    std::shared_ptr<OHOS::Bluetooth::TaiheBluetoothBleCentralManagerCallback> callback_ = nullptr;
};

void StopAdvertising()
{
    std::shared_ptr<OHOS::Bluetooth::BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    ANI_BT_ASSERT_RETURN(bleAdvertiser != nullptr, OHOS::Bluetooth::BT_ERR_INTERNAL_ERROR,
        "bleAdvertiser ani assert failed");

    std::vector<std::shared_ptr<OHOS::Bluetooth::BleAdvertiseCallback>> callbacks = bleAdvertiser->GetAdvObservers();
    if (callbacks.empty()) {
        int ret = bleAdvertiser->StopAdvertising(OHOS::Bluetooth::TaiheBluetoothBleAdvertiseCallback::GetInstance());
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "StopAdvertising return error");
    }
}

void StopBLEScan()
{
    int ret = BleCentralManagerGetInstance()->StopScan();
    ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "StopBLEScan return error");
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

BleScanner CreateBleScanner()
{
    return make_holder<BleScannerImpl, BleScanner>();
}

static void ParseScanFilterDeviceIdParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                              BleScanFilter &bleScanFilter)
{
    if (scanFilter.deviceId.has_value()) {
        bleScanFilter.SetDeviceId(std::string(scanFilter.deviceId.value()));
    }
}

static void ParseScanFilterLocalNameParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                               BleScanFilter &bleScanFilter)
{
    if (scanFilter.name.has_value()) {
        bleScanFilter.SetName(std::string(scanFilter.name.value()));
    }
}

static void ParseScanFilterServiceUuidParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                 BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceUuid.has_value()) {
        std::string uuid = std::string(scanFilter.serviceUuid.value());
        UUID outUuid = ParcelUuid::FromString(uuid);
        bleScanFilter.SetServiceUuid(outUuid);
    }

    if (scanFilter.serviceUuidMask.has_value()) {
        std::string uuid = std::string(scanFilter.serviceUuidMask.value());
        UUID outUuid = ParcelUuid::FromString(uuid);
        bleScanFilter.SetServiceUuidMask(outUuid);
    }
}

static void ParseScanFilterSolicitationUuidParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
                                                      BleScanFilter &bleScanFilter)
{
    if (scanFilter.serviceSolicitationUuid.has_value()) {
        std::string uuid = std::string(scanFilter.serviceSolicitationUuid.value());
        UUID outUuid = ParcelUuid::FromString(uuid);
        bleScanFilter.SetServiceSolicitationUuid(outUuid);
    }

    if (scanFilter.serviceSolicitationUuidMask.has_value()) {
        std::string uuid = std::string(scanFilter.serviceSolicitationUuidMask.value());
        UUID outUuid = ParcelUuid::FromString(uuid);
        bleScanFilter.SetServiceSolicitationUuidMask(outUuid);
    }
}

static void ParseArrayBufferParams(const taihe::array<uint8_t>& data, std::vector<uint8_t> &outParam)
{
    for (const auto item : data) {
        outParam.push_back(item);
    }
}

static void ParseScanFilterServiceDataParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
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
}

static void ParseScanFilterManufactureDataParameters(const ohos::bluetooth::ble::ScanFilter &scanFilter,
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
}

static void ParseScanFilter(const ohos::bluetooth::ble::ScanFilter &scanFilter, BleScanFilter &bleScanFilter)
{
    ParseScanFilterDeviceIdParameters(scanFilter, bleScanFilter);
    ParseScanFilterLocalNameParameters(scanFilter, bleScanFilter);
    ParseScanFilterServiceUuidParameters(scanFilter, bleScanFilter);
    ParseScanFilterSolicitationUuidParameters(scanFilter, bleScanFilter);
    ParseScanFilterServiceDataParameters(scanFilter, bleScanFilter);
    ParseScanFilterManufactureDataParameters(scanFilter, bleScanFilter);
}

static void ParseScanFilterParameters(array<ohos::bluetooth::ble::ScanFilter> filters,
                                      std::vector<BleScanFilter> &params)
{
    for (const auto& scanFilter : filters) {
        BleScanFilter bleScanFilter;
        ParseScanFilter(scanFilter, bleScanFilter);
        params.push_back(bleScanFilter);
    }
}

static void SetReportDelay(ohos::bluetooth::ble::ScanOptions &scanOptions, BleScanSettings &outSettinngs)
{
    // enforce reportDelay to be either 0 or at least the floor value(5000ms) corresponding to normal and batch mode
    long currentReportDelay = scanOptions.interval.value();
    long reportDelayFloorValue = 5000;
    if (currentReportDelay == 0 || currentReportDelay >= reportDelayFloorValue) {
        outSettinngs.SetReportDelay(currentReportDelay);
    } else {
        outSettinngs.SetReportDelay(reportDelayFloorValue);
        HILOGD("reportDelay should be either 0 or at least 5000, got %{public}ld, enforced to 5000.",
            currentReportDelay);
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

void StartBLEScan(array<ohos::bluetooth::ble::ScanFilter> filters,
                  optional_view<ohos::bluetooth::ble::ScanOptions> options)
{
    std::vector<BleScanFilter> scanFilters;
    BleScanSettings settings;
    if (filters.empty()) {
        BleScanFilter emptyFilter;
        scanFilters.push_back(emptyFilter);
    } else {
        ParseScanFilterParameters(filters, scanFilters);
    }

    if (options.has_value()) {
        auto scanOptions = options.value();
        if (scanOptions.interval.has_value()) {
            SetReportDelay(scanOptions, settings);
        }
        if (scanOptions.reportMode.has_value()) {
            settings.SetReportMode((scanOptions.reportMode.value()).get_value());
        }
        if (scanOptions.dutyMode.has_value()) {
            settings.SetScanMode((scanOptions.dutyMode.value()).get_value());
        }
        if (scanOptions.phyType.has_value()) {
            settings.SetPhy((scanOptions.phyType.value()).get_value());
        }
        if (scanOptions.reportMode.has_value()) {
            SetCbTypeSensMode(scanOptions, settings);
        }
    }
    int ret = BleCentralManagerGetInstance()->StartScan(settings, scanFilters);
    ANI_BT_ASSERT_RETURN(ret == BT_NO_ERROR, ret, "StartBLEScan return error");
}

array<string> GetConnectedBLEDevices()
{
    std::lock_guard<std::mutex> lock(TaiheGattServerCallback::deviceListMutex_);
    std::vector<std::string> dstDevicesvec = TaiheGattServerCallback::deviceList_;
    array<string> result(taihe::copy_data_t{}, dstDevicesvec.data(), dstDevicesvec.size());
    return result;
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
// NOLINTEND