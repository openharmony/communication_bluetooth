/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "bluetooth_ble_central_manager.h"
#include "bluetooth_ble_central_manager_callback_stub.h"
#include "bluetooth_def.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_host.h"
#include "bluetooth_log.h"
#include "bluetooth_observer_list.h"
#include "bluetooth_utils.h"
#include "i_bluetooth_ble_central_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bluetooth_profile_manager.h"

namespace OHOS {
namespace Bluetooth {
std::mutex g_bleManagerProxyMutex;
struct BleCentralManager::impl {
    impl();
    ~impl();

    bool InitBleCentralManagerProxy(void);
    void ConvertBleScanSetting(const BleScanSettings &inSettings, BluetoothBleScanSettings &outSetting);
    void ConvertBleScanFilter(const std::vector<BleScanFilter> &filters,
        std::vector<BluetoothBleScanFilter> &bluetoothBleScanFilters);
    void ConvertAdvertiserSetting(const BleAdvertiserSettings &inSettings, BluetoothBleAdvertiserSettings &outSettings);
    void ConvertActiveDeviceInfo(const std::vector<BleActiveDeviceInfo> &inDeviceInfos,
        std::vector<BluetoothActiveDeviceInfo> &outDeviceInfos);

    class BluetoothBleCentralManagerCallbackImp : public BluetoothBleCentralManagerCallBackStub {
    public:
        explicit BluetoothBleCentralManagerCallbackImp(BleCentralManager::impl &bleCentralManger)
            : bleCentralManger_(bleCentralManger){};
        ~BluetoothBleCentralManagerCallbackImp() override = default;
        void OnScanCallback(const BluetoothBleScanResult &result) override
        {
            bleCentralManger_.callbacks_.ForEach([&result](std::shared_ptr<BleCentralManagerCallback> observer) {
                BluetoothBleScanResult tempResult(result);
                BleScanResult scanResult;
                for (auto &manufacturerData : tempResult.GetManufacturerData()) {
                    scanResult.AddManufacturerData(manufacturerData.first, manufacturerData.second);
                }

                for (auto &serviceUuidData : tempResult.GetServiceUuids()) {
                    UUID uuid = UUID::ConvertFrom128Bits(serviceUuidData.ConvertTo128Bits());
                    scanResult.AddServiceUuid(uuid);
                }

                for (auto &serviceData : tempResult.GetServiceData()) {
                    UUID uuid = UUID::ConvertFrom128Bits(serviceData.first.ConvertTo128Bits());
                    scanResult.AddServiceData(uuid, serviceData.second);
                }

                scanResult.SetAdvertiseFlag(tempResult.GetAdvertiseFlag());
                scanResult.SetRssi(tempResult.GetRssi());
                scanResult.SetConnectable(tempResult.IsConnectable());
                BluetoothRemoteDevice device(tempResult.GetPeripheralDevice().GetAddress(), BT_TRANSPORT_BLE);
                scanResult.SetPeripheralDevice(device);
                scanResult.SetPayload(tempResult.GetPayload());
                scanResult.SetName(tempResult.GetName());

                observer->OnScanCallback(scanResult);
            });
        }
        void OnBleBatchScanResultsEvent(std::vector<BluetoothBleScanResult> &results) override
        {
            HILOGI("enter");
            bleCentralManger_.callbacks_.ForEach([&results](std::shared_ptr<BleCentralManagerCallback> observer) {
                std::vector<BleScanResult> scanResults;
                for (auto &result : results) {
                    BleScanResult scanResult;
                    for (auto &manufacturerData : result.GetManufacturerData()) {
                        scanResult.AddManufacturerData(manufacturerData.first, manufacturerData.second);
                    }

                    for (auto &serviceData : result.GetServiceData()) {
                        UUID uuid = UUID::ConvertFrom128Bits(serviceData.first.ConvertTo128Bits());
                        scanResult.AddServiceData(uuid, serviceData.second);
                    }

                    for (auto &serviceUuidData : result.GetServiceUuids()) {
                        UUID uuid = UUID::ConvertFrom128Bits(serviceUuidData.ConvertTo128Bits());
                        scanResult.AddServiceUuid(uuid);
                    }

                    scanResult.SetAdvertiseFlag(result.GetAdvertiseFlag());
                    scanResult.SetConnectable(result.IsConnectable());
                    scanResult.SetRssi(result.GetRssi());
                    BluetoothRemoteDevice device(result.GetPeripheralDevice().GetAddress(), BT_TRANSPORT_BLE);
                    scanResult.SetPeripheralDevice(device);
                    scanResult.SetPayload(result.GetPayload());

                    scanResults.push_back(scanResult);
                }

                observer->OnBleBatchScanResultsEvent(scanResults);
            });
        }

