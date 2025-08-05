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

static void AdvertiseSettingProcess(const AdvertiseSetting &setIn, OHOS::Bluetooth::BleAdvertiserSettings &setOut)
{
    if (setIn.interval.has_value()) {
        setOut.SetInterval(setIn.interval.value());
    }
    if (setIn.txPower.has_value()) {
        setOut.SetTxPower(setIn.txPower.value());
    }
    if (setIn.connectable.has_value()) {
        setOut.SetConnectable(setIn.connectable.value());
    }
}

static void AdvertiseDataProcess(AdvertiseData &advDataIn, OHOS::Bluetooth::BleAdvertiserData &advDataOut)
{
    for (auto &serviceUuid: advDataIn.serviceUuids) {
        advDataOut.AddServiceUuid(OHOS::Bluetooth::UUID::FromString(std::string(serviceUuid)));
    }
    for (auto &manufacture: advDataIn.manufactureData) {
        advDataOut.AddManufacturerData(manufacture.manufactureId,
            std::string(manufacture.manufactureValue.begin(), manufacture.manufactureValue.end()));
    }
    for (auto &service: advDataIn.serviceData) {
        advDataOut.AddServiceData(OHOS::Bluetooth::UUID::FromString(std::string(service.serviceUuid)),
            std::string(service.serviceValue.begin(), service.serviceValue.end()));
    }
    if (advDataIn.includeTxPower.has_value()) {
        advDataOut.SetIncludeTxPower(advDataIn.includeTxPower.value());
    }
    if (advDataIn.includeDeviceName.has_value()) {
        advDataOut.SetIncludeDeviceName(advDataIn.includeDeviceName.value());
    }
}

namespace {

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

    void AddService(GattService service)
    {
        std::unique_ptr<OHOS::Bluetooth::GattService> gattService {nullptr};
        OHOS::Bluetooth::GattServiceType type = service.isPrimary ?
            OHOS::Bluetooth::GattServiceType::PRIMARY : OHOS::Bluetooth::GattServiceType::SECONDARY;
        OHOS::Bluetooth::UUID serviceUuid =
            OHOS::Bluetooth::UUID::FromString(std::string(service.serviceUuid));
        gattService = std::make_unique<OHOS::Bluetooth::GattService>(serviceUuid, type);

        for (auto &characteristic : service.characteristics) {
            int charPermissions = 0;
            int charProperties = 0;
            OHOS::Bluetooth::UUID characteristicUuid =
                OHOS::Bluetooth::UUID::FromString(std::string(characteristic.serviceUuid));
            OHOS::Bluetooth::GattCharacteristic character(characteristicUuid, charPermissions, charProperties);
            character.SetValue(characteristic.characteristicValue.data(), characteristic.characteristicValue.size());

            for (const auto &descriptor : characteristic.descriptors) {
                OHOS::Bluetooth::UUID descriptorUuid = OHOS::Bluetooth::UUID::FromString(std::string(descriptor.descriptorUuid));
                OHOS::Bluetooth::GattDescriptor gattDescriptor(descriptorUuid, 0);
                gattDescriptor.SetValue(descriptor.descriptorValue.data(), descriptor.descriptorValue.size());
                character.AddDescriptor(gattDescriptor);
            }
            gattService->AddCharacteristic(character);
        }
        int ret = server_->AddService(*gattService);
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "AddService return error");
    }

    void RemoveService(string_view serviceUuid)
    {
        OHOS::Bluetooth::UUID uuid = OHOS::Bluetooth::UUID::FromString(std::string(serviceUuid));

        int ret = OHOS::Bluetooth::BT_NO_ERROR;
        auto primaryService = server_->GetService(uuid, true);
        if (primaryService.has_value()) {
            ret = server_->RemoveGattService(primaryService.value());
            ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "RemoveService return error");
        }

        auto secondService = server_->GetService(uuid, false);
        if (secondService.has_value()) {
            ret = server_->RemoveGattService(secondService.value());
            ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "RemoveService return error");
        }
    }

    void SendResponse(ServerResponse serverResponse)
    {
        OHOS::Bluetooth::BluetoothRemoteDevice remoteDevice(std::string(serverResponse.deviceId),
            OHOS::Bluetooth::BTTransport::ADAPTER_BLE);
        int32_t transId = serverResponse.transId;
        int32_t status = serverResponse.status;
        int32_t offset = serverResponse.offset;
        int ret = server_->SendResponse(remoteDevice, transId, status, offset,
            serverResponse.value.data(), serverResponse.value.size());
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "SendResponse return error");
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

    void StartScan(array_view<ScanFilter> filters, optional_view<ScanOptions> options)
    {
        std::vector<OHOS::Bluetooth::BleScanFilter> scanFilters;
        OHOS::Bluetooth::BleScanFilter emptyFilter;
        scanFilters.push_back(emptyFilter);

        OHOS::Bluetooth::BleScanSettings settings;
        if (options.has_value()) {
            const auto &opts = options.value();
            settings.SetReportDelay(opts.interval.value());
            settings.SetScanMode(opts.dutyMode.value());
            settings.SetPhy(opts.phyType.value());
        }
        int ret = BleCentralManagerGetInstance()->StartScan(settings, scanFilters);
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "StartScan return error");
    }

    void StopScan()
    {
        int ret = BleCentralManagerGetInstance()->StopScan();
        ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "StopScan return error");
    }

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