        void OnStartOrStopScanEvent(int resultCode, bool isStartScan) override
        {
            HILOGD("resultCode: %{public}d, isStartScan: %{public}d", resultCode, isStartScan);
            bleCentralManger_.callbacks_.ForEach(
                [resultCode, isStartScan](std::shared_ptr<BleCentralManagerCallback> observer) {
                    observer->OnStartOrStopScanEvent(resultCode, isStartScan);
            });
        }

        void OnNotifyMsgReportFromLpDevice(const bluetooth::Uuid &uuid, int msgType,
            const std::vector<uint8_t> &value) override
        {
            bleCentralManger_.callbacks_.ForEach(
                [uuid, msgType, value](std::shared_ptr<BleCentralManagerCallback> observer) {
                    UUID btUuid = UUID::ConvertFrom128Bits(uuid.ConvertTo128Bits());
                    observer->OnNotifyMsgReportFromLpDevice(btUuid, msgType, value);
            });
        }

    private:
        BleCentralManager::impl &bleCentralManger_;
        BLUETOOTH_DISALLOW_COPY_AND_ASSIGN(BluetoothBleCentralManagerCallbackImp);
    };
    sptr<BluetoothBleCentralManagerCallbackImp> callbackImp_ = nullptr;
    BluetoothObserverList<BleCentralManagerCallback> callbacks_;

    int32_t scannerId_ = BLE_SCAN_INVALID_ID;
    bool enableRandomAddrMode_ = true;
    int32_t profileRegisterId;
};

bool  BleCentralManager::impl::InitBleCentralManagerProxy(void)
{
    callbackImp_ = new BluetoothBleCentralManagerCallbackImp(*this);
    profileRegisterId = DelayedSingleton<BluetoothProfileManager>::GetInstance()->RegisterFunc(BLE_CENTRAL_MANAGER_SERVER,
        [this](sptr<IRemoteObject> remote) {
        sptr<IBluetoothBleCentralManager> proxy = iface_cast<IBluetoothBleCentralManager>(remote);
        CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
        proxy->RegisterBleCentralManagerCallback(scannerId_, enableRandomAddrMode_, callbackImp_);
    });
    return true;
}

BleCentralManager::impl::impl()
{}

void BleCentralManager::impl::ConvertBleScanSetting(const BleScanSettings &inSettings,
    BluetoothBleScanSettings &outSetting)
{
    outSetting.SetReportDelay(0);
    outSetting.SetScanMode(inSettings.GetScanMode());
    outSetting.SetLegacy(inSettings.GetLegacy());
    outSetting.SetPhy(inSettings.GetPhy());
}

void BleCentralManager::impl::ConvertBleScanFilter(const std::vector<BleScanFilter> &filters,
    std::vector<BluetoothBleScanFilter> &bluetoothBleScanFilters)
{
    for (auto filter : filters) {
        BluetoothBleScanFilter scanFilter;
        scanFilter.SetDeviceId(filter.GetDeviceId());
        scanFilter.SetName(filter.GetName());
        if (filter.HasServiceUuid()) {
            scanFilter.SetServiceUuid(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceUuid().ToString()));
        }
        if (filter.HasServiceUuidMask()) {
            scanFilter.SetServiceUuidMask(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceUuidMask().ToString()));
        }
        if (filter.HasSolicitationUuid()) {
            scanFilter.SetServiceSolicitationUuid(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceSolicitationUuid().ToString()));
        }
        if (filter.HasSolicitationUuidMask()) {
            scanFilter.SetServiceSolicitationUuidMask(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceSolicitationUuidMask().ToString()));
        }
        scanFilter.SetServiceData(filter.GetServiceData());
        scanFilter.SetServiceDataMask(filter.GetServiceDataMask());
        scanFilter.SetManufacturerId(filter.GetManufacturerId());
        scanFilter.SetManufactureData(filter.GetManufactureData());
        scanFilter.SetManufactureDataMask(filter.GetManufactureDataMask());
        bluetoothBleScanFilters.push_back(scanFilter);
    }
}

void BleCentralManager::impl::ConvertAdvertiserSetting(const BleAdvertiserSettings &inSettings,
    BluetoothBleAdvertiserSettings &outSettings)
{
    outSettings.SetConnectable(inSettings.IsConnectable());
    outSettings.SetInterval(inSettings.GetInterval());
    outSettings.SetLegacyMode(inSettings.IsLegacyMode());
    outSettings.SetTxPower(inSettings.GetTxPower());
}

void BleCentralManager::impl::ConvertActiveDeviceInfo(const std::vector<BleActiveDeviceInfo> &inDeviceInfos,
    std::vector<BluetoothActiveDeviceInfo> &outDeviceInfos)
{
    for (auto info : inDeviceInfos) {
        BluetoothActiveDeviceInfo deviceInfo;
        deviceInfo.deviceId = info.deviceId;
        deviceInfo.status = info.status;
        deviceInfo.timeOut = info.timeOut;
        outDeviceInfos.push_back(deviceInfo);
    }
}

BleCentralManager::impl::~impl()
{
    HILOGD("start");
    DelayedSingleton<BluetoothProfileManager>::GetInstance()->DeregisterFunc(profileRegisterId);
    scannerId_ = BLE_SCAN_INVALID_ID;
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG(proxy != nullptr, "failed: no proxy");
    proxy->DeregisterBleCentralManagerCallback(scannerId_, callbackImp_);
}


BleCentralManager::BleCentralManager(BleCentralManagerCallback &callback) : pimpl(nullptr)
{
    if (pimpl == nullptr) {
        pimpl = std::make_unique<impl>();
        if (pimpl == nullptr) {
            HILOGE("failed, no pimpl");
        }
    }

    HILOGI("successful");
    std::shared_ptr<BleCentralManagerCallback> pointer(&callback, [](BleCentralManagerCallback *) {});
    bool ret = pimpl->callbacks_.Register(pointer);
    if (ret)
        return;
}

BleCentralManager::BleCentralManager(std::shared_ptr<BleCentralManagerCallback> callback, bool enableRandomAddrMode)
    : pimpl(nullptr)
{
    if (pimpl == nullptr) {
        pimpl = std::make_unique<impl>();
        if (pimpl == nullptr) {
            HILOGE("failed, no pimpl");
        }
    }
    HILOGI("successful");
    pimpl->enableRandomAddrMode_ = enableRandomAddrMode;
    pimpl->callbacks_.Register(callback);
}

BleCentralManager::~BleCentralManager()
{
}

int BleCentralManager::StartScan()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->scannerId_ == BLE_SCAN_INVALID_ID) {
        HILOGE("scannerId is invalid");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    HILOGI("StartScan without param, scannerId: %{public}d", pimpl->scannerId_);
    return proxy->StartScan(pimpl->scannerId_);
}

int BleCentralManager::StartScan(const BleScanSettings &settings)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->scannerId_ == BLE_SCAN_INVALID_ID) {
        HILOGE("scannerId is invalid");
        return BT_ERR_INTERNAL_ERROR;
    }

    HILOGI("StartScan with params, scannerId: %{public}d", pimpl->scannerId_);
    BluetoothBleScanSettings setting;
    // not use report delay scan. settings.GetReportDelayMillisValue()
    setting.SetReportDelay(0);
    setting.SetScanMode(settings.GetScanMode());
    setting.SetLegacy(settings.GetLegacy());
    setting.SetPhy(settings.GetPhy());
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->StartScan(pimpl->scannerId_, setting);
}

int BleCentralManager::StopScan()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl->scannerId_ == BLE_SCAN_INVALID_ID) {
        HILOGE("scannerId is invalid");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    HILOGI("scannerId_: %{public}d", pimpl->scannerId_);

    int ret = proxy->StopScan(pimpl->scannerId_);
    proxy->RemoveScanFilter(pimpl->scannerId_);
    return ret;
}