GattServer CreateGattServer()
{
    return make_holder<GattServerImpl, GattServer>();
}

BleScanner CreateBleScanner()
{
    return make_holder<BleScannerImpl, BleScanner>();
}

void StartBLEScan(array<ScanFilter> filters, optional_view<ScanOptions> options)
{
    std::vector<OHOS::Bluetooth::BleScanFilter> scanFilters;
    OHOS::Bluetooth::BleScanFilter emptyFilter;
    scanFilters.push_back(emptyFilter);
    OHOS::Bluetooth::BleScanSettings settings;
    if (options.has_value()) {
        const auto &opts = options.value();
        settings.SetReportDelay(opts.interval.value());
        settings.SetScanMode(opts.dutyMode.value());
        settings.SetPhy(opts.phyType.value());
    }
    int ret = BleCentralManagerGetInstance()->StartScan(settings, scanFilters);
    ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "StartBLEScan return error");
}

array<string> GetConnectedBLEDevices()
{
    OHOS::Bluetooth::BluetoothHost *host = &OHOS::Bluetooth::BluetoothHost::GetDefaultHost();
    std::vector<OHOS::Bluetooth::BluetoothRemoteDevice> remoteDeviceLists;
    std::vector<std::string> dstDevicesvec;
    int32_t err = host->GetPairedDevices(OHOS::Bluetooth::BT_TRANSPORT_BLE, remoteDeviceLists);
    if (err != OHOS::Bluetooth::BT_NO_ERROR) {
        set_business_error(err, "GetPairedDevices return error");
        return {};
    }
    for (auto vec : remoteDeviceLists) {
        dstDevicesvec.push_back(vec.GetDeviceAddr().c_str());
    }
    array<string> result(taihe::copy_data_t{}, dstDevicesvec.data(), dstDevicesvec.size());
    return result;
}

void StartAdvertising(AdvertiseSetting setting, AdvertiseData advData, optional_view<AdvertiseData> advResponse)
{
    OHOS::Bluetooth::BleAdvertiserSettings nativeSettings;
    AdvertiseSettingProcess(setting, nativeSettings);
    OHOS::Bluetooth::BleAdvertiserData nativeAdvData;
    AdvertiseDataProcess(advData, nativeAdvData);
    OHOS::Bluetooth::BleAdvertiserData nativeRspData;
    if (advResponse.has_value()) {
        AdvertiseData response = advResponse.value();
        AdvertiseDataProcess(response, nativeRspData);
    }
    std::shared_ptr<OHOS::Bluetooth::BleAdvertiser> bleAdvertiser = BleAdvertiserGetInstance();
    int ret = bleAdvertiser->StartAdvertising(nativeSettings, nativeAdvData, nativeRspData, 0,
                                              OHOS::Bluetooth::TaiheBluetoothBleAdvertiseCallback::GetInstance());
    ANI_BT_ASSERT_RETURN(ret == OHOS::Bluetooth::BT_NO_ERROR, ret, "StartAdvertising return error");
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_CreateGattClientDevice(CreateGattClientDevice);
TH_EXPORT_CPP_API_CreateGattServer(CreateGattServer);
TH_EXPORT_CPP_API_CreateBleScanner(CreateBleScanner);
TH_EXPORT_CPP_API_StopAdvertising(StopAdvertising);
TH_EXPORT_CPP_API_StopBLEScan(StopBLEScan);
TH_EXPORT_CPP_API_StartAdvertising(StartAdvertising);
TH_EXPORT_CPP_API_GetConnectedBLEDevices(GetConnectedBLEDevices);
TH_EXPORT_CPP_API_StartBLEScan(StartBLEScan);
// NOLINTEND