int BleCentralManager::ConfigScanFilter(const std::vector<BleScanFilter> &filters)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INVALID_STATE;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy() || pimpl->scannerId_ == BLE_SCAN_INVALID_ID) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    std::vector<BluetoothBleScanFilter> bluetoothBleScanFilters;
    for (auto filter : filters) {
        BluetoothBleScanFilter scanFilter;
        scanFilter.SetDeviceId(filter.GetDeviceId());
        scanFilter.SetName(filter.GetName());
        if (filter.HasServiceUuid()) {
            scanFilter.SetServiceUuid(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceUuid().ToString()));
        }
        if (filter.HasServiceUuidMask()) {
            scanFilter.SetServiceUuidMask(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceUuidMask().ToString()));
        }
        if (filter.HasSolicitationUuid()) {
            scanFilter.SetServiceSolicitationUuid(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceSolicitationUuid().ToString()));
        }
        if (filter.HasSolicitationUuidMask()) {
            scanFilter.SetServiceSolicitationUuidMask(bluetooth::Uuid::ConvertFromString(
                filter.GetServiceSolicitationUuidMask().ToString()));
        }
        scanFilter.SetServiceData(filter.GetServiceData());
        scanFilter.SetServiceDataMask(filter.GetServiceDataMask());
        scanFilter.SetManufacturerId(filter.GetManufacturerId());
        scanFilter.SetManufactureData(filter.GetManufactureData());
        scanFilter.SetManufactureDataMask(filter.GetManufactureDataMask());
        bluetoothBleScanFilters.push_back(scanFilter);
    }
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    int ret = proxy->ConfigScanFilter(pimpl->scannerId_, bluetoothBleScanFilters);
    if (ret != BT_NO_ERROR) {
        HILOGE("failed.");
        return ret;
    }
    return ret;
}

int BleCentralManager::SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }
    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return proxy->SetLpDeviceAdvParam(duration, maxExtAdvEvents, window, interval, advHandle);
}

int BleCentralManager::SetScanReportChannelToLpDevice(bool enable)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    // need start scan first
    if (pimpl->scannerId_ == BLE_SCAN_INVALID_ID) {
        HILOGE("failed, scannerId invalid.");
        return BT_ERR_INTERNAL_ERROR;
    }
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->SetScanReportChannelToLpDevice(pimpl->scannerId_, enable);
}

int BleCentralManager::EnableSyncDataToLpDevice()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return proxy->EnableSyncDataToLpDevice();
}

int BleCentralManager::DisableSyncDataToLpDevice()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return proxy->DisableSyncDataToLpDevice();
}

int BleCentralManager::SendParamsToLpDevice(const std::vector<uint8_t> &dataValue, int32_t type)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return proxy->SendParamsToLpDevice(dataValue, type);
}

bool BleCentralManager::IsLpDeviceAvailable()
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }
    return proxy->IsLpDeviceAvailable();
}

int BleCentralManager::SetLpDeviceParam(const BleLpDeviceParamSet &lpDeviceParamSet)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (pimpl == nullptr || !pimpl->InitBleCentralManagerProxy()) {
        HILOGE("pimpl or ble central manager proxy is nullptr");
        return BT_ERR_INTERNAL_ERROR;
    }

    BluetoothLpDeviceParamSet paramSet;
    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_SCAN_SETTING_VALID_BIT) != 0) {
        pimpl->ConvertBleScanSetting(lpDeviceParamSet.scanSettings, paramSet.btScanSettings);
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_SCAN_FILTER_VALID_BIT) != 0) {
        pimpl->ConvertBleScanFilter(lpDeviceParamSet.scanFilters, paramSet.btScanFilters);
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_ADV_SETTING_VALID_BIT) != 0) {
        pimpl->ConvertAdvertiserSetting(lpDeviceParamSet.advSettings, paramSet.btAdvSettings);
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_ADVDATA_VALID_BIT) != 0) {
        paramSet.btAdvData.SetPayload(std::string(lpDeviceParamSet.advData.begin(),
            lpDeviceParamSet.advData.end()));
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_RESPDATA_VALID_BIT) != 0) {
        paramSet.btRespData.SetPayload(std::string(lpDeviceParamSet.respData.begin(),
            lpDeviceParamSet.respData.end()));
    }

    if ((lpDeviceParamSet.fieldValidFlagBit & BLE_LPDEVICE_ADV_DEVICEINFO_VALID_BIT) != 0) {
        pimpl->ConvertActiveDeviceInfo(lpDeviceParamSet.activeDeviceInfos, paramSet.activeDeviceInfos);
    }
    paramSet.fieldValidFlagBit = lpDeviceParamSet.fieldValidFlagBit;
    paramSet.uuid = bluetooth::Uuid::ConvertFromString(lpDeviceParamSet.uuid.ToString());
    paramSet.advHandle = lpDeviceParamSet.advHandle;
    paramSet.deliveryMode = lpDeviceParamSet.deliveryMode;
    paramSet.duration = lpDeviceParamSet.duration;
    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    return proxy->SetLpDeviceParam(paramSet);
}

int BleCentralManager::RemoveLpDeviceParam(const UUID &uuid)
{
    if (!IS_BLE_ENABLED()) {
        HILOGE("bluetooth is off.");
        return BT_ERR_INTERNAL_ERROR;
    }

    sptr<IBluetoothBleCentralManager> proxy =
        GetRemoteProxy<IBluetoothBleCentralManager>(BLE_CENTRAL_MANAGER_SERVER);
    CHECK_AND_RETURN_LOG_RET(proxy != nullptr, BT_ERR_INTERNAL_ERROR, "failed: no proxy");
    bluetooth::Uuid btUuid = bluetooth::Uuid::ConvertFromString(uuid.ToString());
    return proxy->RemoveLpDeviceParam(btUuid);
}

BleScanResult::BleScanResult()
{}

BleScanResult::~BleScanResult()
{}

std::vector<UUID> BleScanResult::GetServiceUuids() const
{
    return serviceUuids_;
}

std::map<uint16_t, std::string> BleScanResult::GetManufacturerData() const
{
    return manufacturerSpecificData_;
}

std::map<UUID, std::string> BleScanResult::GetServiceData() const
{
    return serviceData_;
}

BluetoothRemoteDevice BleScanResult::GetPeripheralDevice() const
{
    return peripheralDevice_;
}

int8_t BleScanResult::GetRssi() const
{
    return rssi_;
}

bool BleScanResult::IsConnectable() const
{
    return connectable_;
}

uint8_t BleScanResult::GetAdvertiseFlag() const
{
    return advertiseFlag_;
}

std::vector<uint8_t> BleScanResult::GetPayload() const
{
    return payload_;
}

void BleScanResult::AddManufacturerData(uint16_t manufacturerId, const std::string &data)
{
    manufacturerSpecificData_.insert(std::make_pair(manufacturerId, data));
}

void BleScanResult::AddServiceData(const UUID &uuid, const std::string &data)
{
    serviceData_.insert(std::make_pair(uuid, data));
}

void BleScanResult::AddServiceUuid(const UUID &serviceUuid)
{
    serviceUuids_.push_back(serviceUuid);
}

void BleScanResult::SetPayload(std::string payload)
{
    payload_.assign(payload.begin(), payload.end());
}

void BleScanResult::SetPeripheralDevice(const BluetoothRemoteDevice &device)
{
    peripheralDevice_ = device;
}

void BleScanResult::SetRssi(int8_t rssi)
{
    rssi_ = rssi;
}

void BleScanResult::SetConnectable(bool connectable)
{
    connectable_ = connectable;
}

void BleScanResult::SetAdvertiseFlag(uint8_t flag)
{
    advertiseFlag_ = flag;
}

void BleScanResult::SetName(const std::string &name)
{
    name_ = name;
}
std::string BleScanResult::GetName(void)
{
    return name_;
}

BleScanSettings::BleScanSettings()
{}

BleScanSettings::~BleScanSettings()
{}

void BleScanSettings::SetReportDelay(long reportDelayMillis)
{
    reportDelayMillis_ = reportDelayMillis;
}

long BleScanSettings::GetReportDelayMillisValue() const
{
    return reportDelayMillis_;
}

int BleScanSettings::SetScanMode(int scanMode)
{
    if (scanMode < SCAN_MODE_LOW_POWER || scanMode >= SCAN_MODE_INVALID) {
        return RET_BAD_PARAM;
    }

    scanMode_ = scanMode;
    return RET_NO_ERROR;
}

int BleScanSettings::GetScanMode() const
{
    return scanMode_;
}

void BleScanSettings::SetLegacy(bool legacy)
{
    legacy_ = legacy;
}

bool BleScanSettings::GetLegacy() const
{
    return legacy_;
}

void BleScanSettings::SetPhy(int phy)
{
    phy_ = phy;
}

int BleScanSettings::GetPhy() const
{
    return phy_;
}

BleScanFilter::BleScanFilter()
{}

BleScanFilter::~BleScanFilter()
{}

void BleScanFilter::SetDeviceId(std::string deviceId)
{
    deviceId_ = deviceId;
}

std::string BleScanFilter::GetDeviceId() const
{
    return deviceId_;
}

void BleScanFilter::SetName(std::string name)
{
    name_ = name;
}

std::string BleScanFilter::GetName() const
{
    return name_;
}

void BleScanFilter::SetServiceUuid(const UUID &uuid)
{
    serviceUuid_ = uuid;
    hasServiceUuid_ = true;
}

bool BleScanFilter::HasServiceUuid()
{
    return hasServiceUuid_;
}

UUID BleScanFilter::GetServiceUuid() const
{
    return serviceUuid_;
}

void BleScanFilter::SetServiceUuidMask(const UUID &serviceUuidMask)
{
    serviceUuidMask_ = serviceUuidMask;
    hasServiceUuidMask_ = true;
}

bool BleScanFilter::HasServiceUuidMask()
{
    return hasServiceUuidMask_;
}

UUID BleScanFilter::GetServiceUuidMask() const
{
    return serviceUuidMask_;
}

void BleScanFilter::SetServiceSolicitationUuid(const UUID &serviceSolicitationUuid)
{
    serviceSolicitationUuid_ = serviceSolicitationUuid;
    hasSolicitationUuid_ = true;
}

bool BleScanFilter::HasSolicitationUuid()
{
    return hasSolicitationUuid_;
}

UUID BleScanFilter::GetServiceSolicitationUuid() const
{
    return serviceSolicitationUuid_;
}

void BleScanFilter::SetServiceSolicitationUuidMask(const UUID &serviceSolicitationUuidMask)
{
    serviceSolicitationUuidMask_ = serviceSolicitationUuidMask;
    hasSolicitationUuidMask_ = true;
}

bool BleScanFilter::HasSolicitationUuidMask()
{
    return hasSolicitationUuidMask_;
}

UUID BleScanFilter::GetServiceSolicitationUuidMask() const
{
    return serviceSolicitationUuidMask_;
}

void BleScanFilter::SetServiceData(std::vector<uint8_t> serviceData)

{
    serviceData_ = serviceData;
}

std::vector<uint8_t> BleScanFilter::GetServiceData() const
{
    return serviceData_;
}

void BleScanFilter::SetServiceDataMask(std::vector<uint8_t> serviceDataMask)
{
    serviceDataMask_ = serviceDataMask;
}

std::vector<uint8_t> BleScanFilter::GetServiceDataMask() const
{
    return serviceDataMask_;
}

void BleScanFilter::SetManufacturerId(uint16_t manufacturerId)
{
    manufacturerId_ = manufacturerId;
}

uint16_t BleScanFilter::GetManufacturerId() const
{
    return manufacturerId_;
}

void BleScanFilter::SetManufactureData(std::vector<uint8_t> manufactureData)
{
    manufactureData_ = manufactureData;
}

std::vector<uint8_t> BleScanFilter::GetManufactureData() const
{
    return manufactureData_;
}

void BleScanFilter::SetManufactureDataMask(std::vector<uint8_t> manufactureDataMask)
{
    manufactureDataMask_ = manufactureDataMask;
}

std::vector<uint8_t> BleScanFilter::GetManufactureDataMask() const
{
    return manufactureDataMask_;
}
}  // namespace Bluetooth
}  // namespace OHOS
